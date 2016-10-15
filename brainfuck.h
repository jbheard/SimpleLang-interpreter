#ifndef BRAINFUCK_H
#define BRAINFUCK_H	

#define BF_ARRAY_SIZE 32768 // 2^15 bytes usable data space
#define NUM_BYTES	  1024  // Read 1024 bytes at a time

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
				cnt += 1; // Standard operation
				break;
			case '[': case ']':
				cnt += 1 + sizeof(short); // Loop ops require a memory address
				break;
		}
	}

	char *ptr = malloc(cnt); // malloc for formatted code
	*arr = ptr;	
	memset(*arr, 0, cnt);
	
	cnt = 0; // reset cnt for loop
	short tmp, loopstack[64]; // Stores loop pointers

	for(int i = 0; i < 64; i++)
		loopstack[i] = -1;
	
	for(int i = 0; i < strlen(bf); i++)
	{
		switch(bf[i])
		{
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
				cnt += 1 + sizeof(short); // 1 byte for operator, 2 bytes for address
				break;
			case ']':
				// Store the operation, pop loop address, set address variables
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

#endif // BRAINFUCK_H
