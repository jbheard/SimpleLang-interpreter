#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "brainfuck.h"

// Position in array
int where = 0;
// Main memory array
char memory[BF_ARRAY_SIZE] = {0};

/* Prints the error associated with the given error number
 * @param errno The ID of the error to print
 */
void show_err(int err) {
	switch(err) {
		case BAD_BRACKETS:
			printf("Error: Bad bracket notation\n");
			break;
		case LOOP_TOO_DEEP:
			printf("Error: Maximum loop depth exceeded\n");
			break;
		default:
			printf("Error (#%d) unimplemented\n", errno);
			break;
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
		}
	}
	// Check if an array was provided
	if(*arr == NULL) {
		*arr = malloc(cnt); // malloc for formatted code
		if(*arr == NULL) {
			return -1;
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
		} // End switch
	} // End for

	if(shortlen(loopstack) > 0)
		return BAD_BRACKETS;
	
	return cnt;
}

/* Perform a single brainfuck operation.
 * @param op The operation
 * @return The offset to apply to the code pointer
 */
int do_op(char op, char *next) {
	int offset = 0;
	switch(op) {
		case '+':
			++ memory[where];
			break;
		case '-':
			-- memory[where];
			break;
		case '<':
			where --;
			break;
		case '>':
			where ++;
			break;
		case '.':
			printf("%c", memory[where]);
			break;
		case ',':
			memory[where] = (char)getchar();
			break;
		case '[':
			if(memory[where] == 0)
				offset = (int)*((short*)next)-1;
			else
				offset = sizeof(short);
			break;
		case ']':
			offset = (int)*((short*)next)-1;
			break;
	}
	return offset;
}

/*
 *
 */
int parse_request(char *req) {
	char *tmp;
	int a;
	
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
		printf("Cell %d -> contains '%c' | %d | 0x%x\n", where, val, (int)val, (int)val);
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
		} else if(strstr(req, "brainfuck") != NULL) {
			show_help(BRAINFUCK_HELP);
		} else if(strstr(req, "bf") != NULL) {
			show_help(BRAINFUCK_HELP);
		} else {
			show_help(HELP_HELP);
		}
		return 0;

	} else if( strncmp(req, "print", 5) == 0) {

		int a = where;
		if(strlen(req) > 6) {
			a = strtol(req+6, &tmp, 10);
			if(*tmp != '\0' && *tmp != ' ' && *tmp != '\n' & *tmp != '\r') {
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
		printf("%hhx\n", memory[where]);
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
	
	sprintf(pstr, "%%%c  ", format);
	for(int i = 0; i < a; i++) {
		printf(pstr, (unsigned char) memory[pos+i]);
		if(i % 5 == 4) printf("\n");
	}
	printf("\n");
}

