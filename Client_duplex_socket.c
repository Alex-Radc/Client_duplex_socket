#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_BUF_LEN 500
#define SIZE 10
#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS 0
#define PORT 6490
#define MYPORT 6950

//int initial_client();
void create_client(void *id);
int initial_send();
int initial_receive(int *conn,int *sockfd, int local_id);
void send_to_server(void *id);
void receive_from_server(int conn, int sockfd);
void error_print(char *ptr);

int global_socket = 0;
///////////////////////////////////////////////////////////////////////////////////////
int main(void)
{
	long int i = 0;
	pthread_t thread[SIZE];
	int status[SIZE] = {0};
	int status_addr = 0;

	global_socket = initial_send();

	for(i = 0; i < SIZE; i++)
	{
		printf("i = %ld\n", i);
		status[i] = pthread_create(&thread[i], NULL, (void*)create_client, (void*)i);
		if (status[i] != 0)
		{
			printf("main error: can't create thread, status = %d\n", status[i]);
			//exit(ERROR_CREATE_THREAD);
		}

		sleep(1);
	}

	for(i = 0; i < SIZE; i++)
	{
		status[i] = pthread_join(thread[i], (void**)&status_addr);
		if (status[i] != SUCCESS)
		{
			printf("main error: can't join thread, status = %d\n", status[i]);
			//exit(ERROR_JOIN_THREAD);
		}
	}

	close(global_socket);/*Communication ends, close the socket*/

	return 0;
}
/////////////////////////////
void create_client(void *id)
{
	int sockfd = 0;
	int conn = 0;
	int local_id = (long int)id;

	//pid_t pid;
	//pid = fork();
	printf("LOCAL_ID from create_client() = %d\n",local_id);
	//if (pid == -1)
	//{
	//	error_print("fork");
	///}

	//if (pid == 0)

		// receive information
	//initial_receive(&conn, &sockfd, local_id);


	pthread_t thread;
	int status = 0;
	//int status_addr = 0;

	status = pthread_create(&thread, NULL, (void*)send_to_server, (void*)id);
	if (status != 0)
	{
		printf("main error: can't create thread, status = %d\n", status);
		//exit(ERROR_CREATE_THREAD);
	}
	initial_receive(&conn, &sockfd, local_id);
	receive_from_server(conn, sockfd);
}
///////////////////////////////////////////////////////////////////////
/*Handle errors generated in system calls*/
void error_print(char *ptr)
{
	//perror(ptr);
	//exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////
/*Process the signal received by the callback function when the communication ends*/
void quit_tranmission(int sig)
{
	//printf("recv a quit signal = %d\n", sig);
	exit(EXIT_SUCCESS);
}
//////////////////////////////////////////////////////////////////////////
int initial_send()
{
	int sockfd = 0;
	struct sockaddr_in servaddr;
	int conn = 0;
	//printf("Timestamp: %d\n",(int)time(NULL));

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error_print("socket");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = PORT;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	printf("{{servaddr.sin_port}} = %d\n",servaddr.sin_port);
	if ((conn = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)))< 0)
		error_print("connect");

	return sockfd;
}
///////////////////////////////////////////////////////////////////////////
int initial_receive(int *conn,int *sockfd, int client_id)
{
		//printf("initial_receive(client) \n");
	    printf("-->Client_id = %d %d %d\n", *conn, *sockfd, client_id);

		//printf("Timestamp: %d\n",(int)time(NULL));
		int enable = 1;
		struct sockaddr_in servaddr;

		*sockfd = socket(AF_INET, SOCK_STREAM, 0);/*IPV4 streaming protocol is TCP protocol*/
		if (*sockfd < 0)
			error_print("socket");
		//struct sockaddr_in servaddr;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;/*IPV4*/
		servaddr.sin_port = MYPORT + client_id;//+(int)a;
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");/*Use the local loopback address for testing*/
		/*inet_aton("127.0.0.1",&servaddr.sin_addr);//Same function as inet_addr*/

		/*setsockopt ensures that the server can restart the server without waiting for the end of the TIME_WAIT state, and continue to use the original port number*/
		//////////////////////////////////////////////////////////////////////////
		if(setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		{
			perror("setsockopt(SO_REUSEADDR) failed");
		}
		////////////////////////////////////////////////////////////////////////////
		printf("-->Client_id =%d\n",client_id);
		/*Bind local Socket address*/
		if (bind(*sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
			error_print("bind ");

		/*Monitor connection*/
		if (listen(*sockfd, SOMAXCONN) < 0)
			error_print("listen");

		struct sockaddr_in peeraddr;/*Store the client Socket information of a successful connection*/
		socklen_t peerlen = sizeof(peeraddr);

		/*Receive the first connection request from the listening queue*/
		printf("BEFORE ACCEPT\n");
		if ((*conn = accept(*sockfd, (struct sockaddr*) &peeraddr, &peerlen)) < 0)
			error_print("accept");
		printf("PORT from Client = %d\n",servaddr.sin_port);
		return 0;
}
/////////////////////////////////////////
void send_to_server(void *id)
{
	int i = 0;
	struct timespec mt1;
	char recv_buf[MAX_BUF_LEN] = {0};
	printf("send_to_server: Timestamp: %d\n",(int)time(NULL));

	if (3 > global_socket)
	{
		printf("invalid input param\n");
		return;
	}
	//signal(SIGUSR1, quit_tranmission);/*Callback function to handle communication interruption*/
	for(i = 0; i < 1; i++)
	{
		char buf0[20] = {'\0'};
		char buf1[20] = {'\0'};
		char buf2[20] = {'\0'};
		char buf3[20] = {'\0'};
		char buf4[20] = {'\0'};

		clock_gettime (CLOCK_REALTIME, &mt1);
		sscanf(ctime(&mt1.tv_sec), "%s %s %s %s %s", buf0, buf1, buf2, buf3, buf4);

		//sscanf(ctime(&mt1.tv_sec), "%s %s %s %s %s", NULL, NULL, NULL, buf3, NULL);
		sprintf(recv_buf,"Client %ld %s:%ld\n", (long int)id, buf3, mt1.tv_nsec/1000);

		printf("<<send_to_server>>> = %s\n",recv_buf);

		int set = write(global_socket, recv_buf, sizeof(recv_buf));/*Send the data in the send_buf buffer to the peer server*/
		if (set < 0)
			error_print("write");
		//fsync(global_socket);
		bzero(recv_buf, strlen(recv_buf));
	}
}
////////////////////////////
void receive_from_server(int conn, int sockfd)
{
	//printf("receive_from_server()\n");
	//printf("Timestamp: %d\n",(int)time(NULL));
	char recv_buf[MAX_BUF_LEN] = { 0 };
	while (1)
	{
		bzero(recv_buf, strlen(recv_buf));
		int ret = read(conn, recv_buf, sizeof(recv_buf));//Read data sent by conn connection
		printf("recv_buf = %s\n", recv_buf);
		if (ret < 0)
			error_print("read");
		else if (ret == 0)
		{
			printf("server is close!\n");
			break; //The parent process receives the server-side exit information (the server Ctrl+C ends the communication process, the read function returns a value of 0, and the loop exits)
		}
		fputs(recv_buf, stdout);
	}
}
//////////////////////////////////////////////////////////////////

