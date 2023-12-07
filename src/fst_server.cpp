#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#include <iostream>
#include <string>
#include <thread>

#define PORT 9876

void writing_in_fifo(int fd, std::string mess) {
  time_t curr_time = time(NULL);
  std::string fifo_message = ctime(&curr_time);
  fifo_message += mess + "\n";

  write(fd, fifo_message.c_str(), strlen(fifo_message.c_str()));

  close(fd);
  fd = open("log_file_one", O_WRONLY);
}

void task_one(int acception, int fd) {
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
  writing_in_fifo(fd, message);

  pclose(fp);
  send(acception, message.c_str(), strlen(message.c_str()), 0);
}

void task_two(int acception, int fd) {
  auto processor_count = std::thread::hardware_concurrency();
  std::string message {"Number of logical cores in the system is equal to "};
  message += std::to_string(processor_count) + "\n";
  writing_in_fifo(fd, message);
  send(acception, message.c_str(), strlen(message.c_str()), 0);
}

int main() {
  struct sockaddr_in server_addr = {0};
  int sock {0}, acception {0};

  time_t curr_time;

  char buff[32], buff_mess;
  memset(buff, 0, 32);

  int fd = open("log_file_one", O_RDONLY);
  read(fd, buff, sizeof(buff));
  std::string fifo_message {buff};
  memset(buff, 0, 32);

  int pid_log = stoi(fifo_message);
  close (fd);

  fd = open("log_file_one", O_WRONLY);
  if (fd == -1) {
    perror("Fifo can't be open");
    exit(5);
  }

  writing_in_fifo(fd, "Socket creating");

  // Creating socket in IPv4 with TCP
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    writing_in_fifo(fd, "Socket failed");
    kill(pid_log, SIGQUIT);
    perror("Socket failed");
    exit(1);
  }

  // Address family IPv4, port 9876, address only in local
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  writing_in_fifo(fd, "Binding a socket");

  // Connecting socket to address
  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    writing_in_fifo(fd, "Binding failed");
    kill(pid_log, SIGQUIT);
    perror("Binding failed");
    exit(3);
  }
  
  writing_in_fifo(fd, "Listening");

  // How many clients we can put to queue
  listen(sock, 1);

  writing_in_fifo(fd, "Accepting");

  // Accepting client on our socket without writing down and information
  if ((acception = accept(sock, NULL, NULL)) == -1) {
    writing_in_fifo(fd, "Acception failed");
    kill(pid_log, SIGQUIT);
    perror("Acception failed");
    exit(4);
  } 

  while (1) {
    buff_mess = recv(acception, buff, sizeof(buff), 0);
    std::string command {buff};

    // Client decided to close connection
    if (command == "poweroff") {
      kill(pid_log, SIGQUIT);
      sleep(2);
      break;
    }

    int _command = stoi(command);
    
    switch (_command)
    {
      case 1:
        writing_in_fifo(fd, "Command 1");
        task_one(acception, fd);
        break;
      
      case 2:
        writing_in_fifo(fd, "Command 2");
        task_two(acception, fd);
        break;

      // Mostly useless
      default:
        writing_in_fifo(fd, "Unknown command");
        send(acception, "No such command", sizeof("No such command"), 0);
        memset(buff, 0, 32);
        break;
    }
  }

  close(acception);
  close(sock);
  close(fd);
  return 0;
}
