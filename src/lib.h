#ifndef LIB_H_
#define LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include "LinkedListLib.h"

//define the type to be use by TopDownAllocator
typedef struct registerNode {
	char * registerName;
	int frequency;
	int lineDeclaration;
	int lineEnd;
	int length;
	int offset;
	int readCount;
	int writeCount;
} RegisterNode;

typedef struct topdown_meta_node{
	int MAX_LIVES;
	int total_lines;
	int code_lines;
	int registerCount;
	int highestNodeNumber;
} TopDownMetaNode;

int k;
char o;
char * name;

FILE * fp;

void BottomUpAllocation();
void SimpleTopDownAllocation();
void TopDownAllocation();
void OwnTopDownAllocation();

void clearStr(char * str, int n);
char isOnlyWhiteSpace(char * str, int len);
char containRegister (Node * head, char * tar);
int maxNodeNumber (char* ptr, int currHighest, int lenOfPtr);
RegisterNode * createRegisterNode(char * registerName, int frequency, int lineDeclaration, int lineEnd, int offset);
RegisterNode * getRegisterNode(Node * head, char * registerName);

void FreeRegisterList(Node* head_node);

void* readInFile_Simple(FILE * fp, char * delim, Node ** head_node_ref, Node ** tail_node_ref);
void* readInFile_Top(FILE * fp, char * delim, Node ** head_node_ref, Node ** tail_node_ref);
void* readInFile_Own(FILE * fp, char * delim, Node ** head_node_ref, Node ** tail_node_ref);
void* readInFile_Bottom(FILE * fp, char * delim, Node ** head_node_ref, Node ** tail_node_ref, Node ** bottomUp_head, Node ** bottomUp_tail);

void printChanges_Simple(FILE* fp, TopDownMetaNode * meta, char * delim, Node ** head_node_ref, int feasibleCount);
void printChanges_Top(FILE* fp, TopDownMetaNode * meta, char * delim, Node ** physical_node_ref, Node ** spill_node_ref,int feasibleCount);
void printChanges_Own(FILE* fp, TopDownMetaNode * meta, char * delim, Node ** head_node_ref, Node ** spill_node_ref, int feasibleCount);
void printChanges_Bottom(FILE* fp, TopDownMetaNode * meta, char * delim, Node ** registerList_head, Node ** bottomUp_head, int feasibleCount);

void freeRegisterNode(RegisterNode*node);

#endif /* LIB_H_ */
