/*Brian Phan, Alick Xu*/

#include <stdio.h>
#include <errno.h>
#include <stdlib>

int main (int argc, char *argv[], char **envp)
{
  //argc is the count of total command line arguments (including the executable name
  //argv is the array of character strings of each command line argument passed upon execution
  //envp is an array of character strings that has the system environment variables

  /* code to open and read a file */
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

char *prog, **args;
int child_pid;


// Read and parse the input a line at atime
while (readAndParseCmdLine(&prog, &args)) {
child_pid= fork();      // create a childprocess
if (child_pid== 0) {
exec(prog,args);       // I'm the child process.Run program
// NOT REACHED
} else {
wait(child_pid);       // I'm the parent, wait for child
return 0;
}
}
