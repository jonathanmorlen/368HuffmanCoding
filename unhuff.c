#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>

#include "huff.h"

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		printf("Must pass in a filename to decompress.\n");
		return EXIT_FAILURE;
	}

	// Preparation
	char* filename = argv[1];
	FILE* fp = fopen(filename, "rb");
	unsigned char byte = 0;
	int level = 0;

	Node* huffmanTree = reconstructTree(fp, &byte, &level);
	writeDecompressed(fp, huffmanTree, filename, &byte, &level);

	fclose(fp);
	freeTreeHelper(huffmanTree);
	return EXIT_SUCCESS;
}

void writeDecompressed(FILE* fp, Node* tree, char* filename, unsigned char* byte, int* level)
{
	// Create and open filename.txt.huff.unhuff
	char* decompressedFilename = malloc(sizeof("../Uncompressed Output/") + strlen(filename) + sizeof(".unhuff"));
	strcpy(decompressedFilename, "../Uncompressed Output/");
	strcat(decompressedFilename, filename);
	strcat(decompressedFilename, ".unhuff");
	FILE* decompressed = fopen(decompressedFilename, "w");

	// Decompress
	Node* head = tree;
	bool PseudoEOF = false;
	while(!PseudoEOF)
	{
		while((tree -> leftChild != NULL) && (tree -> rightChild != NULL))
		{
			int bit = readBit(fp, byte, level);
			if(bit == 1)
			{
				tree = tree -> rightChild;
			}
			else
			{
				tree = tree -> leftChild;
			}
		}
		if(tree -> value == PSEUDO_EOF_VALUE)
		{
			PseudoEOF = true;
		}
		else
		{
			fputc(tree -> value, decompressed);
		}
		tree = head;
	}	
	fclose(decompressed);
	free(decompressedFilename);
}

Node* reconstructTree(FILE* fp, unsigned char* byte, int* level)
{
	if(readBit(fp, byte, level) == 1)
	{
		if(readBit(fp, byte, level) == 0)
		{
			int character = readByte(fp, byte, level);
			Node* node = createNode(character, 0);
			return node;
		}
		else
		{
			readByte(fp, byte, level);
			int character = PSEUDO_EOF_VALUE;
			Node* node = createNode(character, 0);
			return node;
		}
	}
	else
	{
		Node* leftChild = reconstructTree(fp, byte, level);
		Node* rightChild = reconstructTree(fp, byte, level);
		Node* newNode = createNode('X', 0);
		newNode -> leftChild = leftChild;
		newNode -> rightChild = rightChild;
		return newNode;
	}
}

void skipByte(FILE* fp, unsigned char* byte, int* level)
{
	int i;
	for(i = 7; i >= 0; i--)
	{
		readBit(fp, byte, level);
	}
}

int readBit(FILE* fp, unsigned char* byte, int* level)
{
	if(*level == 0)
	{
		*byte = fgetc(fp);
		*level = 8;
	}
	int bit = ((*byte >> --(*level)) & 1);
	return bit;
}

int readByte(FILE* fp, unsigned char* byte, int* level)
{
	int i;
	int readByte = 0;
	for(i = 7; i >= 0; i--)
	{
		int bit = readBit(fp, byte, level);
		readByte |= bit << (i);
	}
	return readByte;
}

Node* createNode(int value, unsigned long frequency)
{
	Node* node = malloc(sizeof(*node));
	node -> value = value;
	node -> frequency = frequency;
	node -> left = NULL;
	node -> right = NULL;
	node -> leftChild = NULL;
	node -> rightChild = NULL;
	node -> code = NULL;
	return node;
}

void printByte(int byte)
{
	int i;
	for(i = 7; i >= 0; i--)
	{
		printf("%c", (byte & (1 << i)) ? '1' : '0');
	}
	printf("\n");
}

void printNode(Node* node)
{
	// If node has a code
	if(node -> code != NULL)
	{
		if(node -> value == '\n')
		{
			printf("Node: (\\n, %ld, %s)\n", node -> frequency, (char*)node -> code);
		}
		else if(node -> value == ' ')
		{
			printf("Node: ( , %ld, %s)\n", node -> frequency, (char*)node -> code);
		}
		else if(node -> value == PSEUDO_EOF_VALUE)
		{
			printf("Node: (EOF, %ld, EOFCODE)\n", node -> frequency);
		}
		else
		{
			printf("Node: (%d, %ld, %s)\n", node -> value, node -> frequency, (char*)node -> code);
		}
	}
	// If node does not have a code
	else
	{
		if(node -> value == '\n')
		{
			printf("Node: (\\n, %ld)\n", node -> frequency);
		}
		else if(node -> value == ' ')
		{
			printf("Node: ( , %ld)\n", node -> frequency);
		}
		else if(node -> value == PSEUDO_EOF_VALUE)
		{
			printf("Node: (EOF, %ld)\n",node -> frequency);
		}
		else
		{
			printf("Node: (%d, %ld)\n", node -> value, node -> frequency);
		}

	}
}

void printTree(Node* node, int space)
{
	if(node == NULL)
	{
		return;
	}
	space += 20; 
	printTree(node -> rightChild, space); 
	printf("\n");
	int i;
	for (i = 20; i < space; i++)
	{
		printf(" "); 
	}
	printNode(node);
	printTree(node -> leftChild, space); 
}

void freeTreeHelper(Node* node)
{
	if(node -> leftChild != NULL)
	{
		freeTreeHelper(node -> leftChild);
	}
	if(node -> rightChild != NULL)
	{
		freeTreeHelper(node -> rightChild);
	}
	free(node);
}
