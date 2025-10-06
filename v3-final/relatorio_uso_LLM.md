# Relatório de Uso de Inteligência Artificial no Desenvolvimento do Sistema de Chat em Rede


## 📌 Contextualização

Este projeto teve como objetivo o desenvolvimento de um **sistema de chat multi-salas em C++**, utilizando **sockets TCP** para comunicação entre clientes e servidor.
A proposta incluía recursos de criação e autenticação de salas, suporte a múltiplos usuários simultâneos e implementação de mensagens privadas.

Durante o processo, foi utilizado um **assistente de Inteligência Artificial (IA)** como ferramenta de apoio, não para substituir o trabalho, mas para **otimizar etapas repetitivas e oferecer insights técnicos**.

---

## 🛠️ Meu Papel no Projeto

Grande parte das decisões de **arquitetura, lógica de programação, organização de threads e sockets** foram realizadas manualmente.
Fui responsável por:

* Estruturar o servidor e definir como ele lidaria com múltiplos clientes.
* Decidir pelo uso de `std::thread` e `std::mutex` para garantir segurança nas operações concorrentes.
* Implementar o fluxo de comandos (`/user`, `/create`, `/join`, `/list`, `/p`, `/exit`).
* Ajustar o cliente para lidar corretamente com entrada do usuário e mensagens assíncronas.
* Integrar o **logging colorido** para facilitar a depuração.

---

## 🤖 Papel da IA

A IA foi utilizada como **apoio técnico e consultivo**, principalmente para:

1. **Identificação de erros de compilação e runtime** em trechos de código (ex.: problemas de iteradores, mutex e gerenciamento de sockets).
2. **Sugestão de boas práticas** (uso de `pthread_exit`, tratamento de `Ctrl+C`, organização de mensagens privadas).
3. **Revisão de estilo** no cliente (como exibir corretamente o prompt `> ` após mensagens recebidas).
4. **Produção de documentação auxiliar**:

   * Criação inicial do **README.md**.
   * Rascunho de explicações sobre como compilar e executar.
   * Sugestão de exemplos práticos para demonstração.
   * Criação do arquivo makefile

---

## 📊 Avaliação da Colaboração

* **Créditos principais**: O desenvolvimento, testes, depuração e integração ficaram sob minha responsabilidade.
* **Contribuição da IA**: forneceu suporte pontual, servindo como uma “segunda opinião” em momentos de dúvida e poupando tempo em detalhes de sintaxe ou documentação.
* **Resultado**: o projeto manteve **autoria central humana**, com a IA apenas acelerando o processo.

---

## ✅ Conclusão

O uso da IA **não substituiu meu trabalho como desenvolvedor**, mas foi um **instrumento útil** que:

* Otimizou o tempo de desenvolvimento.
* Evitou que erros simples atrasassem o avanço do projeto.
* Forneceu rascunhos de documentação que pude adaptar.

Assim, a **criação do sistema de chat** é mérito principalmente do trabalho manual de implementação, lógica e testes, com a IA funcionando como **ferramenta auxiliar inteligente**, de forma **leviana, prática e pontual**.
