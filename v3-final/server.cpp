#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <thread>
#include <algorithm>
#include <signal.h>
#include <sstream>
#include <mutex>
#include <iostream>
#include "logging.hpp"

#define PORT 54002
#define BUFFER_SIZE 5000

Logging logger;


// ------------------- Estruturas -------------------
struct Client {
    int sock;
    std::string name;
    std::string room;
};

struct Room {
    std::string name;
    std::string password;
    std::vector<Client*> clients;
};

// ------------------- Globals -------------------
int serverSock;
std::vector<Client*> clients;
std::vector<Room> rooms;
std::mutex clientsMutex;
std::mutex roomsMutex;

// ------------------- Funções Auxiliares -------------------
Room* findRoom(const std::string& roomName) {
    for (auto &r : rooms)
        if (r.name == roomName)
            return &r;
    return nullptr;
}

Client* findClientByName(const std::string& username) {
    for (auto c : clients)
        if (c->name == username)
            return c;
    return nullptr;
}

void sendToClient(Client* c, const std::string& msg) {
    send(c->sock, msg.c_str(), msg.size(), 0);
}

// Broadcast seguro: copia os clientes antes de iterar
void broadcastToRoom(const std::string& roomName, const std::string& msg, Client* except=nullptr) {
    std::vector<Client*> clientsCopy;
    {
        std::lock_guard<std::mutex> lock(roomsMutex);
        Room* r = findRoom(roomName);
        if (!r) return;
        clientsCopy = r->clients;
    }
    for (auto c : clientsCopy) {
        if (except && c->sock == except->sock) continue;
        sendToClient(c, msg);
    }
}

// ------------------- Cleanup Ctrl+C -------------------
void cleanup(int signum) {
    std::cout << "\n[INFO] Encerrando servidor..." << std::endl;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto c : clients) close(c->sock);
        clients.clear();
    }
    close(serverSock);
    std::cout << "[INFO] Sockets liberados. Saindo." << std::endl;
    exit(0);
}

// ------------------- Parsing de comandos -------------------
void handleCommand(Client* client, const std::string& msg) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd == "/user") {
        std::string name;
        iss >> name;
        client->name = name;

        logger.info(name + " juntou-se.");

    } else if (cmd == "/create") {
        std::string roomName, password;
        iss >> roomName >> password;
        std::lock_guard<std::mutex> lock(roomsMutex);
        if (findRoom(roomName)) {
            sendToClient(client, "[ERROR] Sala já existe.");
        } else {
            rooms.push_back({roomName, password, {client}});
            client->room = roomName;
            sendToClient(client, "[\033[32mSUCCESS\033[0m] Sala '" + roomName + "' criada e entrou nela.");
            logger.success("Sala \"" + roomName + "\" criada!");
        }

    } else if (cmd == "/join") {
        std::string roomName, password;
        iss >> roomName >> password;
        Room* r = nullptr;
        {
            std::lock_guard<std::mutex> lock(roomsMutex);
            r = findRoom(roomName);
            if (!r) {
                sendToClient(client, "[\033[32mERROR\033[0m] Sala não existe.");
                return;
            } else if (r->password != password) {
                sendToClient(client, "[\033[32mERROR\033[0m] Senha incorreta.");
                return;
            }
            logger.success(client->name + " entrou na sala " + roomName);
            r->clients.push_back(client);
        }
        client->room = roomName;
        sendToClient(client, "[\033[32mSUCCESS\033[0m] Entrou na sala '" + roomName + "'.");
        broadcastToRoom(roomName, "[INFO] " + client->name + " entrou na sala.", client);

    } else if (cmd == "/list") {
        std::lock_guard<std::mutex> lock(roomsMutex);
        if (rooms.empty()) sendToClient(client, "[\033[36mINFO\033[0m] Nenhuma sala criada.");
        else {
            std::string list = "[\033[36mINFO\033[0m] Salas criadas:\n";
            for (auto &r : rooms) list += " - " + r.name + "\n";
            sendToClient(client, list);
            logger.info("Cliente \"" + client->name + "\" solicitou visualizacao das salas criadas");
        }

    } else if (cmd == "/p") {
        std::string targets;
        getline(iss, targets); // resto da linha
        size_t end = targets.find('>');
        logger.log(targets);
        if (targets.empty() || targets[1]!='<' || end == std::string::npos) {
            sendToClient(client, "[ERROR] Formato incorreto: /p <user1,user2> mensagem");
            return;
        }
        std::string userList = targets.substr(2, end-2);
        std::string message = targets.substr(end+1);
        std::istringstream users(userList);
        std::string uname;
        while (getline(users, uname, ',')) {
            uname.erase(std::remove(uname.begin(), uname.end(), ' '), uname.end());
            Client* c = findClientByName(uname);
            if (c) sendToClient(c, "[\033[35mPRIVATE\033[0m][" + client->name + "] " + message);
        }

        logger.info("Cliente " + client->name + " enviou uma mensagem privada.");

    } else if(cmd == "exit" || cmd == "/exit"){ 
        logger.info("Cliente " + client->name + " solicitou sair.");

        // Se ele estiver em uma sala, remover
        if (!client->room.empty()) {
            std::lock_guard<std::mutex> lock(roomsMutex);
            Room* r = findRoom(client->room);
            if (r) {
                r->clients.erase(std::remove(r->clients.begin(), r->clients.end(), client), r->clients.end());

                // Notificar os outros
                broadcastToRoom(r->name, "[INFO] " + client->name + " saiu da sala.", client);

                // Se a sala ficou vazia, destruir
                if (r->clients.empty()) {
                    rooms.erase(std::remove_if(rooms.begin(), rooms.end(),
                        [&](const Room& rm){ return rm.name == r->name; }), rooms.end());
                    logger.info("Sala '" + r->name + "' destruída por estar vazia.");
                }
            }
            client->room.clear();
        }

        // Fechar o socket e remover cliente global
        close(client->sock);
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
        }

        sendToClient(client, "[INFO] Você saiu do servidor.");
        logger.warning("Cliente " + client->name + " desconectado pelo comando exit.");

        // desalocar
        delete client;

        // Importante: encerrar a thread desse cliente
        pthread_exit(nullptr);
    }else {
        // Mensagem normal
        logger.info("Usuario \"" + client->name + "\" enviou uma nova mensagem: \"" + msg + "\"");
        if (!client->room.empty()) broadcastToRoom(client->room, "[" + client->name + "] " + msg, client);
        else sendToClient(client, "[WARNING] Você não está em nenhuma sala.");
    }
}

// ------------------- Thread do Cliente -------------------
void handleClient(int clientSock) {
    char buffer[BUFFER_SIZE];
    Client* client = new Client{clientSock, "Anonimo", ""};

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(client);
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            close(clientSock);
            break;
        }
        std::string msg(buffer, bytesReceived);
        msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
        handleCommand(client, msg);
    }

    // Remover cliente da sala
    if (!client->room.empty()) {
        std::lock_guard<std::mutex> lock(roomsMutex);
        Room* r = findRoom(client->room);
        if (r) {
            r->clients.erase(std::remove(r->clients.begin(), r->clients.end(), client), r->clients.end());
            if (r->clients.empty()) {
                rooms.erase(std::remove_if(rooms.begin(), rooms.end(),
                    [&](const Room& rm){ return rm.name == r->name; }), rooms.end());
            } else {
                broadcastToRoom(r->name, "[INFO] " + client->name + " saiu da sala.");
            }
        }
    }

    // Remover cliente globalmente
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    }

    delete client;
}

// ------------------- Main -------------------
int main() {
    cout << endl;
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1) {
        std::cerr << "[ERROR] Falha ao criar socket.\n";
        return 1;
    }

    signal(SIGINT, cleanup);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[ERROR] Bind falhou.\n";
        return 1;
    }

    if (listen(serverSock, 10) < 0) {
        std::cerr << "[ERROR] Listen falhou.\n";
        return 1;
    }

    logger.log("[INFRA] Servidor rodando na porta 54002", Colors::GREEN);

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        int clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientSize);
        if (clientSock < 0) continue;
        std::thread(handleClient, clientSock).detach();
    }

    close(serverSock);
    return 0;
}
