#ifndef BRAINFUCK_H
#define BRAINFUCK_H	

// Array sizes
#define BF_ARRAY_SIZE 32768 // 2^15 bytes usable data space
#define BUF_SIZE      1024  // Read 1024 bytes at a time

// Request parsing options
#define QUIT  1
#define RESET 2
#define CODE  3

// Error codes
#define BAD_BRACKETS    -2
#define LOOP_TOO_DEEP   -3

// Help function codes
#define HELP_HELP       1
#define BRAINFUCK_HELP  2
#define PRINT_HELP      3
#define WHERE_HELP      4
#define DISP_HELP       5

// variables defined elsewhere
extern int where;
extern char memory[BF_ARRAY_SIZE];

void show_err(int);
void show_help(int);
int shortlen(short*);
void push(short*, short);
short pop(short*);
short peek(short*);
int parse(char*, char**);
int do_op(char, char*);
int parse_request(char*);
void disp(char*);

#endif // BRAINFUCK_H
