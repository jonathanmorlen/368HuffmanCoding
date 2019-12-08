#include "huff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	// Error handling
	if(argc == 1)
	{
		printf("Must pass in a filename to compress.\n");
		return EXIT_FAILURE;
	}
	char* filename = argv[1];

	// Get a sorted, doubly-linked list of frequencies of the characters that appear in the file
	unsigned long* asciiFrequencies = getFrequency(filename);
	List* characterFrequencies = frequencySort(asciiFrequencies);

	// Create the Huffman tree from frequency list
	createTree(characterFrequencies);

	// Write header and contents to file using Huffman tree
	List* encodingList = getBitEncodings(characterFrequencies -> head);
	writeCompressed(filename, encodingList, characterFrequencies -> head);

	// Free all allocated memory
	free(asciiFrequencies);
	freeList(encodingList);
	freeTree(characterFrequencies);
	return EXIT_SUCCESS;
}

unsigned long* getFrequency(char* filename)
{
	// Open file, error handle
	FILE* fp = fopen(filename, "r");
	if(fp == NULL)
	{
		printf("ERROR: File pointer is null.");
		return NULL;
	}

	unsigned long* frequencies = calloc(ASCII_COUNT, sizeof(*frequencies));
	int character = 0;
	while((character = fgetc(fp)) != EOF)
	{
		frequencies[character]++;
	}
	// Add pseudo-eof character
	frequencies[PSEUDO_EOF_VALUE] = PSEUDO_EOF_FREQUENCY;
	
	//free(fileContents);
	fclose(fp);
	return frequencies;
}

List* frequencySort(unsigned long* asciiFrequencies)
{
	// Creates an array of ascii characters (asciiCharacters[65] = 65 or 'A')
	int asciiCharacters[ASCII_COUNT] = {0};
	int i;
	for(i = 0; i < ASCII_COUNT; i++)
	{
		asciiCharacters[i] = i;
	}

	// Bubblesort ASCII characters by frequency
	int j;
	for (i = 0; i < ASCII_COUNT - 1; i++)
	{
		for(j = 0; j < ASCII_COUNT - i - 1; j++)
		{
			if(asciiFrequencies[j] > asciiFrequencies[j + 1])
			{
				swapFrequencies(&asciiFrequencies[j], &asciiFrequencies[j + 1]);
				swapCharacters(&asciiCharacters[j], &asciiCharacters[j + 1]);
			}
		}
	}

	// Trim characters and their frequencies down to only those that exist in the file
	int usedCharacterCount = 0;
	for(i = 0; i < ASCII_COUNT; i++)
	{
		if(asciiFrequencies[i] != 0)
		{
			usedCharacterCount++;
		}
	}
	unsigned long* frequencies = calloc(sizeof(*frequencies), usedCharacterCount);
	int* characters = calloc(sizeof(*characters), usedCharacterCount);
	j = 0;
	for(i = 0; i < ASCII_COUNT; i++)
	{
		if(asciiFrequencies[i] != 0)
		{
			frequencies[j] = asciiFrequencies[i];
			characters[j] = asciiCharacters[i];
			j++;
		}
	}

	// Creates doubly linked list of characters and their frequencies in ascending order
	List* frequencyList = createList(); 

	for(i = 0; i < usedCharacterCount; i++)
	{
		Node* node = createNode(characters[i], frequencies[i]);
		append(frequencyList, node);
	}

	free(frequencies);
	free(characters);
	return frequencyList;
}

void createTree(List* frequencyList)
{
	// While the list still exists
	while(frequencyList -> nodeCount != 1)
	{
		// Stop if there is only one node left (All nodes have been paired up into a tree
		if(frequencyList -> head == frequencyList -> tail)
		{
			return;
		}
		else
		{
			// Select first two nodes, since list is sorted, should be the two lowest frequencies
			Node* leftChild = frequencyList -> head;
			Node* rightChild = frequencyList -> head -> right;

			// Create parent node
			Node* node = createNode('X', leftChild -> frequency + rightChild -> frequency);
			node -> leftChild = leftChild;
			node -> rightChild = rightChild;

			// If this is the last pair of nodes
			if(rightChild -> right == NULL)
			{
				frequencyList -> head = node;
				frequencyList -> tail = node;
				leftChild -> left = NULL;
				leftChild -> right = NULL;
				rightChild -> left = NULL;
				rightChild -> right = NULL;
			}
			// Cut children nodes from list, update list head, insert parent node back into list
			else
			{
				rightChild -> right -> left = NULL;
				frequencyList -> head = rightChild -> right;
				leftChild -> left = NULL;
				leftChild -> right = NULL;
				rightChild -> left = NULL;
				rightChild -> right = NULL;
				insertNode(frequencyList, node);
			}
		}
		// Compensate for removal of two children and addition of one parent
		frequencyList -> nodeCount--;
	}
}

List* getBitEncodings(Node* encodingTree)
{
	// Create new list and bit code variable, start recursive code generation
	List* encodingList = createList();
	int treeDepth = getMaxDepth(encodingTree);
	char* code = calloc((sizeof(*code) * treeDepth) + 1, 1);
	getCodes(encodingTree, encodingList, code, 0, treeDepth);

	free(code);
	return encodingList;
}

void getCodes(Node* node, List* list, char* code, int index, int treeDepth)
{
	// If left child is not null, append a zero to bit code and recurse on left child
	if(node -> leftChild != NULL)
	{
		code[index] = '0';
		getCodes(node -> leftChild, list, code, index + 1, treeDepth);
		code[index] = '\0';
	}
	
	// If right child is not null, append a one to bit code and recurse on right child
	if(node -> rightChild != NULL)
	{
		code[index] = '1';
		getCodes(node -> rightChild, list, code, index + 1, treeDepth);
		code[index] = '\0';
	}

	// If node is a leaf node
	if((node -> leftChild == NULL) && (node -> rightChild == NULL))
	{
		// Create a copy of node
		Node* encodedNode = createNode(node -> value, node -> frequency);

		// Create a copy of the generated code
		char* bitCode = malloc(((sizeof(*bitCode) * index) + 1));
		memcpy(bitCode, code, ((sizeof(*bitCode) * index) + 1));

		// Combine and add node to encoding list
		encodedNode -> code = bitCode;

		append(list, encodedNode);
	}	
}

void writeCompressed(char* originalFilename, List* encodingList, Node* encodingTree)
{
	// Create filename.txt.huff
	char* compressedFilename = malloc(sizeof("../Compressed Output/") + strlen(originalFilename) + sizeof(".huff"));
    strcpy(compressedFilename, "../Compressed Output/");
    strcat(compressedFilename, originalFilename);
	strcat(compressedFilename, ".huff");

	// Prepare for writing
	FILE* compressed = fopen(compressedFilename, "wb");
	FILE* original = fopen(originalFilename, "r");

    compressed == NULL ? printf("Cannot open %s\n", compressedFilename) : 0;
    original == NULL ? printf("Cannot open %s\n", originalFilename) : 0;

	int byte = 0;
	int level = 0;
	int character = 0;
	Node* node = encodingList -> head;

	// Write header
	encodeHeader(compressed, encodingTree, &byte, &level);

	// Write contents to file
	while((character = fgetc(original)) != EOF)
	{
		while(node != NULL)
		{
			if(node -> value == character)
			{
				writeCode(node -> code, compressed, &byte, &level);
			}
			node = node -> right;
		}
		node = encodingList -> head;
	}

	// Write Pseudo-EOF character to denote end of contents
	while(node -> value != PSEUDO_EOF_VALUE)
	{
		node = node -> right;
	}

	writeCode(node -> code, compressed, &byte, &level);

	// Pad last byte with zeros if needed
	if(level > 0)
	{
		while(level != 0)
		{
			writeCode((char*)"0", compressed, &byte, &level);
		}
	}

	// Free and close
	free(compressedFilename);
	fclose(original);
	fclose(compressed);
}

void encodeHeader(FILE* fp, Node* node, int* byte, int* level)
{
	// If leaf node
	if((node -> leftChild == NULL) && (node -> rightChild == NULL))
	{
		// Write a one, then the binary representation of the character
		writeCode((char*)"1", fp, byte, level);
		if(node -> value == PSEUDO_EOF_VALUE)
		{
			writeCode((char*)"100000000", fp, byte, level);
		}
		else
		{
			char* binaryRep = getBinary(node);
			writeCode((char*)"0", fp, byte, level);
			writeCode(binaryRep, fp, byte, level);
			free(binaryRep);
		}
	}
	else
	{
		// Write a zero and recurse
		writeCode((char*)"0", fp, byte, level);
		encodeHeader(fp, node -> leftChild, byte, level);
		encodeHeader(fp, node -> rightChild, byte, level);
	}	
}

char* getBinary(Node* node)
{
	// Set up
	char* binary = malloc(sizeof(*binary) * 9);
	binary[8] = 0;
	int i;

	// Construct bit sequence
	for(i = 7; i >= 0; i--)
	{
		binary[7 - i] = ((node -> value >> i) & 1) + 48;
	}
	return binary;
}

void writeCode(char* code, FILE* fp, int* byte, int* level)
{
	char c;
	for(c = *code++; c != '\0'; c = *code++)
	{
		int bit = c - '0';
		*byte |= bit << (7 - *level);
		(*level)++;

		// If buffer is full, write the byte to file
		if(*level == 8)
		{
			fputc(*byte, fp);
			*level = 0;
			*byte = 0;
		}
	}
}

void printByte(int byte)
{
	printf("Byte: ");
	int i;
	
	// Print the bits of the byte
	for(i = 7; i >= 0; i--)
	{
		printf("%c", (byte & (1 << i)) ? '1' : '0');
	}
	printf("\n");
}


unsigned long getFileSize(FILE* fp)
{
	// Set original position
	int originalPosition = ftell(fp);
	
	// Seek to end, record position
	fseek(fp, 0L, SEEK_END);
	long long fileSize = ftell(fp);

	// Seek back to original position
	fseek(fp, 0L, originalPosition);

	return fileSize;
}

void swapFrequencies(unsigned long* x, unsigned long* y)
{
	unsigned long temp = *x;
	*x = *y;
	*y = temp;
}

void swapCharacters(int* x, int* y)
{
	int temp = *x;
	*x = *y;
	*y = temp;
}

List* createList()
{
	// Initialize blank list
	List* list = malloc(sizeof(*list));
	list -> nodeCount = 0;
	list -> head = NULL;
	list -> tail = NULL;

	return list;
}

Node* createNode(int value, unsigned long frequency)
{
	// Initialize node
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

void append(List* list, Node* node)
{
	// If list is empty, add first node to list
	if(list -> nodeCount == 0)
	{
		node -> left = NULL;
		node -> right = NULL;
		list -> head = node;
		list -> tail = node;
	}

	// Add all other nodes to list
	else
	{
		list -> tail -> right = node;
		node -> left = list -> tail;
		node -> right = NULL;
		list -> tail = node;
	}
	list -> nodeCount++;
}

void insertNode(List* list, Node* node)
{
	Node* current = list -> head;

	// If adding in front
	if(node -> frequency <= current -> frequency)
	{
		list -> head = node;
		current -> left = node;
		node -> right = current;
	}
	else
	{
		// Iterate through list until appropriate place for node is found
		while(node -> frequency > current -> frequency && (current -> right != NULL)) 
		{
			current = current -> right;
		}
		// If adding to end
		if(node -> frequency > list -> tail -> frequency)
		{
			list -> tail = node;
			current -> right = node;
			node -> left = current;
		}
		// If adding in the middle
		else
		{
			current -> left -> right = node;
			node -> left = current -> left;
			node -> right = current;
			current -> left = node;
		}
	}
}

void freeTree(List* list)
{
	// Recursively free nodes in list
	Node* node = list -> head;
	if(node != NULL)
	{
		freeTreeHelper(node);
	}

	// Free list itself
	free(list);
}

void freeTreeHelper(Node* node)
{
	// Recurse on left child
	if(node -> leftChild != NULL)
	{
		freeTreeHelper(node -> leftChild);
	}

	// Recurse on right child
	if(node -> rightChild != NULL)
	{
		freeTreeHelper(node -> rightChild);
	}

	// Free node
	free(node);
}

void printTree(Node* node, int space)
{
	// Base case
	if(node == NULL)
	{
		return;
	}
	// Add level of spacing per iteration
	space += 6; 

	// Recurse on right child
	printTree(node -> rightChild, space); 
	printf("\n");
	int i;

	// Space out node from rest
	for (i = 6; i < space; i++)
	{
		printf(" "); 
	}

	// Print node
	printNode(node);

	// Recurse on left child
	printTree(node -> leftChild, space); 
}

int getMaxDepth(Node* node)
{
	if(node == NULL)
	{
		return 0;
	}
	else
	{
		int leftDepth = getMaxDepth(node -> leftChild);
		int rightDepth = getMaxDepth(node -> rightChild);

		// Add up depth of left and right sub-trees and add the largest to current depth
		if(rightDepth > leftDepth)
		{
			return rightDepth + 1;
		}
		else
		{
			return leftDepth + 1;
		}
	}
}

void freeList(List* list)
{
	Node* node = list -> head;

	// Error handling
	if(node != NULL)
	{
		// Iterate through list, freeing every node
		Node* next = node -> right;
		while(next != NULL)
		{
			free(node -> code);
			free(node);
			node = next;
			next = next -> right;
		}
		free(node -> code);
		free(node);
	}

	// Free list itself
	free(list);
}

void printList(List* list)
{
	printf("\nLIST:\n");
	Node* current = list -> head;
	int i = 0;
	
	// Iterate through list printing every node
	while(current != NULL)
	{
		printf("%d ", i);
		i++;
		printNode(current);
		current = current -> right;
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
			printf("Node: (\\n, %ld, %s)\n", node -> frequency, node -> code);
		}
		else if(node -> value == ' ')
		{
			printf("Node: ( , %ld, %s)\n", node -> frequency, node -> code);
		}
		else if(node -> value == PSEUDO_EOF_VALUE)
		{
			printf("Node: (EOF, %ld, %s)\n", node -> frequency, node -> code);
		}
		else
		{
			printf("Node: (%d, %ld, %s)\n", node -> value, node -> frequency, node -> code);
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

