# Sistema de Chat em Rede (C++ com Sockets)

Este projeto implementa um **sistema de chat em rede** utilizando **C++ e sockets nativos**, sem uso de bibliotecas externas de rede.
A aplicação é dividida em dois programas principais:

* **Servidor**: responsável por gerenciar salas, conexões e eventos da rede.
* **Cliente**: interface simples e amigável em terminal, onde os usuários podem interagir.

---

## 🚀 Funcionalidades

### Servidor

* Disponibiliza os serviços de chat em uma **porta da rede** (definida no código, ex: `4000`).
* Aceita conexões de múltiplos clientes em paralelo através de threads.
* Gera **logs coloridos** (com a biblioteca `logging.hpp`) para cada ação relevante:

  * Criação de sala
  * Entrada e saída de usuários
  * Envio de mensagens
* Mantém uma lista de salas criadas e seus respectivos usuários.
* Responde a comandos especiais, como:

  * `getRooms`: retorna a lista atualizada de salas existentes.

---

### Cliente

* Interface em terminal com instruções de uso e prompt de entrada.
* Possibilita visualizar salas criadas antes de entrar (`getRooms`).
* Usuário informa:

  * **Nome**
  * **Sala** em que deseja entrar (uma nova é criada se não existir).
* Exibe mensagens recebidas em tempo real (de todos na sala).
* Permite ao usuário:

  * Enviar mensagens para a sala.
  * Digitar `exit` para sair da sala e encerrar a conexão.

---

## 🛠️ Como executar

### Compilação

No Linux/WSL, compile com:

```bash
g++ server.cpp -o server -pthread
g++ client.cpp -o client -pthread
```

### Execução

1. **Inicie o servidor** em um terminal:

   ```bash
   ./server
   ```

2. **Abra o cliente** em outro terminal:

   ```bash
   ./client
   ```

3. O cliente mostrará as salas disponíveis (caso existam) e pedirá:

   * Nome de usuário
   * Sala para entrar

4. Envie mensagens digitando no prompt `>`

   * Para sair, digite:

     ```
     exit
     ```

---

## 📋 Exemplo de uso

### Servidor:

```
[SUCCESS] Servidor iniciado na porta 4000
[INFO] Sala criada: sala1
[SUCCESS] Luis entrou na sala sala1
[sala1] Luis: olá!
[WARNING] Luis saiu da sala sala1
```

### Cliente:

```
----------------------------------------------------
|                                                  |
|                SISTEMA DE CHAT                   |
|                                                  |
----------------------------------------------------

Instruções:
    - Informe seu nome e o nome da sala na qual deseja entrar
    - Ao ser conectado, voce podera comecar a enviar e receber mensagens nesta sala
    - Digite "exit" para se retirar da sala

----- SALAS CRIADAS -----
- sala1 (1 usuários)

Digite seu nome: Luis
Digite o nome da sala: sala1
> olá!
```

---

## 📦 Estrutura de Arquivos

```
.
├── server.cpp      # Código-fonte do servidor
├── client.cpp      # Código-fonte do cliente
├── logging.hpp     # Biblioteca para logging colorido com ticket lock
└── README.md       # Este documento
```

---

## 📌 Observações

* O sistema utiliza **sockets TCP** (`AF_INET`, `SOCK_STREAM`).
* O servidor é multi-threaded, garantindo suporte a vários clientes simultâneos.
* Todos os prints no terminal são sincronizados via `logging.hpp` para evitar sobreposição de mensagens.
* Funciona em **Linux/WSL** (não testado em Windows nativo).
