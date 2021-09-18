#include "topdownallocator.h"

void* readInFile_Own(FILE * fp, char * delim, Node ** head_node_ref,
		Node ** tail_node_ref) {
	return readInFile_Simple(fp, delim, head_node_ref, tail_node_ref);
}

void printChanges_Own(FILE* fp, TopDownMetaNode * meta, char * delim,
		Node ** head_node_ref, Node ** spill_node_ref, int feasibleCount) {
	printChanges_Top(fp, meta, delim, head_node_ref, spill_node_ref,
			feasibleCount);
}

void OwnTopDownAllocation() {
	int i = 0;
	int feasibleRegister = 3;	//3 is the minimum number we can have
	int allocatableRegister = k - feasibleRegister;
	if (allocatableRegister < 0) {
		//cannot have allocatable register less than 0
		exit(0);
	}
	//establish link list for physical register
	Node * head_node = NULL;
	Node * tail_node = NULL;

	//max lives link list
	Node * maxLivesList_head = NULL;
	Node * maxLivesList_tail = NULL;
	Node * maxLivesList_curr = NULL;
	Node * maxLivesRegList = NULL;

	//link list for spill register
	Node * spillReg_head = NULL;
	Node * spillReg_tail = NULL;

	char * data = NULL;
	char delim[] = " \t\n";
	//open file
	fp = fopen(name, "r");
	if (fp == NULL)
		exit(0);

	//read in the file
	TopDownMetaNode * meta = (TopDownMetaNode*) readInFile_Top(fp, delim,
			&head_node, &tail_node);
	//sort into priority by frequency count
	//high frequencies = allocated register
	//least frequencies = memory
	sort(head_node);
	//go through each line and find the max lives
	for (i = 1; i <= meta->total_lines; i++) {
		MaxLivesNode * n = (MaxLivesNode*) malloc(sizeof(MaxLivesNode));
		n->line = i;
		n->count = 0;
		//find which register in the register list
		//exist on this line
		FindExistingRegister(head_node, n, i);
		Enqueue(&maxLivesList_tail, n);
		if (maxLivesList_head == NULL)
			maxLivesList_head = maxLivesList_tail;
	}
	//we trim all max lives nodes not exceeding the allocatable register
	FindMaxLivesExceeding(&maxLivesList_head, allocatableRegister);

	//determine spill register by left to right selection
	RegisterNode * spillReg = NULL;
	MaxLivesNode * mlData = NULL;
	int numSpillRegs;
	maxLivesList_curr = maxLivesList_head;
	while (maxLivesList_curr != NULL) {
		mlData = (MaxLivesNode *) maxLivesList_curr->data;
		maxLivesRegList = (Node*) (mlData->registerList);
		numSpillRegs = mlData->count - allocatableRegister;
		while (numSpillRegs > 0) {
			spillReg = NULL;
			data = NULL;
			if (mlData->count == 0) {
				numSpillRegs = 0;
				continue;
			}
			//pick the register to kick out of the max lives list
			//for that given line
			while (maxLivesRegList != NULL) {
				data = (char*) maxLivesRegList->data;
				spillReg = getRegisterNode(head_node, data);
				if (spillReg != NULL) {
					break;
				}
				maxLivesRegList = maxLivesRegList->next;
			}

			//if there were no more register to be a spill
			//then we skip this line
			if (spillReg == NULL) {
				numSpillRegs = 0;
				continue;
			}
			RegisterNode * spill_reg = removeRegisterNode(&head_node,
					spillReg->registerName);
			if (spill_reg == NULL)
				exit(0);
			Enqueue(&spillReg_tail, spill_reg);
			if (spillReg_head == NULL)
				spillReg_head = spillReg_tail;
			numSpillRegs--;
		}
		maxLivesList_curr = maxLivesList_curr->next;
	}
	//assign offset value to each spill registers
	AssignOffSetValue(spillReg_head);

	/*Node * spillCurr = spillReg_head;
	RegisterNode * spillData = NULL;
	while (spillCurr != NULL) {
		spillData = (RegisterNode*) spillCurr->data;

		printf("r: %s\toffset: %d\t\n", spillData->registerName,
				spillData->offset);

		spillCurr = spillCurr->next;
	}
	printf("\n");
	spillCurr = head_node;
	spillData = NULL;
	while (spillCurr != NULL) {
		spillData = (RegisterNode*) spillCurr->data;

		printf("r: %s\toffset: %d\t\n", spillData->registerName,
				spillData->offset);

		spillCurr = spillCurr->next;
	}*/

	//reset file pointer
	fseek(fp, 0, SEEK_SET);
	//print changes
	printChanges_Top(fp, meta, delim, &head_node, &spillReg_head,
			feasibleRegister);
	//free allocated stuff
	free(meta);
	FreeRegisterList(head_node);
	FreeRegisterList(spillReg_head);
	//close the file
	fclose(fp);

}
