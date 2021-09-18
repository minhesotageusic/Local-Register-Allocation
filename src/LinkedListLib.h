#ifndef LINKEDLISTLIB_H_
#define LINKEDLISTLIB_H_

#include <stdio.h>
#include <stdlib.h>

//define the generic link list node
typedef struct node {
	void * data;
	struct node * next;
} Node;
//enqueue a given data into the given list
void Enqueue(Node** head_ref, void * new_data);
//remove the head node and return its data
void* Dequeue(Node** head_ref);
//print the list starting from given node using the given
//print function pointer
void printList(Node * node, void (*fptr)(void*));

#endif
