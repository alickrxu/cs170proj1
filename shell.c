/*Brian Phan, Alick Xu*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>

/*
 	Invalid commands:
	1. two or more <
	2. two ore more >
	3. a | occurring before a <
	4. a | occurring after a >
	5. & not occuring at the end of a command 
*/

/* Used whenever ctrl-c is read to print out a new line and flush the output */
void handle_signal(int sig_num)
{
	printf("\nsish:> ");
	fflush(stdout);
}

/* First, delimit the string by pipe characters | */
int parse_by(char *myInputString, char **tokens, const char delim[])
{
	int numTokens = 0;
	char *mystring = myInputString; //do not want to modify original input string
		
	char *temp;
	temp = strtok(mystring, delim);

	while(temp != NULL)
	{
		*tokens = temp;
		numTokens++;
		tokens++;
		temp = strtok(NULL, delim);
	}
	*tokens = '\0';
	return numTokens;
}

/* After parsing by pipe characters, delimit the string by whitespace */
int  parse_token(char *myInputString, char **tokens)
{
  	int numTokens=0;
  	char *mystring = myInputString; //do not want to modify original input string

  	while (*mystring != '\0') //check if end of string
    	{
      		*tokens = mystring;
      		numTokens++;
      		//read in a word until whitespace or <>| is reached 
		//(don't need to check for & since we know it will be at the end)
      		while (*mystring != '\0' && *mystring != ' ' && *mystring != '\n' && *mystring != '\t') 
		{
	  		mystring++;
		}
		// after white space is reached, increment to next word (if any)
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


int main (int argc, char *argv[])
{
  	//argc is the count of total command line arguments (including the executable name
  	//argv is the array of character strings of each command line argument passed upon execution

  	int child_pid;
  	char *mystring = (char*) malloc (1024+1); //string which stores user input (ex. ls -l | cat)
  	char **tokens = (char**) malloc (1024+1); //tokens divided by pipes (ex. {ls -l} {cat})
	char **words = (char**) malloc (1024+1); //tokens divided by whitespace (ex. {ls} {-l} {cat})
  	int numExecutions;
  	int i; //current index of mystring, also has the length of mystring 
  	int alnum;
	int numPipeArgs; //number of tokens divided by pipes

  
	//store indices of | < > &
	int *pipeLocs;
	int pipeIndex;
	int inputLoc;
	int outputLoc;
	int bgLoc;

  	signal (SIGINT, SIG_IGN); //catches ctrl-c and...
  	signal (SIGINT, handle_signal); //...ignores it, prints out a new prompt

  	//while loop where shell is run - essentially keeps running until prompted to exit.
  	while (1) 
    	{
		//reset pipeIndex, inputLoc, outputLoc, pipeLocs
		pipeLocs = NULL;
		inputLoc = -1;
		outputLoc = -1;
		bgLoc = -1;
		pipeIndex = 0;

      		printf ("sish:> ");

      		if (fgets(mystring, 1024, stdin) == NULL) //exits on ctrl-d (EOF)
		{
	  		break;
		}
		
      		i = 0; //reset i
      		alnum = 1; //1 = all chars are alnum, 0 = some character not alnum


		/*
		*
		*
		* START INPUT ERROR CHECK
		*
		*/

      		while (mystring[i] !='\n')
		{
	  		if (isalnum(mystring[i])==0) //not alphanumeric
          		{
	    			//ignore non alphanumeric characters
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
		mystring[i] = '\0';

		
	

      		if (alnum == 0) //invalid command
			continue;

		//find out where the next locations of |, <, >, & are, if any
		char * inp = strchr(mystring, '<');
		char * out = strchr(mystring, '>');
		char * pip = strchr(mystring, '|');
		char * amp = strchr(mystring, '&');

		//if there is a & anywhere besides the end, this command is invalid
		if(amp != NULL)
		{
			bgLoc = (int)(amp-mystring);
			if(bgLoc < i-1)
			{
				bgLoc = -1;
				printf("ERROR: & must be at end of command\n");			
				continue;
			}
		}

		//if there is more than one <, this commmand is invalid
		if(inp != NULL)
		{
			inputLoc = (int)(inp-mystring);
			inp = strchr(inp+1, '<');
			if (inp != NULL)
			{
				inputLoc = -1;
				printf("ERROR: Can only have one <\n");
				continue;
			}
		}
	
		//if there is more than one >, this command is invalid
		if(out != NULL)
		{
			outputLoc = (int)(out-mystring);
			out = strchr(out+1, '>');
			if (out != NULL)
			{
				outputLoc = -1;
				printf("ERROR: Can only have one >\n");
				continue;
			}
		}

		//if there are two pipes next to each other (||) with no alphanumeric characters in between, this command is invalid
		if(pip != NULL) //initialize pipeLocs
			pipeLocs = (int*) malloc(1024+1);
		
		int invalid = 0;
		while(pip != NULL)
		{

			pipeLocs[pipeIndex] = (int)(pip-mystring); //store this pipelocation
			//printf("pipeLocs %i: %i\n", pipeIndex, (int)(pip-mystring));
			pipeIndex++;

			//check if the next character is a pipe
			char* temp = pip;
			int tempNum = (int)(temp-mystring);
			pip = strchr(pip+1, '|');

			if(pip != NULL)
			{
						
				for(tempNum; tempNum < (int)(pip-mystring); tempNum++)
				{
					if (isalnum(mystring[(int)(temp-mystring)]) == 0) //no alphanumeric characters in between pipes
					{
						invalid = 1;
						break;
					}
					else if (isalnum(mystring[(int)(temp-mystring)]) != 0) //at least one alphanumeric character in between pipe
					{
						break;
					}
					else if (isblank(mystring[(int)(temp-mystring)]) == 0)
						continue;
					
					//if whitespace, continue
				}

				if(invalid == 1)
					break; 
			}
		}
	
		if(invalid == 1)
		{
			printf("ERROR: Pipes must have commands in between them\n");
			continue;
		}

		//if there is a | before <, or a | after >, invalid command
		if(pipeLocs != NULL)
		{
			
			if(pipeLocs[0] < inputLoc && inputLoc != -1)
			{
				printf("ERROR: Cannot have pipes before input redirection\n");
				continue;
			}
			else if (pipeLocs[pipeIndex-1] > outputLoc && outputLoc != -1)
			{
				printf("ERROR: Cannot have pipes after output redirection\n");
				continue;
			}
		}
		
		/*
		*
		*
		* END INPUT ERROR CHECK
		*
		*/

		//parse the string - first separate the arguments by pipes 
		printf("parse pipe\n");
		const char pipeDelim[1] = "|";
		const char whitespaceDelim[3] = " \n\t";
		numPipeArgs = parse_by(mystring, tokens, pipeDelim);

		//call execute on each of tokens[0], tokens[1], etc.

		
		printf("inputLoc: %i\n", inputLoc);
		printf("outputLoc: %i\n", outputLoc);
		printf("pipeLocs: \n");
		for(int x = 0; x < pipeIndex; x++)
		{
			printf("%i: %i\n", x, pipeLocs[x]);
		}
		if(invalid == 1)
		{
			printf("invalid =1\n");
			continue;
		}

		printf("numpipeargs: %i \n", numPipeArgs);
      		printf ("mystring: %s\n", mystring);
      		printf ("tokens are: \n");

      		int j = 0;      
      		while(tokens[j] != NULL)
		{
      			printf ("%s \n", tokens[j]);
			j++;
      		}


		printf("\n");
      
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
			int myPipe[numPipeArgs*2];
	
			if (pipe (myPipe))
			{
				printf("ERROR: Pipeing failed\n");
				continue;
			}

			//check if this command runs in background
			int isBG = 0;
			if(bgLoc != -1)
				isBG = 1;

			int executeError = 0;
			int numPipesRan = 1;
			int pipeInputIndex = 0;
			int cpipe;
			int status; //for waitpid
			pid_t child_pid;
			FILE *fp;
		/*******************************************************
		*
		*
		* EXECUTE COMMAND
		*
		*
		********************************************************/
			//if there are no pipes
			if (numPipeArgs == 1)
			{
				printf("no pipes\n");
				//close all pipes
				for(cpipe = 0; cpipe < numPipeArgs*2; cpipe++)
				{
					close(myPipe[cpipe]);
				}
				
				child_pid = fork();

				if(child_pid < 0)
				{
					printf("ERROR: forking child process failed\n");
					continue;
				}
				else if(child_pid == 0) //child
				{	
					if (execvp(*tokens, tokens) < 0)  /* execute the command  */
					{    
			      			printf("ERROR: %s is an invalid command \n", *tokens);
			      			continue; 
			    		}
				}
				else //parent
				{
					if(isBG == 0)
						waitpid(child_pid, &status, 0);
					else
						;
				}
				continue;
			}
			
			//if pipes exist
			for(int i = 0; i < numPipeArgs; i++) 
			{
				/* separate next command (delimited by pipe) by whitespace */ 
				parse_by(tokens[i], words, whitespaceDelim);

				printf("parse_by tokens: \n");
				int z = 0;
				while(words[z] != NULL)
				{
					printf("%s \n", words[z]);	
					z++;
				}
						
				//first command is beginning of pipe
				int isFirst = 0;
				if(i == 0)
					isFirst = 1;
				//last command is end of pipe
				int isLast = 0;
				if((i+1) == numPipeArgs)
					isLast = 1;

  				if ((child_pid = fork()) < 0)  /* fork a child process           */
  				{    
    					printf("ERROR: forking child process failed\n");
    					exit(1);
  				}

  				else if (child_pid == 0)   /* for the child process:         */
			  	{
					if(isFirst == 1) //first process in pipe
					{
						//only replace standard output with output part of pipe
						dup2(myPipe[1], 1);
						//close all pipes
						for(cpipe = 0; cpipe < numPipeArgs*2; cpipe++)
						{
							close(myPipe[cpipe]);
						}

						//execute command
						if (execvp(*words, words) < 0)  /* execute the command  */
						{    
			      				printf("ERROR: %s is an invalid command \n", *tokens);
			      				executeError = 1;
							break; 
			    			}
						numPipesRan++;
					}
					else if(isLast == 1) //last process in pipe
					{
						//only replace standard input with input part of pipe
						dup2(myPipe[i-1], 0);
						//close all pipes
						for(cpipe = 0; cpipe < numPipeArgs*2; cpipe++)
						{
							close(myPipe[cpipe]);
						}

						if (execvp(*words, words) < 0)  /* execute the command  */
						{    
				      			printf("ERROR: %s is an invalid command \n", *tokens);
				      			executeError = 1;
							break;
				    		}
					}
					else //some in-between pipe
					{
						//if more than 2 pipes have been ran, increment pipeInputIndex by 2:
						if(numPipesRan > 2)
							pipeInputIndex += 2;

						//replace standard input and output of pipe
						dup2(myPipe[pipeInputIndex+3], 1);
						dup2(myPipe[pipeInputIndex], 0);

						//close all pipes
						for(cpipe = 0; cpipe < numPipeArgs*2; cpipe++)
						{
							close(myPipe[cpipe]);
						}

						if (execvp(*words, words) < 0)  /* execute the command  */
						{    
				      			printf("ERROR: %s is an invalid command \n", *tokens);
				      			executeError = 1;
							break;
				    		}

						numPipesRan++;
						
			 		}
			  	}
			  	else  /* for the parent process:      */
			  	{                                 
					if (isBG == 1)
						; //background, don't wait for child
	
					else
						waitpid(child_pid, &status, 0); //wait for last child to finish
			  	}				
				//numPipesRan++;
	  			//execute (words, isBG, isFirst, isLast); //otherwise, execute command
			} //end of for
			if(executeError == 1)
				continue;
			
		} //end of else

    	} //end of while

  	//printf("exited while\n");
  	free (mystring);
  	free (tokens);
	free (pipeLocs);
	free (words);
  	exit(0);
}
