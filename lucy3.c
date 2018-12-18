#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#define PROJID 0xFF
#define SNDMSG 1
#define RCVMSG 2
int mqid;
void terminate_handler(int signo)
{
	msgctl(mqid, IPC_RMID, NULL);
	exit(0);
}
int main(void)
{
	char filenm[] = "shared-file";
	key_t mqkey;
	struct msgbuf {
        	long mtype;     /* message type, must be > 0 */
            char mtext[256];  /* message data */
   }msg;
	int ret;
	mqkey = ftok(filenm, PROJID);
	if (mqkey == -1) {
		perror("ftok error: ");
		exit(-1);
	}
	mqid = msgget(mqkey, IPC_CREAT | IPC_EXCL | 0666);
	if (mqid == -1) {
		perror("msgget error: ");
		exit(-1);
	}
	signal(SIGINT, terminate_handler);
	signal(SIGTERM, terminate_handler);
	while (1) {
		printf("Lucy: ");
		fgets(msg.mtext, 256, stdin);
		if (strncmp("quit", msg.mtext, 4) == 0) {
			msgctl(mqid, IPC_RMID, NULL);
			exit(0);
		}
		msg.mtext[strlen(msg.mtext)-1] = '\0';
		msg.mtype = SNDMSG;
		msgsnd(mqid, &msg, strlen(msg.mtext) + 1, 0);
		msgrcv(mqid, &msg, 256, RCVMSG, 0);
		printf("Peter: %s\n", msg.mtext);	
	}	
}
