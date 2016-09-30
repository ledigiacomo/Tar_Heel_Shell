/**
  Names: Luke DiGiacomo, Marcus Wallace
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
  #include <fcntl.h>


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
int isredirect =0;
 int isdirect =0;
char* filename;
struct stat filestat;
int i;
int goBack = 0;

int execute(char* path, char** params);
int checkCmd(char* cmd, char** params);
void printHeel();

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

    //make input pretty by putting whitespace betweeen all special characters
    char* pretty = malloc(strlen(input)*sizeof(char));
    char* inputPt = malloc(strlen(input)*sizeof(char));
    strcpy(inputPt, input);

    while(inputPt[0] != '\n')
    {
      if(inputPt[0] == '<' || inputPt[0] == '>' || inputPt[0] == '|' || inputPt[0] == '&' || inputPt[0] == '\n')
      {
        fflush(stdout);
        char* tempOut = malloc((strlen(pretty)+4)*sizeof(char));
        sprintf(tempOut, "%s %c ", pretty, inputPt[0]);
        pretty = realloc(tempOut, strlen(tempOut)*sizeof(char));
      }

      else
      {
        char* tempOut = malloc((strlen(pretty)+2)*sizeof(char));
        sprintf(tempOut, "%s%c", pretty, inputPt[0]);
        pretty = realloc(tempOut, strlen(tempOut)*sizeof(char));
      }
      inputPt++;
    }

    if(debug)
      printf("Pretty: %s", pretty);

    //skip over lines with # (Comments)
    if(input[0] == '#')
      continue;

    char* cmd = strtok(pretty, " ");
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

      else if(strstr(par, "&") != NULL)
        goBack = 1;

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
    if(debug)
    {
      printf("VAR: %s\n", var);
      printf("varSet: %s\n", varSet);
    }
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

  else if(strcmp(cmd, "goheels") == 0)
  {
    printHeel();
    return 0;
  }

  //not search path
  else if(cmd[0] == '\\')
  {
    if(stat(cmd, &filestat) == 0)
      {
        return execute(cmd, params);
      }
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

    else 
      perror("ERROR");
  }
}

int execute(char* path, char** params)
{
  int status;
  int pid = fork();
  int out;
  if(pid == 0)
  {
        if(isredirect && stat(filename,&filestat) ==-1)
   {
    int out=open(filename,O_WRONLY| O_CREAT, 0755); 
      dup2(out,1); 
      dup2(1,out);
      close(out);         
  }
  if(isdirect && stat(filename,&filestat) == 0)
  {
    int in =open(filename,O_RDONLY, 0755); 
     dup2(in,0); 
      dup2(0,in);
      close(in);  
  }

    int i;
    char* toRun[MAX_PARAMS];
    toRun[0] = path;
    for(i = 0; params[i] != NULL; i++)
    {
      toRun[i+1]=params[i];
      printf("arg %d: %s\n", i+1, params[i]);
    }
    //printf("cmd is %s", path);
      //   if(isredirect) close(out);
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

void printHeel()
{
  char* toHeel = malloc(MAX_PATH_LEN*sizeof(char));
  sprintf(toHeel, "%s/heel.txt", getenv("HOME"));
  FILE* filePath = fopen(toHeel, "r");

  if (filePath == NULL)
    printf("Cannot open file \n");

  else
  {
    char c = fgetc(filePath);
    while (c != EOF)
    {
      printf ("%c", c);
      fflush(stdout);
      c = fgetc(filePath);
    }
  }
 
  fclose(filePath);                                                                                                                                                                                                                                                                          
}

