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
      while(! isspace(cmd[index]))
        index++;
      args.push_back(cmd.substr(start, index - start));
    }
  }
  return args;
}

// searches args from start index for "&"
// returns index of "&" if found, -1 if not found
int findPara(vector<string> args, int start){
  for(int i = start; i < int(args.size()); i++){
    if(args[i] == "&")
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
      char error_message[30] = "An error has occurred2\n";
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

    // check for parallel cmds
    int parStart = 0;

    while(findPara(wishArgs, parStart) > -1){
      pidPar = fork();

      if(pidPar == 0){
        wishArgs = vector<string>(wishArgs.begin()+parStart, wishArgs.begin()+findPara(wishArgs, parStart));
        break;
      }
      else
        parStart = findPara(wishArgs, parStart) + 1;
    }

    if(pidPar != 0)
      wishArgs = vector<string>(wishArgs.begin()+parStart, wishArgs.end());

    // for testing
    // cout << pid << ": ";
    // for(int i = 0; i < int(wishArgs.size()); i++)
    //   cout << wishArgs[i] << endl;
    // exit(0);
    
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
      //   cout << path[i] << endl;
    }
    // parallel cmds -> fork, need to implement memory allocation & deallocation for wishArgs
    else if(int(wishArgs.size()) > 0 && wishArgs[0] != "exit"){
      int pathNum = 0;

      // check every path for program to run
      while(path.size() != 0 && pathNum < int(path.size()) && access((path[pathNum] + "/" + wishArgs[0]).c_str(), X_OK) != 0)
        pathNum++;
    
      // no paths have program
      if(pathNum == int(path.size())){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      else{
        // convert wish args to program args & add NULL to end
        const char **programArgs = new const char* [wishArgs.size() + 1];
        for(int i = 0; i < int(wishArgs.size()); i++)
          programArgs[i] = wishArgs[i].c_str();
        programArgs[wishArgs.size()] = NULL;

        // run program
        pidBuilt = fork();
        if(pidBuilt == 0){
          execv((path[pathNum] + "/" + wishArgs[0]).c_str(), const_cast<char* const*>(programArgs));
          exit(0);
        }
        wait(NULL);
        delete programArgs;
      }
    }

    if(pidPar == 0)
      exit(0);
    else
      wait(NULL);
  }
  // testing
  if(int(wishArgs.size()) == 1 && wishArgs[0] == "exit")
    cout << "(exit) broke from cmd loop" << endl;
  
  // close the file stream after EOF
  if(argc == 2)
    file.close();

  return 0;
};