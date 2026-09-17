#include <iostream>
 
#include <fcntl.h>
#include <stdlib.h>
 
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstream>
#include <string.h>
#include <vector>

using namespace std;
// g++ -o wish wish.cpp -Wall -Werror
// ./test-wish.sh

/* 
  execv returns value if error happens
  int execv(const char *path, char *const argv[]);
  access() -> checks path

  implement exit, cd, path, 
  redirection (>), if output file exists overwrite (cmd + args > output), any errors rerouted to file output
  parallel cmds (fork) joined by &

  only errror message. Most processes run after print, only exceptions are shell given more than 1 file or bad batch file
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message)); 

  running batch mode, no output, read cmds from file & execute
 */

int main(int argc, char *argv[]) {
  int fileDescriptor;
  string cmd = "";
  vector<string> wishArgs;

  while(cmd != "exit"){
    // running batch file
    if (argc == 2) {
      fileDescriptor = open(argv[1], O_RDONLY);

      // check that file name is valid
      if (fileDescriptor == -1) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        exit(1);
      }
      break;
    }
    // more than 1 file invoked
    else if(argc > 2){
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(1);
    }

    cout << "wish> ";
    getline(cin, cmd);
    // remove whitespace
    wishArgs = parseCmd(cmd);


  }
  
  cout << "broke from cmd loop" << endl;
  return 0;
}

vector<string> parseCmd(string cmd){
  vector<string> args;

  return args;
}