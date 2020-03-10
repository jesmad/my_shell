#include "parse.h"

void getTokens(char userInputCopy[], char * args[], int * argsIndex)
{
	//Break the input line into tokens
	const char delim[2] = " ";
	char * token;
	token = strtok(userInputCopy, delim);
	args[*argsIndex] = token;
	*argsIndex += 1;						

	while (token != NULL)	
	{
		token = strtok(NULL, delim);
		args[*argsIndex] = token;
		*argsIndex += 1;
	}
	args[*argsIndex] = '\0'; //Make sure the array is null-terminated
}

void printTokens(char * args[])
{
	//Parses a command line and prints out each token
	int x;
	for (x = 0; args[x] != NULL; x++)
		printf("args[%d]: %s\n", x, args[x]);

	return;
}

void prettyPrint(char * args[])
{
	//Prints the output in a different way
	int x;
	const char pipe[2] = "|";
	const char less[2] = "<";
	const char great[2] = ">";

	for (x = 0; args[x] != NULL; x++)
	{
		if (strcmp(args[x], pipe) == 0 || strcmp(args[x], great) == 0 || strcmp(args[x], less) == 0)
			printf("%s ", args[x]);

		else
			printf("'%s' ", args[x]);
	}
	printf("\n");
	return;
}

int findNumber(char * args[])
{
	//This function returns the number of pipes
	int loop;
	int numPipes = 0;
	const char pipe[2] = "|";

	for (loop = 0; args[loop] != NULL; loop++)
	{
		if (strcmp(args[loop], pipe) == 0)	//If it sees a pipe, increment the counter.
			numPipes++;
	}
	numPipes++;								//Add one to the result, because single commands are considered to be one pipe
	return numPipes;
}

void findLastInput(int lessPositions[], int * lpIndex, char * args[])
{
	//Returns the index of the last '<' or '<<' seen on the command line
	int loop;

	for (loop = 0; args[loop] != NULL; loop++)
	{
		if (strcmp(args[loop], "<") == 0 || strcmp(args[loop], "<<") == 0)
		{
			lessPositions[*lpIndex] = loop;
			*lpIndex += 1;
		}
	}
	lessPositions[*lpIndex] = 0;		//Make sure the array is null-terminated 
	return;
}

void findLastOutput(int greatPositions[], int * gpIndex, char * args[])
{
	//Returns the index of the last '>'  or '>>' seen on the command line
	int loop;
	
	for (loop = 0; args[loop] != NULL; loop++)
	{
		if (strcmp(args[loop], ">") == 0 || strcmp(args[loop], ">>") == 0)
		{
			greatPositions[*gpIndex] = loop;
			*gpIndex += 1;
		}
	}
	greatPositions[*gpIndex] = 0;
	return;
}

void generateSkipIndices(int indicesToSkip[], int lessPositions[], int greatPositions[], int * skipIndex, int lpIndex, int gpIndex)
{
	//This function will generate the indicesToSkip[] array. This array will be used to skip arguments that are next to '>','<', '<<', '>>'.
	int x;

	//Skip '<' or '<<'
	for (x = 0; x < lpIndex; x++)
	{
		indicesToSkip[*skipIndex] = lessPositions[x];
		*skipIndex += 1;
	}
	//Skip '>' or '>>'
	for (x = 0; x < gpIndex; x++)
	{
		indicesToSkip[*skipIndex] = greatPositions[x];
		*skipIndex += 1;
	}
	//Skip < 'arg'
	for (x = 0; x < lpIndex; x++)
	{
		indicesToSkip[*skipIndex] = lessPositions[x] + 1;
		*skipIndex += 1;
	}
	//Skip > 'arg'
	for (x = 0; x < gpIndex; x++)
	{
		indicesToSkip[*skipIndex] = greatPositions[x] + 1;
		*skipIndex += 1;
	}
	indicesToSkip[*skipIndex] = 0;
	return;
}

int skipOrNot(int indicesToSkip[], int skipIndex, int n)
{
	//A boolean function that checks whether a given index n is in the indicesToSkip[] array
	int loop;
	for (loop = 0; loop < skipIndex; loop++)
	{
		//Skip index
		if (n == indicesToSkip[loop])
			return 1;
	}
	return 0;
}

int lengthOfArray(int array[])
{
	//Takes an array and finds the number of elements it has stored
	int size = 0;
	int loop;
	for (loop = 0; array[loop] != 0; loop++)
		size++;

	return size;
}

void desiredOutput(char * args[], struct commandLine * currInput)
{
	int indicesToSkip[512];				//Contain the indices that we don't want to print (i.e. omitt the argument that comes after '<' or '>'
	int lessPositions[512];				//Contains the indices where '<' appears and '<<'
	int greatPositions[512];			//Contains the indices where '>' appears and '>>'
	
	memset(indicesToSkip, 0, 512);
	memset(lessPositions, 0, 512);
	memset(greatPositions, 0, 512);

	int skipIndex = 0;					//Index of indicesToSkip[]
	int lpIndex = 0;					//Index of lessPositions[]
	int gpIndex = 0;					//Index of greatPositons[]
	char * nextArgLess = NULL;					//Will point to next argument of the last '<' or '<<' seen
	char * nextArgGreat = NULL;					//Will point to next argument of the last '>' or '>>' seen
	char * lastLess = NULL;						//Will point to a '<' or '<<'
	char * lastGreat = NULL;					//Will point to a '>' or '>>'
	
	const char pipe[2] = "|";

	findLastInput(lessPositions, &lpIndex, args);	//Generates lessPositions[]
	findLastOutput(greatPositions, &gpIndex, args);	//generates greatPositions[]

	int lastInputPosition = lessPositions[lpIndex - 1];		//Last '<' or '<<' seen
	int lastOutputPosition = greatPositions[gpIndex - 1];	//Last '>' or '>>' seen

	nextArgLess = args[lastInputPosition + 1];
	nextArgGreat = args[lastOutputPosition + 1];
	lastLess = "<";
	lastGreat = args[lastOutputPosition];				//Will hold '>' or '>>'

	int numPipes = findNumber(args);					//Gets the number of pipes seen
	currInput->numPipes = numPipes;						//***commandLine->pipes update

	generateSkipIndices(indicesToSkip, lessPositions, greatPositions, &skipIndex, lpIndex, gpIndex);
	
	//Here's the output that will be diplayed to the terminal
	int i;
	if (lengthOfArray(lessPositions))
	{
		//printf("%s'%s' ", lastLess, nextArgLess);
		currInput->inputFile[0] = lastLess;			//***commandLine->inputFile
		currInput->inputFile[1] = nextArgLess;
		currInput->inputFile[2] = '\0';
	}
	else
	{	
		currInput->inputFile[0] = '\0';
		currInput->inputFile[1] = '\0';
		currInput->inputFile[2] = '\0';
	}

	//for (i = 0; args[i] != NULL; i++)				//Print out each token in the desired output, skipping indices that are in indicesToSkip[] 
	//{
	//	if (!skipOrNot(indicesToSkip, skipIndex, i))
	//	{
	//		if (strcmp(args[i], pipe) == 0)
	//			printf("%s ", args[i]);
	//		else
	//			printf("'%s' ", args[i]);
	//	}
	//}

	if (lengthOfArray(greatPositions))
	{
		//printf("%s'%s' ", lastGreat, nextArgGreat);
		currInput->outputFile[0] = lastGreat;
		currInput->outputFile[1] = nextArgGreat;
		currInput->outputFile[2] = '\0';
	}
	else
	{
		currInput->outputFile[0] = '\0';
		currInput->outputFile[1] = '\0';
		currInput->outputFile[2] = '\0';
	}
	//printf("\n");
}

void copyTokens(char * args[], struct commandLine * currInput)
{
	//This function will iterate over args[] and copy each token to the struct's
	//commands[] member variable and it also counts the number of arguments on the command line
	
	int x;
	int counter = 0;
	for (x = 0; args[x] != NULL; x++)
	{
		currInput->commands[x] = args[x];
		counter++;
	}
	currInput->commands[x] = '\0';
	currInput->numCommands = counter;
	return;
}

void indicesPipes(char * args[], struct commandLine * currInput)
{
	//This function finds the indices of the pipes seen in the command line
	int index = 0;
	int loop;
	const char delim[2] = "|";

	for (loop = 0; args[loop] != NULL; loop++)
	{
		if (strcmp(args[loop], delim) == 0)
			currInput->pipeIndices[index++] = loop;
	}
	currInput->pipeIndices[index] = 0;
	return;
}

void getProcesses(struct commandLine * currInput)
{
	//This function splits up the command line by "|" so that we have different processes
	char * token;
	char inputLine[2048];
	const char delim[2] = "|";

	strcpy(inputLine, currInput->inLine);
	//printf("inputLine: %s\n", inputLine);

	int index = 0;

	token = strtok(inputLine, delim);
	currInput->processes[index++] = token;

	while (token != NULL)
	{
		token = strtok(NULL, delim);
		currInput->processes[index++] = token;
	}

	currInput->processes[index] = NULL;

	return;
}

struct commandLine Parse(char userInput[])
{
	//Parses the input line and geenrates a commandLine struct that contains useful info. about the commands listed
	struct commandLine currInput;
	char * args[1024];						//Will hold all of the parsed tokens
	int argsIndex = 0;						
	char userInputCopy[4096];				//Will store a copy of the input line

	strcpy(currInput.inLine, userInput);	//***commandLine->inLine

	memset(userInputCopy, '\0', 4096);
	strcpy(userInputCopy, userInput);		
	getTokens(userInputCopy, args, &argsIndex);
	desiredOutput(args, &currInput);

	copyTokens(args, &currInput);			//This will copy the tokens into our struct

	if (currInput.numPipes != 1)			//These conditional blocks will find the indices of the pipes found 
		indicesPipes(args, &currInput);	
	else
		currInput.pipeIndices[0] = 0;	

	getProcesses(&currInput);				//This will create an array of arrays, where each subarray is a different process and its options
	return currInput;
}
