/*
** Name: Matthew Toro
** Class: CS 372 Computer Networking
** Assignment: Project 1
** Due Date: 5/6/2018
** Description: A client program that connects to a server to send messages back and forth.
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>

// function prototypes
void error(const char * message, int exitValue);
void getInput(char * buffer, size_t bufferSize, char * message);
void deleteNewLineCharacter(char * x);
int initiateContact(int socketFD, struct sockaddr_in* serverAddress, struct hostent* serverHostInfo, char* hostName, int portNum);
void sendMessage(int socketFD, char * clientMessage);
void receiveMessage(int socketFD, char* readBuffer, char* serverMessage);
void freeCharMemory(char * clientMessage, char* userMessage, char* userHandle);

// global constants
const int MAX_CHARS = 2048;
const int STD_OUT = 1;


int main(int argc, char* argv[])
{
	// set up c-strings for later use
	char * clientMessage = malloc(MAX_CHARS * sizeof(char));
	memset(clientMessage, '\0', sizeof(clientMessage));
   
	char * userMessage = malloc(MAX_CHARS * sizeof(char));
	memset(userMessage, '\0', sizeof(clientMessage));
   
	char serverMessage[MAX_CHARS];
	char readBuffer[100];
	memset(serverMessage, '\0', sizeof(serverMessage));
	char * token;
	
	char * userHandle = malloc(MAX_CHARS * sizeof(char));
	size_t bufferSize = MAX_CHARS;
	
	// get the user's handle
	getInput(userHandle, bufferSize, "Enter your handle: ");
	strcat(userHandle, "> ");

	
	// set up the socket and connection
	int portNum = atoi(argv[2]);
	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));
	char * hostName = malloc(MAX_CHARS * sizeof(char));
	memset(hostName, '\0', sizeof(hostName));
	strcpy(hostName, argv[1]);
	struct hostent* serverHostInfo;
	
	int socketFD = initiateContact(socketFD,serverAddress, serverHostInfo, hostName, portNum);

	// continue to loop until '\quit' is received
	while (strcmp(serverMessage, "\\quit") != 0)
	{
		// get input from user and prepare it for sending
		// @@ is used as a terminating sequence to know when the message ends in case it gets segmented
		getInput(userMessage, bufferSize, userHandle);	
		strcpy(clientMessage, userHandle);
		strcat(clientMessage, userMessage);
		strcat(clientMessage, "@@\0"); 
		
		sendMessage(socketFD, clientMessage);
	   
		// reset the server's message and receive the new one
		memset(serverMessage, '\0', sizeof(serverMessage));
		receiveMessage(socketFD, readBuffer, serverMessage);
	   
		// get rid of terminating sequence '@@'
		int terminalLocation = strstr(serverMessage, "@@") - serverMessage;
		serverMessage[terminalLocation] = '\n';
		serverMessage[terminalLocation + 1] = '\0';
		
		//get rid of hostName> for /quit check
		char * temp = malloc(MAX_CHARS * sizeof(char));
		memset(temp, '\0', sizeof(temp));
		temp = strcpy(temp, serverMessage);
		token = strtok(temp, ">");
		token = strtok(NULL, " \n");
		
		// if the client sends \quit, the server will send \quit back and we exit the loop
		// if the server sends \quit, we exit the loop as well
		// otherwise, we print the server's message and repeat
		if (strcmp(token, "\\quit") != 0)
		{
			write(STD_OUT, serverMessage, strlen(serverMessage));
		}
		else
		{
			// copy '\quit' to end the loop; the last iteration [5] is for '\0'
			int i = 0;
			for (i = 0; i < 6; i++)
			   serverMessage[i] = token[i];
		}
		free(temp);
	}
	printf("Connection terminated. Thank you for chatting.\n");
	free(serverAddress);
	freeCharMemory(clientMessage,  userMessage,  userHandle);
}



/* 
** Name: getInput
** Parameters: c-string buffer for getting input
	size_t bufferSize for size of c-string
	c-string message for prompt
** Output: stores user input in message
** Description: function used to get input from user, loops in case a signal interrupts getline;
	borrowed from my CS 344 assignment 3 program
*/
void getInput(char * buffer, size_t bufferSize, char * message)
{
	while(1)
	{
		int numCharsEntered;
		write(STD_OUT, message, strlen(message)); fflush(stdout);
		numCharsEntered = getline(&buffer, &bufferSize, stdin);
		if (numCharsEntered == -1)
			clearerr(stdin);
		else
			break;
	}
   deleteNewLineCharacter(buffer);
}

/* 
** Name: error
** Parameters: c-string for printing message
	int for exit value
** Output: prints error message
** Description: prints errors to stderr and exits to passed in exit value;
	function borrowed from client.c from CS 344 lecture
*/
void error(const char* message, int exitValue)
{
   fprintf(stderr, message);
   exit(exitValue);
}

/* 
** Name: deleteNewLineCharacter
** Parameters: c-string x
** Output: c-string without newline character
** Description: helper function that deletes the inserted new line character from user input;
	function borrowed from my CS 344 assignment 2 program
*/
void deleteNewLineCharacter(char * x)
{
	x[strlen(x) - 1] = 0;
}

/* 
** Name: initiateContact
** Parameters: socket file descriptor, server address struct, 
	server host info struct, c-string for host name, int for port number
** Output: integer for connected socket file descriptor
** Description: does all the boilerplate code to set up a socket for connection;
	this code is borrowed from client.c from CS 344 lecture
*/
int initiateContact(int socketFD, struct sockaddr_in* serverAddress, struct hostent* serverHostInfo, char* hostName, int portNum)
{
	// set up the server address struct,
   serverAddress->sin_family = AF_INET;
   serverAddress->sin_port = htons(portNum);

   serverHostInfo = gethostbyname(hostName);
   if (serverHostInfo == NULL)
   {
	error("ERROR: no such host\n", 2);
   }

   memcpy((char*)&serverAddress->sin_addr.s_addr, (char*) serverHostInfo->h_addr, serverHostInfo->h_length);
  
   // create the socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0);
   if (socketFD < 0)
   {
	error("ERROR: error opening socket\n", 2);
   }

   // connect to the server
   if (connect(socketFD, (struct sockaddr*)serverAddress, sizeof((*serverAddress))) < 0)
   {
	error("ERROR: error  connecting\n", 2);
   }
	printf("Connected to server host: %s on port: %d\n", hostName, portNum);
   return socketFD;
}

/* 
** Name: sendMessage
** Parameters: socket file descriptor, c-string for client message
** Output: nothing unless an error occurs
** Description: sends a client message to the server as described in the socket file descriptor;
	borrowed from my CS 344 assignment 4 program
*/
void sendMessage(int socketFD, char * clientMessage)
{
   // set up the send loop, we want to ensure we send all the bytes to the server
   int totalBytes = (int) strlen(clientMessage);
   int currentBytes = 0;
   int leftoverBytes = totalBytes;
   while(currentBytes < totalBytes)
   {
		ssize_t bytesSent;
		bytesSent = send(socketFD, clientMessage + currentBytes, leftoverBytes, 0);
		if (bytesSent < 0)
		{
		   free(clientMessage);
		   error("ERROR: error writing to socket", 2);
		}
		currentBytes += (int) bytesSent;
		leftoverBytes -= (int) bytesSent;
   }
}

/* 
** Name: receiveMessage
** Parameters: socket file descriptor, c-strings for a buffer and the server's message
** Output: serverMessage
** Description: receives a message from the server as described in the socket file descriptor;
	borrowed from my CS 344 assignment 4 program
*/
void receiveMessage(int socketFD, char* readBuffer, char* serverMessage)
{
	int charsRead;
	// I use a terminating sequence of "@@" to know when the message is over if it gets segmented
	while(strstr(serverMessage, "@@") == NULL)
	{
		memset(readBuffer, '\0', sizeof(readBuffer));
		charsRead = recv(socketFD, readBuffer, sizeof(readBuffer), 0);
		if (charsRead < 0)
		{
		   fprintf(stderr, "ERROR: error receiving data\n");
		   break;
		}
		strcat(serverMessage, readBuffer);
	}
}

/* 
** Name: freeCharMemory
** Parameters: c-strings
** Output: nothing
** Description: // helper function to delete allocated memory
*/
void freeCharMemory(char * clientMessage, char* userMessage, char* userHandle)
{
	free(clientMessage);
	free(userMessage);
	free(userHandle);
}