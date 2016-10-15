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
Brainfuck++ is an edition to the brainfuck language outlined by Jacob I. Torrey (https://esolangs.org/wiki/Brainfuck%2B%2B). The language caught my eye while I was browsing Esolang. I personally think that brainfuck is great and challenging as is, but I was still disapointed to see that nobody had written a proper implementation of brainfuck ++.  
At least to my current knowledge, there are only a few interpreters out there for brainfuck++, and none of them actually obey the specifications outlined by the creator, all of which simply add many extra commands that the developer thought would be neat.  
Even the creator of the specification does not seem to have completed anything more than a simple brainfuck interpreter (http://www.jitunleashed.com/bf/index.html).

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
The actual specification for the brainfuck++ language (along with how to open files and sockets) can be found in spec.txt

# Building

To build the standard brainfuck interpreter you can use **gcc brainfuck.c -o brainfuck -Werror -Wall**, this works on unix and Windows (Using MinGW). 

To build the brainfuck++ interpreter use **gcc brainfuckpp.c -o brainfuck++ -Werror -Wall -lws2_32** on Windows platforms and **gcc brainfuckpp.c -o brainfuck++ -Werror -Wall** on linux/unix platforms.
