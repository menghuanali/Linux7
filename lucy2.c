#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#if defined(_GNU_LIBRARY_) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
      int val;                  /* value for SETVAL */
      struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      unsigned short *array;    /* array for GETALL, SETALL */
                                /* Linux specific part: */
      struct seminfo *_buf;    /* buffer for IPC_INFO */
};
#endif

#define PROJID 0xFF
int semid;

void terminate_handler(int signo)
{
	semctl(semid, 0, IPC_RMID);
	exit(0);
}

int main(void)
{
	char filenm[] = "shared-file";
	char zero_blk[4096];
	char * mmap_addr;
	int fd;
	key_t semkey;
	struct sembuf getsem, setsem;
	union semun seminit;
	int ret;
	semkey = ftok(filenm, PROJID);
	if (semkey == -1) {
		perror("ftok error: ");
		exit(-1);
	}
	semid = semget(semkey, 2, IPC_CREAT | IPC_EXCL | 0666);
	if (semid == -1) {
		perror("semget error: ");
		exit(-1);
	}
	seminit.val = 0;
	semctl(semid, 0, SETVAL, seminit);
	semctl(semid, 1, SETVAL, seminit);
	getsem.sem_num = 1;
    getsem.sem_op = -1;
    getsem.sem_flg = SEM_UNDO;

	setsem.sem_num = 0;
    setsem.sem_op = 1;
    setsem.sem_flg = SEM_UNDO;

	signal(SIGINT, terminate_handler);
	signal(SIGTERM, terminate_handler);

	memset(zero_blk, 0, 4096);
	fd = open(filenm, O_RDWR | O_CREAT);
        if (fd == -1) {
		perror("open error: ");
		semctl(semid, 0, IPC_RMID);
		exit(-1);
	}
write(fd, zero_blk, 4096);
mmap_addr = (char *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if (mmap_addr == (char *)-1) {
	perror("mmap error: ");
	semctl(semid, 0, IPC_RMID);
	close(fd);
	exit(-1);
}
	while (1) {
		printf("Lucy: ");
		fgets(mmap_addr, 256, stdin);
		if (strncmp("quit", mmap_addr, 4) == 0) {
			if (munmap(mmap_addr, 4096) == -1) {
				perror("munmap error: ");
			}
			close(fd);
			semctl(semid, 0, IPC_RMID);
			exit(0);
		}
		mmap_addr[strlen(mmap_addr)-1] = '\0';
		ret = semop(semid, &setsem, 1);
		if (ret == -1) {
			perror("semop error: ");
		}
		ret = semop(semid, &getsem, 1);
		if (ret == -1) {
			perror("semop error: ");
		}
		printf("Peter: %s\n", mmap_addr);
	}
}
