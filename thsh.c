/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_PATH_LEN 4096
  
struct stat filestat;

void execute(char* path);
int checkCmd(char* cmd);

int main(int argc, char ** argv, char **envp) 
{
  int finished = 0;
  char *prompt = "thsh> ";
  char cmd[MAX_INPUT];

  while (!finished) 
  {
    char *cursor;
    char last_char;
    int rv;
    int count;


    // Print the prompt
    rv = write(1, prompt, strlen(prompt));
    if (!rv) 
    { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1; rv && (++count < (MAX_INPUT-1)) && (last_char != '\n'); cursor++) 
    { 
      rv = read(0, cursor, 1);
      last_char = *cursor;
    } 
    *cursor = '\0';

    if (!rv) 
    { 
      finished = 1;
      break;
    }


    // Execute the command, handling built-in commands separately 
    // Just echo the command line for now
    write(1, cmd, strnlen(cmd, MAX_INPUT));
    checkCmd(cmd);

  }

  return 0;
}

int checkCmd(char* cmd)
{
  cmd[strlen(cmd)-1]='\0';
  if(strcmp(cmd, "exit") == 0)
  {
    exit(3);
  }

	//not search path
	else if(cmd[0] == '\\')
	{
    printf("in not search path\n");
		stat(cmd, &filestat);
	}

	//search PATH
	else 
	{
  	char* path = getenv("PATH");
    printf("Path: %s", path);
		char* pathI = malloc(MAX_PATH_LEN * sizeof(char));

		//tokenize path along ':' and concantenate '/' and the inputed cmd to it, then check to see if a file at this path exists
		//if it does call execute on that path
    char* token = strtok(path, ":");
		while(token != NULL)
		{
      sprintf(pathI, "%s/%s", token, cmd);
      printf("Pathi: %s\n", pathI);
      printf("Stat: %d\n", stat(pathI, &filestat));
      if(stat(pathI, &filestat) == 0)
      {
        printf("in stat con");
        fflush(stdout);
        execute(pathI);
        break;
      }
      token = strtok(NULL, ":");
      pathI[0]='\0';
		}
	}
}

void execute(char* path)
{
  printf("executing: %s", path);
  // int stat;
  // pid = fork();
  // if(pid == 0)
  // {
  //   if(execvp(*arg, arg) < 0)
  //   {
  //     printf("ERROR in execvp");
  //     exit(1);
  //   }

  //   else if(pid < 0)
  //     printf("ERORR from pid");

  //   else 
  //   {
  //     while(1)
  //     {
  //       if(wait(&stat)==pid)
  //         break;
  //     }
  //   }
  // }

  // return 1;
}