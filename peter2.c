#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdlib.h>

#define PROJID 0xFF
int main(void)
{
	char filenm[] = "shared-file";
	char * mmap_addr;
	int fd, semid;
	key_t semkey;
	struct sembuf getsem, setsem;

	semkey = ftok(filenm, PROJID);
	if (semkey == -1) {
		perror("ftok error: ");
		exit(-1);
	}

	semid = semget(semkey, 0, 0);
	if (semid == -1) {
		perror("semget error: ");
		exit(-1);
	}

	getsem.sem_num = 0;
        getsem.sem_op = -1;
        getsem.sem_flg = SEM_UNDO;

	setsem.sem_num = 1;
        setsem.sem_op = 1;
        setsem.sem_flg = SEM_UNDO;

	fd = open(filenm, O_RDWR | O_CREAT);
        if (fd == -1) {
		perror("open error: ");
		semctl(semid, 0, IPC_RMID);
		exit(-1);
	}
	mmap_addr = (char *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mmap_addr == (char *)-1) {
		perror("mmap error: ");
		close(fd);
		exit(-1);
	}

	while (1) {
		semop(semid, &getsem, 1);
		printf("Lucy: %s\n", mmap_addr);
		printf("Peter: ");
		fgets(mmap_addr, 256, stdin);
		if (strncmp("quit", mmap_addr, 4) == 0) {
			mmap_addr[strlen(mmap_addr)-1] = '\0';
			semop(semid, &setsem, 1);
			if (munmap(mmap_addr, 4096) == -1) {
				perror("munmap error: ");
			}
			close(fd);
			exit(0);
		}
		mmap_addr[strlen(mmap_addr)-1] = '\0';
		semop(semid, &setsem, 1);
	}
		
}
