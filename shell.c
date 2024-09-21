#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
// função que limpa o shell 
#define clear() printf("\033[H\033[J")




void find_path(char* path) {
    if (path == NULL) {
        // Get the CAMINHO environment variable
        char* caminho = getenv("CAMINHO");
        if (caminho == NULL) {
            printf("\nCAMINHO is not set. Defaulting to current directory (.)\n");
            setenv("CAMINHO", ".", 1);  // Set to current directory if CAMINHO is not set
        } else {
            printf("\nCurrent CAMINHO: %s\n", caminho);
        }
    } else {
        // Set the CAMINHO environment variable
        setenv("CAMINHO", path, 1);
        printf("\nCAMINHO updated to: %s\n", path);
    }
}


// Início  do shell
void init_shell()
{
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****Super Shell****");
    printf("\n\n\n\n*******************"
        "***********************");
    char* username = getenv("USER");
    printf("\n\n\nBem vindo, @%s", username);
    printf("\n");
    sleep(1);
    clear();
}

// input do shell copiado em um buffer
int takeInput(char* str)
{
    char* buffer;

    buffer = readline("\n-> ");
    if (buffer == NULL) {
        // Handle the case where readline returns NULL (Ctrl+D)
        return 1;
    }

    if (strlen(buffer) != 0) {
        add_history(buffer);
        strcpy(str, buffer);
        free(buffer);  // Free the dynamically allocated memory by readline
        return 0;
    } else {
        free(buffer);  // Free even if the string is empty
        return 1;
    }

}
// função pwd
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\n%s", cwd);
}

// executa um comando qualquer
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork(); 

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nComando não encontrado.");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL); 
        return;
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2]; 
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

// Help command builtin


// Function to execute builtin commands
int ownCmdHandler(char** parsed) {
    int NoOfOwnCmds = 3;  // Update the number of commands
    int switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    
    ListOfOwnCmds[0] = "cd";
    ListOfOwnCmds[1] = "pwd";
    ListOfOwnCmds[2] = "CAMINHO";  // Add CAMINHO to built-in commands

    // Check which built-in command is being called
    for (int i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }

    // Execute the appropriate built-in command
    switch (switchOwnArg) {
    case 1:
        chdir(parsed[1]);  // Change directory
        return 1;
    case 2:
        printDir();  // Print current directory
        return 1;
    case 3:
        if (parsed[1] == NULL) {
            find_path(NULL);  // Display CAMINHO if no argument is provided
        } else {
            find_path(parsed[1]);  // Set CAMINHO with the provided path
        }
        return 1;
    default:
        break;
    }

    return 0;  // Return 0 if it's not a built-in command
}

// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}

// function for parsing command words
void parseSpace(char* str, char** parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int processString(char* str, char** parsed, char** parsedpipe) {
    char* strpiped[2];
    int piped = 0;

    // Check for pipes
    piped = parsePipe(str, strpiped);

    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
    } else {
        parseSpace(str, parsed);  // Parse the input into command tokens
    }

    // Detect if the input starts with '$' to show an environment variable
    if (str[0] == '$') {
        char* env_var = getenv(str + 1);  // Skip the '$' and get the environment variable
        if (env_var) {
            printf("\n%s\n", env_var);  // Print the value of the environment variable
        } else {
            printf("\nVariable %s not found.\n", str + 1);
        }
        return 0;  // Return to indicate a successful built-in command execution
    }

    // Handle CAMINHO assignment if detected
    if (strncmp(str, "CAMINHO=", 8) == 0) {
        char* path = strchr(str, '=') + 1;
        find_path(path);  // Call find_path with the extracted path
        return 0;
    }

    // Handle other built-in commands (cd, pwd)
    if (ownCmdHandler(parsed)) {
        return 0;
    }

    return 1 + piped;
}

