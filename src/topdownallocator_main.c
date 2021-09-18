#include "topdownallocator.h"

void AssignOffSetValue(Node * head) {
	int _offset = -1024;
	Node * curr = head;
	RegisterNode * data = NULL;
	while (curr != NULL) {
		data = (RegisterNode*) curr->data;
		if (data != NULL) {
			data->offset = _offset;
			_offset += 4;
		}
		curr = curr->next;
	}
}
//sort by the highest max lives and
//the max lives that exceed the threshold
void FindMaxLivesExceeding(Node ** maxLivesList_head_ref, int threshold) {
	if (maxLivesList_head_ref == NULL)
		return;
	if (threshold < 0)
		return;
	Node *ptr1;
	Node *prev;
	MaxLivesNode * data = NULL;
	//remove all node not exceeding given threshold
	ptr1 = *maxLivesList_head_ref;
	data = NULL;
	prev = NULL;
	while (ptr1 != NULL) {
		data = (MaxLivesNode*) ptr1->data;
		//check if the given line contain less
		//max lives than the threshold
		if (data != NULL && data->count <= threshold) {
			if (prev != NULL) {
				prev->next = ptr1->next;
			}
			//remove all register string associated
			//with this line
			char * _data = NULL;
			Node * n = data->registerList;
			data->registerList = NULL;
			while (n != NULL) {
				n = data->registerList;
				_data = (char*) Dequeue(&(n));
				if (_data != NULL)
					free(_data);
			}
			if (prev == NULL) {
				*maxLivesList_head_ref = ptr1->next;
				ptr1->next = NULL;
				free(ptr1);
				ptr1 = *maxLivesList_head_ref;
				continue;
			}
			ptr1->next = NULL;
			free(ptr1);
			ptr1 = prev->next;
			continue;
		}
		prev = ptr1;
		ptr1 = ptr1->next;
	}

}

void FindExistingRegister(Node * head_ref, MaxLivesNode * maxLivesNode,
		int line) {
	if (head_ref == NULL)
		return;
	Node * tail = head_ref;
	RegisterNode * data = NULL;
	Node * regNode_head = NULL;
	Node * regNode_tail = NULL;
	while (tail != NULL) {
		data = (RegisterNode*) tail->data;
		if (data != NULL) {
			if (data->lineDeclaration <= line && data->lineEnd > line) {
				char * regName = (char*) malloc(
						sizeof(char) * strlen(data->registerName));
				if (regName == NULL)
					exit(0);
				strcpy(regName, data->registerName);
				Enqueue(&regNode_tail, data->registerName);
				if (regNode_head == NULL) {
					regNode_head = regNode_tail;
					maxLivesNode->registerList = regNode_head;
				}
				maxLivesNode->count++;
			}
		}
		tail = tail->next;
	}

	maxLivesNode->registerList = regNode_head;

}

//create a feasible register
FeasibleRegisterNode * createFeasibleNode(char * feasibleName,
		RegisterNode * assignedRegister) {
	FeasibleRegisterNode * node = (FeasibleRegisterNode*) malloc(
			sizeof(FeasibleRegisterNode));
	if (node == NULL)
		return NULL;
	char * name = (char*) malloc(sizeof(char) * strlen(feasibleName));
	if (name == NULL)
		return NULL;
	strcpy(name, feasibleName);
	node->feasibleRegisterName = name;
	node->assignedRegister = assignedRegister;
	node->inUsed = 1;
	if (node->assignedRegister == NULL)
		node->inUsed = 0;
	return node;
}

//free feasible register
RegisterNode * freeFeasibleNode(FeasibleRegisterNode * node) {
	if (node == NULL)
		return NULL;
	RegisterNode * ret = NULL;
	free(node->feasibleRegisterName);
	ret = node->assignedRegister;
	free(node);
	return ret;
}

//create a feasible set
void CreateFeasibleSet(Node** head_node_ref, int feasibleRegisterCount,
		int offset) {
	if (head_node_ref == NULL)
		return;
	if (feasibleRegisterCount <= 0)
		return;
	char str[1024];
	int i;
	Node * curr = NULL;
	for (i = 0; i < feasibleRegisterCount; i++) {
		sprintf(str, "r%d", (i + 1 + offset));
		FeasibleRegisterNode * node = createFeasibleNode(str, NULL);
		Enqueue(&curr, node);
		if (*(head_node_ref) == NULL)
			*(head_node_ref) = curr;
	}
}

void FreeFeasibleSet(Node * head_node) {
	Node * node = NULL;
	FeasibleRegisterNode * data = NULL;
	//free all the data first
	while (node != NULL) {
		data = (FeasibleRegisterNode *) node->data;
		if (data != NULL) {
			//this will return regisetnode,
			//but we dont need to free it
			freeFeasibleNode(data);
		}
		node = node->next;
	}
	//free the set
	while (head_node != NULL) {
		Dequeue(&head_node);
	}
}

//determine if the given target register name is in
//the given feasible set
FeasibleRegisterNode * containRegisterInFeasibleSet(Node * head_node,
		char * tar) {
	//sequentially search if target is in
	//this feasible set
	if (tar == NULL)
		return NULL;
	Node * curr = head_node;
	FeasibleRegisterNode* data = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode*) curr->data;
		if (data != NULL && data->assignedRegister != NULL) {
			if (strcmp(data->assignedRegister->registerName, tar) == 0)
				return data;
		}
		curr = curr->next;
	}
	return NULL;
}

//find an unusued feasible register
FeasibleRegisterNode * unusedFeasibleRegister(Node * head_node) {
	//sequentially search for an unused register
	Node * curr = head_node;
	FeasibleRegisterNode * data = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode*) curr->data;
		if (data != NULL && data->inUsed == 0)
			return data;
		curr = curr->next;
	}
	return NULL;
}

void ToggleUnusuedFeasibleSet(Node * head) {
	Node * node = head;
	FeasibleRegisterNode * data = NULL;
	while (node != NULL) {
		data = (FeasibleRegisterNode*) node->data;
		if (data != NULL)
			data->inUsed = 0;
		node = node->next;
	}
}
//swap two node's data
void swap(Node *a, Node *b) {
	void* temp = a->data;
	a->data = b->data;
	b->data = temp;
}
//sort the register list by highest frequency and shortest length
void sort(Node * head_ref) {
	int swapped;
	Node *ptr1;
	Node *lptr = NULL;
	RegisterNode * data = NULL;
	RegisterNode * data2 = NULL;

	/* Checking for empty list */
	if (head_ref == NULL)
		return;

	do {
		swapped = 0;
		ptr1 = head_ref;

		while (ptr1->next != lptr) {
			data = (RegisterNode*) ptr1->data;
			data2 = (RegisterNode*) ptr1->next->data;
			if (data->frequency < data2->frequency
					|| (data->frequency == data2->frequency
							&& (data->lineEnd - data->lineDeclaration)
									> (data2->lineEnd - data2->lineDeclaration))) {
				swap(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}
		lptr = ptr1;
	} while (swapped);
}

void debug_print_physicalregister(Node * head_ref, char * str) {
	FeasibleRegisterNode * data = NULL;
	int stroffset = 0;
	while (head_ref != NULL) {
		data = (FeasibleRegisterNode*) head_ref->data;
		if (data != NULL) {
			RegisterNode * assignedRegister = data->assignedRegister;
			sprintf(str + stroffset,
					"\t[PR: %s, inused: %d, hasOperand: %d, assigned to: ",
					data->feasibleRegisterName, data->inUsed,
					data->containOperandRegister);
			stroffset = strlen(str);
			if (assignedRegister == NULL)
				sprintf(str + stroffset, "NULL]\t|");
			else
				sprintf(str + stroffset, "%s, length: %d, read: %d, write: %d]\t|",
						assignedRegister->registerName,
						assignedRegister->length,
						assignedRegister->readCount,
						assignedRegister->writeCount);
			stroffset = strlen(str);
		}
		head_ref = head_ref->next;
	}
}

RegisterNode * removeRegisterNode(Node ** head_ref, char * tar) {
	if (head_ref == NULL)
		return NULL;
	if (tar == NULL)
		return NULL;
	Node * curr = *head_ref;
	Node * prev = NULL;
	RegisterNode * data = NULL;
	while (curr != NULL) {
		data = (RegisterNode*) curr->data;
		if (data != NULL && strcmp(data->registerName, tar) == 0) {
			if (prev == NULL) {
				*head_ref = curr->next;
			} else {
				prev->next = curr->next;
			}
			free(curr);
			return data;
		}
		prev = curr;
		curr = curr->next;
	}
	return NULL;
}

FeasibleRegisterNode * removeFeasibleRegisterNode(Node ** head, char * tar) {
	if (head == NULL)
		return NULL;
	if (tar == NULL)
		return NULL;
	Node * curr = *head;
	Node * prev = NULL;
	FeasibleRegisterNode * data = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode*) curr->data;
		if (data != NULL && strcmp(data->feasibleRegisterName, tar) == 0) {
			if (prev == NULL) {
				*head = curr->next;
			} else {
				prev->next = curr->next;
			}
			free(curr);
			return data;
		}
		prev = curr;
		curr = curr->next;
	}
	return NULL;
}
