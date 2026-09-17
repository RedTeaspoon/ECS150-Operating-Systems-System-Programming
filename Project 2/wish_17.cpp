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
#include <algorithm>

using namespace std;
// g++ -o wish wish.cpp -Wall -Werror
// ./test-wish.sh
// ./wish

/* 
  execv returns value if error happens
  int execv(const char *path, char *const argv[]);
  access() -> checks path

  only error message; Most processes run after print, only exceptions are shell given more than 1 file or bad batch file
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 

  running batch mode, no output, read cmds from file & execute

  TO DO:
    implement batch mode -- done
    implement (simple) interactive mode -- done
    implement exit, cd, path -- done
    implement in-built cmds -- done
    redirection (>), if output file exists overwrite (cmd + args > output), any errors rerouted to file output (dup2) -- to implement
    parallel cmds (fork) joined by & -- done
 */

// Removes extra whitespace (/t, /n, ' ') from cmd & converts into vector
// returns vector with elements identified as arguments from cmd
vector<string> parseCmd(string cmd){
  vector<string> args;
  int index = 0, start;
  int n = cmd.length();

  while(index < n){
    if(isspace(cmd[index]))
      index++;
    else{
      start = index;
      while(index < n && ! isspace(cmd[index]) && cmd[index] != '&' && cmd[index] != '>')
        index++;
      args.push_back(cmd.substr(start, index - start));

      if(cmd[index] == '&'){
        args.push_back("&");
        index++;
      }
      else if(cmd[index] == '>'){
        args.push_back(">");
        index++;
      }
    }
  }
  return args;
}

// searches args from start index for "&"
// returns index of "&" if found, -1 if not found
int findString(vector<string> args, string symbol, int start){
  for(int i = start; i < int(args.size()); i++){
    if(args[i] == symbol)
      return i;
  }
  return -1;
}

int main(int argc, char *argv[]) {
  string cmd = "";
  vector<string> wishArgs = {}, path = {"/bin"};
  ifstream file;
  pid_t pidPar = 1, pidBuilt = 2;
  
  // running batch file
  if (argc == 2) {
    file.open(argv[1]);

    // check that file name is valid, otherwise bad batch file
    if (! file.is_open()) {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(1);
    }
  }
  // more than 1 file invoked -> error
  else if(argc > 2){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
    exit(1);
  }

  // figure out how to loop wishArgs instead of cmd (batch file "exit" not working)
  while(getline(file, cmd) || !(int(wishArgs.size()) == 1 && wishArgs[0] == "exit")){
    // testing
    // cout << cmd << endl;
    // interactive mode
    if(argc == 1){
      cout << "wish> ";
      getline(cin, cmd);
    }

    // remove whitespace & convert into vector
    wishArgs = parseCmd(cmd);

    // for testing
    // for(int i = 0; i < int(wishArgs.size()); i++)
    //   cout << wishArgs[i] << endl;

    // check for parallel cmds
    int parStart = 0;

    while(findString(wishArgs, "&", parStart) > -1){
      pidPar = fork();

      if(pidPar == 0){
        wishArgs = vector<string>(wishArgs.begin()+parStart, wishArgs.begin()+findString(wishArgs, "&", parStart));
        break;
      }
      else
        parStart = findString(wishArgs, "&", parStart) + 1;
    }

    if(pidPar != 0)
      wishArgs = vector<string>(wishArgs.begin()+parStart, wishArgs.end());
    
    // change directory
    if(int(wishArgs.size()) > 0 && wishArgs[0] == "cd"){
      // char s[100];
      // printf("%s\n", getcwd(s, 100));
      if(int(wishArgs.size()) > 2 || chdir(wishArgs[1].c_str()) != 0){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      // printf("%s\n", getcwd(s, 100));
    }
    // change path
    else if(int(wishArgs.size()) > 0 && wishArgs[0] == "path"){
      path = vector<string>(wishArgs.begin()+1, wishArgs.begin()+int(wishArgs.size()));
      // for testing
      // for(int i = 0; i < int(path.size()); i++)
      //   cout << path[i] << " ";
      // cout << endl;
    }
    // check for extra args for "exit" cmd
    else if(int(wishArgs.size()) > 1 && wishArgs[0] == "exit"){
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
    // in-built commands
    else if(int(wishArgs.size()) > 0 && wishArgs[0] != "exit"){
      int pathNum = 0;
      int fileDescriptor = -1;

      // check every path for program to run
      while(path.size() != 0 && pathNum < int(path.size()) && access((path[pathNum] + "/" + wishArgs[0]).c_str(), X_OK) != 0)
        pathNum++;
    
      // no paths have program
      if(pathNum == int(path.size())){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      // found program in path
      else{        
        pidBuilt = fork();

        // redirect to file
        if(pidBuilt == 0 && int(wishArgs.size()) >= 3 && findString(wishArgs, ">", parStart) == int(wishArgs.size()) - 2){
          fileDescriptor = open(wishArgs[int(wishArgs.size()) - 1].c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);

          // check that file opened correctly cout
          if (fileDescriptor < 0) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            exit(1);
          }
          // flush cout & cerr just in case
          cout.flush();
          cerr.flush();

          // redirect STDOUT and STDERR to file
          dup2(fileDescriptor, STDOUT_FILENO);
          dup2(fileDescriptor, STDERR_FILENO);
          close(fileDescriptor);

          // remove redirect from wishArgs
          wishArgs = vector<string>(wishArgs.begin(), wishArgs.end()-2);
        }
        // more than 1 redirect arg or no redirect arg -> error
        else if(pidBuilt == 0 && findString(wishArgs, ">", parStart) != int(wishArgs.size()) - 2){
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message)); 
          exit(1);
        }

        if(pidBuilt == 0){
          // convert wish args to program args & add NULL to end
          const char **programArgs = new const char* [wishArgs.size() + 1];
          for(int i = 1; i < int(wishArgs.size()); i++)
            programArgs[i-1] = wishArgs[i].c_str();
          programArgs[wishArgs.size()] = NULL;

          // run program
          execv((path[pathNum] + "/" + wishArgs[0]).c_str(), const_cast<char* const*>(programArgs));
          delete programArgs;

          // flush cout & cerr just in case
          cout.flush();
          cerr.flush();

          exit(0);
        }
        wait(NULL);
      }
    }

    // child exit & parent wait for parallel cmds
    if(pidPar == 0)
      exit(0);
    else
      wait(NULL);
  }
  // testing
  // if(int(wishArgs.size()) >= 1 && wishArgs[0] == "exit")
  //   cout << "(exit) broke from cmd loop" << endl;
  
  // close the file stream after EOF
  if(argc == 2)
    file.close();

  return 0;
};