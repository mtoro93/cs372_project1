# Name: Matthew Toro
# Class: CS 372 Computer Networking
# Assignment: Project 1
# Due Date: 5/6/2018
# Description: A server for hosting chat between a client and itself

import sys
import signal
from socket import *

# Name: signal_handler
# Parameters: signal to catch, function to run
# Output: print to console
# Description: Signal Handler to catch SIGINT commands to shutdown the server and output text
# reference: https://stackoverflow.com/questions/1112343/how-do-i-capture-sigint-in-python
# reference: https://docs.python.org/2.7/library/signal.html
def signal_handler(signal, frame):
	print('SIGINT received, server shutting down')
	sys.exit(0)

# Name: startUpServer
# Parameters: string for host name, integer for port number, and socket for server
# Output: server socket is updated
# Description: Uses passed in host name and port number to bind the socket and start listening for a connection;
# 		borrowed from CS 372 Lecture 15
def startUpServer(serverHostName, serverPortNumber, serverSocket):
	serverSocket.bind((serverHostName, serverPortNumber))
	serverSocket.listen(1)
	print "Server started on host: " + serverHostName + " and port: " + str(serverPortNumber)

	
# Name: sendMessage
# Parameters: socket for client connection and string for the message to send 
# Output: message is sent
# Description: Sends the passed in server message across the connection socket;
# 		ensures we send all the bytes by looping until all is sent;
# 		reference: https://docs.python.org/2/howto/sockets.html#socket-howto
def sendMessage(connectionSocket, serverMessage):
	currentBytes = 0
	while currentBytes < len(serverMessage):
		bytesSent = connectionSocket.send(serverMessage[currentBytes:])
		currentBytes += bytesSent;

# Name: receiveMessage
# Parameters: socket for client connection and string for the received message
# Output: client's message in a string
# Description: Receives a message from the connection socket into the passed in parameter;
# 		Loops for the end of message sequence '@@' to signify the message has been received
def receiveMessage(connectionSocket, clientMessage):
	while clientMessage.find("@@") == -1:
		readBuffer = connectionSocket.recv(1024)
		clientMessage+=readBuffer
	return clientMessage

	
# Name: shutdownConnection
# Parameters: socket for client connection
# Output: shutdowns socket and returns false to end the inner loop in the script
# Description: function to shut down the connection socket;
# 		0 argument in shutdown() says the server is done receiving data, 
# 		and will shutdown once sending is finished
def shutdownConnection(connectionSocket):
	print "Connection terminated, listening for new connection"
	connectionSocket.shutdown(0)
	connectionSocket.close()
	return False

# BEGIN SCRIPT:
# set up variables
signal.signal(signal.SIGINT, signal_handler)
hostName = 'HOST_A> '
connectionOn = False
serverPort = int(sys.argv[1])
serverHost = gethostname()
serverSocket = socket(AF_INET, SOCK_STREAM)
startUpServer(serverHost, serverPort, serverSocket)
#outer loop keeps the server running
while 1:
	connectionSocket, addr = serverSocket.accept()
	connectionOn = True;
	print "Connection received, awaiting initial client message"
	#inner loop keeps the connection running
	while connectionOn:
		clientMessage = ""
		clientMessage = receiveMessage(connectionSocket, clientMessage)
		#check for \quit here
		#if \quit is true, send \quit back and close the connection
		#else, print message, get input, and send
		if clientMessage.find("\quit") != -1:
			quitMessage = hostName + "\quit@@"
			sendMessage(connectionSocket, quitMessage)
			connectionOn = shutdownConnection(connectionSocket)
		else:
			# get rid of @@ terminating sequence
			clientMessage = clientMessage.replace("@@", " ")
			print clientMessage
			userInput = raw_input(hostName)
			serverMessage = hostName + userInput + "@@"
			sendMessage(connectionSocket, serverMessage)
			# check for server side \quit here
			if serverMessage.find("\quit") != -1:
				connectionOn = shutdownConnection(connectionSocket)