#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "brainfuck.h"
#include "brainfuckpp.h"

/*** EXTERNAL VARIABLES ***/
// Brainfuck++ control variable (brainfuck mode vs brainfuck++)
int bfpp = 0;
// Position in array
int where = 0;
// Main memory array
char memory[BF_ARRAY_SIZE] = {0};
/**************************/

/*** INTERNAL VARIABLES ***/
// File and socket control variables
static char file_open = 0, sock_open = 0;
// Internal file pointer for brainfuck
static FILE* bf_fp = NULL;
// Client socket
static SOCKET sock_c;
// Server socket
static SOCKET sock_s = INVALID_SOCKET;
// Port number for working with servers
static int port;
/**************************/

/* Meant for basic cleanup upon exiting application
 * Closes file pointer and sockets.
 */
void cleanup() {
	if(file_open) {
		fclose(bf_fp);
		file_open = 0;
	}
	if(sock_open) {
		close_sock(sock_s);
		sock_s = INVALID_SOCKET;
		close_sock(sock_c);
		sock_open = 0;
	}
}

/* Gets an error message corresponding to an error code
 * @param err The error code
 * @return The error message corresponding to the error code (const char*)
 */
const char* get_error(int err) {
	switch(err) {
		case INDEX_OOB:
			return "Pointer index out of bounds.";
		case FILE_ERR:
			return "File not open for reading.";
		case BF_ARRAY_SIZE:
			return "Maximum buffer size surpassed.";
		case BAD_BRACKETS:
			return "Bad bracket notation";
		case LOOP_TOO_DEEP:
			return "Maximum loop depth exceeded";
		case MEMORY_ERR:
			return "Allocating memory";
		default:
			return "Unimplemented error";
	}
}

/* Shows a help page for a given command, or a general help page if no command is provided
 * @param help_id The id of the help page to show (e.g. HELP_HELP, BRAINFUCK_HELP)
 */
void show_help(int help_id) {
switch(help_id) {
	case HELP_HELP:
		printf("Available commands:\n");
		printf("help - Show this help page, you can use help [command_name] to view help for \n");
		printf("       that specific command. Use \"help brainfuck\" or \"help bf\" to see a\n");
		printf("       language overview.\n");
		printf("where - Show the current position of the cursor.\n");
		printf("print - Print the memory at a given position as string.\n");
		printf("disp - Display memory at a given position in various formats.\n\n");
		break;
	case BRAINFUCK_HELP:
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
		printf("   ]   End of loop, jumps back to start.\n\n");
		printf("Use \"help bf++\" to see brainfuck++ specific operations\n\n");
		break;
	case BFPP_HELP:
		printf("-------------------------------------------------------------------------------\n");
		printf("                        Brainfuck++ Operations Guide\n");
		printf("-------------------------------------------------------------------------------\n");
		printf("  Op | Explanation\n");
		printf("-------------------------------------------------------------------------------\n");
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
		printf("Use \"help bf\" to see general brainfuck operations\n\n");
		break;
	case PRINT_HELP:
		printf("print - Prints the text at pointer location, stops at a \n");
		printf("NULL (0) character.\n");
		printf("Usage: print [n]\n");
		printf("       n    - The address to print from, if not specified,\n");
		printf("              uses current pointer location.\n\n");
		break;
	case WHERE_HELP:
		printf("where - Display current pointer position and information.\n");
		printf("Usage: where\n\n");
		break;
	case DISP_HELP:
		printf("disp - Display memory at address\n");
		printf("Usage: disp [n] [type] [howmany]\n");
		printf("       n    - The address to print from, leave as -1 to use\n");
		printf("              current pointer location.\n");
		printf("       type - The format to display output. c for ascii \n");
		printf("              character, d for decimal number or x for \n");
		printf("              hexadecimal number.\n");
		printf("    howmany - How many bytes to show.\n\n");
		break;
	}
}

/* Finds the length of a negative-terminated array (type short)
 * @param arr The short array
 * @return The number of items before the first negative in the array (int)
 */
int shortlen(short *arr) {
	short *ptr = arr, len = 0;
	while(*ptr++ >= 0) ++len;
	return len;
}

/* Pushes a short onto a stack (array)
 * @param stack The array pointer
 * @param data The into to push
 */
void push(short *stack, short data) {
	short *tmp = &stack[shortlen(stack)];
	*tmp = data;
}

/* Removes and returns the last element in a stack (array)
 * @param stack The array pointer
 * @return The last element of stack (short)
 */
short pop(short *stack) {
	int len = shortlen(stack);
	if(len == 0)
		return -1;
	short tmp = stack[len-1];
	stack[len-1] = -1;
	return tmp;
}

/* Returns the last element in a stack (array)
 * @param stack The array pointer
 * @return The last element of stack (short)
 */
short peek(short *stack) {
	int len = shortlen(stack);
	if(len == 0)
		return -1;
	return stack[len-1];
}

/* Parses Brainfuck code to be read by the interpreter.
 * Removes comments/invalid chars, establishes loops, 
 * @param bf The brainfuck code to parse
 * @param arr Pointer to the location to store the modified 
 *				code (if NULL will be created using malloc)
 * @return The length of the newly parsed array
 */
int parse(char *bf, char **arr) {
	int cnt = 0;
	for(int i = 0; i < strlen(bf); i++) {
		// Counts the number of actual instructions
		switch(bf[i]) {
			case '<': case '>': case '+':
			case '-': case ',': case '.':
				cnt += 1; // Standard operation
				break;
			case '[': case ']':
				cnt += 1 + sizeof(short); // Loop ops require a memory address
				break;
			default:
			// handle any brainfuck++ code, but only in bf++ mode
				if(bfpp) {
					switch(bf[i]) {
						case '#': case '^': case '!':
						case ';': case ':': case '%':
							cnt += 1;
							break;
					}
				}
				break;
		}
	}
	// Check if an array was provided
	if(*arr == NULL) {
		*arr = malloc(cnt); // malloc for formatted code
		if(*arr == NULL) {
			return MEMORY_ERR;
		}
		// Zero the array
		memset(*arr, 0, cnt);
	}

	// set output array pointer
	char *ptr = *arr;
	cnt = 0; // reset cnt for loop

#define MAX_LOOPS 128 // Maximum number of nested loops
	short tmp, loopstack[MAX_LOOPS]; // Stores loop pointers

	for(int i = 0; i < MAX_LOOPS; i++)
		loopstack[i] = -1;

	for(int i = 0; i < strlen(bf); i++) {
		switch(bf[i]) {
			case '<': case '>': case '+':
			case '-': case ',': case '.':
				// Store the operation
				ptr[cnt] = bf[i];
				cnt += 1;
				break;
			case '[':
				// Store the operation, push loop address
				ptr[cnt] = bf[i];
				push(loopstack, cnt);
				
				if(shortlen(loopstack) == MAX_LOOPS)
					return LOOP_TOO_DEEP;
				
				cnt += 1 + sizeof(short); // 1 byte for operator, 2 bytes for address
				break;
			case ']':
				// Store the operation, pop loop address, set address variables
				ptr[cnt] = bf[i];
				tmp = pop(loopstack);
				if(tmp == -1)
					return BAD_BRACKETS;
				*((short*)&ptr[cnt+1]) = tmp-cnt;
				cnt += 1 + sizeof(short);
				*((short*)&ptr[tmp+1]) = cnt-tmp;
				break;
			default:
			// handle any brainfuck++ code, but only in bf++ mode
				if(bfpp) {
					switch(bf[i]) {
						case '#': case '^': case '!':
						case ';': case ':': case '%':
							ptr[cnt] = bf[i];
							cnt += 1;
							break;
					}
				}
				break;
		} // End switch
	} // End for

	if(shortlen(loopstack) > 0)
		return BAD_BRACKETS;

	return cnt;
}

/* Perform a single brainfuck operation. 
 * Passes any brainfuck++ to do_op_bfpp().
 * @param op The operation
 * @return The offset to apply to the code pointer
 */
int do_op(char op, char *next) {
	int offset = 0;
	switch(op) {
		case '+': // Increment cell at pointer
			++ memory[where];
			break;
		case '-': // Decrement cell at pointer
			-- memory[where];
			break;
		case '<': // Move pointer left
			where --;
			break;
		case '>': // Move pointer right
			where ++;
			break;
		case '.': // Print character
			printf("%c", memory[where]);
			break;
		case ',': // Get character
			memory[where] = (char)getchar();
			break;
		case '[': // Begin loop
			if(memory[where] == 0) {
				offset = (int)*((short*)next)-1;
			} else {
				offset = sizeof(short);
			}
			break;
		case ']': // End loop
			offset = (int)*((short*)next)-1;
			break;
		default:
			// Perform any brainfuck++ operations if in bf++ mode
			if(bfpp) do_op_bfpp(op);
			break;
	}
	if(where < 0 || where >= BF_ARRAY_SIZE) {
		where = INDEX_OOB;
	}
	return offset;
}

/* Perform a single brainfuck++ operation.
 * @param op The operation
 */
void do_op_bfpp(char op) {
	int move;
	switch(op) {
		case '#': // Open file
			if(file_open && bf_fp != NULL) {
				fclose(bf_fp);
				bf_fp = NULL;
				file_open = 0;
			} else {
				move = memory[where];
				bf_fp = fopen(&memory[where+move], "rb+");
				if(bf_fp == NULL) { // Failure :(
					memory[where] = 0xff;
				} else { 			  // Success :)
					memory[where] = 0;
					file_open = 1; // File is now open
				}
			}
			break;
		case ';': // Write byte to file
			if(bf_fp == NULL) {
				where = FILE_ERR;
			} else {
				putc(memory[where], bf_fp);
			}
			break;
		case ':': // Read byte from file
			if(bf_fp == NULL) {
				where = FILE_ERR;
			} else {
				memory[where] = (char)getc(bf_fp);
				if(memory[where] == EOF) // return 0 at EOF
					memory[where] = 0;
			}
			break;
		case '%':
			if(sock_open) {
				close_sock(sock_s);
				close_sock(sock_c);
				sock_open = 0;
			} else {
		   /*
			* The memory layout when opening a client port should be:
			* 		[1 byte return value] [n byte URL/IP] [1 NULL byte] [2 byte port number]
			* Alternatively, 
			* 		[1 byte return value] [1 NULL byte] [2 bytes port number]
			* Opens a server port as well as a connection to the client port
			*
			* 1. The first line just gets the location of our data, as per the bf++ spec
			* 2. The second line here uses port as a temporary variable to store the length
			* 		of the URL/IP. 
			* 3. The third line uses this to access the memory where the port 
			* 		number is. It is always in Big Endian format, so the first byte is 
			*		multiplied by 256 and added to the second byte.
			* 4. The if/else checks whether an IP/URL was passed. If no host was selected, 
			* 	 	we create a server and wait for a connection. Otherwise, we attempt to
			*		connect to the given host on the given port.
			*/
				move = memory[where];
				port = strlen(&memory[where+move]);
				port = memory[where+move+port+1]*0x100 + memory[where+move+port+2];
				if(memory[where+move] == '\0') {
					memory[where] = open_server(&sock_s, &sock_c, port);
				} else {
					memory[where] = open_client(&sock_c, &memory[where+move], port);
				}
				sock_open = 1; // Set socket control variable
			}
			break;
		case '^': // Send 1 byte through socket
			if(sock_open) {
				send_sock(sock_c, memory[where]);
			}
			break;
		case '!': // Recv 1 byte through socket
			if(sock_open) {
				memory[where] = recv_sock(sock_c);
			}
			break;
	}
}

/* Parses a command line request and either calls a method or allow 
 * the request to run as code
 * @param req The request to parse
 * @return The result of the request (QUIT, RESET, CODE, or 0)
 */
int parse_request(char *req) {
	char *tmp;

	// If the user wants to quit
	if(strncmp(req, "quit", 4) == 0) {
		return QUIT;
	} else if(strncmp(req, "reset", 5) == 0) {
		// If the user wants to start over
		// Reset state variables
		where = 0;
		memset(memory, 0, BF_ARRAY_SIZE);
		return RESET;

	} else if(strncmp(req, "where", 5) == 0) {
		char val = memory[where];
		printf("Cell %d -> contains '%c' | %d | 0x%x\n", where, val, (unsigned int)val, (unsigned int)val);
		return 0;

	} else if( strncmp(req, "help", 4) == 0) {

		// check for substrings to parse help request
		// TODO: change this at some point
		if(strstr(req, "where") != NULL) {
			show_help(WHERE_HELP);
		} else if(strstr(req, "print") != NULL) {
			show_help(PRINT_HELP);
		} else if(strstr(req, "disp") != NULL) {
			show_help(DISP_HELP);
		} else if(strstr(req, "brainfuck++") != NULL) {
			show_help(BRAINFUCK_HELP);
		} else if(strstr(req, "bf++") != NULL) {
			show_help(BFPP_HELP);
		} else if(strstr(req, "bf") != NULL) {
			show_help(BRAINFUCK_HELP);
		} else if(strstr(req, "brainfuck") != NULL) {
			show_help(BRAINFUCK_HELP);
		} else {
			show_help(HELP_HELP);
		}
		return 0;

	} else if( strncmp(req, "print", 5) == 0) {

		int a = where;
		if(strlen(req) > 6) {
			a = strtol(req+6, &tmp, 10);
			if(*tmp != '\0' && *tmp != ' ' && *tmp != '\n' && *tmp != '\r') {
				printf("Error parsing request at character '%c'\n", *tmp);
				return 0;
			} else if(a < 0 || a >= BF_ARRAY_SIZE) {
				printf("position %d is out of range.\n", a);
				return 0;
			}
		}
		printf("memory at %d: %s\n", a, &memory[a]);
		return 0;

	} else if( strncmp(req, "disp", 4) == 0) {
		disp(req);
		return 0;
	}

	return CODE;
}

/* Displays memory using arguments (format: n type howmany, see disp help page)
 * @param req The string of arguments to parse
 */
void disp(char *req) {
	char format = 'x';
	char *tmp;
	char pstr[64], *err;
	int pos = where, a;
	
	tmp = strtok(req, " "); // Ignore the first thingy
	tmp = strtok(NULL, " ");
	if(tmp == NULL) {
		printf("%x\n", (unsigned int)memory[where]);
		return;
	}

	a = strtol(tmp, &err, 10);
	if(*err != '\0' && *err != '\n' && *err != '\r' && *err != ' ') {
		printf("Error at '%c'\n", *err);
		return;
	}
	pos = a;
	
	tmp = strtok(NULL, " ");
	if(tmp != NULL) {
		if(tmp[0] == 'x' || tmp[0] == 'X') {
			format = 'x';
		} else if(tmp[0] == 'c' || tmp[0] == 'C') {
			format = 'c';
		} else if(tmp[0] == 'd' || tmp[0] == 'D') {
			format = 'd';
		} else {
			printf("Invalid format.\n");
			return;
		}
		
		tmp = strtok(NULL, " ");
		if(tmp != NULL) {
			a = strtol(tmp, &err, 10);
			if(*err != '\0' && *err != '\n' && *err != '\r' && *err != ' ') {
				printf("Error at '%c'\n", *err);
				return;
			}
		} else {
			a = 1;
		}
	} else {
		a = 1;
	}
	
	// Build the format string
	sprintf(pstr, "%%%c  ", format);
	for(int i = 0; i < a; i++) {
		// Display the data, 5 bytes per line
		printf(pstr, (unsigned int) memory[pos+i]);
		if(i % 5 == 4) printf("\n");
	}
	printf("\n");
}

/*
 *
 */
void do_file(char *fname) {
	FILE *fp;
	char *raw;
	int bytesread;
	long filelen;

	// This one should probably never happen, but just in case
	if(fname == NULL) {
		fprintf(stderr, "Error opening file: could not resolve filename.\n");
		exit(1);
	}

	// Open the file for reading, check that it is, in fact, open
	fp = fopen(fname, "rb");
	if(fp == NULL) {
		fprintf(stderr, "Error: file could not be opened.\n");
		exit(1);
	}

	// Get the length of the file
	fseek(fp, 0, SEEK_END);          // Seek end of file
	filelen = ftell(fp);             // Get position (of end)
	rewind(fp);                      // Go back to start of file

	// Allocate enough memory for file contents + \0
	raw = malloc((filelen+1));
	if(raw == NULL) {
		fprintf(stderr, "Error allocating memory.\n");
		fclose(fp);
		exit(1);
	}

	// Read in the entire file
	bytesread = fread(raw, 1, filelen, fp);
	fclose(fp);
	
	// Set null terminator just in case
	*(raw+filelen) = 0;

	// If we read less than the number of bytes in the file, something is wrong
	if(bytesread < filelen) {
		fprintf(stderr, "Error reading file contents.\n");
		free(raw);
		exit(1);
	}

	// Run the code
	run_code(raw);
	// Free the memory
	free(raw);
}

/*
 *
 */
void do_console() {
	int res;
	char *raw; // buffer for raw code input
	// For rolling memory to previous version on an error
	int roll_where = 0;
	char rollback[BF_ARRAY_SIZE] = {0};

	raw = malloc(BUF_SIZE+1);
	if(raw == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		return;
	}

	while( 1 ) {
		printf(": ");
		memset(raw, 0, BUF_SIZE); // Zero the buffer
		fgets(raw, BUF_SIZE, stdin); // Read from standard input
		res = parse_request(raw);
		
		// Exit cleanly
		if(res == QUIT) {
			free(raw);
			return;
		} else if(res == RESET) {
			// Reset everything
			roll_where = 0;
			memset(rollback, 0, BF_ARRAY_SIZE);
			continue;
		}
	
		// Attempt to run the given brainfuck code segment
		res = run_code(raw);
		if(res < 0) { // If something goes awol
			printf("Rolling back brainfuck memory...\n");
			memcpy(&memory, rollback, BF_ARRAY_SIZE);
			where = roll_where;
			continue;
		}
		
		// Save the current data, if something goes wrong next time we can roll back
		roll_where = where;
		memcpy(&rollback, memory, BF_ARRAY_SIZE);
	} // End while

	// Clean up
	free(raw);
}

/*
 * @return an error code, or 0 if everything runs fine
 */
int run_code(char *code) {
	// Process raw input, get parse length
	char *buf = NULL;
	int len = parse(code, &buf);
	if(len < 0) {
		printf("Error: %s\n", get_error(len));
		if(buf != NULL)
			free(buf);
		return len;
	}

	// Excecute the Brainfuck code
	for(int i = 0; i < len; i++) {
		i += do_op(buf[i], &buf[i+1]);

		// Handle errors
		if(where < 0) {
			printf("Runtime error at operation %d; %c\n", i-1, buf[i]);
			printf("  : %s\n", get_error(where));
			free(buf);
			return where;
		}
	} // End for
	
	printf("\n");
	free(buf);
	return 0;
}
