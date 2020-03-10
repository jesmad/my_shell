#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

struct commandLine
{
	int numPipes;	//Will have a counter of the number of programs seen
	int pipeIndices[1024];	//Will store the indices of the pipes
	char inLine[2048]; 	//Will contain the line of input 
	char * commands[1024];	//Will contain the different commands from the line of input (parsed)
	char * inputFile[3];	//Will contain the file input
	char * outputFile[3]; 	//Will contain the file output
	char * processes[512];	//Will contain the processes
	int numCommands;		//Will contain the number of arguments on the command line
};

//parse.c
void copyTokens(char * args[], struct commandLine * currInput);
void desiredOutput(char * args[], struct commandLine * currInput);
int lengthOfArray(int array[]);
int skipOrNot(int indicesToSkip[], int skipIndex, int n);
void generateSkipIndices(int indicesToSkip[], int lessPositions[], int greatPositions[], int * skipIndex, int lpIndex, int gpIndex);
void findLastOutput(int greatPositions[], int * gpIndex, char * args[]);
void findLastInput(int lessPositions[], int * lpIndex, char * args[]);
int findNumber(char * args[]);
void prettyPrint(char * args[]);
void printTokens(char * args[]);
void getTokens(char userInputCopy[], char * args[], int * argsIndex);
void indicesPipes(char * args[], struct commandLine * currInput);
void getProcesses(struct commandLine * currInput);
struct commandLine Parse(char input[]);


//main.c
void trimWhiteSpace(char * process);
void argsForRedirection(struct commandLine * currInput, char * arguments[]);
void tokenizeProcesses(char * args[], char * process);
void executeArgsPiped(char * proc1[], char * proc2[]);
int executeCommands(struct commandLine * currInput);
#endif /* PARSE_H_INCLUDED */
