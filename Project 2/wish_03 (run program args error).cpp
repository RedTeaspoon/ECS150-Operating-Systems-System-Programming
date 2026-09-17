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
// ./wish

/* 
  execv returns value if error happens
  int execv(const char *path, char *const argv[]);
  access() -> checks path

  implement exit, cd, path, 
  redirection (>), if output file exists overwrite (cmd + args > output), any errors rerouted to file output (dup2)
  parallel cmds (fork) joined by &

  only errror message; Most processes run after print, only exceptions are shell given more than 1 file or bad batch file
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

      // read each line from the file and run cmd, incomplete
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
      }
      // printf("%s\n", getcwd(s, 100));
    }
    // change path
    else if(wishArgs[0] == "path"){
      path = vector<string>(wishArgs.begin()+1, wishArgs.begin()+n);

      // for testing
      // n = path.size();
      // for(int i = 0; i < n; i++)
      //   cout << path[i] << endl;
    }
    // parallel cmds -> fork
    // else if(){

    //   // check for redirection (dup2)
    // }
    else{
      n = path.size();
      int i = 0;
      // check every path for program to run
      while(access((path[i] + "/" + wishArgs[0]).c_str(), X_OK) != 0){
        i++;
        // convert wish args to program args
        vector<string> programArgs = vector<string>(wishArgs.begin()+i+1, wishArgs.end());

        // run program
        if(execv(path[i].c_str(), "") != 0){
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
          break;
      }

      // no paths have program
      if(i == n){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      
    }
  }
  // testing
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