# a68k #
An attempt to optimize the classic a68k assembler for Amiga, by:


  * compiling with a modern compiler (gcc-6.x)
  * improving internal workings (hash-function, toUpper() etc)
  
  
while retaining Amiga OS 1.3 compatibility



### Additions ###
 * Incdir directive.  (Binary size from 49436 bytes to 49520 bytes)
 * Handling of '\n' in strings (49520 -> 49648)
