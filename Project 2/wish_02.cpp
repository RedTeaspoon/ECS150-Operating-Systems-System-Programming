#include <iostream>
 
#include <fcntl.h>
#include <stdlib.h>
 
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <vector>
#include <fstream>

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

vector<string> parseCmd(string cmd){
  vector<string> args;
  int index = 0, start;
  int n = cmd.length();

  while(index < n){
    if(isspace(cmd[index]))
      index++;
    else{
      start = index;
      while(! isspace(cmd[index]))
        index++;
      args.push_back(cmd.substr(start, index - start));
    }
  }
  return args;
}

int main(int argc, char *argv[]) {
  string cmd = "";
  vector<string> wishArgs, path;

  while(cmd != "exit"){
    // running batch file
    if (argc == 2) {
      ifstream file(argv[1]);

      // check that file name is valid (bad batch file)
      if (! file.is_open()) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        exit(1);
      }

      // read each line from the file and run cmd
      while (getline(file, cmd)) {
          wishArgs = parseCmd(cmd);
      }

      // close the file stream after EOF
      file.close();

      exit(0);
    }
    // more than 1 file invoked -> error
    else if(argc > 2){
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(1);
    }

    cout << "wish> ";
    getline(cin, cmd);
    // remove whitespace
    wishArgs = parseCmd(cmd);
    int n = wishArgs.size();

    // for testing
    // for(int i = 0; i < n; i++)
    //   cout << wishArgs[i] << endl;
    
    if(n == 0)
      continue;
    
    // change directory
    if(wishArgs[0] == "cd"){
      // char s[100];
      // printf("%s\n", getcwd(s, 100));
      if(n > 2 || chdir(wishArgs[1].c_str()) != 0){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        continue;
      }
      // printf("%s\n", getcwd(s, 100));
    }
    // change path
    else if(wishArgs[0] == "path"){
      path = std::vector<std::string>(wishArgs.begin()+1, wishArgs.begin()+n);

      n = path.size();
      for(int i = 0; i < n; i++)
        cout << path[i] << endl;

    }

    // if(wishArgs.size() == 1){
    //   cout << wishArgs[0];
    // }

    // check for parallel cmds
    // check for redirection
    // implement cd, must have only 1 arg
    // implement path

  }
  
  cout << "(exit) broke from cmd loop" << endl;
  return 0;
}



// pid_t p=fork();
// //if fork returns 0, that means we are in child process
// if(p==0){
// cout<<"The child process is running\n";
// cout<<"The child process id is "<<getpid()<<endl;
// }
// //for parent process, fork returns the pid of child
// else{
// wait(NULL);
// cout<<"The parent process is running\n";
// cout<<"The parent process id is "<<getpid()<<endl;