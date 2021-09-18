#include "lib.h"
void printInt(void* n) {
	if (n == NULL)
		return;
	printf("%d\n", *(int*) n);
}
void clearStr(char * str, int n) {
	if (str == NULL)
		return;
	n--;
	for (; n >= 0; n--) {
		str[n] = '\0';
	}
}
char isOnlyWhiteSpace(char * str, int len) {
	if (str == NULL)
		return 0;
	if (len <= 0)
		return 0;
	int i;
	for (i = 0; i < len; i++) {
		if (!isspace(str[i]))
			return 0;
	}
	return 1;
}
//determine if a specific register exist in the LL
//it is asssumed that the given head utilize
//the ReigsterNode
//Return 0 if target could not be found
//Return 1 otherwise
char containRegister(Node * head, char * tar) {
	if (head == NULL || tar == NULL)
		return 0;
	Node * curr = head;
	RegisterNode * d = curr->data;
	while (curr != NULL) {
		d = (RegisterNode*) curr->data;
		//return if we found a match
		if (d != NULL && strcmp(tar, d->registerName) == 0)
			return 1;
		//go to next node
		curr = curr->next;
	}
	//return 0 if we could not find target
	return 0;
}
//Create a register node and return the pointer
//Return NULL if RegisterNode could not be created
RegisterNode * createRegisterNode(char * registerName, int frequency,
		int lineDeclaration, int lineEnd, int offset) {
	RegisterNode * ret = (RegisterNode *) malloc(sizeof(RegisterNode));
	if (ret == NULL)
		return NULL;
	ret->registerName = (char*) malloc(sizeof(char) * strlen(registerName));
	if (ret->registerName == NULL)
		return NULL;
	strcpy(ret->registerName, registerName);
	ret->frequency = frequency;
	ret->lineDeclaration = lineDeclaration;
	ret->lineEnd = lineEnd;
	ret->offset = offset;
	return ret;
}
void freeRegisterNode(RegisterNode*node) {
	free(node->registerName);
	free(node);
}

void FreeRegisterList(Node* head_node) {
	Node * node = NULL;
	RegisterNode * data = NULL;
	//free all the data first
	while (node != NULL) {
		data = (RegisterNode *) node->data;
		if (data != NULL) {
			freeRegisterNode(data);
		}
		node = node->next;
	}
	//free the set
	while (head_node != NULL) {
		Dequeue(&head_node);
	}
}
//return the pointer to a given register name if it
//exist in the given head LinkedList
RegisterNode * getRegisterNode(Node * head, char * registerName) {
	if (registerName == NULL || head == NULL)
		return NULL;
	Node * curr = head;
	RegisterNode * data = NULL;
	while (curr != NULL) {
		data = (RegisterNode*) curr->data;
		if (data != NULL && strcmp(registerName, data->registerName) == 0) {
			return data;
		}
		curr = curr->next;
	}
	return NULL;
}

int maxNodeNumber(char* ptr, int currHighest, int lenOfPtr) {
	if (ptr == NULL || lenOfPtr <= 1)
		return currHighest;
	int i;
	int val = 0;
	for (i = 1; i < lenOfPtr; i++) {
		val = atoi(ptr + 1);
		if (currHighest < val)
			currHighest = val;
	}
	return currHighest;
}
int main(int argc, char * argv[]) {
	if (argc != 4)
		return 0;

	k = atoi(argv[1]);

	if (k == 0)
		return 0;

	o = argv[2][0];

	name = argv[3];

	struct timeval start, end;
	double cpu_time_used;
	
	gettimeofday(&start, NULL);
	switch (o) {
	case 'b':	//bottom up
		BottomUpAllocation();
		break;
	case 's':	//simple top down
		SimpleTopDownAllocation();
		break;
	case 't':	//top down
		TopDownAllocation();
		break;
	case 'o':	//own top down
		OwnTopDownAllocation();
		break;
	default:
		break;
	}
	gettimeofday(&end, NULL);
	cpu_time_used = ((end.tv_sec - start.tv_sec) * 1e6 + end.tv_usec - start.tv_usec)*0.001;

	printf("\nexecution took: %f msec\n", cpu_time_used);
	return 0;
}

