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

	/* Redefine *nix socket stuff so that Windows code works too */
	typedef struct sockaddr_in  SOCKADDR_IN;
	typedef struct hostent*     LPHOSTENT;
	typedef struct in_addr*     LPIN_ADDR;
	typedef struct sockaddr*    LPSOCKADDR;
#endif

#include "SimpleLangpp.h"

/* Opens a socket and connects to a given host on a given port
 * NOTE: Winsock code is from http://johnnie.jerrata.com/winsocktutorial/ and has 
 * 		 been modified for portability
 * @param s The socket to open
 * @param hostname The URL or IP of the host to connect to
 * @param portno The port to use when we connect
 * @return 0 if everything went well, -1 otherwise (char)
 */
char open_client(SOCKET* s, char* hostname, int portno) {
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
char open_server(SOCKET* s, SOCKET* c, int portno) {
#ifdef __WIN32__
	WORD sockVersion;
	WSADATA wsaData;
	
	sockVersion = MAKEWORD(1, 1);
	
	// Initialize Winsock
	WSAStartup(sockVersion, &wsaData);
#endif
	int ret; // For storing return values
	
	*s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*s == INVALID_SOCKET) {
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
// Make socket non-blocking:
// This doesn't seem to work. At least not as I expected
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
	if(ret == SOCKET_ERROR) {
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
	ret = listen(*s, 1); // Up to 5 connections
	if(ret == SOCKET_ERROR) {
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

    struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;

	*c = accept(*s, (struct sockaddr *)&their_addr, &addr_size);
	if(*c == INVALID_SOCKET) {
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}
	
	return 0;
}

/* Closes an open socket
 * @param s The socket to close
 */
void close_sock(SOCKET s) {
	if(s != INVALID_SOCKET) {
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
void send_sock(SOCKET s, char byte) {
	send(s, &byte, 1, 0);
}

/* Recieves and returns a single byte from an open socket
 * @param s The socket to read from
 * @return The byte recieved from the socket (char)
 */
char recv_sock(SOCKET s) {
	char byte;
	recv(s, &byte, 1, 0);
	return byte;
}

