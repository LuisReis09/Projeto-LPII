# Makefile para chat C++ (WSL/Windows Terminal)

CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread

SERVER = server
CLIENT = client

all: $(SERVER) $(CLIENT)
	@echo "Abrindo servidor e cliente em novos terminais..."
	# Servidor em um terminal
	wt.exe wsl ./server
	# Espera 1s para o servidor iniciar
	sleep 1
	# Cliente em outro terminal
	wt.exe wsl ./client

$(SERVER): server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o $(SERVER)

$(CLIENT): client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
