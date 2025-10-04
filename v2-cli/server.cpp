#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "logging.hpp"

#define PORT 4000

using namespace std;

Logging logger;
mutex rooms_mutex;

// Estrutura de uma sala
struct Room {
    string name;
    vector<int> clients; // sockets
    map<int,string> names; // socket -> nome
};

map<string, Room> rooms;

void broadcast(const string& room, const string& msg, int sender) {
    lock_guard<mutex> lock(rooms_mutex);
    for (int client : rooms[room].clients) {
        if (client != sender) {
            send(client, msg.c_str(), msg.size(), 0);
        }
    }
}

string list_rooms() {
    lock_guard<mutex> lock(rooms_mutex);
    if (rooms.empty()) return "Nenhuma sala criada ainda.";

    string result = "Salas existentes:\n";
    for (const auto& pair : rooms) {
        result += "- " + pair.first + " (" + to_string(pair.second.clients.size()) + " usuários)\n";
    }
    return result;
}

void handle_client(int client_sock) {
    char buffer[1024];
    string room_name, user_name;

    // Primeiro recebimento: pode ser "getRooms" ou "username|room"
    int bytes = recv(client_sock, buffer, 1024, 0);
    if (bytes <= 0) {
        close(client_sock);
        return;
    }
    buffer[bytes] = '\0';
    string first_msg(buffer);

    if (first_msg == "getRooms") {
        string rooms_list = list_rooms() + "\n";
        send(client_sock, rooms_list.c_str(), rooms_list.size(), 0);

        // Recebe agora o username|room
        bytes = recv(client_sock, buffer, 1024, 0);
        if (bytes <= 0) {
            close(client_sock);
            return;
        }
        buffer[bytes] = '\0';
        first_msg = string(buffer);
    }

    // Separar username e room_name
    size_t sep = first_msg.find('|');
    user_name = first_msg.substr(0, sep);
    room_name = first_msg.substr(sep + 1);

    {
        lock_guard<mutex> lock(rooms_mutex);
        if (rooms.find(room_name) == rooms.end()) {
            rooms[room_name] = Room{room_name, {}, {}};
            logger.info("Sala criada: " + room_name);
        }
        rooms[room_name].clients.push_back(client_sock);
        rooms[room_name].names[client_sock] = user_name;
    }

    logger.success(user_name + " entrou na sala " + room_name);

    string join_msg = "[" + room_name + "] " + user_name + " entrou.";
    broadcast(room_name, join_msg, client_sock);

    // Loop de mensagens
    while (true) {
        bytes = recv(client_sock, buffer, 1024, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        string msg(buffer);

        if (msg == "exit") break;

        string full_msg = "[" + room_name + "] " + user_name + ": " + msg;
        logger.log(full_msg, Colors::CYAN);
        broadcast(room_name, full_msg, client_sock);
    }

    // Saiu da sala
    {
        lock_guard<mutex> lock(rooms_mutex);
        auto &clients = rooms[room_name].clients;
        clients.erase(remove(clients.begin(), clients.end(), client_sock), clients.end());
        rooms[room_name].names.erase(client_sock);
    }
    logger.warning(user_name + " saiu da sala " + room_name);
    close(client_sock);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        logger.error("Falha ao criar socket.");
        return -1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        logger.error("Falha ao fazer bind.");
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        logger.error("Falha ao escutar porta.");
        return -1;
    }

    logger.success("Servidor iniciado na porta " + to_string(PORT));

    while (true) {
        socklen_t addrlen = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            logger.error("Falha ao aceitar conexão.");
            continue;
        }
        thread(handle_client, new_socket).detach();
    }

    return 0;
}
