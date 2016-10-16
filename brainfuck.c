/** Interactive Brainfuck Interpreter
 * @author Jacob Heard
 * Date: October 11th, 2016 
 * 
 * Description: A command line Brainfuck interpreter, runs brainfuck code from a file or from the command line.
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
 * Anything that is not one of the above 8 operations is ignored, and can be used to comment 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "brainfuck.h"

int main(int argc, char* argv[])
{
	FILE * fp; // File pointer
	if(argc > 1)
	{
		if(strcmp(argv[1], "--help") == 0)
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
	
	char *raw; // Buffer for unprocessed input
	char *buf; // Buffer for processed input
	char memory[BF_ARRAY_SIZE] = {0}; // The Brainfuck program space
	char rollback[BF_ARRAY_SIZE] = {0}; // Copy of memory in case of error

	if(argc == 1)
	{
		raw = malloc(NUM_BYTES);
	}
	else
	{
		long filelen;
		
		fp = fopen(argv[1], "rb");  // Open the file in binary mode
		if(fp == NULL)
		{
			printf("Error: file not found.\n");
			return 1;
		}
		fseek(fp, 0, SEEK_END);          // Jump to the end of the file
		filelen = ftell(fp);             // Get the current byte offset in the file
		rewind(fp);                      // Jump back to the beginning of the file
		
		raw = malloc((filelen+1)); // Enough memory for file + \0
		fread(raw, filelen, 1, fp); // Read in the entire file
		fclose(fp); // Close the file;
	}
	
	do
	{
		if(argc == 1) // Command line mode	
		{
			printf(": ");
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
			{
				printf("Cell %d -> contains %c - %d\n", where, memory[where], (int)memory[where]);
				continue;
			}
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
				printf("   ]   End of loop, jumps back to start.\n\n");
				continue;
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
				case '+': 
					++ memory[where];
					break;
				case '-':
					-- memory[where];
					break;
				case '<':
					-- where;
					break;
				case '>':
					++ where;
					break;
				case '.':
					putchar(memory[where]);
					break;
				case ',':
					memory[where] = (char)getchar();
					break;
				case '[':
					if(memory[where] == 0)
						i += (int)*((short*)&buf[i+1])-1;
					else
						i += sizeof(short);
					break;
				case ']':
					i += (int)*((short*)&buf[i+1])-1;
					break;
			}
			
			// Handle errors
			if(where < 0 || where >= BF_ARRAY_SIZE)
			{
				printf("Runtime error at operation %d; %c\n", i, buf[i]);
				if(argc == 1)
				{
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
	} while( argc == 1 ); // End while	
	
	free(raw);	
	return 0;
}
