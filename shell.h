#ifndef __SHELL_H__
#define __SHELL_H__

#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>


#define MAXCOM 1000
#define MAXLIST 100

#define clear() printf("\033[H\033[J")

void init_shell();
int takeInput(char*);
void printDir();
void execArgs(char** );
void execArgsPiped(char** , char** );
void openHelp();
int ownCmdHandler(char** );
int parsePipe(char* , char** );
void parseSpace(char* , char** );
int processString(char* , char** , char** );
void find_path(char*);


#endif