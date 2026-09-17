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

  implement exit, cd, path -- done
  redirection (>), if output file exists overwrite (cmd + args > output), any errors rerouted to file output (dup2)
  parallel cmds (fork) joined by & -- done?, need to test

  only error message; Most processes run after print, only exceptions are shell given more than 1 file or bad batch file
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 

  running batch mode, no output, read cmds from file & execute
 */

// Removes extra whitespace from cmd & converts into vector
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
  vector<string> wishArgs, path = {"/bin"};
  ifstream file;

  // testing execv on Windows
  // std::vector<char*> args;
  // args.push_back(const_cast<char*>("ls")); 
  // args.push_back(const_cast<char*>("-l"));
  // args.push_back(nullptr);
  // char** argv2 = args.data();
  // cout << execv("/bin/ls", argv2) << endl;
  
  // running batch file
  if (argc == 2) {
    ifstream file(argv[1]);

    // check that file name is valid, otherwise bad batch file
    if (! file.is_open()) {
      char error_message[30] = "An error has occurred2\n";
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(1);
    }

    // read each line from the file and run cmd, incomplete
    while (getline(file, cmd)) {
      cout << cmd << endl;
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

  // figure out how to loop with batch file
  //  && getline(file, cmd)
  while(cmd != "exit"){
    // interactive mode
    if(argc == 1){
      cout << "wish> ";
      getline(cin, cmd);
    }

    // remove whitespace & convert into vector
    wishArgs = parseCmd(cmd);

    // check for parallel cmds
    int parStart = 0;
    pid_t pid = 1;

    while(findPara(wishArgs, parStart) > -1){
      pid = fork();

      if(pid == 0){
        wishArgs = vector<string>(wishArgs.begin()+parStart, wishArgs.begin()+findPara(wishArgs, parStart));
        break;
      }
      else
        parStart = findPara(wishArgs, parStart) + 1;
    }

    if(pid != 0)
      wishArgs = vector<string>(wishArgs.begin()+parStart, wishArgs.end());

    // for testing
    // cout << pid << ": ";
    // for(int i = 0; i < int(wishArgs.size()); i++)
    //   cout << wishArgs[i] << endl;
    
    // change directory
    if(int(wishArgs.size()) != 0 && wishArgs[0] == "cd"){
      // char s[100];
      // printf("%s\n", getcwd(s, 100));
      if(int(wishArgs.size()) > 2 || chdir(wishArgs[1].c_str()) != 0){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      // printf("%s\n", getcwd(s, 100));
    }
    // change path
    else if(int(wishArgs.size()) != 0 && wishArgs[0] == "path"){
      path = vector<string>(wishArgs.begin()+1, wishArgs.begin()+int(wishArgs.size()));

      // for testing
      // for(int i = 0; i < int(path.size()); i++)
      //   cout << path[i] << endl;
    }
    // parallel cmds -> fork
    // else if(){

    //   // check for redirection (dup2)
    // }
    else if(int(wishArgs.size()) != 0 && wishArgs[0] != "exit"){
      int pathNum = 0;

      // check every path for program to run
      while(path.size() != 0 && ((path[pathNum] + "/" + wishArgs[0]).c_str(), X_OK) != 0)
        pathNum++;

      // no paths have program
      if(pathNum == int(path.size())){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        break;
      }

      // convert wish args to program args & add NULL to end
      const char **programArgs = new const char* [wishArgs.size()];
      for(int i = 1; i < int(wishArgs.size()); i++)
        programArgs[i-1] = wishArgs[i].c_str();
      programArgs[wishArgs.size() - 1] = NULL;

      // run program
      execv(path[pathNum].c_str(), const_cast<char* const*>(programArgs));
    }

    if(pid == 0)
      exit(0);
    else if(findPara(wishArgs, parStart) > -1){
      cout << "waiting" << endl;
      wait(NULL);
    }
  }
  // testing
  if(cmd == "exit")
    cout << "(exit) broke from cmd loop" << endl;
  
  // close the file stream after EOF
  if(argc == 2)
    file.close();

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