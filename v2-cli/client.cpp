#include <iostream>
#include <thread>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "logging.hpp"

#define PORT 4001

using namespace std;

Logging logger;

void receive_messages(int sock) {
    char buffer[1024];
    while (true) {
        int bytes = recv(sock, buffer, 1024, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';

        // Move cursor para cima (simula chat acima do input)
        cout << "\033[1A\r\033[K";
        logger.log(string(buffer), Colors::GREEN);

        // Reescreve a linha de input
        cout << "> " << flush;
    }
}

int main() {
    string server_ip = "127.0.0.1";
    string username, room;

    cout << "----------------------------------------------------" << endl;
    cout << "|                                                  |" << endl;
    cout << "|                SISTEMA DE CHAT                   |" << endl;
    cout << "|                                                  |" << endl;
    cout << "----------------------------------------------------" << endl;

    cout << "\nInstruções: " << endl;
    cout << "\t- Informe seu nome e o nome da sala na qual deseja entrar" << endl;
    cout << "\t- Ao ser conectado, voce podera comecar a enviar e receber mensagens nesta sala" << endl;
    cout << "\t- Digite \"exit\" para se retirar da sala" << endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        logger.error("Erro ao criar socket.");
        return -1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        logger.error("Endereço inválido.");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        logger.error("Falha na conexão.");
        return -1;
    }

    // Envia comando getRooms
    string command = "getRooms";
    send(sock, command.c_str(), command.size(), 0);
    // Recebe a lista de salas do servidor
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0'; // garante string terminada
        cout << "\n----- SALAS CRIADAS -----\n";
        cout << buffer << endl;
    }

    cout << "Digite seu nome: ";
    getline(cin, username);
    cout << "Digite o nome da sala: ";
    getline(cin, room);
   
    // Dá um espaço pro início do chat:
    cout << endl;

    // Envia username|sala
    string init_msg = username + "|" + room;
    send(sock, init_msg.c_str(), init_msg.size(), 0);

    thread listener(receive_messages, sock);
    listener.detach();

    string msg;
    while (true) {
        cout << "> ";
        getline(cin, msg);
        if (msg.empty()) continue;

        send(sock, msg.c_str(), msg.size(), 0);
        if (msg == "exit") break;
    }

    close(sock);
    return 0;
}
