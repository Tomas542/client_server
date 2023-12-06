#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>
#include <string>

#define PORT_FST 9876
#define PORT_SND 9877

int current_server {0};

void connect(int* out_socket, int port) {

  // Creating a socket structure
  struct sockaddr_in server_addr = {0};

  // Creating a SOCKET with IPv4 address and TCP protocol
  if ((*out_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket failed");
    exit(1);
  };

  // Address family IPv4, port 9876, address only in local
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  // Connect though socket to address with adrlen
  if (connect(*out_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Connection to server failed");
    exit(2);
  }

  // Delete it!!!
  std::cout << "Mi connected" << std::endl;
}

int main(void) {

  // Socket for connection aka fd
  int socket[2];
  int port[2] = {PORT_FST, PORT_SND};
  std::string command;
  char buff [128];

  std::cout << "Hello and welcome to my server!" << std::endl;

  while (1) {

    // We haven't choosen the server
    if (current_server == 0) {

      // Client gave server number we don't have
      if (command != "1" and command != "2") {
        std::cout << "Enter the server number 1 or 2" << std::endl;
        std::cin >> command;
        continue;
      } else {
        current_server = stoi(command);
        if (socket[current_server-1] != 0) { continue; }
        connect(&socket[current_server-1], port[current_server-1]);
        continue;
      }  

    } else {
      std::cout << "Enter a command for current server or switch or close" << std::endl;
      std::cin >> command;

      if (command == "switch") {
        current_server = 0;
      } else if (command == "close") {
        int i {0};

        for (; i < 2; ++i) { 
          send(socket[i], "poweroff", sizeof("poweroff"), 0); 
          close(socket[i]);
        }
        
        break;
      } else if (command == "1" or command == "2") {
        send(socket[current_server-1], command.c_str(), strlen(command.c_str()), 0); 
        memset(buff, 0, 128);
        recv(socket[current_server-1], buff, sizeof(buff), 0);
        std::cout << buff << std::endl;
      } else {
        std::cout << "Wrong command. Pls try again!" << std::endl;
      }
    }
  }

  std::cout << "Client is dead!" << std::endl;
  return 0;
}