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
int main(void)
{
	char filenm[] = "shared-file";
	int mqid;
	key_t mqkey;
	struct msgbuf {
        	long mtype;    
                char mtext[256];  
        }msg;
	int ret;
	mqkey = ftok(filenm, PROJID);
	if (mqkey == -1) {
		perror("ftok error: ");
		exit(-1);
	}
	mqid = msgget(mqkey, 0);
	if (mqid == -1) {
		perror("msgget error: ");
		exit(-1);
	}
	while (1) {
		msgrcv(mqid, &msg, 256, SNDMSG, 0);
		printf("Lucy: %s\n", msg.mtext);
		printf("Peter: ");
		fgets(msg.mtext, 256, stdin);
		if (strncmp("quit", msg.mtext, 4) == 0) {
			exit(0);
		}
		msg.mtext[strlen(msg.mtext)-1] = '\0';
		msg.mtype = RCVMSG;
		msgsnd(mqid, &msg, strlen(msg.mtext) + 1, 0);
	}	
}
