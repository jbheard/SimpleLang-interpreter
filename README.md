# Brainfuck Interpreter

This is a command line Brainfuck interpreter. it can either run brainfuck code from a file or from the command line.
Brainfuck is an esoteric programming language in which there are only 8 operations, these are as follows:

Operation | Explanation
:---: | :---
  < | Decrements the data pointer  
  > | Increments the data pointer  
  - | Decrements the byte at the data pointer  
  + | Increments the byte at the data pointer  
  [ | Begins a loop, if byte at data pointer is 0, skip to the command after end of loop  
  ] | Jump to beginning of loop  
  , | Take a single byte of input  
  . | Print a the byte at the data pointer  

Anything that is not one of the above 8 operations is ignored, and can be used to comment.

# Brainfuck++ Interpreter 
Brainfuck++ is an addition to the brainfuck language outlined by Jacob I. Torrey (https://esolangs.org/wiki/Brainfuck%2B%2B). The language caught my eye while I was browsing Esolang. I personally think that brainfuck is great as is, but I was still disappointed to see that nobody had written a proper implementation of brainfuck ++, even now, years later.


The added operations in the brainfuck++ language are:

Operation | Explanation
:---: | :---
\# |	Open a file for reading/writing
; |	write the character in the current cell to the file, overwriting what is in the file
: |	Read a character from the file
% |	Opens a socket for reading/writing. A second call closes the socket.
^ |	Sends the character in the current cell
! |	Reads a character from socket into current cell

The comment syntax does not change. Any brainfuck program can be run using brainfuck++, so long as none of the comments contain any of the new operations.  
The actual specification for the brainfuck++ language (includes how to open files and sockets in more depth) can be found in spec.txt

# Building

To build the interpreter use **gcc main.c brainfuck.c brainfuckpp.c -o brainfuck -Werror -Wall -lws2_32** on Windows platforms (using MinGW) and **gcc main.c brainfuck.c brainfuckpp.c -o brainfuck -Werror -Wall** on linux/unix platforms.

