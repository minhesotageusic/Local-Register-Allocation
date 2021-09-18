#include "topdownallocator.h"

typedef struct localityNode {
	char * reg;
	int length;
	int line;
} LocalityNode;

//push a new node with the given data to
//the front of the stack
void Push(Node ** head_ref, void * new_data) {
	if (head_ref == NULL)
		return;
	if (new_data == NULL)
		return;

	Node * new_node = (Node *) malloc(sizeof(Node));
	new_node->data = new_data;
	new_node->next = *head_ref;

	*head_ref = new_node;
}
//remove the top of the stack and return
//the data
void* Pop(Node ** head_ref) {
	return Dequeue(head_ref);
}

//find the first register from the head_ref
LocalityNode * findRegister_BottomUp(Node * head_ref, char * reg) {
	if (head_ref == NULL)
		return NULL;
	Node * node = head_ref;
	MaxLivesNode * data = NULL;
	//iterate through each line
	while (node != NULL) {
		data = (MaxLivesNode*) node->data;
		//iterate through the localNode's register list
		if (data != NULL) {
			Node * localNode = data->registerList;
			LocalityNode * localData = NULL;
			while (localNode != NULL) {
				localData = (LocalityNode*) localNode->data;
				//check if local node is the one we are looking for
				if (localData != NULL && strcmp(localData->reg, reg) == 0) {
					return localData;
				}
				localNode = localNode->next;
			}
		}
		node = node->next;
	}
	return NULL;
}
//find the given register on the given line
//return NULL if no such line or register
//exists on that line
LocalityNode * findRegisterAt_BottomUp(Node * head_ref, char * reg, int line) {
	if (head_ref == NULL)
		return NULL;
	Node * node = head_ref;
	MaxLivesNode * data = NULL;
	while (node != NULL) {
		data = (MaxLivesNode*) node->data;
		if (data != NULL && data->line == line) {
			Node * localNode = data->registerList;
			LocalityNode * localData = NULL;
			while (localNode != NULL) {
				localData = (LocalityNode*) localNode->data;
				//check if local node is the one we are looking for
				if (localData != NULL && strcmp(localData->reg, reg) == 0) {
					return localData;
				}
				localNode = localNode->next;
			}
		}
		node = node->next;
	}
	return NULL;
}
//return a physical register suitable to be replaced
FeasibleRegisterNode * getPhysicaRegister_BottomUp_Operand(Node * head_ref) {
	if (head_ref == NULL)
		return NULL;
	Node * curr = head_ref;
	FeasibleRegisterNode * data = NULL;
	FeasibleRegisterNode * furthestPR = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode *) curr->data;
		if (data != NULL && data->containOperandRegister == 0) {
			RegisterNode * regNode = data->assignedRegister;
			//return data only if there are no assigned register
			//or the assigned register has a length of zero
			if (regNode == NULL
					|| (data->inUsed == 0 && regNode->length <= 0)) {
				return data;
			}
			if (furthestPR == NULL
					|| furthestPR->assignedRegister->length < regNode->length) {
				furthestPR = data;
			}
		}
		curr = curr->next;
	}
	return furthestPR;
}
FeasibleRegisterNode * getPhysicaRegister_BottomUp_Assignment(Node * head_ref,
		char isInstrStore) {
	if (head_ref == NULL)
		return NULL;
	Node * curr = head_ref;
	FeasibleRegisterNode * data = NULL;
	FeasibleRegisterNode * furthestPR = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode *) curr->data;
		if (data != NULL
				&& ((isInstrStore == 0)
						|| (data->containOperandRegister == 0
								&& isInstrStore == 1))) {
			RegisterNode * regNode = data->assignedRegister;
			//return data only if there are no assigned register
			//or the assigned register has a length of zero
			if (regNode == NULL
					|| (data->inUsed == 0 && regNode->length <= 0)) {
				return data;
			}
			if (furthestPR == NULL
					|| furthestPR->assignedRegister->length < regNode->length) {
				furthestPR = data;
			}
		}
		curr = curr->next;
	}
	return furthestPR;
}
void DecrementPhysicalRegister(Node * physical_reg_head) {
	if (physical_reg_head == NULL)
		return;
	Node * curr = physical_reg_head;
	FeasibleRegisterNode * data = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode *) curr->data;
		if (data != NULL) {
			RegisterNode * regNode = data->assignedRegister;
			//return data only if there are no assigned register
			//or the assigned register has a length of zero
			if (regNode != NULL)
				regNode->length--;
		}
		curr = curr->next;
	}
}

void TogglePhysicalRegister(Node * physical_reg_head) {
	if (physical_reg_head == NULL)
		return;
	Node * curr = physical_reg_head;
	FeasibleRegisterNode * data = NULL;
	while (curr != NULL) {
		data = (FeasibleRegisterNode *) curr->data;
		if (data != NULL) {
			data->containOperandRegister = 0;
		}
		curr = curr->next;
	}
}

void FreeBottomUpList(Node * head) {
	if (head == NULL)
		return;
	Node * curr = head;
	MaxLivesNode * data = NULL;
	while (curr != NULL) {
		data = (MaxLivesNode*) Dequeue(&curr);
		if (data != NULL) {
			Node * localNode = data->registerList;
			LocalityNode * localData = NULL;
			while (localNode != NULL) {
				localData = (LocalityNode*) Dequeue(&localNode);
				if (localData != NULL) {
					free(localData->reg);
					free(localData);
				}
			}
		}
	}
}

void* readInFile_Bottom(FILE * fp, char * delim, Node ** head_node_ref,
		Node ** tail_node_ref, Node ** bottomUp_head, Node ** bottomUp_tail) {
	TopDownMetaNode* ret = NULL;

	LocalityNode * localNode_Register = NULL;

	char isLoadTo = 0;
	char isWriteTo = 0;
	char assignmentFound = 0;

	char copy[2048];
	char * ptr = NULL;
	char * r0 = "r0";
	char * store = "store";
	char * storeAI = "storeAI";
	char * cstore = "cstore";
	char * cstoreAI = "cstoreAI";
	char * assignment = "=>";

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	ret = (TopDownMetaNode*) malloc(sizeof(TopDownMetaNode));
	ret->MAX_LIVES = 0;
	ret->code_lines = 0;
	ret->total_lines = 0;
	ret->registerCount = 0;

	clearStr(copy, 2048);

	while ((read = getline(&line, &len, fp)) != -1) {
		ret->total_lines++;
		//make a copy of the line
		strcpy(copy, line);
		//read = 1 => read = "\0"
		//increment code line count only if the line is not a comment
		//and it does not contain only white space
		if (read > 1 && !isOnlyWhiteSpace(copy, read) && line[0] != '/'
				&& line[1] != '/') {
			Node * locality_tail = NULL;
			isWriteTo = 1;
			isLoadTo = 0;
			ret->code_lines++;

			//create a node for this line
			MaxLivesNode * lineNode = (MaxLivesNode*) malloc(
					sizeof(MaxLivesNode));
			lineNode->line = ret->code_lines;
			lineNode->registerList = NULL;
			//parse this instruction
			ptr = strtok(copy, delim);
			if (strcmp(ptr, store) == 0 || strcmp(ptr, storeAI) == 0
					|| strcmp(ptr, cstore) == 0 || strcmp(ptr, cstoreAI) == 0) {
				isLoadTo = 1;
				isWriteTo = 0;
			}
			//skip the first token since it is a command
			ptr = strtok(NULL, delim);
			//we need to count the registers for this one instruction
			while (ptr != NULL) {
				//remove return carriage
				if (ptr[strlen(ptr) - 1] == 13)
					ptr[strlen(ptr) - 1] = '\0';
				//remove comma for register token
				if (ptr[0] == 'r' && ptr[strlen(ptr) - 1] == ',')
					ptr[strlen(ptr) - 1] = '\0';
				//skip r0 register
				if (strcmp(ptr, r0) == 0) {
					//printf("%s ", ptr);
					ptr = strtok(NULL, delim);
					continue;
				}
				//skip assignment operator "=>"
				if (strcmp(ptr, assignment) == 0) {
					ptr = strtok(NULL, delim);
					assignmentFound = 1;
					continue;
				}
				//token is a register token
				if (ptr[0] == 'r') {
					ret->highestNodeNumber = maxNodeNumber(ptr,
							ret->highestNodeNumber, strlen(ptr));
					//add register to list of register encountered
					if (!containRegister(*(head_node_ref), ptr)) {
						//create a new RegisterNode if it is not
						RegisterNode * rn = createRegisterNode(ptr, 1,
								ret->total_lines, ret->total_lines, 0);
						if (rn == NULL)
							exit(0);
						rn->readCount = 0;
						rn->writeCount = 0;
						Enqueue(tail_node_ref, rn);
						if (*(head_node_ref) == NULL)
							*(head_node_ref) = *(tail_node_ref);
						ret->registerCount++;
						if (assignmentFound) {
							if (isLoadTo) {
								rn->readCount++;
							} else {
								rn->writeCount++;
							}
						} else {
							rn->readCount++;
						}
						assignmentFound = 0;
					} else {
						//find the register and increment frequency count
						//also update its line end
						RegisterNode * node = getRegisterNode(*(head_node_ref),
								ptr);
						node->frequency++;
						node->lineEnd = ret->total_lines;
						if (assignmentFound) {
							if (isLoadTo) {
								node->readCount++;
							} else {
								node->writeCount++;
							}
						} else {
							node->readCount++;
						}
						assignmentFound = 0;
					}
					//check register's locality by updating
					//its farthest usage for each line
					if ((localNode_Register = findRegister_BottomUp(
							*bottomUp_head, ptr)) != NULL) {
						//update the length of the previous first occurrences
						localNode_Register->length = lineNode->line
								- localNode_Register->line;

					}
					localNode_Register = NULL;
					//create a new locality node for this occurrences on this line
					localNode_Register = (LocalityNode *) malloc(
							sizeof(LocalityNode));
					localNode_Register->reg = (char*) malloc(
							sizeof(char) * strlen(ptr));
					strcpy(localNode_Register->reg, ptr);
					localNode_Register->length = -1;
					localNode_Register->line = lineNode->line;
					//add to this line's list of encountered registers
					Enqueue(&locality_tail, localNode_Register);
					if (lineNode->registerList == NULL)
						lineNode->registerList = locality_tail;
				}
				//go to the next token
				ptr = strtok(NULL, delim);
			}

			//push this line's node into the bottomUp list
			Push(bottomUp_head, lineNode);
			if (*bottomUp_tail == NULL)
				*bottomUp_tail = *bottomUp_head;
		}
		clearStr(copy, 2048);
	}

//close the file and free the line
	if (line != NULL)
		free(line);

	return (void*) ret;
}

void printChanges_Bottom(FILE* fp, TopDownMetaNode * meta, char * delim,
		Node ** registerList_head, Node ** bottomUp_head, int feasibleCount) {

//list for physical register
	Node * physical_reg_head_node = NULL;
	Node * physical_reg_tail_node = NULL;
	FeasibleRegisterNode * physicalRegister = NULL;

//extra toggle
	char assignmentFound = 0;
	char commaAfterRegister = 0;
	char isWriteTo = 0;
	char isLoadTo = 0;

//print string offset
	int currInstroffset = 0;
	int afterInstroffset = 0;
	int beforeInstroffset = 0;

//holds print strings
	char copy[2048];
	char beforeInstr[2048];
	char currInstr[2048];
	char afterInstr[2048];
	char testStr[2048];

//extra string comparable
	char * ptr = NULL;
	char * r0 = "r0";
	char * r0_comma = "r0,";
	char * assignment = "=>";
	char * store = "store";
	char * storeAI = "storeAI";
	char * cstore = "cstore";
	char * cstoreAI = "cstoreAI";

//file helper
	int totalLine = 0;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	//create physical set
	CreateFeasibleSet(&physical_reg_head_node, feasibleCount,
			meta->highestNodeNumber);
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
	clearStr(testStr, 2048);

//read back the file to print the changes
	while ((read = getline(&line, &len, fp)) != -1) {
		//make copy of the line
		strcpy(copy, line);
		//only operates on non-white lines and non-comma
		if (read > 1 && !isOnlyWhiteSpace(copy, read) && line[0] != '/'
				&& line[1] != '/') {
			totalLine++;
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
					//find locality node corresponding to this register
					LocalityNode * localNode = findRegisterAt_BottomUp(
							*bottomUp_head, ptr, totalLine);

					//assignment
					if (assignmentFound == 1) {
						//check if register token is already assigned to a
						//physical register
						if ((physicalRegister = containRegisterInFeasibleSet(
								physical_reg_head_node, ptr)) == NULL) {

							//find a suitable physical register to assign current
							//virtual register
							physicalRegister =
									getPhysicaRegister_BottomUp_Assignment(
											physical_reg_head_node, isLoadTo);
							//store replaced register's value
							if (physicalRegister->assignedRegister != NULL
									&& ((isLoadTo == 0)
											|| (isLoadTo == 1
													&& physicalRegister->containOperandRegister
															== 0))) {
								physicalRegister->containOperandRegister = 0;
								physicalRegister->inUsed = 0;
								//debug_print_physicalregister(
								//		physical_reg_head_node, testStr);
								sprintf(beforeInstr + beforeInstroffset,
										"storeAI %s => r0, %d\n",
										physicalRegister->feasibleRegisterName,
										physicalRegister->assignedRegister->offset);
								//clearStr(testStr, 2048);
								beforeInstroffset = strlen(beforeInstr);
							}
							//assign physical register to the current virtual register
							physicalRegister->assignedRegister =
									getRegisterNode(*registerList_head, ptr);

							//insert spill code (loadAI if it is a read from)
							if (isLoadTo && isWriteTo == 0) {
								//debug_print_physicalregister(
								//		physical_reg_head_node, testStr);
								sprintf(beforeInstr + beforeInstroffset,
										"loadAI r0, %d => %s\n",
										physicalRegister->assignedRegister->offset,
										physicalRegister->feasibleRegisterName);
								beforeInstroffset = strlen(beforeInstr);
								//clearStr(testStr, 2048);
							}
						}
						physicalRegister->inUsed = 1;
						physicalRegister->assignedRegister->length =
								localNode->length;
						if (physicalRegister->assignedRegister->length <= 0) {
							physicalRegister->inUsed = 0;
						}
						//add changes to current instruction
						sprintf(currInstr + currInstroffset, "%s ",
								physicalRegister->feasibleRegisterName);
						currInstroffset = strlen(currInstr);
						//insert spill code (storeAI if it is a write to)
					}
					//operand
					else {
						if ((physicalRegister = containRegisterInFeasibleSet(
								physical_reg_head_node, ptr)) == NULL) {
							//find a suitable physical register to assign current
							//virtual register
							physicalRegister =
									getPhysicaRegister_BottomUp_Operand(
											physical_reg_head_node);
							//store replaced register's value
							if (physicalRegister->assignedRegister != NULL) {
								physicalRegister->containOperandRegister = 0;
								physicalRegister->inUsed = 0;
								//debug_print_physicalregister(
								//		physical_reg_head_node, testStr);
								sprintf(beforeInstr + beforeInstroffset,
										"storeAI %s => r0, %d\n",
										physicalRegister->feasibleRegisterName,
										physicalRegister->assignedRegister->offset);
								beforeInstroffset = strlen(beforeInstr);
								//clearStr(testStr, 2048);
							}
							//assign physical register to this virtual register
							physicalRegister->assignedRegister =
									getRegisterNode(*registerList_head, ptr);
							//set the length till next usage to be this register's
							//local node's next usage
							physicalRegister->assignedRegister->length =
									localNode->length;
							//printf format
							//debug_print_physicalregister(physical_reg_head_node,
							//		testStr);
							sprintf(beforeInstr + beforeInstroffset,
									"loadAI r0, %d => %s\n",
									physicalRegister->assignedRegister->offset,
									physicalRegister->feasibleRegisterName);
							//clearStr(testStr, 2048);
							beforeInstroffset = strlen(beforeInstr);
						}
						physicalRegister->containOperandRegister = 1;
						physicalRegister->inUsed = 1;
						physicalRegister->assignedRegister->length =
								localNode->length;
						if (physicalRegister->assignedRegister->length <= 0) {
							physicalRegister->inUsed = 0;
						}
						debug_print_physicalregister(physical_reg_head_node,
								testStr);
						//printf format
						if (commaAfterRegister == 0)
							sprintf(currInstr + currInstroffset, "%s ",
									physicalRegister->feasibleRegisterName);
						else
							sprintf(currInstr + currInstroffset, "%s, ",
									physicalRegister->feasibleRegisterName);
						currInstroffset = strlen(currInstr);
						commaAfterRegister = 0;
						clearStr(testStr, 2048);
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
			//debug_print_physicalregister(physical_reg_head_node, testStr);
			printf("%s\n", currInstr);
			//clearStr(testStr, 2048);
			//after the current instr
			printf("%s", afterInstr);
			TogglePhysicalRegister(physical_reg_head_node);
			DecrementPhysicalRegister(physical_reg_head_node);
			//clear the print string
			clearStr(currInstr, 2048);
			clearStr(beforeInstr, 2048);
			clearStr(afterInstr, 2048);
			currInstroffset = 0;
			beforeInstroffset = 0;
			afterInstroffset = 0;
			printf("\n");
		}
		//print out misc
		else {
			printf("%s", copy);
		}
	}

	FreeFeasibleSet(physical_reg_head_node);
}

void BottomUpAllocation() {
	int allocatableRegister = k;
	if (allocatableRegister < 3) {
		//cannot have allocatable register less than 3
		exit(0);
	}
//establish link list for physical register
	Node * head_node = NULL;
	Node * tail_node = NULL;

	Node * bottomUp_head = NULL;
	Node * bottomUp_tail = NULL;

	char delim[] = " \t\n";

	fp = fopen(name, "r");
	if (fp == NULL)
		exit(0);

	TopDownMetaNode * meta = (TopDownMetaNode*) readInFile_Bottom(fp, delim,
			&head_node, &tail_node, &bottomUp_head, &bottomUp_tail);

//assign offset value to all registers found
	AssignOffSetValue(head_node);

	//reset file pointer
	fseek(fp, 0, SEEK_SET);
	printChanges_Bottom(fp, meta, delim, &head_node, &bottomUp_head, k);

	free(meta);
	FreeBottomUpList(bottomUp_head);
	FreeRegisterList(head_node);
	fclose(fp);
}
