Matthew Toro
CS 372 Computer Networking
Project 1
Due Date: 5/6/2018

RUNNING CHATSERVER.PY:
I wrote a tcsh shell script named chatserve to start the server. Just run "chatserve 'port_number'" replacing 'port_number' with your desired number.
It will run the python interpreter to start chatserver.py.

RUNNING CHATCLIENT.C:
I have included a makefile to compile the program.
To compile the client, run 'make chatclient'. 
To run the program, type "chatclient 'host_name' 'port_number'" where host_name is where chatserver is running and port number is the port number of chatserver.

For my testing, I ran my server on flip3.engr.oregonstate.edu using port 30023 and my client on flip1.engr.oregonstate.edu.

My programs are set up to alternate messages back and forth. When you run chatclient, it will prompt you for your handle.
Then, it will connect to the server and prompt you for a message. Type your message and press enter to send it. 
The server will print out the message and prompt for a reply. It will send the reply to the client and the client can then respond back.
Either the server or the client can send '\quit' and the connection will be terminated.