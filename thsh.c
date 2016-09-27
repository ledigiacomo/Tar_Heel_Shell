#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_PATH_LEN 4096
#define MAX_PARAMS 1000
#define pathGLBL getenv("PATH")
#define HOME getenv("HOME")
char* pwd;
char* lwd;
char* token;
int debug;

struct stat filestat;

int execute(char* path, char** params);
int checkCmd(char* cmd, char** params);

int main(int argc, char ** argv, char **envp) 
{
  int finished = 0;
  char *prompt = "thsh> ";
  char input[MAX_INPUT];
  pwd = malloc(MAX_PATH_LEN);
  lwd = malloc(MAX_PATH_LEN);
  int i;
  for(i =0; argv[i]!=NULL; i++)
  {
    if(strcmp(argv[i],"-d")==0)
    {
      debug =1;
      fprintf(stderr,"Debug mode on");
    }
    else 
      debug = 0;
  }

  while (!finished) 
  {
    char *cursor; 
    char last_char;
    int rv;
    int count;
    strcpy(lwd, pwd);
    getcwd(pwd, MAX_PATH_LEN);


    // Print the prompt and pwd
    char* pwdPrompt = malloc((strlen(pwd)+strlen(prompt)+3)*sizeof(char));
    sprintf(pwdPrompt, "[%s] %s", pwd, prompt);
    rv = write(1, pwdPrompt, strlen(pwdPrompt));
    if (!rv) 
    { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = input, last_char = 1; rv && (++count < (MAX_INPUT-1)) && (last_char != '\n'); cursor++) 
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
    write(1, input, strnlen(input, MAX_INPUT));
    if(debug)
      printf("input: %s\n", input);

    char* cmd = strtok(input, " ");
    if(debug)
      printf("RUNNING cmd: %s\n", cmd);

    //loop through the passed in parameters and store them in char** params
    char** params = malloc(MAX_PARAMS * sizeof(char*));
    char* par = strtok(NULL, " ");
    i = 0;
    while(par != NULL)
    {
      if(par != NULL && par[strlen(par)-1] == '\n')
        par[strlen(par)-1]='\0';

      if(par[0] == '$')
      {
        char* parp = malloc(strlen(par)*sizeof(char));
        memcpy(parp, (par + sizeof(char)), strlen(par));
        par = getenv(parp);
        if(par == NULL)
          sprintf(par, "$%s", parp);
      }

      params[i] = par;
      par = strtok(NULL, " ");
      i++;
    }

    int check = checkCmd(cmd, params);
    if(debug)
    {
      for(i = 0; params[i] != NULL; i++)
        printf("PARAM: %s\n", params[i]);
      printf("ENDED: %s (ret=%d)\n", cmd, check);
    }

    fflush(stdout);
  }

  return 0;
}

int checkCmd(char* cmd, char** params)
{
  int err;

  if(cmd[strlen(cmd)-1] == '\n')
    cmd[strlen(cmd)-1]='\0';

  //if cmd is exit close the program
  if(strcmp(cmd, "exit") == 0)
  {
    exit(3);
  }

  else if(strcmp(cmd, "set") == 0)
  {

    char* var = strtok(params[0], "=");
    char* varSet = strtok(NULL, "=");
    printf("VAR: %s\n", var);
    printf("varSet: %s\n", varSet);
    setenv(var, varSet, 1);
  }

  //if command is cd execute chdir()
  else if(strcmp(cmd, "cd") == 0)
  {
    if(params[0] == NULL)
      err = chdir(getenv("HOME"));

    else if(strcmp(params[0], "-") == 0)
      err = chdir(lwd);

    else if(params[0][0] == '~')
    {
      char* noTil = malloc(strlen(params[0])*sizeof(char));
      char* newPath = malloc((strlen(params[0])+strlen(getenv("HOME")))*sizeof(char));
      params[0]++;
      memcpy(noTil, (params[0] + sizeof(char)), strlen(params[0]));
      sprintf(newPath, "%s%s", getenv("HOME"), params[0]);
      err = chdir(newPath);
    }


    else
      err = chdir(params[0]);

    if(err != 0)
        perror("ERROR");

    return err;
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
    char* path = malloc(strlen(pathGLBL)*sizeof(char));
    strcpy(path, pathGLBL);
    if(debug)
      printf("Path: %s\n", path);

    char* pathI = malloc(MAX_PATH_LEN * sizeof(char));

    //tokenize path along ':' and concantenate '/' and the inputed cmd to it, then check to see if a file at this path exists
    //if it does call execute on that path
    token = strtok(path, ":");
    while(token != NULL)
    {
      sprintf(pathI, "%s/%s", token, cmd);
      if(debug)
      {
        printf("Pathi: %s\n", pathI);
        printf("Stat: %d\n", stat(pathI, &filestat));
      }

      if(stat(pathI, &filestat) == 0)
      {
        return execute(pathI, params);
        break;
      }
      token = strtok(NULL, ":");
      pathI[0]='\0';
    }

    sprintf(pathI, "%s/%s", pwd, cmd);
    if(stat(pathI, &filestat) == 0)
      return execute(pathI, params);
  }
}

int execute(char* path, char** params)
{
  int status;
  int pid = fork();
  if(pid == 0)
  {
    int i;
    char* toRun[MAX_PARAMS];
    toRun[0] = path;
    for(i = 0; params[i] != NULL; i++)
    {
      toRun[i+1]=params[i];
    }
    status = execv(path, toRun);
  }

  else
  {
    while(1)
    {
      if(wait(&status >= 0))
        break;
    }
    return status;
  }
}




//fflush(stdout);
