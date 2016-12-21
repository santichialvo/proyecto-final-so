#ifndef MINISHELL_H
#define MINISHELL_H

#define FALSE           		0
#define TRUE            		1
#define LINE_LEN        		80
#define MAX_ARGS        		64
#define MAX_ARG_LEN     		64
#define MAX_PATHS       		64
#define MAX_PATH_LEN    		96
#define MAX_PIPESCOMMANDS 	5
#define DEBUG_PARSEPATH		1
#define DEBUG_READCOM			0
#define DEBUG_PARSECOM		0

static char *pathv[MAX_PATHS];
static char commandInput = '\0';
static char commandLine[LINE_LEN];
static int buf_chars = 0;

struct command_t {
	char *name;
	int argc;
	char *argv[MAX_ARGS];
};

void parsePath				(char* dirs[]);
void welcomeMessage			();
void printPrompt			();
int readCommand				(char * buffer, char * commandInput);
int parseCommand			(char * commandLine, struct command_t * command);
void changeDir				();
void clearScreen			();
void addCamino				();
int checkInternalCommand	();
char * lookupPath			(char **argv, char **dir);
int excuteCommand			();
int fork_pipe_proc			(int in, int out, command_t pipecommand);
int executePipedCommand		(int n, command_t pipecommands[]);
int processPipedCommand		(int cantpipes);
int processCommand			();


#endif
