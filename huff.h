#ifndef __huff_h_
#define __huff_h_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

//_______________________________________________________________________________________
// DISCLAIMER

/*
 *
 *	Supports file sizes up to ~4GB (4,294,967,295 Bytes)
 * 	Supports ASCII values [0 - 255]
 *
 */

//_______________________________________________________________________________________
// CONSTANTS

// Replaces 'Magic Number' of 127 or 255 for ASCII set supported
#define ASCII_COUNT 257

// ASCII value of EOF character, needs to be removed
#define PSEUDO_EOF_VALUE 256

// Frequency of Pseudo-EOF character, used when adding it to Huffman Tree
#define PSEUDO_EOF_FREQUENCY 1

//_______________________________________________________________________________________
// STRUCTURES

typedef struct Node Node;
typedef struct Dictionary Dictionary;

// Functions as a node in a doubly-linked list as well as a node in a
// binary tree for easy construction of Huffman tree from list
struct Node
{				
	int        			value; // Numerical representation of ASCII character
	unsigned long   frequency; // Number of times character happens in given file
	Node*       		 left; // List neighbor to left
	Node*      			right; // List neighbor to right
	Node*  			leftChild; // Tree child to left
	Node* 		   rightChild; // Tree child to right
	char*     		     code; // Bit code generated from node's position in Huffman tree
};

// Manages doubly-linked list
typedef struct 
{
	int nodeCount; // Number of nodes in list
	Node*    head; // Head of list
	Node*    tail; // Tail of list
} List;

//_______________________________________________________________________________________
// FUNCTIONS


// 								  **** HUFF.C ****


// Returns the frequency of all ASCII characters in the file in an arry
unsigned long* getFrequency(char* filename);

// Sorts characters by frequency, then puts the data into a doubly linked list
List* frequencySort(unsigned long* asciiFrequency);

// Creates the binary huffman tree based on the frequency list
void createTree(List* frequencyList);

// Sets the 'code' field of the Node structure to the binary code generated from tree
List* getBitEncodings(Node* encodingTree);

// Using the tree and list of encodings, write data by bit to file
void writeCompressed(char* originalFilename, List* encodingList, Node* encodingTree);


// 								**** UNHUFF.C ****


// Reconstructs Huffman tree from header
Node* reconstructTree(FILE* fp, unsigned char* byte, int* level);

// Using the huffman tree, decompresses and writes characters to file
void writeDecompressed(FILE* fp, Node* tree, char* fileName, unsigned char* byte, int* level);


// 								**** HELPERS ****

// Utility
unsigned long getFileSize(FILE* fp);
void swapFrequencies(unsigned long* x, unsigned long* y);
void swapCharacters(int* x, int* y);
int getMaxDepth(Node* node);
void getCodes(Node* node, List* list, char* code, int index, int treeDepth);
char* getBinary(Node* node);

// Writing
void writeCode(char* code, FILE* fp, int* byte, int* level);
void encodeHeader(FILE* fp, Node* node, int* byte, int* level);

// Reading
int readBit(FILE* fp, unsigned char* byte, int* level);
int readByte(FILE* fp, unsigned char* byte, int* level);
void skipByte(FILE* fp, unsigned char* byte, int* level);

// Data structure manipulation
List* createList();
Node* createNode(int value, unsigned long frequency);
void append(List* list, Node* node);
void insertNode(List* list, Node* node);

// Memory management functions
void freeList(List* list);
void freeTree(List* list);
void freeTreeHelper(Node* node);

// Debugging functions
void printNode(Node* node);
void printByte(int byte);
void printList(List* list);
void printTree(Node* node, int space);

#endif // __huff_h_
