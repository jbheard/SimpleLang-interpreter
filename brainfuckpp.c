/** Interactive Brainfuck++ Interpreter
 * @author Jacob Heard
 * Date: October 15th, 2016 
 * 
 * Description: A command line brainfuck++ interpreter, runs brainfuck++ code from a file or from the command line.
 * Brainfuck is an esoteric programming language in which there are only 8 operations, these are as follows:
 * 		< 	Decrements the data pointer
 * 		> 	Increments the data pointer
 * 		- 	Decrements the byte at the data pointer
 * 		+ 	Increments the byte at the data pointer
 * 		[	Begins a loop, if byte at data pointer is 0, skip to the command after end of loop
 * 		] 	Jump to beginning of loop
 *		,	Take a single byte of input
 * 		. 	Print a the byte at the data pointer 
 * 
 * Brainfuck++ is an edition to the brainfuck language outlined by Jacob I. Torrey (https://esolangs.org/wiki/Brainfuck%2B%2B).
 * I stumbled across the language browsing through Esolang and it caught my eye. I personally think that brainfuck is great as
 * is, but I was still disapointed to see that nobody has written a proper implementation of brainfuck ++. At least to my 
 * current knowledge, there are only a few interpreters out there for this, and none of them actually obey the specifications,
 * all of which simply add many extra commands that the developer thought would be neat. Even the creator of the specification 
 * did not complete anything more than a simple brainfuck interpreter (http://www.jitunleashed.com/bf/index.html).
 * 
 * The added operations in the brainfuck++ language are:
 *		#	Open a file for reading/writing
 *		; 	write the character in the current cell to the file, overwriting what is in the file
 * 		: 	Read a character from the file
 *		% 	Opens a socket for reading/writing. A second call closes the socket.
 *		^ 	Sends the character in the current cell
 * 		!	Reads a character from socket into current cell
 *
 * As per brainfuck standard, anything that is not one of the above 14 operations is ignored, and can be used to comment 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "brainfuckpp.h"


int main(int argc, char* argv[])
{
	FILE * fp; // File pointer
	if(argc > 1)
	{
		if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
		{ // If the user is begging for help
			printf("Usage: %s <filename>\n", argv[0]);
			return 1;
		}
		else // Open the given file
			fp = fopen(argv[1], "r");
	}
	else
	{
		// Otherwise, run in interactive mode, accept brainfuck from the command line
		fp = stdin;
	}

	int where = 0, roll_where = 0, len; // memory location, prev. memory location, length of input
	char file_open = 0, sock_open = 0; // Whether we have sockets or files open
	FILE* bf_fp = NULL; // Brainfuck file pointer
	
	SOCKET sock; // Socket for working with servers
	int port;	 // Port number for working with servers
	
	char move = 0; // Number of cells to move on file/socket open
	char *raw; // Buffer for unprocessed input
	char *buf; // Buffer for processed input
	char memory[BF_ARRAY_SIZE] = {0}; // The Brainfuck program space
	char rollback[BF_ARRAY_SIZE] = {0}; // Copy of memory in case of error

	if(argc == 1) // Command line mode
	{
		raw = malloc(NUM_BYTES);
	}
	else
	{
		long filelen; // length of the file
		
		fp = fopen(argv[1], "rb");  // Open the file in binary mode
		if(fp == NULL)
		{
			printf("Error: file not found.\n"); // Uh-oh spaghettios
			return 1;
		}
		fseek(fp, 0, SEEK_END);          // Jump to the end of the file
		filelen = ftell(fp);             // Get the current byte offset in the file
		rewind(fp);                      // Jump back to the beginning of the file
		
		raw = malloc((filelen+1)); // Enough memory for file + '\0'
		fread(raw, filelen, 1, fp); // Read in the entire file
		fclose(fp); // Close the file;
	}
	
	do
	{
		if(argc == 1) // Command line mode	
		{
			printf(": ");			   // Make it look pretty
			memset(raw, 0, NUM_BYTES); // Zero the buffer
			fgets(raw, NUM_BYTES, fp); // Read from input
			
			// If the user wants to quit
			if(strncmp(raw, "quit", 4) == 0)
			{
				free(raw);
				return 1;
			}
			// If the user wants to start over
			else if(strncmp(raw, "reset", 5) == 0)
			{
				// Reset variables
				where = 0;
				memset(memory, 0, BF_ARRAY_SIZE);
				continue;
			}
			else if(strncmp(raw, "where", 5) == 0)
				printf("Cell %d -> contains %c - %d\n", where, memory[where], (int)memory[where]);
			else if( strncmp(raw, "help", 4) == 0)
			{
				printf("-------------------------------------------------------------------------------\n");
				printf("                        Brainfuck++ Operations Guide\n");
				printf("-------------------------------------------------------------------------------\n");
				printf("  Op | Explanation\n");
				printf("-------------------------------------------------------------------------------\n");
				printf("   +   Increments the current cell.\n");
				printf("   -   Decrements the current cell.\n");
				printf("   <   Moves the data pointer left.\n");
				printf("   >   Moves the data pointer right.\n");
				printf("   ,   Reads one byte from stdin into the current cell.\n");
				printf("   .   Writes the byte in the current cell to stdout.\n");
				printf("   [   Loop if the current cell is non-zero, otherwise jump to the op after ']'\n");
				printf("   ]   End of loop, jumps back to start.\n");
				printf("   #   Opens a file, the file name starts n bytes away and ends at '\\0', where\n");
				printf("       n is the value of the current cell. The result (0 if success, -1 if\n");
				printf("       failure) is stored at current cell\n");
				printf("   :   Read one byte from file into current cell.\n");
				printf("   ;   Write one byte from current cell into file.\n");
				printf("   %%   Open a socket. The URL/IP of the socket starts n bytes away and ends at\n");
				printf("       '\\0', where n is the value of the current cell. This is directly\n");
				printf("       followed by a 2 byte port number. The result (0 if success, -1 if \n");
				printf("       failure) is stored at the current cell.\n");
				printf("   ^   Writes one byte from current cell to socket.\n");
				printf("   !   Reads one byte from socket to current cell.\n\n");
			}
			else if( strncmp(raw, "print", 5) == 0)
			{
				printf("%s\n", &memory[where]);
				continue;
			}

		}
		
		len = parse(raw, &buf); // Process raw input, get parse length
		if(len < 0)
			continue;
		
		// Excecute the Brainfuck code
		for(int i = 0; i < len; i++)
		{
			switch(buf[i])
			{
				case '+': // Increment byte
					++ memory[where];
					break;
				case '-': // Decrement byte
					-- memory[where];
					break;
				case '<': // Move pointer -
					-- where;
					break;
				case '>': // Move pointer +
					++ where;
					break;
				case '.': // Write byte
					putchar(memory[where]);
					break;
				case ',': // Read byte
					memory[where] = (char)getchar();
					break;
				case '[': // Start loop
					if(memory[where] == 0)
						i += (int)*((short*)&buf[i+1])-1;
					else
						i += sizeof(short);
					break;
				case ']': // End loop
					i += (int)*((short*)&buf[i+1])-1;
					break;
				case '#': // Open file
					if(file_open && bf_fp != NULL)
					{
						fclose(bf_fp);
						file_open = 0;
					}
					else
					{
						move = memory[where];
						bf_fp = fopen(&memory[where+move], "rb+");
						if(bf_fp == NULL) // Failure :(
							memory[where] = -1;
						else 			  // Success :)
							memory[where] = 0;
						file_open = 1; // File is now open
					}
					break;
				case ';': // Write byte to file
					if(bf_fp == NULL)
						where = -2;
					else
						putc(memory[where], bf_fp);
					break;
				case ':': // Read byte from file
					if(bf_fp == NULL)
						where = -2;
					else
					{
						memory[where] = (char)getc(bf_fp);
						if(memory[where] == EOF) // return 0 at EOF
							memory[where] = 0;
					}
					break;
				case '%':
					if(sock_open)
					{
						close_sock(sock);
						sock_open = 0;
					}
					else
					{
					/* I hate writing compicated code, because now I have to explain it  
					* for future me and anyone who might see this.
					* 
					* First off, the memory layout when opening a port should be:
					* 		[1 byte return value] [n byte URL/IP] [1 NULL byte] [2 byte port number]
					*
					* 1. The first line just gets the location of our data, as per the bf++ spec
					* 2. The second line here uses port as a temporary variable to store the length
					* 		of the URL/IP. 
					* 3. The third line uses this to access the memory where the port 
					* 		number is. It is always in Big Endian format, so the first byte is 
					*		multiplied by 256 and added to the second byte.
					* 4. The fourth line opens the port and sets the return value.
					*/
						move = memory[where];
						port = strlen(&memory[where+move]);
						port = memory[where+move+port]*0x100 + memory[where+move+1+port];
						memory[where] = open_sock(&sock, &memory[where+move], port);
						sock_open = 1;
					}
					break;
				case '^': // Send 1 byte through socket
					if(sock_open)
						send_sock(sock, memory[where]);
					break;
				case '!': // Recv 1 byte through socket
					if(sock_open)
						memory[where] = recv_sock(sock);
					break;
			}
			
			// Handle errors
			if(where < 0 || where >= BF_ARRAY_SIZE)
			{
				printf("Runtime error at operation %d; %c : %s\n", i, buf[i], get_error(where));
				if(argc == 1)
				{
					printf("Rolling back brainfuck memory...\n");
					strncpy(memory, rollback, BF_ARRAY_SIZE);
					where = roll_where;
				}
				break;
			}
		} // End for
		
		printf("\n"); // Makes output look prettier
		free(buf); // Free this so we can use it again
		
		// Save the current data, if something goes wrong next time we can roll back
		roll_where = where;
		strncpy(rollback, memory, BF_ARRAY_SIZE);
	} while( argc == 1 ); // End while
	
	// Clean up after the user
	if(file_open) 
		fclose(bf_fp);
	if(sock_open)
		close_sock(sock);
	
	free(raw);
	return 0;
}
