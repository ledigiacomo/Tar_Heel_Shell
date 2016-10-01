/**
  Names: Luke DiGiacomo, Marcus Wallace

  Honor Pledge:  I certify that no unauthorized assistance has been received or given in the completion of this work
  Signature: Luke DiGiacomo, Marcus Wallace
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
int debug = 0;
int isredirect =0;
char* outputArr[5];
int isdirect =0;
int isPipe =0;
char* outputFile;
char* inputFile;
struct stat filestat;
int i;
int goBack = 0;
int inter = 1;

int execute(char* path, char** params);
int checkCmd(char* cmd, char** params);
void printHeel();

int main(int argc, char ** argv, char **envp) 
{
  setenv("?", "0", 1);
  int finished = 0;
  char *prompt = "thsh> ";
  char input[MAX_INPUT];
  pwd = malloc(MAX_PATH_LEN);
  lwd = malloc(MAX_PATH_LEN);
  int i;
  for(i =1; argv[i]!=NULL; i++)
  {
    if(strcmp(argv[i],"-d")==0)
    {
      debug =1;
      fprintf(stderr,"Debug mode on");
    }

    if(debug)
      fprintf(stderr, "arg[i]: %s\n", argv[i]);

    if(stat(argv[i], &filestat) == 0)
    {
      int in = open(argv[i], O_RDONLY);
      close(0);
      dup2(in, 0);
      inter = 0;
    }
  }
  
  //is finished when CMD-C is read from input
  while (!finished) 
  {
    char *cursor; 
    char last_char;
    int rv;
    int count;
    strcpy(lwd, pwd);
    getcwd(pwd, MAX_PATH_LEN);


    // Print the prompt and pwd
    if(inter)
    {
      char* pwdPrompt = malloc((strlen(pwd)+strlen(prompt)+3)*sizeof(char));
      sprintf(pwdPrompt, "[%s] %s", pwd, prompt);
      rv = write(1, pwdPrompt, strlen(pwdPrompt));
    }

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

    //make input "pretty" by putting whitespace betweeen all special characters
    char* pretty = malloc(strlen(input)*sizeof(char));
    char* inputPt = malloc(strlen(input)*sizeof(char));
    strcpy(inputPt, input);

    while(inputPt[0] != '\n')
    {
      if(inputPt[0] == '<' || inputPt[0] == '>' || inputPt[0] == '|' || inputPt[0] == '&' || inputPt[0] == '\n')
      {
        char* tempOut = malloc((strlen(pretty)+4)*sizeof(char));
        sprintf(tempOut, "%s %c ", pretty, inputPt[0]);
        pretty = realloc(tempOut, strlen(tempOut)*sizeof(char));
      }

      //if a tilde is found replace it with $HOME
      else if(inputPt[0] == '~')
      {
        char* tempOut = malloc((strlen(pretty)+strlen(getenv("HOME")))*sizeof(char));
        sprintf(tempOut, "%s%s", pretty, getenv("HOME"));
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
      fprintf(stderr, "Pretty: %s", pretty);

    //skip over lines with # (Comments)
    if(input[0] == '#')
      continue;

    char* cmd = strtok(pretty, " ");
    if(debug)
      fprintf(stderr, "RUNNING cmd: %s\n", cmd);

    //loop through the passed in parameters and store them in char** params
    char** params = malloc(MAX_PARAMS * sizeof(char*));
    char* par = strtok(NULL, " ");
    i = 0;
    while(par != NULL)
    {
      if(par != NULL && par[strlen(par)-1] == '\n')
        par[strlen(par)-1]='\0';

      //if param starts with $ change the param to the value of the variable
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
      if(strcmp(params[i], "|"))
        isPipe =1;

      par = strtok(NULL, " ");
      i++;
    }

    //check the command to see if it is built it then run the command with the given parameters
    int check = checkCmd(cmd, params);
    char* ques = malloc(15);
    sprintf(ques, "%d", check);
    setenv("?", ques, 1);
    if(debug)
    {
      for(i = 0; params[i] != NULL; i++)
        fprintf(stderr, "PARAM: %s\n", params[i]);
      fprintf(stderr, "ENDED: %s (ret=%d)\n", cmd, check);
    }

    fflush(stdout);
  }

  return 0;
}

//check the command to see if it is built it then run the command with the given parameters
//will return the return value of exec or a similar value for built-in commands
int checkCmd(char* cmd, char** params)
{
  int err;
  int count;
  //loop through input and look for a <
  for(count = 0; params[count] != NULL; count++)
  {
    if(strcmp(params[count], "<") == 0)
    {
      inputFile = params[count+1];
      
      if(params[count+2]==NULL)
      {
        isdirect =1;
       
        params[0] ='\0';
        params[1] = '\0';
      }
  
      else
      {
        int i;
        inputFile = params[count+1];
        for(i=count; params[i+2] != NULL; i++)
          strcpy(params[i], params[i+2]);

        strcpy(params[i], params[i+1]);
        params[i+1] = '\0';
      }
    }
  }

  for(count = 0; params[count] != NULL; count++)
  {
    if(strcmp(params[count],">") == 0)
    {
      isredirect=1;
      outputFile = params[count+1];
      params[count]='\0';
      params[count+1]= '\0';
    }
  }

  if(cmd[strlen(cmd)-1] == '\n')
    cmd[strlen(cmd)-1]='\0';

  //if cmd is exit close the program
  if(strcmp(cmd, "exit") == 0)
  {
    exit(3);
  }
  
  //if the command is set, set the next param to be the value of the next param
  else if(strcmp(cmd, "set") == 0)
  {
    char* var = strtok(params[0], "=");
    char* varSet = strtok(NULL, "=");
    if(debug)
    {
      fprintf(stderr, "VAR: %s\n", var);
      fprintf(stderr, "varSet: %s\n", varSet);
    }
    setenv(var, varSet, 1);
  }

  //if command is cd execute chdir() after looking for special cases: '' and '-'
  else if(strcmp(cmd, "cd") == 0)
  {
    if(params[0] == NULL)
      err = chdir(getenv("HOME"));

    else if(strcmp(params[0], "-") == 0)
      err = chdir(lwd);

    else
      err = chdir(params[0]);

    if(err != 0)
        perror("ERROR");

    return err;
  }

  //if command is goheels call printheel to print our ascii art
  else if(strcmp(cmd, "goheels") == 0)
  {
    printHeel();
    return 0;
  }

  //if the command begins with a '\' look for the value at the absolute address
  //if found execute it, otherwise, return -1 
  else if(cmd[0] == '\\')
  {
    if(stat(cmd, &filestat) == 0)
    {
      return execute(cmd, params);
    }

    else 
      return -1;
  }

  //search PATH for the command and execute it
  else 
  {
    char* path = malloc(strlen(pathGLBL)*sizeof(char));
    strcpy(path, pathGLBL);
    if(debug)
      fprintf(stderr, "Path: %s\n", path);

    char* pathI = malloc(MAX_PATH_LEN * sizeof(char));

    //tokenize path along ':' and concantenate '/' and the inputed cmd to it, then check to see if a file at this path exists
    //if it does call execute on that path
    token = strtok(path, ":");
    while(token != NULL)
    {
      sprintf(pathI, "%s/%s", token, cmd);
      if(debug)
      {
        fprintf(stderr, "Pathi: %s\n", pathI);
        fprintf(stderr, "Stat: %d\n", stat(pathI, &filestat));
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
    if(isredirect && stat(outputFile,&filestat) ==-1)
    {
      int out=open(outputFile,O_WRONLY| O_CREAT, 0755); 
      dup2(out,1); 
      dup2(1,out);
      close(out);         
    }
    if(isdirect && stat(inputFile,&filestat) == 0)
    {
      int in = open(inputFile,O_RDONLY, 0755); 
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
      if(debug)
        fprintf(stderr, "arg %d: %s\n", i+1, params[i]);
    }
    //printf("cmd is %s", path);
      //   if(isredirect) close(out);
    status = execvp(path, toRun);
    
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

//print the ascii art located at the specified location
//in this case the location is heel.txt in $HOME
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

