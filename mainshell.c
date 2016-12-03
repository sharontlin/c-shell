#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <wordexp.h>
#include "mainshell.h"

char home[1024];
int StdIn, StdOut = -1;

//************************************************
//      strip(char * str)
//      removes white space from strings
//      output: stripped string
//
//************************************************

char * strip(char * str) {
  char * end;
  while(isspace(*str)) str++;
  if(*str == 0) return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  *(end+1) = 0;
  return str;
}


 //*********************************************************
 //      parse(char ** cmd, char * arg, char ** parent)
 //      splits the line across all semicolons
 //          cmd = array of arguments
 //          str = string with unparsed line args
 //          parent = pointer to str
 //      output: new value of str
 //
 //*********************************************************

char * parse(char ** cmd, char * str, char ** parent) {
  // Pre-parsing
  char * token = strsep(&str, ";");
  token = strip(token); // Command, Ready For Parsing
  if (token == NULL || str == NULL) return 0; // No Commands

  // Parsing into cmd
  int i = 0;
  while (token) {
    cmd[i] = strsep(&token, " ");
    if (strcmp(cmd[i], "") == 0) i--; // Ignore extra spaces
    i++;
  }
  cmd[i] = 0;
  *parent = str;
  return str;
}

//*********************************************************
//      verify(char * in)
//      reads a string and catches any bad tokens
//      output: any bad tokens in the string
//
//*********************************************************

char * verify(char * str) {
  if (strstr(str, ";;")) return strcpy(str, ";;");
  if (strstr(str, "&;")) return strcpy(str, "&;");
  if (strstr(str, ";&")) return strcpy(str, ";&");
  while (*str) {
    if (isspace(*str))
      ;
    else if (*str == ';')
      return strcpy(str, "leading ;");
    str++;
  }
  return NULL;
}


//************************************************
//      sighandler(int signo)
//      handles and interprets signals
//      output: none
//
//************************************************

static void sighandler(int signo) {
  if (signo == SIGINT) exit(0);
}

//************************************************
//      resetIO(int fd, int type)
//      resets stdin/stdout to place on file table
//        fd - standard file descriptor
//        type:
//            0 = stdin
//            1 = stdout
//      output: none
//
//************************************************

void resetIO(int fd, int type){
  int a = dup2(fd, type);
}

//************************************************
//      resetStdIO()
//      resets stdin/stdout to fd 0 and 1
//      output: none
//
//************************************************

void resetStdIO(){
  if (StdIn != -1) resetIO(StdIn, 0);
  if (StdOut != -1) resetIO(StdOut, 1);
  StdIn = -1;
  StdOut = -1;
}

//************************************************
//      redirect(char * redirectTo, int type)
//      redirects the input depending on the sign
//        where:
//            0 = input
//            1 = output
//      output: int - new fd of stdin/stdout
//
//************************************************

int redirect(char * redirectTo, int type){
  int fd;
  int fdToReplace;
  if (type == 0){
    fd = open(redirectTo, O_CREAT | O_RDWR, 0644);
    fdToReplace = 0;
  }
  else {
    fd = open(redirectTo, O_CREAT | O_RDWR, 0644);
    fdToReplace = 1;
  }
  int ret = dup(fdToReplace);
  dup2(fd, fdToReplace);
  return ret;
}

//************************************************
//      redirCheck(char ** command, int type)
//      checks for redirects in line
//          0 = <
//          1 = >
//      output: index of redir symbol
//
//************************************************

int redirCheck(char ** command, int type){
  char sign[2];
  if (type == 0) strcpy(sign, "<");
  else strcpy(sign, ">");
  int index = 0;
  while(* command) {
    if (strcmp(* command, sign) == 0) return index;
    index++;
    command++;
  }
  return -1;
}

//************************************************
//      handleRedir(char ** cmd)
//      redirects stdin/stdout
//      output: command with NULL in place of symbol
//
//************************************************

char ** handleRedir(char ** cmd){
  int redirInput = redirCheck(cmd, 0); // index of the input
  int redirOutput = redirCheck(cmd, 1); // index of the outputr
  if (redirInput != -1){
    StdIn = redirect(cmd[redirInput + 1], 0);
    cmd[redirInput] = 0;
  }

  if (redirOutput != -1){
      StdOut = redirect(cmd[redirOutput + 1], 1);
      cmd[redirOutput] = 0;
  }
  return cmd;
}

//************************************************
//      pipeCheck(char ** cmd)
//      searches for pipe symbol in command
//      output: index of "|"
//
//************************************************

int pipeCheck(char ** cmd){
  int index = 0;
  while(* cmd) {
    if (strcmp(* cmd, "|") == 0) return index;
    index++;
    cmd++;
  }
  return -1;
}

//************************************************
//      pipeExec(char ** cmd)
//      pipes and executes command
//      output: 1 if ran, 0 if no pipe
//
//************************************************

int pipeExec(char ** cmd){
  int index = pipeCheck(cmd);
  if (index == -1) return 0;

  //pointer to beginning of command after the pipe
  char ** secondCommand = (cmd + index + 1);
  cmd[index] = 0;

  int pipefd[2];
  pipe(pipefd);

  int status;
  int j = -1;

  int fd = fork();
  if (fd == 0) { //chlild
    close(pipefd[0]);
    StdOut = dup(1);
    dup2(pipefd[1], 1);
    execute(cmd); // Execute First Command
    close(pipefd[1]);
    exit(0);
  }
  else {
    wait(&status);
    close(pipefd[1]);
    StdIn = dup(0);
    dup2(pipefd[0], 0);
    execute(secondCommand); // Execute Second Command
    close(pipefd[0]);
  }
  return 1; //boolean
}

//************************************************
//      cd(char ** cmd)
//      changes directory
//      output: none
//
//************************************************

void cd(char ** cmd) {
  int res;

  if (!cmd[1] || strcmp(cmd[1], "~") == 0) //cd to the home dir
    res = chdir(home);

  else
    res = chdir(cmd[1]);

  if (res == -1)
    printf("This directory does not exist\n");
}


//****************************************************
//      convTildes(char ** cmd)
//      converts ~ to home dir path
//      output: command with home path in place of ~
//
//****************************************************

char ** convTildes(char ** cmd){
  char ** ret = cmd;
  while (* cmd){
    if (* cmd[0] == '~') {
      char path[1024];
      strncpy(path, home, 1023);
      strncat(path, ++*cmd, 1023);
      * cmd = path;
    }
    cmd++;
  }
  return ret;
}

//****************************************************
//      execute(char ** cmd)
//      forks and executes the command
//      output: none
//
//****************************************************

void execute(char ** cmd) {
  // Forks and executes
  int f = fork();
  int j = -1;
  int status;
  if (f == 0) {

    j = execvp(cmd[0], cmd);

    if (j == -1)
      printf("%s: command not found\n", cmd[0]);
    exit(0);
  } else {
    wait(&status);
  }
}


int main() {
  umask(0000);
  signal (SIGINT, sighandler);
  char cwd[1024];

  strncpy(home, getenv("HOME"), 1024);

  char d[1024];
  char * dest = d;
  char ** pdest = &dest;
  char * cmd[1024];
  char ** command = cmd;

  while (1) {
    getcwd(cwd, sizeof(cwd));
    printf("%s> ", cwd);

    fgets(dest, 256, stdin);
    if (!*(dest+1)) continue;

    if (verify(dest) != 0) {
      printf("Bad Token: %s\n", dest);   // Verify input
      continue;
    }

    // Preparsing
    dest = strip(dest);
    dest[strlen(dest)+1] = 0; // Move terminating char one down
    dest[strlen(dest)] = ';'; // Add semicolon to end for efficient parsing


    //************************************
    //      Start of loop
    //************************************
    while (1) {
      if (!parse(cmd, dest, pdest)) break;
      command = convTildes(command);
      command = handleRedir(command);

      //cd
      if (strcmp(cmd[0], "cd") == 0) cd(command);
      //exit
      else if (strcmp(cmd[0], "exit") == 0) exit(0);
      //piping
      else if (! pipeExec(command)) execute(command);

      }
      resetStdIO();
  }
    //************************************
    //     End of loop
    //************************************

  return 0;
}
