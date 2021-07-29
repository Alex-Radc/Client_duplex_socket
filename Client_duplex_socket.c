#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>

#define MAX_BUF_LEN 128
#define SIZE 10
#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

/*Handle errors generated in system calls*/
void error_print(char *ptr)
{
	perror(ptr);
	exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////
/*Process the signal received by the callback function when the communication ends*/
void quit_tranmission(int sig)
{
	printf("recv a quit signal = %d\n", sig);
	exit(EXIT_SUCCESS);
}
//////////////////////////////////////////////////////////////////////////
void create_client(void *a)
{
	int sockfd = 0;
	struct sockaddr_in servaddr;
	int conn = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error_print("socket");
	//struct sockaddr_in servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = 1334 + (int)a;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*inet_aton("127.0.0.1",&servaddr.sin_addr);*/
	//int conn;
	if ((conn = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)))< 0)
		error_print("connect");

	pid_t pid;
	pid = fork();
	if (pid == -1)
	{
		error_print("fork");
	}

	if (pid == 0)
	{
		char recv_buf[MAX_BUF_LEN] ={0};
		while (1)
		{
			bzero(recv_buf, sizeof(recv_buf));
			int ret = read(sockfd, recv_buf, sizeof(recv_buf));
			if (ret == -1)
				error_print("read");
			else if (ret == 0)
			{
				printf("server is close!\n");
				break; //The child process receives the server-side exit information (the server Ctrl+C ends the communication process, the read function returns a value of 0, and the loop exits)
			}
			fputs(recv_buf, stdout);/*Output the received information to the standard output stdout*/
		}
		close(sockfd);/*The child process exits and the socket is closed when the communication ends*/
		kill(getppid(), SIGUSR1);/*When the child process ends, a signal must also be sent to the parent process to tell the parent process to terminate the reception, otherwise the parent process will always wait for input*/
		exit(EXIT_SUCCESS);/*The child process exits normally and returns to the parent process EXIT_SUCCESS*/
	}
	else
	{
		signal(SIGUSR1, quit_tranmission);/*Callback function to handle communication interruption*/
		char send_buf[MAX_BUF_LEN] =
		{	0};
		/*If the server Ctrl+C ends the communication process, what fgets gets is NULL, otherwise it enters the loop and sends data normally*/
		while (fgets(send_buf, sizeof(send_buf), stdin) != NULL)
		{
			int set = write(sockfd, send_buf, strlen(send_buf));/*Send the data in the send_buf buffer to the peer server*/
			if (set < 0)
				error_print("write");
			bzero(send_buf, strlen(send_buf));
		}
		close(sockfd);/*Communication ends, close the socket*/
	}

}
///////////////////////////////////////////////////////////////////////
int main(void)
{
	int i = 0;
	pthread_t thread[SIZE];
	int status[SIZE] = {0};
	int status_addr = 0;

	for(i = 0; i < SIZE; i++)
	{
		status[i] = pthread_create(&thread[i], NULL, (void*)create_client, (void*)i) ;
		if (status[i] != 0)
		{
			printf("main error: can't create thread, status = %d\n", status[i]);
			exit(ERROR_CREATE_THREAD);
		}
		status[i] = pthread_join(thread[i], (void**)&status_addr);
		if (status[i] != SUCCESS)
		{
			printf("main error: can't join thread, status = %d\n", status[i]);
			exit(ERROR_JOIN_THREAD);
		}
	}
	return 0;
}
