/*
 * topdownallocator.h
 *
 *  Created on: Feb 17, 2021
 *      Author: Minhesota Geusic
 */

#ifndef TOPDOWNALLOCATOR_H_
#define TOPDOWNALLOCATOR_H_

#include "lib.h"

typedef struct feasibleRegisterNode {
	char* feasibleRegisterName;
	RegisterNode * assignedRegister;
	char inUsed;
	char containOperandRegister;
} FeasibleRegisterNode;

typedef struct maxLivesNode {
	int line;
	Node * registerList;
	int count;
} MaxLivesNode;

void AssignOffSetValue(Node * head);
FeasibleRegisterNode * createFeasibleNode(char * feasibleName,
		RegisterNode * assignedRegister);
FeasibleRegisterNode * containRegisterInFeasibleSet(Node * head_node,
		char * tar);
FeasibleRegisterNode * unusedFeasibleRegister(Node * head_node);
FeasibleRegisterNode * removeFeasibleRegisterNode(Node ** head, char * tar);
RegisterNode * freeFeasibleNode(FeasibleRegisterNode * node);
RegisterNode * removeRegisterNode(Node ** head_ref, char * tar);

void FindMaxLivesExceeding(Node ** maxLivesList_head_ref, int threshold);
void FindExistingRegister(Node * head_ref, MaxLivesNode * maxLivesNode,
		int line);
void CreateFeasibleSet(Node** head_node_ref, int feasibleRegisterCount,
		int offset);
void FreeFeasibleSet(Node * head_node);
void ToggleUnusuedFeasibleSet(Node * head);
void swap(Node *a, Node *b);
void sort(Node * head_ref);
void debug_print_physicalregister(Node * head_ref, char * str);

#endif /* TOPDOWNALLOCATOR_H_ */
