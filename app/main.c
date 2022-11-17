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

void copy_files(char *from, char *to){
    int src_fd, dst_fd, out ,in;
    unsigned char buffer[4096];


    src_fd = open(from, O_RDONLY);
    dst_fd = open(to, O_CREAT | O_WRONLY);

    if(src_fd < 0 || dst_fd < 0){
        printf("Error opening file.\n");
        perror("open");
        exit(1);
    }


    while(1){
        in = read(src_fd, buffer, 4096);
        if(in == -1){
            printf("Error reading file.\n");
            perror("read");
            exit(1);
        }

        if(in == 0) break;

        out = write(dst_fd, buffer, in);

        if (out == -1) {
           printf("Error writing to file.\n");
           perror("write");
           exit(1);
        }
    }

    close(src_fd);
    close(dst_fd);
}

void createDirContainer(char *command){
      char temp_dir_path[] = "/tmp/tmpdir.XXXXXX";
      char command_path[4096];
      char *temp_dir = mkdtemp(temp_dir_path);
      if(temp_dir == NULL) {
          printf("Cannot create temp directory.\n");
          exit(1);
      }

      sprintf(command_path, "%s%s", temp_dir_path, command);
      mk_dir(temp_dir_path, command_path);
      copy_files(command, command_path);
      chroot(temp_dir_path);
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
	 	 createDirContainer(command);
	     execv(command, &argv[3]);
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
