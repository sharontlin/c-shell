# Main Shell
by Sharon Lin

## Features
- Forks and executes commands
- Parses multiple commands on one line (separated by ;)
- Redirects using >, <
- Pipes using |
- Replaced ~ with home directory
- Exits with "exit" and SIGINT
- Changes directory with cd and recognizes nonexistent directories

## Attempted
- Implementing additional redirection
- Multiple pipes were not successfully implemented
- Readline and storing history were not successfully implemented
  
## Bugs
- Bad tokens are called for multiple pipes 
	
## Files & Function Headers

**mainshell.c**
- Handles all line parsing and execution

- ```char * strip(char * str)```
  - Inputs: ```char * str``` the string
  - Returns: The stripped string
  - Description: Strips all whitespace from the start and the end of a string
- ```char * parse(char ** cmd, char * str, char ** parent)```
  - Inputs: ```char ** cmd``` an array of commands 
           ```char * str``` the string
           ``` char ** parent``` a pointer to the string
  - Returns: The new value of the string
  - Description: Splits the string across semicolons
- ```char * verify(char * str)```
  - Inputs: ```char * str``` the string
  - Returns: The bad tokens in the string
  - Description: Verifies whether or not there are bad tokens within the string
- ```static void sighandler(int signo)```
  - Inputs: ```int signo``` the value of the signal
  - Returns: Nothing
  - Description: Checks for the exist shortcut
- ```void resetIO(int fd, int type)```
  - Inputs: ```int fd``` standard file descriptor
            ```int type``` type of file - 0=stdin, 1=stdout
  - Returns: Resets stdin and stdout to original places on the file table
  - Description: Strips all whitespace from the start and the end of a string
- ```void resetStdIO()```
  - Inputs: Nothing
  - Returns: Nothing
  - Description: Resets stdin/stdout to fd 0 and 1
- ```int redirect(char * redirectTo, int type)```
  - Inputs: ```char * redirectTo``` loction to redirect
            ```int type``` type of file - 0=input, 1=output
  - Returns: The new fd of the stdin/stdout
  - Description: Redirects input depending on the given sign
- ```int redirCheck(char ** command, int type)```
  - Inputs: ```char ** command``` pointer to commands
            ```int type``` type of redirection - 0="<", 1=">"
  - Returns: Index of the redirection symbol
  - Description: Checks for redirection symbol in the line
- ```char ** handleRedir(char ** cmd)```
  - Inputs: ```char ** cmd``` pointer to commands
  - Returns: The command with NULL in place of the redirection symbol
  - Description: Redirects stdin and stdout
- ```int pipeCheck(char ** cmd)```
  - Inputs: ```char ** cmd``` pointer to commands
  - Returns: The index of the pipe symbol
  - Description: Searches for pipe symbols in the commands
- ```int pipeExec(char ** cmd)```
  - Inputs: ```char ** cmd``` pointer to commands
  - Returns: 1 if the function ran, 0 if it didn't
  - Description: Pipes and executes the commands
- ```void cd(char ** cmd)```
  - Inputs: ```char ** cmd``` pointer to commands
  - Returns: Nothing
  - Description: Changes the location and current directory
- ```char ** convTildes(char ** cmd)```
  - Inputs: ```char ** cmd``` pointer to commands
  - Returns: The command with the home path in place of the tilde
  - Description: Converts tildes to expand to the home directory path
- ```void execute(char ** cmd)```
  - Inputs: ```char ** cmd``` pointer to commands
  - Returns: Nothing
  - Description: Forks and executes the commands
