/*Brian Phan, Alick Xu*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>

/*
  meaning of mode:
	0 = normal
	1 = &
	2 = <
	3 = >
	4 = |
*/
//parse tokens from mystring
//return an array of individual words
int parse_token(char *mystring, char **tokens, int *mode)
{
  int numTokens=0;
  
  int isInput = 0;

  mode = 0; //initially, in normal mode
  while (*mystring != '\0') //check if end of string
    {
      *tokens = mystring;
      numTokens++;
      while (*mystring != '\0' && *mystring != ' ' && *mystring != '\n' && *mystring != '\t') 
	{
	  //switch statement for dealing with i/o redirection, piping, and &
	  switch(*mystring)
	  {
		case '&':
			*mode = 1;
			break;
		case '<':
			*mode = 2;
			if(isInput ==1) //already read in a <
			{
				//not a valid command
				//set tokens to empty
			}
			else
				isInput = 1;

			break;
		case '>':
			*mode = 3;
			break;
		case '|':
			*mode = 4;
			break;
	  }
	  mystring++;
	}
      while (*mystring == ' ' || *mystring == '\n' || *mystring == '\t')
	{
	  *mystring='\0';
	  mystring++;
	}
      tokens++;
    }
  //end of string
  *tokens='\0';
  return numTokens;
}



// Read the parsed string and execute the command
int  execute(char **tokens)
{
  pid_t  child_pid;
  int    status;
  if ((child_pid = fork()) < 0) {     /* fork a child process           */
    printf("ERROR: forking child process failed\n");
    exit(1);
  }
  else if (child_pid == 0) {          /* for the child process:         */
    if (execvp(*tokens, tokens) < 0) {     /* execute the command  */
      printf("ERROR: %s is an invalid command \n", tokens[0]);
      exit(1);
    }
  }
  else {                                  /* for the parent:      */
    //while (wait(&status) != child_pid);      /* wait for completion  */
    waitpid(child_pid, &status, 0);
    return 0;
  }
}


int main (int argc, char *argv[])
{
  //argc is the count of total command line arguments (including the executable name
  //argv is the array of character strings of each command line argument passed upon execution
  //envp is an array of character strings that has the system environment variables
  
  /* code to open and read a file 
  FILE * pFile;
  char mystring[1024];
  pFile = fopen (argv[0], "r");
  if (pFile == NULL) perror ("Error opening file");
  else
    {
      if (fgets(mystring, 1024, pFile) != NULL)
	{puts (mystring);}
      fclose(pFile);
    }
  */

  int child_pid;
  char mystring[1024];
  char *tokens [100];
  int numTokens;
  int i;
  int alnum;
  int mode;
  while (1)
    {
      printf ("sish:> ");
      if (fgets(mystring, 1024, stdin) == NULL)
	{
	  break;
	}
      i = 0;
      alnum = 1; //1 = all chars are alnum, 0 = some character not alnum
      while (mystring[i] !='\n')
	{
	  if (isalnum(mystring[i])==0)
          {
	    if (mystring[i] == '|' || mystring[i] == '"' || mystring[i] == '<' || mystring[i] == '>' || mystring[i] == '&' || mystring[i] == '\''  || mystring[i] == ' ' || mystring[i] == '.' || mystring[i]== ',' || mystring[i]== '/' || mystring[i]== '\\' || mystring[i] == '~' || mystring[i] == '!' || mystring[i] == '@' || mystring[i] == '#' || mystring[i] == '$' || mystring[i] == '%' || mystring[i] == '^' || mystring[i] == '*' || mystring[i] == '(' || mystring[i] == ')' || mystring[i] == '-' || mystring[i] == '_' || mystring[i] == '+' || mystring[i] == '=' || mystring[i] == ';' || mystring[i] == ':' || mystring[i] == '{' || mystring[i] == '}' || mystring[i] == '[' || mystring[i] == ']' || mystring[i] == '?' || mystring[i] == '`' ||mystring[i] == '\t')
		{
		  i++;
		  continue;
		}
	      else
		{
		  alnum = 0;
		}
	  }
	  i++;
	}	  
      if (alnum == 0)
	continue;
      numTokens = parse_token(mystring, tokens);
      printf ("mystring: %s\n", mystring);
      printf ("tokens are: ");
      
      
      printf ("%s \n", tokens[0]);
      
      
      
      if (strcmp(tokens[0], "exit") == 0 ) //check if command is an exit
	break;
      else if (strcmp (tokens[0], "\0") == 0) //if no input (pressing enter)
	continue;
      else if (strcmp (tokens[0], "cd") == 0 ) //change directory
	{
	  chdir(tokens[1]);
	}
      else
	{
	  execute (tokens); //otherwise, execute command
	}
    } //end of while
  printf("exited while\n");
  exit(0);
}
