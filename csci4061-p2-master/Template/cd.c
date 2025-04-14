#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>

void cd(char* arg){
	char result[FILENAME_MAX];
	
	if(chdir(arg) != 0){
		printf("Failed to change directory.\n");
	}

	if(getcwd(result, FILENAME_MAX) == NULL){
		printf("Failed to get the current working directory.\n");
	}
	printf("Currently in %s\n", result);
}

int main(int argc, char** argv){
	if(argc<2){
		printf("Pass the path as an argument\n");
		return 0;
	}
	cd(argv[1]);
	return 0;
}