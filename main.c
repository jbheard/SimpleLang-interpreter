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

/// TODO implement getopt_long

int main(int argc, char *argv[]) {
	char *fname = NULL;
	int console = 0;

	if(argc > 1) {
		if(strcmp(argv[1], "--help") == 0) { // If the user is begging for help
			printf("Usage: %s <argument> <filename>\n", argv[0]);
			printf("     --help   Shows this help page and exits.\n");
			printf("     --bf++   Enables brainfuck++ commands.\n");
			return 1;
		} else if(strcmp(argv[1], "--bf++") == 0 || strcmp(argv[1], "--brainfuck++") == 0) {
			bfpp = 1; // Set brainfuck++ flag
			if(argc > 2) { // Implies a filename was passed
				fname = argv[2];
			} else {
				console = 1;
			}
		} else {
			fname = argv[1];
		}
	} else {
		// Otherwise, run in interactive mode, accept brainfuck from the command line
		console = 1;
	}

	if(console) {
		do_console();
	} else {
		do_file(fname);
	}

	// Clean up loose ends if in bf++ mode
	if(bfpp) {
		cleanup();
	}

	return 0;
}

