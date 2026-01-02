#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_CMD_LINES 512
#define MAX_ARGS 32

// Function to trim leading/trailing whitespaces	
char *trim_whitespaces(char *str){
  if(str == NULL) return NULL;
  while (isspace((unsigned char)*str)) str++;
  if(*str == 0) return str;
  char *end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

void execute_command(char *cmd){
  char *args[MAX_ARGS];
  char *input_file = NULL;
  char *output_file = NULL;
  int arg_count = 0;

  char *token = strtok(cmd, " ");
  while(token != NULL){
    if(strcmp(token , "<") == 0){
      token = strtok(NULL, " ");
      input_file = token;
    } else if(strcmp(token, ">") == 0){
    	token = strtok(NULL, " ");
    	output_file = token;
    } else args[arg_count++] = token;
    token = strtok(NULL, " ");
  }
  
  args[arg_count] = NULL;
  if(arg_count == 0) return;

  if(input_file){
    int fd_in = open(input_file, O_RDONLY);
    if(fd_in < 0){
      perror("Input Error");
      exit(1);
    }
    dup2(fd_in, STDIN_FILENO);
    close(fd_in);
  }

  if(output_file){
  	int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  	if(fd_out < 1){
  	  perror("Ouput Error");
  	  exit(1);
  	}
  	dup2(fd_out, STDOUT_FILENO);
  	close(fd_out);
  }
  if(execvp(args[0], args) < 0){
  	perror("Exec Failed");
  	exit(1);
  }
}

int main(){
  char input[MAX_CMD_LINES];
  char *commands[MAX_ARGS];

  while(1){
    printf("Dynamic Abhi:~$");
    fflush(stdout);

    if(fgets(input, MAX_CMD_LINES, stdin) == NULL) break;
    input[strcspn(input, "\n")] = 0;
    if(strcmp(input, "exit") == 0) break;

    int cmd_count = 0;
    char *token = strtok(input, "|");
    while(token != NULL){
      commands[cmd_count++] = trim_whitespaces(token);
      token = strtok(NULL, "|");
    }

    int pipefd[2];
    int prev_pipe_read = 0;

    for(int i = 0; i < cmd_count; i++){
      if(i < cmd_count - 1){
        if(pipe(pipefd) < 0){
          perror("Pipe Failed");
          exit(1);
        }
      }
      
      pid_t pid = fork();

      if(pid < 0){
        perror("Fork Failed");
        return 1;
      } else if(pid == 0){
      	if(prev_pipe_read != 0){
      	  dup2(prev_pipe_read, STDIN_FILENO);
      	  close(prev_pipe_read);
      	}
      	if(i < cmd_count - 1){
      	  dup2(pipefd[1], STDOUT_FILENO);
      	  close(pipefd[1]);
      	  close(pipefd[0]);
      	}
      	execute_command(commands[i]);
      } else{
      	  if(i < cmd_count - 1){
      	  	close(pipefd[1]);
      	  }
      	  if(prev_pipe_read != 0){
      	  	close(prev_pipe_read);
      	  }
      	  if(i < cmd_count - 1){
      	  	prev_pipe_read = pipefd[0];
      	  }
      	  wait(NULL);
      }
    }
  }
  return 0;
}
