#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_CMD_LINES 512	// Maximum length of input command line
#define MAX_ARGS 32			// Maximum number of arguments per command

// Function to trim leading and trailing whitespaces from a string
char *trim_whitespaces(char *str){
  if(str == NULL) return NULL;				// Safety check
  while (isspace((unsigned char)*str)) str++;		// Trim leading spaces
  if(*str == 0) return str;				// String contains only spaces
  char *end = str + strlen(str) - 1;			// Point to last character
  while(end > str && isspace((unsigned char)*end)) end--;	// Trim trailing spaces
  end[1] = '\0';						// Null-terminate string
  return str;
}

// Executes a single command with possible I/O redirection
void execute_command(char *cmd){
  char *args[MAX_ARGS];				// Argument list for execvp
  char *input_file = NULL;			// Input redirection file
  char *output_file = NULL;			// Output redirection file
  int arg_count = 0;				// Argument counter

  // Tokenize command by spaces
  char *token = strtok(cmd, " ");
  while(token != NULL){
    if(strcmp(token , "<") == 0){		// Input redirection detected
      token = strtok(NULL, " ");
      input_file = token;
    } else if(strcmp(token, ">") == 0){		// Output redirection detected
    	token = strtok(NULL, " ");
    	output_file = token;
    } else args[arg_count++] = token;		// Normal argument
    token = strtok(NULL, " ");
  }
  
  args[arg_count] = NULL;			// Null-terminate argument list
  if(arg_count == 0) return;			// Empty command

  // Handle input redirection
  if(input_file){
    int fd_in = open(input_file, O_RDONLY);
    if(fd_in < 0){
      perror("Input Error");
      exit(1);
    }
    dup2(fd_in, STDIN_FILENO);		// Redirect stdin
    close(fd_in);
  }

  // Handle output redirection
  if(output_file){
  	int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  	if(fd_out < 1){
  	  perror("Ouput Error");
  	  exit(1);
  	}
  	dup2(fd_out, STDOUT_FILENO);	// Redirect stdout
  	close(fd_out);
  }

  // Execute the command
  if(execvp(args[0], args) < 0){
  	perror("Exec Failed");
  	exit(1);
  }
}

int main(){
  char input[MAX_CMD_LINES];			// Input buffer
  char *commands[MAX_ARGS];			// Array of piped commands

  while(1){
    printf("Dynamic Abhi:~$");		// Shell prompt
    fflush(stdout);

    // Read user input
    if(fgets(input, MAX_CMD_LINES, stdin) == NULL) break;
    input[strcspn(input, "\n")] = 0;	// Remove trailing newline

    // Exit condition
    if(strcmp(input, "exit") == 0) break;

    // Split input by pipe operator
    int cmd_count = 0;
    char *token = strtok(input, "|");
    while(token != NULL){
      commands[cmd_count++] = trim_whitespaces(token);
      token = strtok(NULL, "|");
    }

    int pipefd[2];				// Pipe file descriptors
    int prev_pipe_read = 0;			// Read end of previous pipe

    // Loop through each command in the pipeline
    for(int i = 0; i < cmd_count; i++){
      if(i < cmd_count - 1){
        if(pipe(pipefd) < 0){
          perror("Pipe Failed");
          exit(1);
        }
      }
      
      pid_t pid = fork();			// Create child process

      if(pid < 0){
        perror("Fork Failed");
        return 1;
      } else if(pid == 0){
      	// Child process

      	// Read from previous pipe if exists
      	if(prev_pipe_read != 0){
      	  dup2(prev_pipe_read, STDIN_FILENO);
      	  close(prev_pipe_read);
      	}

      	// Write to next pipe if not last command
      	if(i < cmd_count - 1){
      	  dup2(pipefd[1], STDOUT_FILENO);
      	  close(pipefd[1]);
      	  close(pipefd[0]);
      	}

      	// Execute the command
      	execute_command(commands[i]);
      } else{
      	// Parent process

      	// Close write end of current pipe
      	if(i < cmd_count - 1){
      	  	close(pipefd[1]);
      	}
        
      	// Close previous read end
      	if(prev_pipe_read != 0){
      	  	close(prev_pipe_read);
      	}

      	// Save read end for next command
      	if(i < cmd_count - 1){
      	  	prev_pipe_read = pipefd[0];
      	}

      	// Wait for child process to finish
      	wait(NULL);
      }
    }
  }
  return 0;
}
