# brainfuck-interpreter

This is a command line Brainfuck interpreter, runs brainfuck code from a file or from the command line.
Brainfuck is an esoteric programming language in which there are only 8 operations, these are as follows:

  **<**   Decrements the data pointer  
  **>**   Increments the data pointer  
  **-**   Decrements the byte at the data pointer  
  **+**   Increments the byte at the data pointer  
  **[**   Begins a loop, if byte at data pointer is 0, skip to the command after end of loop  
  **]**   Jump to beginning of loop  
  **,**   Take a single byte of input  
  **.**   Print a the byte at the data pointer  

Anything that is not one of the above 8 operations is ignored, and can be used to comment.
