#include "topdownallocator.h"

void* readInFile_Top(FILE * fp, char * delim, Node ** head_node_ref,
		Node ** tail_node_ref) {
	return readInFile_Simple(fp, delim, head_node_ref, tail_node_ref);
}
void printChanges_Top(FILE* fp, TopDownMetaNode * meta, char * delim,
		Node ** head_node_ref, Node ** spill_node_ref, int feasibleCount) {

	Node * feasible_reg_head_node = NULL;
	Node * feasible_reg_tail_node = NULL;
	Node * physical_reg_head_node = NULL;
	Node * physical_reg_tail_node = NULL;
	FeasibleRegisterNode * feasibleRegister = NULL;
	FeasibleRegisterNode * physicalRegister = NULL;

	char assignmentFound = 0;
	char commaAfterRegister = 0;
	char isWriteTo = 0;
	char isLoadTo = 0;

	int currInstroffset = 0;
	int afterInstroffset = 0;
	int beforeInstroffset = 0;

	char copy[2048];
	char beforeInstr[2048];
	char currInstr[2048];
	char afterInstr[2048];
	char debug[2048];

	char * ptr = NULL;
	char * r0 = "r0";
	char * r0_comma = "r0,";
	char * assignment = "=>";
	char * store = "store";
	char * storeAI = "storeAI";
	char * cstore = "cstore";
	char * cstoreAI = "cstoreAI";

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	//create feasible set
	CreateFeasibleSet(&feasible_reg_head_node, feasibleCount,
			meta->highestNodeNumber);
	feasible_reg_tail_node = feasible_reg_head_node;
	while (feasible_reg_tail_node->next != NULL) {
		feasible_reg_tail_node = feasible_reg_tail_node->next;
	}
	//create physical set
	CreateFeasibleSet(&physical_reg_head_node, k - feasibleCount,
			meta->highestNodeNumber + feasibleCount + 1);
	physical_reg_tail_node = physical_reg_head_node;
	if (physical_reg_tail_node != NULL) {
		while (physical_reg_tail_node->next != NULL) {
			physical_reg_tail_node = physical_reg_tail_node->next;
		}
	}

	//clear all the strings
	clearStr(currInstr, 2048);
	clearStr(beforeInstr, 2048);
	clearStr(afterInstr, 2048);
	clearStr(debug, 2048);
	//replace virtual register with physical register and
	//insert spill code for register going to memory
	while ((read = getline(&line, &len, fp)) != -1) {
		//make a copy of the line
		strcpy(copy, line);
		//operate only on non-white line, and non-comment
		if (read > 1 && !isOnlyWhiteSpace(copy, read) && line[0] != '/'
				&& line[1] != '/') {
			isWriteTo = 1;
			isLoadTo = 0;
			assignmentFound = 0;
			//tokenize the copy string
			ptr = strtok(copy, delim);
			//skip the command token
			sprintf(currInstr, "%s ", ptr);
			currInstroffset = strlen(currInstr);
			//check for store command
			if (strcmp(ptr, store) == 0 || strcmp(ptr, storeAI) == 0
					|| strcmp(ptr, cstore) == 0 || strcmp(ptr, cstoreAI) == 0) {
				isLoadTo = 1;
				isWriteTo = 0;
			}
			ptr = strtok(NULL, delim);
			//iterate through each token for this instruction
			while (ptr != NULL) {
				//remove return carriage
				if (ptr[strlen(ptr) - 1] == 13)
					ptr[strlen(ptr) - 1] = '\0';
				//skip r0 register token
				if (strcmp(ptr, r0) == 0 || strcmp(ptr, r0_comma) == 0) {
					sprintf(currInstr + currInstroffset, "%s ", ptr);
					currInstroffset = strlen(currInstr);
					ptr = strtok(NULL, delim);
					continue;
				}
				//skip assignment operator "=>"
				if (strcmp(ptr, assignment) == 0) {
					sprintf(currInstr + currInstroffset, "=> ");
					currInstroffset = strlen(currInstr);
					ptr = strtok(NULL, delim);
					assignmentFound = 1;
					continue;
				}
				//handle register token
				if (ptr[0] == 'r') {
					//remove comma
					if (ptr[strlen(ptr) - 1] == ',') {
						ptr[strlen(ptr) - 1] = '\0';
						commaAfterRegister = 1;
					}
					//assignment register
					if (assignmentFound == 1) {
						//the register token is part of the spill registers
						if (containRegister(*spill_node_ref, ptr)) {
							if ((feasibleRegister =
									containRegisterInFeasibleSet(
											feasible_reg_head_node, ptr))
									== NULL) {
								//since the feasible register is just a queue
								//we can take the top one and assume it is free
								//then move it to the back of the queue, denoting
								//it as used
								feasibleRegister =
										(FeasibleRegisterNode*) Dequeue(
												&feasible_reg_head_node);
								feasibleRegister->assignedRegister =
										getRegisterNode(*spill_node_ref, ptr);
								//feasible set does not use inused
								//so we dont need to modify the value
								//add this feasible register to the
								//back of the free queue
								Enqueue(&feasible_reg_tail_node,
										feasibleRegister);
								//insert spill code (loadAI if it is a read from)
								//only issue when feasible register is changed
								if (isLoadTo && isWriteTo == 0) {
									sprintf(beforeInstr + beforeInstroffset,
											"loadAI r0, %d => %s\n",
											feasibleRegister->assignedRegister->offset,
											feasibleRegister->feasibleRegisterName);
									beforeInstroffset = strlen(beforeInstr);
								}
							} else {
								//move the feasible register assigned to the given spill
								//register to the back of the list, if it is not the last one
								if (feasibleRegister
										!= ((FeasibleRegisterNode*) feasible_reg_tail_node->data)) {
									feasibleRegister =
											removeFeasibleRegisterNode(
													&feasible_reg_head_node,
													feasibleRegister->feasibleRegisterName);
									Enqueue(&feasible_reg_tail_node,
											feasibleRegister);
									if (feasible_reg_head_node == NULL)
										feasible_reg_head_node =
												feasible_reg_tail_node;
								}
							}
							sprintf(currInstr + currInstroffset, "%s ",
									feasibleRegister->feasibleRegisterName);
							currInstroffset = strlen(currInstr);
							//insert spill code (storeAI if it is a write to)
							if (isWriteTo && isLoadTo == 0) {
								sprintf(afterInstr + afterInstroffset,
										"storeAI %s => r0, %d",
										feasibleRegister->feasibleRegisterName,
										feasibleRegister->assignedRegister->offset);
								afterInstroffset = strlen(afterInstr);
							}
						}
						//the register token is part of the physical registers
						else {
							//determine if the physical sets contain the virtual register
							if ((physicalRegister =
									containRegisterInFeasibleSet(
											physical_reg_head_node, ptr))
									== NULL) {
								//assign the virtual register a physical register
								physicalRegister = unusedFeasibleRegister(
										physical_reg_head_node);
								physicalRegister->assignedRegister =
										getRegisterNode(*head_node_ref, ptr);
								physicalRegister->assignedRegister->writeCount--;
							}
							physicalRegister->inUsed = 1;
							//check if the instruction for this line is a
							//write to the assignment register or a read from
							if (isLoadTo && isWriteTo == 0) {
								//decrement read count for this virtual register
								physicalRegister->assignedRegister->readCount--;
								//designate this physical register as free
								if (physicalRegister->assignedRegister->readCount
										<= 0) {
									physicalRegister->inUsed = 0;
								}
							}

							if (physicalRegister->assignedRegister->readCount
									<= 0) {
								physicalRegister->inUsed = 0;
							}

							//print format
							if (commaAfterRegister == 0)
								sprintf(currInstr + currInstroffset, "%s ",
										physicalRegister->feasibleRegisterName);
							else
								sprintf(currInstr + currInstroffset, "%s, ",
										physicalRegister->feasibleRegisterName);
							currInstroffset = strlen(currInstr);
							commaAfterRegister = 0;
						}
					}
					//operand register
					else {
						//register token is part of the spill register
						if (containRegister(*spill_node_ref, ptr)) {
							//determine if there is an assigned feasible register to
							//this virtual register
							if ((feasibleRegister =
									containRegisterInFeasibleSet(
											feasible_reg_head_node, ptr))
									== NULL) {
								//find a free feasible register
								feasibleRegister =
										(FeasibleRegisterNode*) Dequeue(
												&feasible_reg_head_node);
								feasibleRegister->assignedRegister =
										getRegisterNode(*spill_node_ref, ptr);
								//insert spill code
								sprintf(beforeInstr + beforeInstroffset,
										"loadAI r0, %d => %s\n",
										feasibleRegister->assignedRegister->offset,
										feasibleRegister->feasibleRegisterName);
								beforeInstroffset = strlen(beforeInstr);
								//add the feasible register to the back off the list
								//to designate it as in used
								Enqueue(&feasible_reg_tail_node,
										feasibleRegister);
							} else {
								//move the feasible register assigned to the given spill
								//register to the back of the list, if it is no the last one
								if (feasibleRegister
										!= ((FeasibleRegisterNode*) feasible_reg_tail_node->data)) {
									feasibleRegister =
											removeFeasibleRegisterNode(
													&feasible_reg_head_node,
													feasibleRegister->feasibleRegisterName);
									Enqueue(&feasible_reg_tail_node,
											feasibleRegister);
									if (feasible_reg_head_node == NULL)
										feasible_reg_head_node =
												feasible_reg_tail_node;
								}
							}
							//printf format
							if (commaAfterRegister == 0)
								sprintf(currInstr + currInstroffset, "%s ",
										feasibleRegister->feasibleRegisterName);
							else
								sprintf(currInstr + currInstroffset, "%s, ",
										feasibleRegister->feasibleRegisterName);
							currInstroffset = strlen(currInstr);
							commaAfterRegister = 0;
						}
						//register token is part of the physical register
						else {
							physicalRegister = containRegisterInFeasibleSet(
									physical_reg_head_node, ptr);
							physicalRegister->assignedRegister->readCount--;
							if (physicalRegister->assignedRegister->readCount
									<= 0) {
								physicalRegister->inUsed = 0;
							}
							if (commaAfterRegister == 0)
								sprintf(currInstr + currInstroffset, "%s ",
										physicalRegister->feasibleRegisterName);
							else
								sprintf(currInstr + currInstroffset, "%s, ",
										physicalRegister->feasibleRegisterName);
							currInstroffset = strlen(currInstr);
							commaAfterRegister = 0;
						}
					}
				}
				//misc
				else {
					sprintf(currInstr + currInstroffset, "%s ", ptr);
					currInstroffset = strlen(currInstr);
				}
				//go to the next token
				ptr = strtok(NULL, delim);
			}
			//print out changes
			//before the current instr
			printf("%s", beforeInstr);
			//current instr
			printf("%s\n", currInstr);
			clearStr(debug, 2048);
			//after the current instr
			printf("%s", afterInstr);
			//clear the print string
			clearStr(currInstr, 2048);
			clearStr(beforeInstr, 2048);
			clearStr(afterInstr, 2048);
			currInstroffset = 0;
			beforeInstroffset = 0;
			afterInstroffset = 0;
			printf("\n");
		} else {
			//print out the misc line
			printf("%s", copy);
		}
	}

	//free the list
	FreeFeasibleSet(feasible_reg_head_node);
	FreeFeasibleSet(physical_reg_head_node);
}
void TopDownAllocation() {
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

	fp = fopen(name, "r");
	if (fp == NULL)
		exit(0);

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

	//determine the spill register from the remaining
	//max lives list for each line
	RegisterNode * spillReg = NULL;
	RegisterNode * smallestReg = NULL;
	MaxLivesNode * mlData = NULL;
	int numSpillRegs;
	maxLivesList_curr = maxLivesList_head;
	while (maxLivesList_curr != NULL) {
		mlData = (MaxLivesNode *) maxLivesList_curr->data;
		maxLivesRegList = (Node*) (mlData->registerList);
		numSpillRegs = mlData->count - allocatableRegister;
		//iterate through the register and find which one is the smallest
		while (numSpillRegs > 0) {
			smallestReg = NULL;
			spillReg = NULL;
			data = NULL;
			//determine the smallest one
			while (maxLivesRegList != NULL) {
				data = (char*) maxLivesRegList->data;
				smallestReg = getRegisterNode(head_node, data);
				if (smallestReg == NULL) {
					maxLivesRegList = maxLivesRegList->next;
					continue;
				}
				if (spillReg == NULL) {
					spillReg = smallestReg;
				}
				if (smallestReg->frequency < spillReg->frequency
						|| (smallestReg->frequency == spillReg->frequency
								&& (smallestReg->lineEnd
										- smallestReg->lineDeclaration)
										> (spillReg->lineEnd
												- spillReg->lineDeclaration))) {
					spillReg = smallestReg;
				}
				maxLivesRegList = maxLivesRegList->next;
			}
			//if there were no more register to be a spill
			//then we skip this line
			if (spillReg == NULL) {
				numSpillRegs = 0;
				continue;
			}
			//copy over data onto the spill register list
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

	//reset file pointer
	fseek(fp, 0, SEEK_SET);
	/*
	 Node * spillCurr = spillReg_head;
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

	printChanges_Top(fp, meta, delim, &head_node, &spillReg_head,
			feasibleRegister);

	//free allocated stuff
	free(meta);
	FreeRegisterList(head_node);
	FreeRegisterList(spillReg_head);
	//close the file
	fclose(fp);
}
