/** Interactive Brainfuck++ Interpreter
 * @author Jacob Heard
 * Last Updated: February 15th, 2018
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
 * It uses the same commands as above, with 6 added operations.
 *
 * The added operations in the brainfuck++ language are:
 *		#	Open a file for reading/writing
 *		; 	write the character in the current cell to the file, overwriting what is in the file
 * 		: 	Read a character from the open file
 *		% 	Opens a TCP socket for reading/writing. A second call closes the socket.
 *		^ 	Sends the character in the current cell through socket
 * 		!	Reads a character from socket into current cell
 *
 * As per brainfuck standard, anything that is not one of the above 14 operations is ignored, and can be used to comment
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "brainfuck.h"
#include "brainfuckpp.h"

int main(int argc, char* argv[]) {
	FILE * fp; // File pointer
	int console = 0;
	if(argc > 1) {
		if(strcmp(argv[1], "--help") == 0) { // If the user is begging for help
			printf("Usage: %s <filename>\n", argv[0]);
			printf("  Use %s --bf++ to operate in brainfuck++ mode.\n", argv[0]);
			return 1;
		} else if(strcmp(argv[1], "--bf++") == 0 || strcmp(argv[1], "--brainfuck++") == 0) {
			bfpp = 1; // Set brainfuck++ flag
			console = 1;
			fp = stdin;
		}
	} else {
		// Otherwise, run in interactive mode, accept brainfuck from the command line
		fp = stdin;
		console = 1;
	}

	int roll_where = 0, len; // prev. memory location, length of input
	
	char *raw; // Buffer for unprocessed input
	char *buf; // Buffer for processed input
	char rollback[BF_ARRAY_SIZE] = {0}; // Copy of memory in case of error

	if(console) {
		raw = malloc(BUF_SIZE);
		if(raw == NULL) {
			fprintf(stderr, "Error allocating memory\n");
			return 1;
		}
	} else {
		long filelen;

		fp = fopen(argv[1], "rb");  // Open the file in binary mode
		if(fp == NULL) {
			printf("Error: file not found.\n");
			return 1;
		}
		fseek(fp, 0, SEEK_END);          // Jump to the end of the file
		filelen = ftell(fp);             // Get the current byte offset in the file
		rewind(fp);                      // Jump back to the beginning of the file

		raw = malloc((filelen+1)); // Enough memory for file + \0
		int bytesread = fread(raw, 1, filelen, fp); // Read in the entire file
		fclose(fp); // Close the file;
		*(raw+filelen) = 0; // Set null terminator

		if(bytesread < filelen) {
			fprintf(stderr, "Error reading file contents.\n");
			free(raw);
			return 1;
		}
	}

	do {
		if(console) { // Command line mode
			printf(": ");
			memset(raw, 0, BUF_SIZE); // Zero the buffer
			fgets(raw, BUF_SIZE, fp); // Read from input
			int res = parse_request(raw);
			
			// Exit cleanly
			if(res == QUIT) {
				free(raw);
				return 0;
			} else if(res == RESET) {
				// Reset everything
				roll_where = 0;
				memset(rollback, 0, BF_ARRAY_SIZE);
				continue;
			}
		}

		// Process raw input, get parse length
		buf = NULL;
		len = parse(raw, &buf);
		if(len < 0) {
			printf("Error: %s\n", get_error(len));
			if(buf != NULL)
				free(buf);
			continue;
		}

		// Excecute the Brainfuck code
		for(int i = 0; i < len; i++) {
			i += do_op(buf[i], &buf[i+1]);

			// Handle errors
			if(where < 0) {
				printf("Runtime error at operation %d; %c\n", i-1, buf[i]);
				printf("  : %s\n", get_error(where));
				if(console) { // Revert memory if in console mode
					printf("Rolling back brainfuck memory...\n");
					memcpy(&memory, rollback, BF_ARRAY_SIZE);
					where = roll_where;
				}
				break;
			}
		} // End for
		
		printf("\n");
		free(buf);
		
		// Save the current data, if something goes wrong next time we can roll back
		roll_where = where;
		memcpy(&rollback, memory, BF_ARRAY_SIZE);
	} while( console ); // End while	

	// Clean up loose ends if in bf++ mode
	if(bfpp) {
		cleanup();
	}
	
	free(raw);
	return 0;
}

