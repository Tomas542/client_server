#include <winsock2.h>
#include <windows.h>
#include <psapi.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>
#include <regex>

#define PORT 9877

void task_one(int acception) {
  std::string message {"Amount of processes in the system is equal to "};
  DWORD aProcesses[1024], cbNeeded;
  int proc = EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded) ? (cbNeeded/sizeof(DWORD)) : -1;
  message += std::to_string(proc);
  send(acception, message.c_str(), strlen(message.c_str()), 0);
}

void task_two(int acception) {
  const std::regex r("#include <.+>");
  std::ifstream new_file ("src/snd_server.cpp");
  std::string message {"Number of modules on server is equal to "};
  std::string sa;
  int counter {0};

  if (new_file.is_open()) { 
    sa = "";

    // Read data from the file object and put it into a string.
    while (getline(new_file, sa)) { 
        if (std::regex_match(sa, r) == 1) { ++counter;}
    }
    
    new_file.close();
  }

  message += std::to_string(counter) + "\n";
  send(acception, message.c_str(), strlen(message.c_str()), 0);
}

int main() {
  // Loading winsock2.h
  WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if(WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

  SOCKADDR_IN server_addr;
  SOCKET sock, acception;
  int sizeofaddr = sizeof(server_addr);
  char buff[8], buff_mess;
  memset(buff, 0, 8);

  // Creating socket in IPv4 with TCP
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket failed");
    exit(1);
  }

  // Address family IPv4, port 9876, address only in local
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  // Connecting socket to address
  if (bind(sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Binding failed");
    exit(3);
  }

  // int iRes;

  // BOOL optVal = TRUE;
  // int bOptLen = sizeof (BOOL);

  // iRes = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&optVal, bOptLen);  
  // How many clients we can put to queue
  listen(sock, SOMAXCONN);

  // Accepting client on our socket without writing down and information
  if ((acception = accept(sock, (SOCKADDR*)&server_addr, &sizeofaddr)) == -1) {
    perror("Acception failed");
    exit(4);
  }

  while (1) {
    buff_mess = recv(acception, buff, sizeof(buff), 0);
    std::string command {buff};

    // Client decided to close connection
    // if (command == "poweroff") {
    //   sleep(2);
    //   break;
    // }

    int _command = stoi(command);
    if (_command == 42) {
      sleep(2);
      break;
    }

    switch (_command)
    {
      case 1:
        task_one(acception);
        break;
      
      case 2:
        task_two(acception);
        break;

      // Mostly useless
      default:
        send(acception, "No such command", sizeof("No such command"), 0);
        break;
    }
  }

  close(sock);
  return 0;
}