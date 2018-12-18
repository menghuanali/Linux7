#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
        int pipe1_fd[2], pipe2_fd[2];

        char parent_buf[256], child_buf[256];
        int len;
        int child_status;
 
        if(pipe(pipe1_fd)<0)
        {
                printf("pipe1 create error\n");
                return -1;
        }

        if(pipe(pipe2_fd)<0)
        {
                printf("pipe2 create error\n");
                return -1;
        }

        if(fork()==0)   //child
        {
                printf("\n");

                //pipe1_fd[0] is used to read, pipe2_fd[1] is used to write
                close(pipe1_fd[1]);
                close(pipe2_fd[0]);

                while(1)
                {
                        len = read(pipe1_fd[0], child_buf, 255);
                        child_buf[len] = '\0';
                        printf("peter_Lucy: %s\n", child_buf);
	                			printf("Peter: ");
	                			fgets(child_buf, 256, stdin);
												child_buf[strlen(child_buf)-1] = '\0';
	                			if (strncmp(child_buf,"quit", 4) == 0) {
		                			close(pipe1_fd[0]);
		                			close(pipe2_fd[1]);
	                        exit(0);
	                }
							write(pipe2_fd[1], child_buf, strlen(child_buf));
							sleep(1);
		}
                close(pipe1_fd[0]);
                close(pipe2_fd[1]);
                exit(0);
    }
        else {  //parent

                //pipe1_fd[1] is used to write, pipe2_fd[0] is used to read
                close(pipe1_fd[0]);
                close(pipe2_fd[1]);

                while (1) {
			printf("Lucy: ");
			fgets(parent_buf, 256, stdin);
			if (strncmp(parent_buf,"quit", 4) == 0) {
			        close(pipe1_fd[1]);
			        close(pipe2_fd[0]);
	                        exit(0);
			}
			write(pipe1_fd[1],parent_buf,strlen(parent_buf));
			sleep(1);
                        len = read(pipe2_fd[0],parent_buf,255);
                        parent_buf[len] = '\0';
                        printf("Lucy_peter: %s\n", parent_buf);
                }

        close(pipe1_fd[1]);
        close(pipe2_fd[0]);
        wait(&child_status);
        exit(0);
        }
}
