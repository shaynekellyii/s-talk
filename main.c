/***************************************************************
 * S-talk                                                      *
 * Author: Shayne Kelly II                                     *
 * Date: June 7, 2017                                          *
 ***************************************************************/

/***************************************************************
 * Imports                                                     *
 ***************************************************************/
#include "list.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/***************************************************************
 * Defines                                                     *
 ***************************************************************/
#define BUF_SIZE	512
#define TERMINAL_FD	0
#define NUM_THREADS	4

/***************************************************************
 * Statics                                                     *
 ***************************************************************/
static LIST *sendList;
static LIST *receiveList;
static char buffer[BUF_SIZE];
static char rcvBuffer[BUF_SIZE];
static pthread_mutex_t mutex;
static pthread_cond_t msg_avail_to_send;
static pthread_cond_t rcvd_msg_avail;
static char *hostName;
static int localPort;
static int remotePort;
static struct sockaddr_in local_addr, remote_addr;
static int sock;
static pthread_t keyboardThread, terminalThread, sendThread, receiveThread;
static int ready = 0;

/***************************************************************
 * Global Functions                                            *
 ***************************************************************/

int main(int argc, char *argv[]) {
	char *keyboardMessage = "Keyboard thread";
	char *terminalMessage = "Terminal thread";
	char *sendMessage = "Send thread";
	char *receiveMessage = "Receive thread";

	printf("\n***************************************************************\n");
	printf("* Welcome to s-talk! Setting up your session now...           *\n");
	printf("***************************************************************\n\n");

	/* Get network info from terminal args */
	if (argc < 3) {
		printf("Invalid amount of arguments supplied\n");
		exit(0);
	}
	hostName = argv[2];
	localPort = atoi(argv[1]);
	remotePort = atoi(argv[3]);

	/* Create lists */
	sendList = ListCreate();
	receiveList = ListCreate();

	/* Create mutex and condition vars */
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&msg_avail_to_send, NULL);
	pthread_cond_init(&rcvd_msg_avail, NULL);

	/* Open socket */
	InitSocket();

	/* Create threads */
	pthread_create(&keyboardThread, NULL, AcceptKeyboardInput, 
		(void *)keyboardMessage);
	pthread_create(&terminalThread, NULL, PrintMessages,
		(void *)terminalMessage);
	pthread_create(&sendThread, NULL, SendToSocket,
		(void *)sendMessage);
	pthread_create(&receiveThread, NULL, ReceiveFromSocket,
		(void *)receiveMessage);

	/* Join threads after they're done */
	pthread_join(keyboardThread, NULL);
	pthread_join(terminalThread, NULL);
	pthread_join(sendThread, NULL);
	pthread_join(receiveThread, NULL);

	exit(0);
}

/**
 * Initiates the socket, along with local and remote sockaddr_in structs.
 */
void InitSocket() {
	struct hostent *host;

	printf("INIT: Setting up network connection...\n");
	/* Create socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		printf("INIT: ERROR - Couldn't open socket.\n");
		exit(0);
	}

	/* Setup remote address sockaddr_in struct */
	memset((char *)&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(remotePort);

	/* Setup remote hostname */
	host = gethostbyname(hostName);
	if (!host) {
		printf("INIT: ERROR - Couldn't get remote address by name.\n");
		exit(0);
	}
	memcpy((void *)&remote_addr.sin_addr, host->h_addr_list[0], host->h_length);

	/* Setup local address sockaddr_in struct */
	memset((char *)&local_addr, 0, sizeof(local_addr));
	local_addr.sin_port = htons(localPort);
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&local_addr.sin_zero, '\0', 8);

	/* Bind socket */
	if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in)) < 0) {
		printf("INIT: ERROR - Binding socket failed.\n");
		exit(0);
	}
	printf("INIT: Ready to initiate network connection.\n");
}

/**
 * Thread function that accepts input from the terminal.
 * Adds terminal input to a shared list and signals the send thread.
 */
void *AcceptKeyboardInput(void *ptr) {
	ptr = (char *)ptr; /* Unused function param required by pthread functions */

	printf("INIT: Setting up keyboard input...\n");
	/* Need to set I/O to non-blocking for user-level threads */
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

	/* Infinite loop waiting for keyboard input */
	printf("INIT: Ready for keyboard input.\n");
	CheckIfThreadsReady();
	while (1) {
		/* I/O read is non-blocking, must check buffer is non-empty before continuing */
		do {
			read(TERMINAL_FD, buffer, BUF_SIZE);
		} while (buffer[0] == '\0');

		/* Add the message to the shared list of messages to be sent */
		char *newStr = malloc(strlen(buffer) + 1);
		strcpy(newStr, buffer);
		pthread_mutex_lock(&mutex);
		ListAdd(sendList, newStr);
		pthread_mutex_unlock(&mutex);

		/* Signal send thread that a user-typed message is added to the list to send */
		pthread_cond_signal(&msg_avail_to_send);

		/* Clear the buffer for the next non-blocking read calls */
		memset(buffer, 0, BUF_SIZE);
	}

	return NULL;
}

/**
 * Thread function to print messages received from the remote user to the terminal.
 */
void *PrintMessages(void *ptr) {
	ptr = (char *)ptr; /* Unused function param required by pthread functions */
	printf("INIT: Setting up output for received messages...\n");
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
	printf("INIT: Ready to output received messages.\n");
	CheckIfThreadsReady();

	/**
	 * Infinite loop waiting for messages to be received.
	 * Receive thread will signal this thread when messages from the list are available to print.
	 */
	while (1) {
		/* Lock the mutex and wait on the received message condition variable */
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&rcvd_msg_avail, &mutex);

		/* Print out all the received messages on the list (there may be multiple) */
		while (ListCount(receiveList) > 0) {
			char *writeStr = (char *)ListTrim(receiveList);
			write(TERMINAL_FD, writeStr, strlen(writeStr));
			printf("[Sent by %s]\n\n", hostName);

			/* Check if termination message was received */
			if (writeStr[0] == '!') {
				pthread_mutex_unlock(&mutex);
				printf("%s quit the chat.\n", hostName);
				die();
			}
		}

		/* Release the mutex when done printing messages */
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

/**
 * Thread function that sends messages typed by the user to the remote connection.
 * Waits on condition var signaled by keyboard thread before sending a message.
 */
void *SendToSocket(void *ptr) {
	ptr = (char *)ptr; /* Unused function param required by pthread functions */
	printf("INIT: Setting up message sending to the remote connection...\n");
	printf("INIT: Ready to send messages to the remote connection.\n");
	CheckIfThreadsReady();

	/* Infinite loop, waits for messages to send from keyboard thread */
	while (1) {
		/* Lock mutex and wait on condition var signaled by keyboard thread */
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&msg_avail_to_send, &mutex);

		/* Clear out send list (there may be more than one message available) */
		while (ListCount(sendList) > 0) {
			char *sendStr = ListTrim(sendList);
			if (sendto(sock, sendStr, strlen(sendStr), 0, (struct sockaddr *)&remote_addr, 
					sizeof(struct sockaddr_in)) < 0) {
				printf("SEND: ERROR - Couldn't send to socket\n");
				die();
			}
			/* Check if termination message was sent */
			if (sendStr[0] == '!') {
				pthread_mutex_unlock(&mutex);
				die();
			}
			printf("[Sent by you]\n\n");
		}

		/* Unlock mutex after sending all available messages in the shared list */
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

/**
 * Thread function that receives messages from the remote connection.
 * Signals condition var for the thread that prints messages received to the terminal.
 */
void *ReceiveFromSocket(void *ptr) {
	printf("INIT: Setting up message reception from the remote connection...\n");
	int recvLen;
	socklen_t slen = sizeof(struct sockaddr_in);
	ptr = (char *)ptr; /* Unused function param required by pthread functions */
	printf("INIT: Ready to receive messages from the remote connection.\n");
	CheckIfThreadsReady();

	/* Infinite loop, waits to receive messages from UDP socket */
	while (1) {
		/* Wait to receive something from the socket */
		if ((recvLen = recvfrom(sock, rcvBuffer, BUF_SIZE, 0, (struct sockaddr *)&remote_addr, &slen)) == -1) {
			printf("RECV: ERROR - Failed to receive from socket\n");
			die();
		}

		/* Add the received message to the shared list to print to the terminal */
		pthread_mutex_lock(&mutex);
		char *newStr = malloc(strlen(rcvBuffer) + 1);
		strcpy(newStr, rcvBuffer);
		ListAdd(receiveList, newStr);
		memset(rcvBuffer, 0, BUF_SIZE);

		/* Unlock the mutex and signal the terminal thread that a message is available */
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&rcvd_msg_avail);
	}

	return NULL;
}

/**
 * Terminates the program because of an error or because the user requested it.
 */
void die() {
	printf("\n***************************************************************\n");
	printf("* Exiting s-talk...                                           *\n");
	printf("***************************************************************\n\n");

	/* Let other threads process any remaining messages */
	sleep(3);

	/* Cleanup  */
	pthread_cancel(keyboardThread);
	pthread_cancel(terminalThread);
	pthread_cancel(sendThread);
	pthread_cancel(receiveThread);

	pthread_cond_destroy(&msg_avail_to_send);
	pthread_cond_destroy(&rcvd_msg_avail);
	pthread_mutex_destroy(&mutex);

	close(sock);
	ListFree(sendList, FreeItem);
	ListFree(receiveList, FreeItem);

	exit(0);
}

void FreeItem(void *item) {
	free(item);
}

void CheckIfThreadsReady() {
	if (++ready == NUM_THREADS) {
		PrintReadyMessage();
	}
}

void PrintReadyMessage() {
	printf("\n***************************************************************\n");
	printf("* Ready to talk!                                              *\n");
	printf("* Type a message, then press enter to send.                   *\n");
	printf("* Type ! and press enter to quit.                             *\n");
	printf("***************************************************************\n\n");
}