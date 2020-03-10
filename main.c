#include "parse.h"


void trimWhiteSpace(char * process)
{
	//Helper function that removes the leading space that a string process may have
	int index;
	int begin = 0;
	int end = strlen(process) - 1;

	while (isspace((unsigned char) process[begin]))
		begin++;

	while ((end >= begin) && isspace((unsigned char) process[end]))
		end--;

	//Shift all chars to the beginning of the string array	
	for (index = begin; index <= end; index++)
		process[index - begin] = process[index];

	process[index - begin] = '\0';
}

void argsForRedirection(struct commandLine * currInput, char * arguments[])
{
	//Generates the arguments for redirection. It makes sure to skip < > 
	int index = 1;		//Will be the index of the arguments[]
	int i = 1;
	for ( ; currInput->commands[i] != NULL; i++)
	{
		if (strcmp(currInput->commands[i], "<") == 0)
			continue;
		if (strcmp(currInput->commands[i], ">") == 0)
			break;
		if (strcmp(currInput->commands[i], ">>") == 0)
			break;

		arguments[index++] = currInput->commands[i];
	}
	arguments[index] = NULL;
	return;
}

void tokenizeProcesses(char * args[], char * process)
{
	//Tokenize the string representation of a process by " ", so sort -k 5nr ==> 'sort' '-k' '5nr'
	char copyProcess[512];
	strcpy(copyProcess, process);
	int index = 0;
	char * token;
	const char delim[2] = " ";

	token = strtok(copyProcess, delim);
	args[index++] = token;

	printf("token: %s\n", token);
	while (token != NULL)
	{
		token = strtok(NULL, delim);
		args[index++] = token;
		printf("token: %s\n", token);
	}
	args[index] = NULL;
	//memset(copyProcess, 0, 512);
}

void executeArgsPiped(char * proc1[], char * proc2[])
{
	//Executes the two commands	
	int fd[2];
	pid_t p1;

	if (pipe(fd) < 0)
	{
		printf("ERROR: pipe failed\n");
		return;
	}
	p1 = fork();
	if (p1 < 0)
	{
		printf("ERROR: fork failed\n");
		return;
	}
	else if (p1 == 0)
	{
		//child process
		close(fd[0]);
		if (dup2(fd[1], 1) < 0)
		{
			printf("ERROR: dup2 failed\n");
			return;
		}
		close(fd[1]);
		//printf("About to execute the first process\n");
		execvp(proc1[0], proc1);
		printf("ERROR: program %s not found\n", proc1[0]);
		exit(1);
	}

	else
	{
		//parent
		close(fd[1]);
		if (dup2(fd[0], 0) < 0)
		{
			printf("ERROR: dup2 failed\n");
			return;
		}
		close(fd[0]);
		//printf("About to exexecute the seocnd process\n");
		execvp(proc2[0], proc2);
		printf("ERROR: program %s not found\n", proc2[0]);
		exit(1);
	}
	

}

int executeCommands(struct commandLine * currInput)
{
	//This function will handles the input line and execute the commands. Supports one pipe at most
	switch(currInput->numPipes)
	{
		pid_t pid;
		case 1:				//Only one pipe 
			pid = fork();	//Create child process
			if (pid < 0)	
			{
				printf("ERROR: fork failed\n");
				exit(1);
			}
			else if (pid == 0)	//Child process
			{
				if (currInput->inputFile[1])
				{
					//Encountered an '<'
					int fd;
					if ( (fd = open(currInput->inputFile[1], O_RDONLY)) < 0 )
					{
						perror("File does not exist");
						exit(1);
					}
					if (dup2(fd, 0) < 0)
					{
						printf("ERROR: dup2 failed\n");
						exit(1);
					}
					close(fd);
				}
				if (currInput->outputFile[1])
				{
					//Encountered an '>'
					int fd2;
					if (strcmp(currInput->outputFile[0], ">>") == 0)
					{
						fd2 = open(currInput->outputFile[1], O_CREAT | O_WRONLY | O_APPEND, 0666);
						if (fd2 < 0)
						{
							perror("open()");
							exit(1);
						}
					}
					else if ( (fd2 = creat(currInput->outputFile[1], 0644)) < 0 )
					{
						printf("ERROR: creat failed\n");
						exit(1);
					}
					if (dup2(fd2, 1) < 0)
					{
						printf("ERROR: dup2 failed\n");
						exit(1);
					}
					close(fd2);
				}

				if (currInput->inputFile[1] || currInput->outputFile[1])
				{
					//We saw '<' or '>' or both
					char * arguments[512];
					arguments[0] = currInput->commands[0];
					argsForRedirection(currInput, arguments);
				
					execvp(currInput->commands[0], arguments);
					printf("-nsh: %s: command not found\n");
					exit(1);
				}
				else
				{
					//No redirection (i.e. '<' or '>' not found)
					execvp(currInput->commands[0], currInput->commands);
					printf("-nsh: %s: command not found\n", currInput->commands[0]);
					exit(1);
				}

			}
			else
			{
				wait(NULL);
			}
			break;	
		case 2: ;
			//This case handles when there's one pipe on the command line (i.e. command1 | command2)	
			
			char * leftProcess = currInput->processes[0];				//Contains the string that represents the left process
			char * rightProcess = currInput->processes[1];				//Contains the string that represents the right process
			
			char * proc1[512];											//Parsed tokens of leftProcess
			char * proc2[512];											//Parsed tokens of rightProcess
			
			char * token;											
			const char delim[2] = " ";									//delim will be used to seperate the tokens to generate proc1 and proc2
			int index = 0;
			
			pid_t p2;													//Will hold the process id

			trimWhiteSpace(currInput->processes[0]);					//Removes space from the beginning of each process string
			trimWhiteSpace(currInput->processes[1]);
			
		
			//Parse the left process by tokenizing the string by space
			token = strtok(leftProcess, delim);
			proc1[index++] = token;

			while (token != NULL)
			{
				token = strtok(NULL, delim);
				proc1[index++] = token;
			}
			proc1[index] = NULL;

			index = 0;
			
			//Parse the right process by tokenizing the string by space
			token = strtok(rightProcess, delim);
			proc2[index++] = token;

			while (token != NULL)
			{
				token = strtok(NULL, delim);
				proc2[index++] = token;
			}
			proc2[index] = NULL;

			
			//-----------------------Execute processes-----------------------------
			
			fflush(stdout);		//Flush the stdout buffer
			p2 = fork();
			if (p2 < 0)
			{
				//ERROR
				printf("ERROR: fork() failed\n");
				return;
			}
			else if (p2 == 0)
			{
				//CHILD
				executeArgsPiped(proc1, proc2);
			}
			else
			{
				//PARENT
				wait(NULL);
			}
			break;
			
		default:
			printf("More pipes not supported\n");
	}

	return 0;
}

int main(int argc, char * argv[])
{
	FILE * fd = stdin;
	if (argc == 2)
	{
		//"One argumen was given"
		fd = fopen(argv[1], "r");
		if (fd == NULL)
		{
			perror(argv[1]);
			exit(1);
		}
	}

	char inputLine[4096];						//Will store the input line
	memset(inputLine, 0, 4096);
	char * result;								//Holds the return value of fgets()
	struct commandLine currInput;				//Will be our parse objects that contains useful info. about the input line

	while (1)
	{
		printf("? ");
		fflush(stdout);
		result = fgets(inputLine, sizeof(inputLine), fd);
		if (result)
		{
			//Success, fgets() did not fail or EOF was not encountered
			if (inputLine[0] == '\n')	//Ensure that if the user doesn't input anything beside hitting ENTER, that it doesn't exit
				continue;
			//Removing the trailing '\n' that fgets puts 
			if ( (result = strchr(inputLine, '\n')) != NULL )
				*result = '\0';
		}
		else
		{
			//if fgets() fails or ctrl-D was hit on the keyboard
			printf("ERROR: EOF encountered or fgets(...) failed\n");
			exit(1);
		}
		
		//if the user enters "exit", exit the while loop and stop asking for input 
		if (strcmp(inputLine, "exit") == 0)
			return 0;

		if (strcmp(inputLine, "#!/bin/bash") == 0)
			continue;

		if (strlen(inputLine) == 0)
			continue;

		currInput = Parse(inputLine);			//Parse the input and create the commandLine struct
	
		//If the user enters "cd"
		if (strcmp(currInput.commands[0], "cd") == 0)
		{
			int result;
			if (currInput.numCommands == 1)
			{
				//cd with no arguments
				//Will cd into home directory
				result = chdir(getenv("HOME"));
				if (result < 0)
					perror("cd");
				continue;
			}

			char * path = currInput.commands[1];	
			result = chdir(path);
			if (result < 0 )
			{
				//path did not exist so chdir failed
				int ern = errno;
				fprintf(stderr, "*** Error in chdir('%s') errno=%d: %s\n", path, ern, strerror(ern));
			}
			continue;
		}

		executeCommands(&currInput);
		memset(inputLine, 0, 4096);

		if (fd == stdin)
			fflush(stdout);

		fflush(stdout);

	}

	return 0;

			
}

