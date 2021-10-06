Ruchit Patel

rpatel18

Spring 2021

Assignment 6: Huffman Coding

---------------------
DESCRIPTION

- In this lab, a compression algorithm, Huffman compression, is implemented. 
- It compresses the input file byte by byte. 
- The lab can produce four executables: Encode, Decode, and Entropy (source code given).
- Common Arguments for encoder and decoder:    -h (prints help message), 
		            -i (specifies input file (default:stdin)), 
		            -o (specifies output file (default:stdout)), 
			    -v (Prints encoding or decoding statistics) 

---------------------
FILES

1. encode.c
- This source file contains the main method and the implementation for the encoder (compressor).

2. decode.c
- This source file contains the main method and the implementation for the decoder (decompressor). 

3. entropy.c 
- This source file (provided) contains the main method and the implementation for the entropy program. 

4. defines.h
- This header file declares macros to be used in multiple source files.

5. header.h
- This header file defines the Header strucutre to be written out by the encoder and read by the decoder.

6. node.h
- This header file declares the Node abstract data structure and the methods to manipulate it.

7. node.c
- This source file implements the methods declared in node.h to work with a Node.

8. pq.h
- This header file declares the PriorityQueue abstract data structure and the methods to manipulate it.

9. pq.c
- This source file implements the methods declared in pq.h to work with a PriorityQueue.

10. code.h
- This header file declares the Code abstract data structure and the methods to manipulate it.

11. code.c
- This source file implements the methods declared in code.h to work with a Code.

12. io.h
- This header file declares the methods associated with handling input/output for this lab.

13. io.c
- This source file implements the methods declared in io.h for input/output.

14. stack.h
- This header file declares the Stack abstract data structure and the methods to manipulate it.

15. stack.c
- This source file implements the methods declared in stack.h to work with a Stack.

16. huffman.h 
- This header file declares the methods related with the huffman interface.

17. huffman.c
-  This source file implements the methods declared in huffman.h (implementation of huffman interface).

18. Makefile

- This is a Makefile that can be used with the make utility to build the executables.

19. DESIGN.pdf 

- This PDF explains the design for this lab. It includes a brief description of the lab and pseudocode alongwith implementation description. 

20. Temporary text file

- This file is created by the encoder when it is taking input from the stdin stream (for accomodating two pass). The name of the file is defined in a macro inside encode.c and is currently named "temp_infile.txt". Please make sure there are no other files with the similar name (in the same directory) or change the macro value inside encode.c file. 

---------------------
INSTRUCTIONS

With make:
1. Keep the Makefile in the same directory as all other files. 

2. Execute “make” or “make all” in terminal in order to produce the all four (encode, decode, error, entropy) executables.

3. Execute "make x" where x is either encode, decode, or entropy to build the respective executables.

4. Run encode or decode executables with their respective arguments to encode or decode a file. Use the entropy program measure entropy of a file respectively. The program would run as described in the description and the DESIGN.pdf based on the arguments. 

5. In order to scan-build the source file, run “make scan-build” in the terminal.

6. In order to clean up (remove object and executable files), run “make clean” in the terminal.

7. In order to format files, run “make format” in the terminal.

