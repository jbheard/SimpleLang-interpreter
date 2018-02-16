#ifndef BRAINFUCKPP_H
#define BRAINFUCKPP_H	

// Stupid windows with their stupid winsock, making me do extra work
#ifdef __WIN32__
	#include <winsock.h>
#else
	#define INVALID_SOCKET 	-1
	#define SOCKET_ERROR 	-1

	// We will just use SOCKET for everything to make the code a bit more portable
	typedef int SOCKET;
#endif

#define BF_ARRAY_SIZE 32768 // 2^15 bytes usable data space
#define NUM_BYTES	  1024  // Read up to 1024 bytes at a time in the console

char open_client(SOCKET*, char*, int);
char open_server(SOCKET*, SOCKET*, int);
void close_sock(SOCKET);
void send_sock(SOCKET, char);
char recv_sock(SOCKET);


#endif // BRAINFUCKPP_H
