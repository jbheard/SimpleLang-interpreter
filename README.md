# SimpleLang Interpreter

This is a command line interpreter for a simple, Turing complete, programming language. It can run code from a file or from the command line (interactive mode).
SimpleLang is actually a rebranding of a popular esoteric programming language called brain<expletive>. SimpleLang is just a change in name and nothing more, since I didn't want to plaster cuss words all over my online portfolio. Similarly, SimpleLang++ is a rebranding of a lesser known addition to the original language by Jacob L. Torrey. Both languages can be easily found on https://esolangs.org
SimpleLang is an esoteric programming language in which there are only 8 operations, these are as follows:

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

# SimpleLang++ Interpreter 
SimpleLang++ is an addition to SimpleLang, which adds some new operations to extend the usefulness (if it was useful to begin with) and possibilities of the language.


The added operations in the SimpleLang++ language are:

Operation | Explanation
:---: | :---
\# |	Open a file for reading/writing
; |	write the character in the current cell to the file, overwriting what is in the file
: |	Read a character from the file
% |	Opens a socket for reading/writing. A second call closes the socket.
^ |	Sends the character in the current cell
! |	Reads a character from socket into current cell

The comment syntax does not change. Any SimpleLang program can be run using SimpleLang++, so long as none of the comments contain any of the new operations.  
The actual specification for the SimpleLang++ language (includes how to open files and sockets in more depth) can be found in spec.txt

# Building

To build the interpreter use **gcc main.c SimpleLang.c SimpleLangpp.c -o SimpleLang -Werror -Wall -lws2_32** on Windows platforms (using MinGW) and **gcc main.c SimpleLang.c SimpleLangpp.c -o SimpleLang -Werror -Wall** on linux/unix platforms.

