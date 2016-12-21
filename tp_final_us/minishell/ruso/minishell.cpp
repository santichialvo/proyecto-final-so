/*
============================================================================
Name        : minishell.c
Author      : Sardor Isakov
Version     : v2.0
Copyright   : All rights reserved
Description : Simple Shell in C, Ansi-style
============================================================================
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include "minishell.h"
#include <sys/wait.h>
struct command_t command;

// print welcome message
void welcomeMessage() {
	printf("\nWelcome to mini-shell\n");
}

// print prompt
void printPrompt() {
	printf("mshell > ");
}

// looks up path using environment variable PATH
// function looks up is command is actually in path
// if it finds command in that path, it will return path
char * lookupPath(char **argv, char **dir) {
	char *result;
	char pName[MAX_PATH_LEN];
	if( *argv[0] == '/') {
		return argv[0];
	}
	else if( *argv[0] == '.') {
		if( *++argv[0] == '.') {
			if(getcwd(pName,sizeof(pName)) == NULL)
				perror("getcwd(): error\n");
			else {
				*--argv[0];
				asprintf(&result,"%s%s%s",pName,"/",argv[0]);
			}
			return result;
		}
		*--argv[0];
		if( *++argv[0] == '/') {
			if(getcwd(pName,sizeof(pName)) == NULL)
				perror("getcwd(): error\n");
			else {
				asprintf(&result,"%s%s",pName,argv[0]);
			}
			return result;
		}
	}
	
	// look in PAH directories, use access() to see if the
	// file is in the dir
	int i;
	for(i = 0 ; i < MAX_PATHS ; i++ ) {
		if(dir[i] != NULL) {
			asprintf(&result,"%s%s%s",  dir[i],"/",argv[0]);
			if(access(result, X_OK) == 0) {
				return result;
			}
		}
		else continue;
	}
	
	fprintf(stderr, "%s: command not found\n", argv[0]);
	return NULL;
}

// this function populates "pathv" with environment variable PATH
int parsePath(char* dirs[]){
	int debug = 0;
	char* pathEnvVar;
	char* thePath;
	int i;
	
	for(i=0 ; i < MAX_ARGS ; i++ )
		dirs[i] = NULL;
	pathEnvVar = (char*) getenv("PATH");
	thePath = (char*) malloc(strlen(pathEnvVar) + 1);
	strcpy(thePath, pathEnvVar);
	
	char* pch;
	pch = strtok(thePath, ":");
	int j=0;
	// loop through the thePath for ':' delimiter between each path name
	while(pch != NULL) {
		pch = strtok(NULL, ":");
		dirs[j] = pch;
		j++;
	}
	
	//===================== debug ===============
	// print the directories if debugging
	if(debug){
		printf("Directories in PATH variable\n");
		for(i=0; i<MAX_PATHS; i++)
			if(dirs[i] != '\0')
				printf("    Directory[%d]: %s\n", i, dirs[i]);
	}
}

// this function parses commandLine into command.argv and command.argc
int parseCommand(char * commandLine, struct command_t * command) {
	int debug = 0;
	
	char * pch;
	pch = strtok (commandLine," ");
	int i=0;
	while (pch != NULL) {
		command->argv[i] = pch;
		pch = strtok (NULL, " ");
		i++;
	}
	command->argc = i;
	command->argv[i++] = NULL;
	
	// pay no attention to this
	//===================== debug ===============
	if(debug) {
		printf("Stub: parseCommand(char, struct);\n");
		printf("Array size: %i\n", sizeof(*command->argv));
		int j;
		for(j=0; j<i; j++) {
			printf("command->argv[%i] = %s\n", j, command->argv[j]);
		}
		printf("\ncommand->argc = %i\n", command->argc);
		
		if(command->argv[0] != NULL) {
			//printf("*command->argv[%i] = %c\n", j, *command->argv[0]);
			char **p;
			for(p = &command->argv[1]; *p != NULL; p++) {
				printf("%s\n", *p);
			}
		}
	}
	return 0;
}

// this function read user input and save to commandLine
int readCommand(char * buffer, char * commandInput) {
	int debug = 0;
	buf_chars = 0;
	
	
	while((*commandInput != '\n') && (buf_chars < LINE_LEN)) {
		buffer[buf_chars++] = *commandInput;
		*commandInput = getchar();
	}
	buffer[buf_chars] = '\0';
	
	//===================== debug ===============
	if(debug){
		printf("Stub: readCommand(char *)\n");
		
		int i;
		for(i=0; i<buf_chars; i++) {
			printf("buffer[%i] = %c\n", i, buffer[i]);
		}
		printf("\nlength: %i\n", buf_chars-1);
		printf("\n1. buffer %s\n", buffer);
		printf("2. buffer[%i] = %c\n", buf_chars-2, buffer[buf_chars-2]);
		if(buffer[buf_chars-1] == '\n')
			printf("3. buffer[%i] = '\\n'\n", buf_chars-1);
		if(buffer[buf_chars] == '\0')
			printf("4. buffer[%i] = '\\0'\n", buf_chars);
	}
	return 0;
}

// this function is called from processFileInCommand(int)
// this function executes command with "<"
int executeFileInCommand(char * commandName, char * argv[], char * filename) {
	int pipefd[2];
	
	if(pipe(pipefd)) {
		perror("pipe");
		exit(127);
	}
	
	switch(fork()) {
	case -1:
		perror("fork()");
		exit(127);
	case 0:
		close(pipefd[0]);  /* the other side of the pipe */
		dup2(pipefd[1], 1);  /* automatically closes previous fd 1 */
		close(pipefd[1]);  /* cleanup */
		FILE * pFile;
		char mystring;
		
		pFile = fopen (filename , "r");
		if (pFile == NULL) perror ("Error opening file");
		else {
			
			while ((mystring=fgetc(pFile)) != EOF) {
				putchar(mystring); /* print the character */
			}
			fclose (pFile);
		}
		exit(EXIT_SUCCESS);
		
	default:
		
		close(pipefd[1]);  /* the other side of the pipe */
		dup2(pipefd[0], 0);  /* automatically closes previous fd 0 */
		close(pipefd[0]);  /* cleanup */
		
		execve(commandName, argv, 0);
		perror(commandName);
		exit(125);
		
	}
	
	return 0;
}

// this function is called from processFileOutCommand(int)
// this function executes command with ">"
int executeFileOutCommand(char * commandName, char * argv[], char * filename) {
	int def = dup(1);
	
	//First, we're going to open a file
	int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
	if(file < 0)    return 1;
	
	//Now we redirect standard output to the file using dup2
	if(dup2(file,1) < 0)    return 1;
	int pid;
	if( pid = fork() == 0) {
		close(file);
		close(def);
		execve(commandName, argv,0);
		return 0;
	}
	dup2(def, 1);
	close(file);
	close(def);
	wait(NULL);
	close(file);
	return 0;
}

// this function is called from processPipedCommand(int)
// this function executes command with "|"
void executePipedCommand(char *argvA[], char  *argvB[], char * nameA, char * nameB) {
	int pipefd[2];
	
	if(pipe(pipefd)) {
		perror("pipe");
		exit(127);
	}
	
	switch(fork()) {
	case -1:
		perror("fork()");
		exit(127);
	case 0:
		close(pipefd[0]);  /* the other side of the pipe */
		dup2(pipefd[1], 1);  /* automatically closes previous fd 1 */
		close(pipefd[1]);  /* cleanup */
		/* exec ls */
		execve(nameA, argvA, 0);
		/* return value from execl() can be ignored because if execl returns
		* at all, the return value must have been -1, meaning error; and the
		* reason for the error is stashed in errno *///
		perror(nameA);
		exit(126);
	default:
		/* parent */
		/*
		* It is important that the last command in the pipeline is execd
		* by the parent, because that is the process we want the shell to
		* wait on.  That is, the shell should not loop and print the next
		* prompt, etc, until the LAST process in the pipeline terminates.
		* Normally this will mean that the other ones have terminated as
		* well, because otherwise their sides of the pipes won't be closed
		* so the later-on processes will be waiting for more input still.
		*/
		/* do redirections and close the wrong side of the pipe */
		close(pipefd[1]);  /* the other side of the pipe */
		dup2(pipefd[0], 0);  /* automatically closes previous fd 0 */
		close(pipefd[0]);  /* cleanup */
		/* exec tr */
		execve(nameB, argvB, 0);
		perror(nameB);
		exit(125);
		
	}
}

// internal command clears terminal screen
//
// @return void
void clearScreen() {
	printf("\033[2J\033[1;1H");
}

// not implemented yet
void self() {
	printf("self...\n");
}

// internal command to change diractory
//
// return void
void changeDir() {
	if (command.argv[1] == NULL) {
		chdir(getenv("HOME"));
	} else {
		if (chdir(command.argv[1]) == -1) {
			printf(" %s: no such directory\n", command.argv[1]);
		}
	}
}

// This function check is for built in commands
// and processes if there any
//
// @return boolean
int checkInternalCommand() {
	
	if(strcmp("cd", command.argv[0]) == 0) {
		changeDir();
		return 1;
	}
	if(strcmp("clear", command.argv[0]) == 0) {
		clearScreen();
		return 1;
	}
	if(strcmp("self", command.argv[0]) == 0) {
		clearScreen();
		return 1;
	}
	
	return 0;
}

// excuteCommand() executes regular command, commands which doesn't have > < |
// rediractions
//
// example: ls -il, cat filname
//
// @return 0 if exec if successful
int excuteCommand() {
	
	int child_pid;
	int status;
	pid_t thisChPID;
	
	
	child_pid = fork();
	if(child_pid < 0 ) {
		fprintf(stderr, "Fork fails: \n");
		return 1;
	}
	else if(child_pid==0) {
		/* CHILD */
		execve(command.name, command.argv,0);
		printf("Child process failed\n");
		return 1;
	}
	else if(child_pid > 0) {
		/* PARENT */
		
		do {
			thisChPID = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
			if (thisChPID == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			
			if (WIFEXITED(status)) {
				//printf("exited, status=%d\n", WEXITSTATUS(status));
				return 0;
			} else if (WIFSIGNALED(status)) {
				printf("killed by signal %d\n", WTERMSIG(status));
			} else if (WIFSTOPPED(status)) {
				printf("stopped by signal %d\n", WSTOPSIG(status));
			} else if (WIFCONTINUED(status)) {
				printf("continued\n");
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		return 0;
	}
	
}

// this function process commands that have pipeline in it "|"
// this function is called from processCommand() function
// functin accepts indes location of | and processes accordingly
//
// @param	int		is index location of |
// @return	bool	return true on success
int processPipedCommand(int i) {
	char *argvA[5];
	char *argvB[5];
	char *nameA, *nameB;
	
	int ii;
	for(ii=0;ii<i;ii++) {
		argvA[ii] = command.argv[ii];
	}
	argvA[ii]=NULL;
	nameA = lookupPath(argvA, pathv);
	
	int j,jj=0;
	for(j=i+1; j<command.argc; j++) {
		argvB[jj] = command.argv[j];
		jj++;
	}
	argvB[jj]=NULL;
	nameB = lookupPath(argvB, pathv);
	
	int pid, status;
	fflush(stdout);
	
	switch ((pid = fork())) {
	case -1:
		perror("fork");
		break;
	case 0:
		/* child */
		executePipedCommand(argvA, argvB, nameA, nameB);
		break;  /* not reached */
	default:
		/* parent; fork() return value is child pid */
		/* These two pids output below will be the same: the process we
		* forked will be the one which satisfies the wait().  This mightn't
		* be the case in a more complex situation, e.g. a shell which has
		* started several "background" processes. */
		pid = wait(&status);
		return 0;
	}
	return 1;
}

// this function process commands that have redirection ">"
// this function is called from processCommand() function
// functin accepts indes location of > and processes accordingly
//
// @param	int		is index location of > within *argv[]
// @return	bool	return true on success
int processFileOutCommand(int i) {
	
	char *argv[5];
	char *commandName;
	int j;
	for(j=0; j<i; j++) {
		argv[j] = command.argv[j];
	}
	argv[j] = NULL;
	commandName = lookupPath(argv, pathv);
	
	return executeFileOutCommand(commandName, argv, command.argv[i+1]);
}

// this function process commands that have redirection "<"
// this function is called from processCommand() function
// functin accepts indes location of < and processes accordingly
//
// @param	int		is index location of < within *argv[]
// @return	bool	return true on success
int processFileInCommand(int i) {
	char *argv[5];
	char *commandName;
	
	int j;
	for(j=0; j<i; j++) {
		argv[j] = command.argv[j];
	}
	argv[j] = NULL;
	commandName = lookupPath(argv, pathv);
	
	int pid, status;
	fflush(stdout);
	
	switch ((pid = fork())) {
	case -1:
		perror("fork");
		break;
	case 0:
		/* child */
		executeFileInCommand(commandName, argv, command.argv[i+1]);
		break;  /* not reached */
	default:
		/* parent; fork() return value is child pid */
		/* These two pids output below will be the same: the process we
		* forked will be the one which satisfies the wait().  This mightn't
		* be the case in a more complex situation, e.g. a shell which has
		* started several "background" processes. */
		pid = wait(&status);
		return 0;
	}
	
	return 0;
}

// this function process commands and searches for < > |
// if there any redirection than processes accordingly
// if no rediraction than execute regular command
//
// @return	bool	return true on success
int processCommand() {
	
	int i;
	int infile=0, outfile=0, pipeLine=0;
	char *outFileName;
	char *inFileName;
	for(i=0;i<command.argc; i++) {
		if(strcmp(command.argv[i], ">") == 0) {
			return processFileOutCommand(i);
		}
		else if(strcmp(command.argv[i], "<") == 0) {
			return processFileInCommand(i);
			
		}
		else if(strcmp(command.argv[i], "|") == 0) {
			return processPipedCommand(i);
		}
	}
	return excuteCommand();
}

/*
============================================================================
Main method of this program
============================================================================
*/
int main(int argc, char* argv[]) {
	int i;
	int debug = 0;
	
	parsePath(pathv);
	welcomeMessage();
	
	// main loop
	while(TRUE) {
		printPrompt();
		
		commandInput = getchar(); //gets 1st char
		if(commandInput == '\n') { // if not input print prompt
			continue;
		}
		else {
			readCommand(commandLine, &commandInput); // read command
			
			if((strcmp(commandLine, "exit") == 0) || (strcmp(commandLine, "quit") == 0))
				break;
			
			parseCommand(commandLine, &command); //parses command into argv[], argc
			
			if(checkInternalCommand() == 0) {
				command.name = lookupPath(command.argv, pathv);
				
				if(command.name == NULL) {
					printf("Stub: error\n");
					continue;
				}
				
				processCommand();
			}
		}
	}
	
	printf("\n");
	exit(EXIT_SUCCESS);
}
