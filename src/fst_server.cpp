#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>
#include <thread>

#define PORT 9876

void task_one(int acception) {
  char buff [32];
  std::string message {"CPU architecture is "};

  // Putting the output of the command to pipe
  FILE *fp = popen("uname -p", "r");
  memset(buff, 0, 32);

  // Reading everything from pipe
  for (int i {0}; fgets(buff, sizeof(buff), fp) != NULL; ++i) {
    std::string _str{buff};
    message += _str;
  }

  pclose(fp);
  send(acception, message.c_str(), strlen(message.c_str()), 0);
}

void task_two(int acception) {
  auto processor_count = std::thread::hardware_concurrency();
  std::string message {"Number of logical cores in the system is equal to "};
  message += std::to_string(processor_count) + "\n";
  send(acception, message.c_str(), strlen(message.c_str()), 0);
}

int main() {
  struct sockaddr_in server_addr = {0};
  int sock {0}, acception {0};
  char buff[32], buff_mess;
  memset(buff, 0, 32);

  // Creating socket in IPv4 with TCP
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket failed");
    exit(1);
  }

  // Address family IPv4, port 9876, address only in local
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Connecting socket to address
  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("Binding failed");
    exit(3);
  }
  
  // How many clients we can put to queue
  listen(sock, 1);

  // Accepting client on our socket without writing down and information
  if ((acception = accept(sock, NULL, NULL)) == -1) {
    perror("Acception failed");
    exit(4);
  }

  while (1) {
    buff_mess = recv(acception, buff, sizeof(buff), 0);
    std::string command {buff};

    // Client decided to close connection
    if (command == "poweroff") {
      sleep(2);
      break;
    }

    int _command = stoi(command);
    
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