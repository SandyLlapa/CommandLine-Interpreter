#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include<string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

#define PATH_MAXLS 1024

void list_directory(char *path, bool recurse_flag) {
    DIR *dir;
    struct dirent *entry;
    struct stat info;

    // Open the directory
    if ((dir = opendir(path)) == NULL) {
        perror("opendir"); // Print error message if opendir fails
        exit(EXIT_FAILURE);
    }

    // Print the current directory
    printf("In directory: %s\n", path);

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Build the full path for each entry
        char full_path[PATH_MAXLS];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Skip the current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Get information about the entry
        if (stat(full_path, &info) != 0) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        // If the entry is a directory and recursion flag is set, make a recursive call
        if (recurse_flag && S_ISDIR(info.st_mode)) {
            // Recursive call for subdirectories
            list_directory(full_path, true);
        }

        printf("%s\n", entry->d_name); // Print name of the entry
    }

    closedir(dir); // Close directory
}

void ls(char *path, bool recurse_flag) {
	/* Insert code here*/
	if (path == NULL) {
        // No path provided, use the current working directory
        char cwd[PATH_MAXLS];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
        list_directory(cwd, recurse_flag); // Call list_directory with the current working directory
    	} else {
        list_directory(path, recurse_flag); // Call list_directory with the provided path
    	}
}

int main(int argc, char *argv[]){
	if(argc < 2){ // No -R flag and no path name
		ls(NULL, false);
	} else if(strcmp(argv[1], "-R") == 0) { 
		if(argc == 2) { // only -R flag
			ls(NULL, true);
		} else { // -R flag with some path name
			ls(argv[2], true);
		}
	}else { // no -R flag but path name is given
    	ls(argv[1], false);
  }
	return 0;
}
