// code fore lab ///
// Konrad Kurzynowski kkurzy4_hw3

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXLINE 128
#define MAXARGS 128

// Function prototypes

void sigint_handler (int sig);
void sigtstp_handler (int sig);
void parseline(char*buf, char **argv);
void eval(char* cmdline);
int builtin_command(char **argv);
pid_t Fork(void);
void unix_error(char *msg);


int main()
{

  char cmdline[MAXLINE];

  while (1)
  {
    //check for SIGINT
    if(signal(SIGINT,sigint_handler) == SIG_ERR){
      printf("CS361> ");
    }

    // check for SIGTSTP
    if(signal(SIGTSTP,sigtstp_handler) == SIG_ERR){
      printf("CS361> ");
    }

    // Read input from user
    printf("Konrad Kurzynowski kkurzy4\n");
    printf("CS361> ");
    fgets (cmdline, MAXLINE, stdin); 
    printf("input : %s\n", cmdline);

    // Evaluate shell
    eval(cmdline);

  }  
 
}

// function parses line into arguments
void parseline(char*buf, char **argv)
{
  char *delim;       // Points to first space delimiter
  int argc;          // argument counter

  buf[strlen(buf)-1] = ' ';   // put space at end for the last command to be read
  while (*buf && (*buf == ' ')) // Ignore leading white spaces
    buf++;

  // Build argument array
  argc = 0;
  while ((delim = strchr(buf, ' ')))
  {
    argv[argc++] = buf;    // parse cmdline into array and increment argument count
    *delim = '\0';         // strings end with th sentinel \0
    buf = delim +1;        // buffer advances to character after white space
    while (*buf && (*buf == ' '))   // ignore white spaces
      buf++;
  }

  argv[argc] = NULL;

  if (argc == 0)   // Ignore blank line
    return;
}

// evaluate cmdline
void eval(char* cmdline)
{
  char* argv[MAXARGS];  // Argument list for execve()
  char buf[MAXLINE];    // Holds modified command line
  pid_t pid;		// Process id
  int status, fd0, fd1, fd2, i, in = 0, out = 0;
  char input[64], output[64];

  strcpy(buf, cmdline);
  parseline(buf, argv);
  if (argv[0] == NULL)
    return;

  if (!builtin_command(argv))
  {
    if ((pid = Fork()) == 0)
    {
      if (execvp(argv[0], argv) < 0)
      {
	for(i=0;argv[i]!='\0';i++)
        {
          if(strcmp(argv[i],"<")==0)      // check for input redirection
	  {
            //argv[i]=NULL;
            strcpy(input,argv[i+1]);
            in=1;           
          }               
          if(strcmp(argv[i],">")==0)      // check for output redirection
          {      
            //argv[i]=NULL;
            strcpy(output,argv[i+1]);
            out=1;
          }
	  if(strcmp(argv[i],">>")==0)      // check for output redirection
          {
            //argv[i]=NULL;
            strcpy(output,argv[i+1]);
            out=2;
          }         
        }
	if (in)
	{
	  if ((fd0 = open(input, O_RDONLY, 0)) < 0) 
	  {
            perror("Couldn't open input file");
            exit(0);
          }           
          // dup2() copies content of fd0 in input of preceeding file
          dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 
          close(fd0); // necessary
	}
	if (out)
    	{
	  if (out == 2)
	  {
	    if ((fd2 = open(output, O_RDWR|O_APPEND, 0)) < 0)
            {
            perror("Couldn't open output file");
            exit(0);
            }
	    dup2(fd2, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
            close(fd2);
	  }
	  else
	  {
            if ((fd1 = creat(output , 0644)) < 0) 
	    {
              perror("Couldn't open the output file");
              exit(0);
            }
	    dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
            close(fd1);           
    	  }
        } 
      } 
    }
    else
    {
      while (wait(&status) != pid);    // wait for completion
      printf("Exit: 0\n");
    }
    printf("PID: %d \n", pid);
  }
  return;
}

// if first arg is a builtin command run it and return true
int builtin_command(char **argv)
{
  
  if (!strcmp(argv[0], "exit")) // exit shell
  {
    printf("Exiting shell...\n");
    exit(0);
  }
  if (!strcmp(argv[0], "&" ))  // ignore singleton
    return 1;
  return 0;                    // not a builtin command 
}

void sigint_handler (int sig) // SIGINT handler
{
  printf("Caught SIGINT!\n");
  fflush(stdout);
  printf("CS361> ");
  fflush(stdout);
}

void sigtstp_handler (int sig) // SIGTSTP handler
{
  printf("Caught SIGTSTP!\n");
  fflush(stdout);
  printf("CS361> ");
  fflush(stdout);
}

pid_t Fork(void) // wrapper function for fork
{
  pid_t pid;

  if ((pid = fork()) < 0 )
    unix_error("Fork error");
  return pid;
}

void unix_error(char *msg)  // unix-style error msg
{
  fprintf(stderr,"%s: %s\n", msg, strerror(errno));
  exit(0);
}


