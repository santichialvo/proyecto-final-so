#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "minishell.h"
using namespace std;

struct command_t command;

void welcomeMessage() {
	printf("Bienvenido al mini-shell\n\n");
}


void parsePath(char* dirs[]) {
	char* pathEnvVar;
	char* thePath;
	
	for(int i=0 ; i < MAX_ARGS ; i++ )
		dirs[i] = NULL;
	
	//Se cargan separadas por :
	pathEnvVar = (char*) getenv("PATH");
	thePath = (char*) malloc(strlen(pathEnvVar) + 1);
	strcpy(thePath, pathEnvVar);
	
	char* pch;
	pch = strtok(thePath, ":");
	dirs[0] = pch;
	int j=1;
	// loop en thePath para el token ':'
	while(pch != NULL)
	{
		pch = strtok(NULL, ":");
		dirs[j] = pch;
		j++;
	}
	
	if (DEBUG_PARSEPATH) 
	{
		printf("Directorios en la variable PATH:\n");
		for(int i=0; i<MAX_PATHS; i++)
			if(dirs[i] != '\0')
				printf("Directorio[%d]: %s\n", i, dirs[i]);
	}
}

void printPrompt() {
	char pName[MAX_PATH_LEN];
	getcwd(pName,sizeof(pName));
	printf("%s %% ",pName);
}

int readCommand(char * buffer, char * commandInput) {
	buf_chars = 0;
	
	while((*commandInput != '\n') && (buf_chars < LINE_LEN)) {
		buffer[buf_chars++] = *commandInput;
		*commandInput = getchar();
	}
	buffer[buf_chars] = '\0';
	
	if (DEBUG_READCOM)
	{
		for(int i=0; i<buf_chars; i++) {
			printf("buffer[%i] = %c\n", i, buffer[i]);
		}
		printf("\nlength: %i\n", buf_chars-1);
		printf("\n1. buffer %s\n", buffer);
	}
	
	return 0;
}

int parseCommand(char * commandLine, struct command_t * command) {
	
	char * pch;
	pch = strtok(commandLine," ");
	int i=0;
	while (pch != NULL) {
		command->argv[i] = pch;
		pch = strtok (NULL, " ");
		i++;
	}
	command->argc = i;
	command->argv[i++] = NULL;
	
	if (DEBUG_PARSECOM) {
		printf("Size del array: %i\n", sizeof(*command->argv));
		
		int j;
		for(j=0; j<i; j++) {
			printf("command->argv[%i] = %s\n", j, command->argv[j]);
		}
		
		if(command->argv[0] != NULL) {
			char **p;
			for(p = &command->argv[1]; *p != NULL; p++) {
				printf("%s\n", *p);
			}
		}
	}
	return 0;
}

void changeDir() {
	if (command.argv[1] == NULL) {
		chdir(getenv("HOME"));
	} else {
		if (chdir(command.argv[1]) == -1) {
			printf("cd: %s:  No existe el archivo o el directorio\n", command.argv[1]);
		}
	}
}

void clearScreen() {
	printf("\033[2J\033[1;1H");
}

void addCamino() {
	char pName[MAX_PATH_LEN];
	char * result;
	
	if(getcwd(pName,sizeof(pName)) == NULL)
		perror("getcwd(): error\n");
	
	char *pathEnvVar;
	pathEnvVar = (char*) getenv("PATH");
	
	if (strcmp("+", command.argv[1]) == 0) {
		for(int i=0;i<MAX_PATHS;i++ ) {
			if(pathv[i] != NULL && strcmp(pName, pathv[i]) == 0) {
				printf("Path ya agregado a PATH \n");
				return;
			}
		}
		result = "";
		asprintf(&result,"%s:%s",pathEnvVar,pName);
		setenv("PATH",result,true);
		printf("Path agregado a PATH \n");
	}
	else if (strcmp("-", command.argv[1]) == 0) {
		string str_pathEnvVar(pathEnvVar);
		if (str_pathEnvVar.find(pName)==string::npos) {
			printf("Path no presente en PATH \n");
			return;
		}
		result = strcmp(pName, pathv[0]) != 0?pathv[0]:pathv[1];
		for(int i=1;i<MAX_PATHS;i++ ) {
			if(pathv[i] != NULL && strcmp(pName, pathv[i]) != 0) {
				asprintf(&result,"%s:%s",result,pathv[i]);
			}
		}
		setenv("PATH",result,true);
		printf("Path eliminado de PATH \n");
	}
	else
	{
		printf("Error usando camino \n");
	}
}

int checkInternalCommand() {
	
	if(strcmp("cd", command.argv[0]) == 0) {
		changeDir();
		return 1;
	}
	if(strcmp("clear", command.argv[0]) == 0) {
		clearScreen();
		return 1;
	}
	if(strcmp("camino", command.argv[0]) == 0) {
		addCamino();
		parsePath(pathv);
		return 1;
	}
	return 0;
}

//Arma los path tanto para ejecutar como para buscarlo en un dir del PATH
char * lookupPath(char **argv, char **dir) {
	char *result;
	char pName[MAX_PATH_LEN];
	if(*argv[0] == '/') {
		return argv[0];
	}
	else if(*argv[0] == '.') {
		if(*++argv[0] == '.') {
			//armar con el path completo ../.. lo que sea
			if(getcwd(pName,sizeof(pName)) == NULL)
				perror("getcwd(): error\n");
			else {
				*--argv[0];
				asprintf(&result,"%s%s%s",pName,"/",argv[0]);
			}
			return result;
		}
		//armar con el path completo ./execute
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
	
	int i;
	for(i = 0 ; i < MAX_PATHS ; i++ ) {
		if(dir[i] != NULL) {
			//se fija en los dirs y arma la secuencia dir/comando. La funcion
			//access() da el ok si tiene permisio de ejecucion (x)
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

//excuteCommand() ejecuta comandos regulares, sin > < |
//ejemplo: ls -il, cat filname
//@return 0 si exec es exitoso
int excuteCommand() {
	
	int child_pid;
	int estado_proceso;
	pid_t thisChPID;
	
	//creo el proceso hijo con fork, me devuelve el pid
	child_pid = fork();
	if(child_pid < 0 ) {
		fprintf(stderr, "Fallo el fork: \n");
		return 1;
	}
	else if(child_pid==0) {
		//proceso hijo
		
		//https://linux.die.net/man/3/execvp
		//The exec() functions only return if an error has occurred. 
		//The return value is -1, and errno is set to indicate the error.
		execvp(command.name, command.argv);
		printf("Fallo el proceso hijo \n");
		return 1;
	}
	else if(child_pid > 0) {
		//proceso padre
		do {
			//espero que se ejecute el proceso hijo, en estado_proceso chequeo
			//si termino, fue detenido o matado
			thisChPID = waitpid(child_pid, &estado_proceso, WUNTRACED|WCONTINUED);
			
			if (thisChPID == -1) {
				//The perror() function produces a message on standard error describing
				//the last error encountered during a call to a system or library
				//function.
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			
			//https://linux.die.net/man/3/waitpid
			//Evaluates to a non-zero value if status was returned 
			//for a child process that terminated normally.
			if (WIFEXITED(estado_proceso)) {
//				printf("Terminado, estado=%d \n", WEXITSTATUS(estado_proceso));
				return 0;
			//Evaluates to a non-zero value if status was returned for a child 
			//process that terminated due to the receipt of a signal that was not caught
			//If the value of WIFSIGNALED(stat_val) is non-zero, this macro 
			//evaluates to the number of the signal that caused the termination of the child process.
			} else if (WIFSIGNALED(estado_proceso)) {
				printf("Asesinado con la señal %d \n", WTERMSIG(estado_proceso));
			//Evaluates to a non-zero value if status was returned for a 
			//child process that is currently stopped.
			//If the value of WIFSTOPPED(stat_val) is non-zero, this macro 
			//evaluates to the number of the signal that caused the child process to stop. 
			} else if (WIFSTOPPED(estado_proceso)) {
				printf("Detenido con la señal %d \n", WSTOPSIG(estado_proceso));
			//Evaluates to a non-zero value if status was returned for a 
			//child process that has continued from a job control stop.
			} else if (WIFCONTINUED(estado_proceso)) {
				printf("Continuado \n");
			}
		} while (!WIFEXITED(estado_proceso) && !WIFSIGNALED(estado_proceso));
		return 0;
	}
	
	return 0;
}

int fork_pipe_proc(int in, int out, command_t pipecommand) {
	pid_t pid;
	if ((pid = fork ()) == 0)
	{
		//The dup2() system call creates a copy of the file descriptor oldfd,
		//using the lowest-numbered unused file descriptor for the new
		//descriptor.
		if (in != 0)
		{
			dup2(in,0);
			close(in);
		}
		if (out != 1)
		{
			dup2(out,1);
			close(out);
		}
		return execvp(pipecommand.name,pipecommand.argv);
	}
	return pid;
}

//ejemplo: find . -name *log | xargs rm -f
int executePipedCommand(int n, command_t pipecommands[])
{
	int i;
	int in, fd[2];
	int stdin_copy = dup(0);
//	int stdout_copy = dup(1);
	
	//Para el primer proceso, STDIN_FILENO, no obtiene entrada de ningun otro proceso
	in = 0;
	close(0);
	for (i=0; i<n-1;++i)
	{
		if(pipe (fd)) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
		
		//f [1] es la salida de escritura del pipe, va de salida para el proceso actual
		fork_pipe_proc(in, fd[1], pipecommands[i]);
		//Cierro la salida de escritura del pipe aca, la va a usar solo el hijo
		close(fd[1]);
		//El proximo hijo va a leer de aca
		in = fd[0];
	}
	
	//Si obtuve salida del anteultimo comando, la mando a stdin
	if (in != 0)
		dup2(in, 0);
	
	//Ejecuto el ultimo comando en el proceso acutal
	int child_pid = fork();
	int estado_proceso;
	pid_t thisChPID;
	if (child_pid == 0)
	{ 
		execvp(pipecommands[i].name, pipecommands[i].argv);
		printf("Fallo el proceso hijo \n");
		return 1;
	}
	else 
	{
		do {
			thisChPID = waitpid(child_pid, &estado_proceso, WUNTRACED|WCONTINUED);
		} while(!WIFEXITED(estado_proceso) && !WIFSIGNALED(estado_proceso));
		dup2(stdin_copy, 0);
//		dup2(stdout_copy, 1);
	}
	return 0;
}

// procesa comandos con pipelines "|"
// llamada de la funcion processCommand()
//
// @param	int		cantidad de "|"
// @return	bool	return true si es exitoso
int processPipedCommand(int cantpipes) {
	command_t pipes_commands[MAX_PIPESCOMMANDS];
	
	int cant_commands=0,cant_argv=0;
	for(int i=0;i<command.argc;i++) 
	{
		if (strcmp(command.argv[i],"|")==0)
		{
			pipes_commands[cant_commands].argv[cant_argv] = NULL;
			pipes_commands[cant_commands].name = lookupPath(&pipes_commands[cant_commands].argv[0], pathv);
			pipes_commands[cant_commands].argc = cant_argv;
			cant_commands++;
			cant_argv = 0;
			continue;
		}
		pipes_commands[cant_commands].argv[cant_argv] = command.argv[i];
		cant_argv++;
	}
	pipes_commands[cant_commands].argv[cant_argv] = NULL;
	pipes_commands[cant_commands].name = lookupPath(&pipes_commands[cant_commands].argv[0], pathv);
	pipes_commands[cant_commands].argc = cant_argv;
	cant_commands++;
	
	return executePipedCommand(cant_commands,pipes_commands);
}

//Procesa el comando y busca >,< o |
//return bool true en exito
int processCommand() {
	int i;
	for(i=0;i<command.argc; i++) 
	{
		if(strcmp(command.argv[i], "|") == 0) { //si encuentro un pipe
			//cuento la cantidad que hay
			int pipeslength=0;
			for (int j=0;j<command.argc;j++)
				if (strcmp(command.argv[j], "|") == 0)
					pipeslength++;
			return processPipedCommand(pipeslength);
		}
	}
	return excuteCommand();
}

int main(int argc, char *argv[]) {
	
	parsePath(pathv);
	welcomeMessage();
	// loop principal
	while(TRUE)
	{
		printPrompt();
		commandInput = getchar(); 	//primer char
		if(commandInput == '\n') 	//si fue un enter
		{ 
			continue;
		}
		else 
		{
			readCommand(commandLine, &commandInput); //lee comando
			
			if((!strcmp(commandLine, "exit")))
				break;
			
			parseCommand(commandLine, &command); //parsea los comandos en argv[], argc
			
			if(checkInternalCommand() == 0) {
				command.name = lookupPath(command.argv, pathv);
				processCommand();
			}
		}
	}
	exit(EXIT_SUCCESS);
}
