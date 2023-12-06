// #include <netinet/in.h>
#include <winsock2.h>
#include <windows.h>
// #pragma comment(lib, "ws2_32.lib")
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>

#define PORT_FST 9876
#define PORT_SND 9877

int current_server {0};

void connection(int* out_socket, int port) {
  SOCKADDR_IN server_addr;

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  *out_socket = socket(AF_INET, SOCK_STREAM, 0);

  // int iRes;

  // BOOL optVal = TRUE;
  // int bOptLen = sizeof (BOOL);

  // iRes = setsockopt(Connetction, IPPROTO_TCP, TCP_NODELAY, (char *)&optVal, bOptLen);  

  // Connect though socket to address with adrlen
  if (connect(*out_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Connection to server failed");
    exit(2);
  } 

  // Delete it!!!
  std::cout << "Mi connected" << std::endl;
}

int main(void) {
  // Loading winsock2.h
  WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if(WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

  // Socket for connection aka fd
  int socket[2] {0, 0};
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
        connection(&socket[current_server-1], port[current_server-1]);
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
          send(socket[i], "42", sizeof("42"), 0); 
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