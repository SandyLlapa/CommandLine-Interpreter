#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include<string.h>


void read_helper(int mode,FILE *file){
	int firstCol =0;  // number of lines present in a file 
	int secondCol=0; // number of words present
	int thirdCol=0; // number of characters including whitespaces 
	// print all -> 0 
	// -l -> print number of lines   1 
	// -w -> print number of words   2 
	// -c -> print number of characters   3 
	char c;
	char prev='\0';




	while((c = fgetc(file))!=EOF){
		// printf("[%c]", c); // Display value of each character
		if(c == '\n'){
			firstCol++;
		}
		if((c!=' '&& c!='\n') && (prev==' '||prev=='\n'||prev=='\0' )){
			secondCol++;
		}
		prev =c; 

		thirdCol++;
	}


	if(mode == 0){
		printf("  %d  %d  %d \n", firstCol, secondCol, thirdCol);
	}
	else if(mode ==1){
		printf("%d\n", firstCol);
	}
	else if(mode==2){
		printf("%d\n", secondCol);
	}
	else if(mode ==3){
		printf("%d\n", thirdCol);
	}

}

void wc(int mode, char* path){
	/*Feel free to change the templates as needed*/
	/*insert code here*/

	FILE *fp;

	if(path !=NULL){ // read from file
		fp = fopen(path,"r");
		if(fp==NULL){
			perror("Failed to open file");
			return ;
		}

		read_helper(mode,fp);
		fclose(fp);
	}
	else{ // read from STDIN
		read_helper(mode,stdin);

	}


}

int main(int argc, char** argv){
	if(argc>2){
		if(strcmp(argv[1], "-l") == 0) { 
			wc(1, argv[2]);
		} else if(strcmp(argv[1], "-w") == 0) { 
			wc(2, argv[2]);
		} else if(strcmp(argv[1], "-c") == 0) { 
			wc(3, argv[2]);
		} else {
			printf("Invalid arguments\n");
		}
	} else if (argc==2){
	 	if(strcmp(argv[1], "-l") == 0) { 
			wc(1, NULL);
		} else if(strcmp(argv[1], "-w") == 0) { 
			wc(2, NULL);
		} else if(strcmp(argv[1], "-c") == 0) { 
			wc(3, NULL);
		} else {
    		wc(0, argv[1]);
    	}
  	} else {
  		wc(0,NULL);
  	}

	return 0;
}