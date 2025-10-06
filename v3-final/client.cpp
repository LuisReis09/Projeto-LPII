#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <string>
#include <signal.h>

#define PORT 54002
#define BUFFER_SIZE 5000

using namespace std;
int sock;

void receiveMessages(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cout << "\033[31m\n[INFO] Desconectado do servidor.\n \033[0m";
            break;
        }
        cout << buffer << endl;
        cout << "\033[33m> \033[0m" << flush;
    }
}

void exitRoom(int signum){
    send(sock, "/exit", 10, 0);
    close(sock);
    cout << "\n[INFO] Encerrando cliente..." << endl;
    exit(0);
}

int menu() {
    int opcao;
    cout << "\n----------------------------------------------------" << endl;
    cout << "|                                                  |" << endl;
    cout << "|                SISTEMA DE CHAT                   |" << endl;
    cout << "|                                                  |" << endl;
    cout << "----------------------------------------------------" << endl;

    cout << "\n0. Sair" << endl;
    cout << "1. Manual de Uso" << endl;
    cout << "2. Listar Salas Criadas" << endl;
    cout << "3. Criar uma sala" << endl;
    cout << "4. Entrar em uma sala" << endl;
    cout << "Opcao: ";
    cin >> opcao;
    cin.ignore(); // descarta '\n' do buffer
    cout << endl;
    return opcao;
}

void chatLoop(int sock) {
    string input;
    while (true) {
        getline(cin, input);
        if (input == "exit") {
            send(sock, "exit\n", 5, 0);
            break;
        }
        input += "\n";
        send(sock, input.c_str(), input.size(), 0);
        cout << "\033[33m> \033[0m";
    }
}

int main() {
    string username, room, password;
    string server_ip = "127.0.0.1";
    signal(SIGINT, exitRoom);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "[ERROR] Falha ao criar socket.\n";
        return 1;
    }
    
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip.c_str(), &serverAddr.sin_addr);
    
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "[ERROR] Falha ao conectar ao servidor.\n";
        return 1;
    }
    
    

    // Enviar nome
    cout << "Nome: ";
    getline(cin, username);
    username = "/user " + username + "\n";
    send(sock, username.c_str(), username.size(), 0);

    // Thread para receber mensagens
    thread(receiveMessages, sock).detach();

    while (true) {
        int op = menu();
        if (op == 0) break;

        switch (op) {
            case 1:
                cout << "\033[36m\nInstruções:\033[0m" << endl;
                cout << "\t- Digite seu nome e entre/crie uma sala" << endl;
                cout << "\t- Após entrar, você pode enviar e receber mensagens" << endl;
                cout << "\t- Digite \"exit\" para sair da sala" << endl;
                cout << "\t- Digite \"/p <Nome1, Nome2, ...> {mensagem}\" para mensagem privada\n";
                break;

            case 2:
                send(sock, "/list\n", 6, 0);
                this_thread::sleep_for(chrono::milliseconds(200));
                break;

            case 3:
                cout << "Nome da sala: ";
                getline(cin, room);
                cout << "Senha da sala: ";
                getline(cin, password);
                cout << endl;

                send(sock, ("/create " + room + " " + password + "\n").c_str(),
                     room.size() + password.size() + 10, 0);
                this_thread::sleep_for(chrono::milliseconds(200));
                chatLoop(sock); // entra no loop de chat
                break;

            case 4:
                cout << "Nome da sala: ";
                getline(cin, room);
                cout << "Senha da sala: ";
                getline(cin, password);
                send(sock, ("/join " + room + " " + password + "\n").c_str(),
                     room.size() + password.size() + 8, 0);
                this_thread::sleep_for(chrono::milliseconds(200));
                chatLoop(sock); // entra no loop de chat
                break;

            default:
                cout << "[WARNING] Opção inválida.\n";
                break;
        }
    }

    close(sock);
    cout << "\033[33m\nPrograma finalizado, adeus!\033[0m" << endl;
    return 0;
}
