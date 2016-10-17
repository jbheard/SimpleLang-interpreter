#ifndef BRAINFUCK_H
#define BRAINFUCK_H	

// Stupid windows with their stupid winsock, making me do extra work
#ifdef __WIN32__
	#include <windows.h>
	#include <winsock.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h> //inet_addr
	#include <netdb.h> //gethostbyname
	#include <netinet/in.h> //sockaddr_in
	#include <unistd.h> //close
	
	#define INVALID_SOCKET 	-1
	#define SOCKET_ERROR 	-1

	/* NOTE: In winsock LP**** just defines a FAR pointer, and for some reason 
	* 		Microsoft likes to capitalize things, so I have to do a bunch of 
	* 		stuff here to make the code portable.
	*/
	typedef int SOCKET;
	typedef struct sockaddr_in SOCKADDR_IN;
	typedef struct hostent* LPHOSTENT;
	typedef struct in_addr* LPIN_ADDR;
	typedef struct sockaddr* LPSOCKADDR;
#endif

#define BF_ARRAY_SIZE 32768 // 2^15 bytes usable data space
#define NUM_BYTES	  1024  // Read up to 1024 bytes at a time in the console


/* Opens a socket and connects to a given host on a given port
 * NOTE: Winsock code is from http://johnnie.jerrata.com/winsocktutorial/ and has 
 * 		 been modified for portability
 * @param s The socket to open
 * @param hostname The URL or IP of the host to connect to
 * @param portno The port to use when we connect
 * @return 0 if everything went well, -1 otherwise (char)
 */
char open_client(SOCKET* s, char* hostname, int portno)
{
	int ret;
#ifdef __WIN32__
	WORD sockVersion;
	WSADATA wsaData;
	
	sockVersion = MAKEWORD(1, 1);
	
	// Initialize Winsock
	WSAStartup(sockVersion, &wsaData);
#endif
	
	// Store information about the server
	LPHOSTENT hostEntry;
	
	// Specifying the server by its name;
	if (!(hostEntry = gethostbyname(hostname)))
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

	// Create the socket
	*s = socket(AF_INET,			// Go over TCP/IP
			   SOCK_STREAM,			// This is a stream-oriented socket
			   IPPROTO_TCP);		// Use TCP rather than UDP

	if (*s == INVALID_SOCKET)
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
	// Fill a SOCKADDR_IN struct with address information
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;

	// Change to network-byte order and
	// insert into port field
	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	serverInfo.sin_port = htons(portno);

	ret = connect(*s, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));
	if(ret == SOCKET_ERROR)
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

	return 0;
}

/* Opens a socket as a server and waits for incoming connections. 
 * Gets first incoming connection in a socket.
 * NOTE: Winsock code is from http://johnnie.jerrata.com/winsocktutorial/ and has 
 * 		 been modified for portability
 * @param s The server socket to open
 * @param c The client socket to open
 * @param portno The port to use
 * @return 0 if everything went well, -1 otherwise (char)
 */
char open_server(SOCKET* s, SOCKET* c, int portno)
{
#ifdef __WIN32__
	WORD sockVersion;
	WSADATA wsaData;
	
	sockVersion = MAKEWORD(1, 1);
	
	// Initialize Winsock
	WSAStartup(sockVersion, &wsaData);
#endif
	int ret; // For storing return values
	
	*s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*s == INVALID_SOCKET)
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
// Make socket non-blocking:
/*#ifdef __WIN32__
	u_long iMode = 1;
	ret = ioctlsocket(m_socket, FIONBIO, &iMode);
	if(ret != 0)
	{
		WSACleanup();
		return -1;
	}
#else
	ret = fcntl(*s, F_SETFL, O_NONBLOCK);
	if(ret != 0) return -1;
#endif*/

	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons( portno );
	
	ret = bind(*s, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));
	if(ret == SOCKET_ERROR)
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
	ret = listen(*s, 1); // Up to 5 connections
	if(ret == SOCKET_ERROR)
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

    struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;

	*c = accept(*s, (struct sockaddr *)&their_addr, &addr_size);
	if(*c == INVALID_SOCKET)
	{
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
	return 0;
}

void close_sock(SOCKET s)
{
	if(s != INVALID_SOCKET)
	{
#ifdef __WIN32__
		closesocket(s);
		WSACleanup();
#else
		close(s);
#endif
	}
}

/* Sends a single byte from an open socket
 * @param s The socket to write to 
 * @param byte The data to send
 */
void send_sock(SOCKET s, char byte)
{
	send(s, &byte, 1, 0);
}

/* Recieves and returns a single byte from an open socket
 * @param s The socket to read from
 * @return The byte recieved from the socket (char)
 */
char recv_sock(SOCKET s)
{
	char byte;
	recv(s, &byte, 1, 0);
	return byte;
}

/* Finds the length of a negative-terminated short array
 * @param arr The short array
 * @return The number of shorts before the first negative in the array (int)
 */
int shortlen(short *arr)
{
	short *ptr = arr, len = 0;
	while(*ptr++ >= 0) ++len;
	return len;
}

/* Pushes a short onto an array
 * @param stack The array pointer
 * @param data The into to push
 */
void push(short *stack, short data)
{
	short *tmp = &stack[shortlen(stack)];
	*tmp = data;
}

/* Removes and returns the last element in a short array
 * @param stack The array pointer
 * @return The last element of stack (short)
 */
short pop(short *stack)
{
	int len = shortlen(stack);
	if(len == 0)
		return -1;
	short tmp = stack[len-1];
	stack[len-1] = -1;
	return tmp;
}

/* Returns the last element in an int array without removing it
 * @param stack The array pointer
 * @return The last element of stack (short)
 */
short peek(short *stack)
{
	int len = shortlen(stack);
	if(len == 0)
		return -1;
	return stack[len-1];
}

/* Parses Brainfuck code into more easily-executed operations
 * @param bf The brainfuck code to parse
 * @param arr Pointer to the location to store the modified code
 * @return The length of the newly parsed array
 */
int parse(char *bf, char **arr)
{
	int cnt = 0;
	for(int i = 0; i < strlen(bf); i++)
	{ // Counts the number of actual instructions
		switch(bf[i])
		{
			case '<': case '>': case '+':
			case '-': case ',': case '.':
			case '#': case '^': case '!':
			case ';': case ':': case '%':
				cnt += 1; // Standard operation
				break;
			case '[': case ']':
				cnt += 1 + sizeof(short); // Loop ops require a 2 byte relative address
				break;
		}
	}

	char *ptr = malloc(cnt); // malloc for formatted code
	*arr = ptr;	
	memset(*arr, 0, cnt);
	
	cnt = 0; // reset cnt for loop
	short tmp, loopstack[64]; // Stores loop pointers

	for(int i = 0; i < 64; i++)
		loopstack[i] = -1; // Init all loop locations to -1
	
	for(int i = 0; i < strlen(bf); i++)
	{
		switch(bf[i])
		{
			case '<': case '>': case '+':
			case '-': case ',': case '.':
			case '#': case '^': case '!':
			case ';': case ':': case '%':
				// Store the operation
				ptr[cnt] = bf[i];
				cnt += 1;
				break;
			case '[':
				// Store the operation, push loop address
				ptr[cnt] = bf[i];
				push(loopstack, cnt);
				cnt += 1 + sizeof(short); // 1 byte for operator, 2 bytes for address
				break;
			case ']':
				// Store the operation, pop loop address, set address variables
				
				/* This code is a stupid awful hack, but I love it.
				* Basically, we keep 2 bytes after every loop op ('[' and ']'),
				* we store an integer there, the relative position of the other
				* end of the loop. This means we can quickly jump from the end to the 
				* beginning or vice versa.
				*/
				ptr[cnt] = bf[i];
				tmp = pop(loopstack);
				*((short*)&ptr[cnt+1]) = tmp-cnt;
				cnt += 1 + sizeof(short);
				*((short*)&ptr[tmp+1]) = cnt-tmp;
				break;
		} // End switch
	} // End for

	return cnt;
}

/* Gets an error message corresponding to an error code
 * @param err The error code
 * @return The error message corresponding to the error code (const char*)
 */
const char* get_error(int err)
{
	switch(err)
	{
		case -1:
			return "Pointer index out of bounds.";
		case -2: 
			return "File not open for reading.";
		case BF_ARRAY_SIZE:
			return "Maximum buffer size surpassed.";
		default:
			return "Unknown error.";
	}
}

#endif // BRAINFUCK_H
