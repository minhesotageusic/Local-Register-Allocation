#include "LinkedListLib.h"

//enqueue a given data into the given list
void Enqueue(Node** tail_ref, void * new_data){
	if(tail_ref == NULL) return;
	if(new_data == NULL) return;

	Node* new_node = (Node*)malloc(sizeof(Node));
	new_node->data = new_data;
	new_node->next = NULL;

	if (*tail_ref != NULL) (*tail_ref)->next = new_node;
	*tail_ref = new_node;
}
//remove the head node and return its data
void* Dequeue(Node** head_ref){
	if(head_ref == NULL || *head_ref == NULL) return NULL;
	//retrieve next node as head
	Node* next_head = (*head_ref)->next;
	//retrieve current head's data
	void* curr_head_data = (*head_ref)->data;
	//free current head
	free((*head_ref));
	//set next head as head_ref
	*head_ref = next_head;
	return curr_head_data;
}
//print the list starting from given node using the given
//print function pointer
void printList(Node * node, void (*fptr)(void*)){
	if(fptr == NULL) {
		printf("no suitable print method\n");
		return;
	}
	while(node != NULL){
		(*fptr)(node->data);
		node = node->next;
	}
}
