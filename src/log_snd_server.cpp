#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>

std::ofstream myfile;

void handler(int signo) {
  time_t curr_time = time(NULL);
  std::string fifo_message = ctime(&curr_time);
  myfile << fifo_message;
  myfile << "Power Off";
  myfile.close();
  exit(0);
}

int main(void) {
  char buff [512];
  memset(buff, 0, 512);
  myfile.open ("logs/log2.txt");
  
  if (mkfifo("log_file_two", 0777) == -1) {
    if (errno != EEXIST) {
      perror("FIFO failed");
      exit(6);
    }
  }

  int fd = open("log_file_two", O_WRONLY);
  std::string fifo_message = std::to_string(getpid());
  write(fd, fifo_message.c_str(), strlen(fifo_message.c_str()));
  close (fd);

  fd = open("log_file_two", O_RDONLY);
  if (fd == -1) {
    perror("Fifo can't be open");
    exit(5);
  }

  while (1) {
    read(fd, buff, sizeof(buff));

    if (buff[0] != 0 and buff != " " and buff != "\n" and buff != "\t") { 
      myfile << buff;
    }
    
    signal(SIGQUIT, handler);

    memset(buff, 0, 512);
  }
  return 0;
}
