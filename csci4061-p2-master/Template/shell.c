#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include "util.h"

#define PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define WRITE_APPEND (O_WRONLY | O_CREAT | O_APPEND) // adds on to file 
void execute_command(char *firstArg, char *cmd[] ,int input, int output){ // executes commands -> ls , wc, cd and default commands
	if (input!= STDIN_FILENO) {
		dup2(input, STDIN_FILENO);
		close(input);
	}

	if (output!= STDOUT_FILENO) {
		dup2(output, STDOUT_FILENO);
		close(output);
	}

	if(strcmp(firstArg,"ls")==0){ //command is ls
		execv("./ls", cmd);
		perror("failed execv");
		exit(1);
	}
	else if(strcmp(firstArg,"cd")==0){ // command is cd
		execv("./cd", cmd);
		perror("failed execv");
		exit(1);
	}

	else if(strcmp(firstArg,"wc")==0){ // command is wc 
		execv("./wc", cmd);
		perror("failed execv");
		exit(1);
	}

	else{ // default command
		execvp(firstArg, cmd);
		perror("execvp");
		exit(1);
	}
	return;
}
void handle_neither(char *firstArg[], int length){ // NEITHER PIPE OR REDIRECTION
	int fd[2];
	if(pipe(fd)==-1){
		perror("Pipe() failed");
		return;
	}
	pid_t  pid1=fork();
	if(pid1==0){//child process
		close(fd[1]); // read is not necessary
		execute_command(firstArg[0],firstArg,fd[0], STDOUT_FILENO); // execute singular command 
	}

	close(fd[0]);
	close(fd[1]);
	waitpid(pid1, NULL, 0);

	return;
}
void handle_redirection(char *tokens[], int numInput) { //REDIRECTION ONLY
	char* firstArg[MAX_CMD_LEN]; // command type
	int fd;
	int fd2[2];

	for (int i = 0; i < numInput; ++i) { // iterate through each input from command line
		if (strcmp(tokens[i], ">") == 0) { // normal output redirection
			firstArg[0] = tokens[i-1]; // first argument is what is before the redirection
			fd = open(tokens[i+1], WRITE, PERM); //file to redirect two is what is after the symbol
			firstArg[1]=NULL;
		} 
		else if (strcmp(tokens[i], ">>") == 0) { // append 
			firstArg[0] = tokens[i-1]; // command type 
			fd = open(tokens[i+1], WRITE_APPEND,PERM);  // file to be added on to 
			firstArg[1]=NULL;
		}
	}

	if(fd==-1){ // file opening for either case, > or >>, failed
		perror("Failed to open file\n");
		exit(-1);
	}

	pid_t pid = fork();
	if (pid == 0) { // Child process
		// Redirect STDOUT to the file
		if (dup2(fd, STDOUT_FILENO) == -1) {
     	perror("Failed to redirect \n");
			exit(-1);
		}
		close(fd);
		close(fd2[0]);
		execute_command(firstArg[0], firstArg, STDIN_FILENO, fd2[1]); // execute command 

	} 
	close(fd2[0]);
	close(fd2[1]);
	close(fd);
	waitpid(pid, NULL, 0); // wait for the certain child process forked above to complete
}




void handle_pipe_and_redirection(char *tokens[], int numInput) { // ex: ls | wc > out.txt
	int fd[2], file;
	char *firstArg[MAX_CMD_LEN]; // left side argument 
	char *secondArg[MAX_CMD_LEN]; // right side argument 

	for (int i = 0; i < numInput; i++) {
		if (strcmp(tokens[i], "|") == 0) {
			firstArg[0] = tokens[i-1]; // first command
			firstArg[1]=NULL; // Terminate with NULL

		} 
		else if (strcmp(tokens[i], ">") == 0) {
			secondArg[0] = tokens[i-1]; // second command 
			secondArg[1]=NULL;			
			file = open(tokens[i + 1], WRITE, PERM);


		} else if (strcmp(tokens[i], ">>") == 0) {
			secondArg[0] = tokens[i-1]; // second command 
			secondArg[1]=NULL;			
			file = open(tokens[i + 1],WRITE_APPEND, PERM);

		}
	}

	if (pipe(fd) == -1) {
		perror("Pipe failed");
		return;
	}

	pid_t pid1 = fork();
	if (pid1 == 0) {
			close(fd[0]); 
			dup2(fd[1], STDOUT_FILENO); // redirect 
			close(fd[1]);
			execute_command(firstArg[0], firstArg, STDIN_FILENO, STDOUT_FILENO); // execute commands 
	}

	pid_t pid2 = fork();
	if (pid2 == 0) {
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);  // redirect 
		close(fd[0]);
		dup2(file, STDOUT_FILENO); // redirect file
		close(file);
		execute_command(secondArg[0], secondArg, STDIN_FILENO, STDOUT_FILENO); // execute the commands 
	}

	close(fd[0]);
	close(fd[1]);

	waitpid(pid1, NULL, 0); // wait for child 
	waitpid(pid2, NULL, 0);
}



void handle_pipe(char * tokens[],int length){
	char* firstArg[MAX_CMD_LEN];  // holds left side of the pipe argument 
	char* secondArg[MAX_CMD_LEN]; // holds right side of the pipe argument 
	int lenFirst=0;
	int lenSecond=0;

	int i=0;
	while(strcmp(tokens[i],"|")!=0){  // save the left side arguments into firstArg
		firstArg[i]=tokens[i];
		i++;
	}
	lenFirst=i;
	int x=0;
	while(i+1<length){  // save the right side arguments into secondArg
		secondArg[x]=tokens[i+1];
		x++;i++;
	}
	lenSecond=length - lenFirst-1;

	firstArg[lenFirst] = NULL;
	secondArg[lenSecond] = NULL;

	int fd[2];
	if(pipe(fd)==-1){
		perror("Pipe() failed");
		return;
	}

	pid_t  pid1=fork();
	if(pid1==0){//child process
		close(fd[0]); // read is not necessary
		execute_command(firstArg[0],firstArg, STDIN_FILENO, fd[1]); // execute the commands
	}

	pid_t pid2=fork();
	if(pid2==0){//child process
		close(fd[1]); // write is not necessary
		execute_command(secondArg[0], secondArg,fd[0],STDOUT_FILENO); // execute the commands
	}

	close(fd[0]);
	close(fd[1]);

	waitpid(pid1, NULL, 0);
  waitpid(pid2, NULL, 0);
	return;
}


int main(){
	char input[MAX_CMD_LEN]; // command line input 
	char *tokens[MAX_TOKENS]; // will holdthe 
	char cwd[MAX_CMD_LEN]; // current working directory 

	getcwd(cwd,MAX_CMD_LEN);
	while(1){ // while shell is running...
		bool pipe=false;
		bool redirection = false;
		// implement the unix shell design layout - > [4061-shell]/home/csci4061/project $
		printf("[4061-shell] %s $ ",cwd);

		if (fgets(input, MAX_CMD_LEN, stdin) == NULL) { // nothing was read in
			break;
		}
		input[strlen(input) - 1] = '\0'; // Remove newline character

		if (strcmp(input, "exit") == 0) { // exit command from the shell 
			printf("Exiting shell\n");
			break;
		}

		int numInput = parse_line(input, tokens," \"" ); // parsing the inputs from the command line, storing in tokens

		if (numInput <= 0) {// prevents parsing empty inputs
    	continue;
		}

		for(int i=0;i<numInput;i++){ // looping through each input from the command line
			if(strcmp(tokens[i],"|")==0){ 
				pipe=true;
			}
			else if(strcmp(tokens[i],">")==0 || strcmp(tokens[i],">>")==0){
				redirection=true;
			}
		}
		if(pipe && redirection){ // PIPE AND REDIRECTION
			handle_pipe_and_redirection(tokens,numInput);
		}
		else if(pipe && !redirection){ // PIPE ONLY
			handle_pipe(tokens, numInput);
		}
		else if(redirection && !pipe){ // REDIRECTION ONLY 
			/// handle redirection  ">" and ">>"
			handle_redirection(tokens,numInput);
		}
		else { //NEITHER PIPE OR REDIRECTION
			handle_neither(tokens, numInput);
		}
	}
}