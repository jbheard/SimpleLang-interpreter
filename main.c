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
 * Brainfuck++ is an addition to the brainfuck language outlined by Jacob I. Torrey (https://esolangs.org/wiki/Brainfuck%2B%2B).
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
#include <getopt.h>

#include "brainfuck.h"
#include "brainfuckpp.h"


int main(int argc, char *argv[]) {
	int c;
	int ret = 0;
	char fname[128] = {0};
	static int console = 1;

	while( 1 ) {
		static struct option long_options[] = {
			{"bfpp", no_argument, &bfpp, 1},
			{"bf++", no_argument, &bfpp, 1},
			{"brainfuck++", no_argument, &bfpp, 1},
			{"help", no_argument, 0, 'h'},
			{"file", required_argument, 0, 'f'},
			{"no-oob", no_argument, &oob, 0},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "hf:", long_options, &option_index);

		/* Detect the end of the options. */
		if(c == -1) break;

		switch( c ) {
		case 0: // Flag is set by getopt_long
			break;

		case 'h': // Help page
			printf("Usage: %s [options]\n", argv[0]);
			printf("     -h,--help   Shows this help page and exits\n");
			printf("     --bf++      Enables brainfuck++ commands\n");
			printf("     -f file     Runs the brainfuck(++) source code from the given file\n");
			printf("     --no-oob    Disables out-of-bounds exceptions. This essentially makes\n");
			printf("                 memory circular (0-1 rolls over to 32,767 and vice versa)\n");
			return 0;
			break;

		case 'f':
			strncpy(fname, optarg, 127);
			console = 0;
			break;
			
		case '?':
			// getopt_long already prints an error message
			break;

		default:
			fprintf(stderr, "An error occurred, exiting...\n");
			return 1;
			break;
		}
	}

	if(console) {
		do_console();
	} else {
		ret = do_file(fname);
	}

	// Clean up loose ends if in bf++ mode
	if(bfpp) {
		cleanup();
	}

	return ret;
}

