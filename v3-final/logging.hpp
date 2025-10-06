#ifndef _LOGGING_HPP_
#define _LOGGING_HPP_

#include <iostream>
#include <pthread.h>
#include <string>

using namespace std;

/*
    Espaço de cores possíveis de serem impressas em terminal
*/
namespace Colors {
    const char* RESET   = "\033[0m";
    const char* RED     = "\033[31m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* BLUE    = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN    = "\033[36m";
}

class Logging {
private:
    /**
     * @brief Sistema de ticket lock para garantir ordem de acesso ao terminal.
     */
    pthread_mutex_t mutex_ticket = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    unsigned long ticket_counter = 0;   // Próximo ticket a ser entregue
    unsigned long now_serving = 0;      // Ticket em atendimento

public:
    Logging() = default;
    ~Logging() = default;

    /**
     * @brief Imprime mensagem colorida com controle de ordem (ticket lock).
     * 
     * @param msg Mensagem a imprimir
     * @param color Cor ANSI
     */
    void log(const string& msg, const char* color = Colors::RESET) {
        unsigned long my_ticket;

        // Cada thread pega um ticket exclusivo
        pthread_mutex_lock(&mutex_ticket);
        my_ticket = ticket_counter++;
        while (my_ticket != now_serving) {
            pthread_cond_wait(&cond, &mutex_ticket);
        }
        pthread_mutex_unlock(&mutex_ticket);

        // Região crítica: impressão no terminal
        cout << color << msg << Colors::RESET << endl;

        // Libera para o próximo ticket
        pthread_mutex_lock(&mutex_ticket);
        now_serving++;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex_ticket);
    }

    /**
     * @brief Função utilizada especificamente para notificar erros ao usuário.
     * 
     * @param msg Mensagem de erro
     */
    void error(const string &msg){
        this->log("[\033[31mERROR\033[0m] " + msg);
    }

    /**
     * @brief Função utilizada especificamente para notificar sucessos de operações ao usuário.
     * 
     * @param msg Mensagem de sucesso
     */
    void success(const string &msg){
        this->log("[\033[32mSUCCESS\033[0m] " + msg);
    }

    /**
     * @brief Função utilizada especificamente para notificar avisos ao usuário.
     * 
     * @param msg Mensagem de aviso
     */
    void warning(const string &msg){
        this->log("[\033[33mWARNING\033[0m] " + msg);
    }

    /**
     * @brief Função utilizada especificamente para notificar informações em destaque ao usuário.
     * 
     * @param msg Mensagem de informação
     */
    void info(const string &msg){
        this->log("[\033[36mINFO\033[0m] " + msg);
    }
};

#endif
