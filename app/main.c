#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

void mk_dir(char *temp_dir_path, char *command_path){
	for (int i = strlen(temp_dir_path) + 1; command_path[i] != '\0'; i++){
		if (command_path[i] == '/'){
			command_path[i] = '\0';
			if (mkdir(command_path, 0777) < 0){
			    printf("cannot make directory %s\n", command_path);
			    perror("mkdir");
			}
			command_path[i] = '/';
		}
	}
}

void copy_files(char *from_path, char *to_path){
  	int  from;
  	int  to;
  	char buffer[4096];
  	int  ret;

  	from = open(from_path, O_RDONLY);
  	if (from < 0){
  	    perror("error open from path");
  		exit(1);
  	}

  	to = open(to_path, O_WRONLY | O_CREAT | O_EXCL, 0777);
  	if (to < 0){
  	    perror("error open to path");
  		exit(1);
  	}

  	while ((ret = read(from, buffer, 4096)) > 0){
  		if (ret < 0 || write(to, buffer, ret) < 0){
  		    perror("error write");
  	        exit(1);
  		}
  	}

  	close(from);
  	close(to);
}

void createDirContainer(char *command){
      char temp_dir_path[] = "/tmp/mydocker";
      char command_path[4096];
      int n = mkdir(temp_dir_path, 0777);
      if(n < 0) {
          printf("Cannot create temp directory.\n");
          exit(1);
      }

      sprintf(command_path, "%s%s", temp_dir_path, command);
      mk_dir(temp_dir_path, command_path);
      copy_files(command, command_path);
}

// Usage: your_docker.sh run <image> <command> <arg1> <arg2> ...
int main(int argc, char *argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	//printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage

	 char *command = argv[3];
	 int child_pid = fork();
	 if (child_pid == -1) {
	     printf("Error forking!");
	     return 1;
	 }

	 if (child_pid == 0) {
	 	   // Replace current program with calling program
	 	 char temp_dir_path[] = "/tmp/mydocker";
	 	 createDirContainer(command);

	 	 if(chroot(temp_dir_path) != 0) {
            perror("chroot");
            exit(1);
         }

	     execv(command, &argv[3]);
	     perror("ERR: execv");
	 } else {
	 	   // We're in parent
	 	    int status;
	 	    waitpid(child_pid, &status, 0);
            if (WIFEXITED(status)){
                return WEXITSTATUS(status);
            }
	 	  // printf("Child terminated");
	 }

	return 0;
}
