#include "topdownallocator.h"

//read the file
void* readInFile_Simple(FILE * fp, char * delim, Node ** head_node_ref,
		Node ** tail_node_ref) {

	TopDownMetaNode* ret = NULL;

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
	ret->highestNodeNumber = 0;

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
			isWriteTo = 1;
			isLoadTo = 0;
			ret->code_lines++;
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
				if (ptr[strlen(ptr) - 1] == 13)
					ptr[strlen(ptr) - 1] = '\0';
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
				//check if current token is a register
				if (ptr[0] == 'r') {
					ret->highestNodeNumber = maxNodeNumber(ptr,
							ret->highestNodeNumber, strlen(ptr));
					//check if register is already in the list
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
				}
				//printf("%s ", ptr);	//debug
				//go to the next token
				ptr = strtok(NULL, delim);
			}
			//printf("\n");	//debug
		}
		clearStr(copy, 2048);
		//printf("[strlen %d] %d:%s", strlen(line), trueLineCount, line);
	}
	ret->MAX_LIVES = ret->registerCount;

	//close the file and free the line
	if (line != NULL)
		free(line);

	return (void*) ret;
}

void printChanges_Simple(FILE* fp, TopDownMetaNode * meta, char* delim,
		Node** node, int feasibleCount) {

	//printf("simple top down allocator printChanges\n");

	Node * feasible_reg_head_node = NULL;
	Node * feasible_reg_tail_node = NULL;

	FeasibleRegisterNode * feasibleRegister = NULL;

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
	//clear all strings
	clearStr(currInstr, 2048);
	clearStr(beforeInstr, 2048);
	clearStr(afterInstr, 2048);
	while ((read = getline(&line, &len, fp)) != -1) {
		//make a copy of the line
		strcpy(copy, line);
		//modify only non-white space, and non-comment line
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

			if (strcmp(ptr, store) == 0 || strcmp(ptr, storeAI) == 0
					|| strcmp(ptr, cstore) == 0 || strcmp(ptr, cstoreAI) == 0) {
				isLoadTo = 1;
				isWriteTo = 0;
			}
			ptr = strtok(NULL, delim);

			//parse through the rest of the instruction
			while (ptr != NULL) {
				if (ptr[strlen(ptr) - 1] == 13)
					ptr[strlen(ptr) - 1] = '\0';
				//skip r0 register token
				if (strcmp(ptr, r0) == 0 || strcmp(ptr, r0_comma) == 0) {
					//printf("%s ", ptr);			//debug
					sprintf(currInstr + currInstroffset, "%s ", ptr);
					currInstroffset = strlen(currInstr);
					ptr = strtok(NULL, delim);
					continue;
				}
				//skip assignment operator "=>"
				if (strcmp(ptr, assignment) == 0) {
					//printf("%s ", ptr);			//debug
					sprintf(currInstr + currInstroffset, "=> ");
					currInstroffset = strlen(currInstr);
					ptr = strtok(NULL, delim);
					assignmentFound = 1;
					continue;
				}
				//check if current token is a register
				if (ptr[0] == 'r') {
					if (ptr[strlen(ptr) - 1] == ',') {
						ptr[strlen(ptr) - 1] = '\0';
						commaAfterRegister = 1;
					}
					//assignment operand
					if (assignmentFound == 1) {
						//whatever register we use
						//it is a write to that register
						//is token register in R?
						if (containRegister(*node, ptr)) {
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
										getRegisterNode(*node, ptr);
								//feasibleRegister->inUsed = 1;
								Enqueue(&feasible_reg_tail_node,
										feasibleRegister);
							} else {
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
							//feasibleRegister->inUsed = 1;
							sprintf(currInstr + currInstroffset, "%s ",
									feasibleRegister->feasibleRegisterName);
							currInstroffset = strlen(currInstr);
							//insert spill code (storeAI)
							if (isWriteTo && isLoadTo == 0) {
								sprintf(afterInstr + afterInstroffset,
										"storeAI %s => r0, %d",
										feasibleRegister->feasibleRegisterName,
										feasibleRegister->assignedRegister->offset);
								afterInstroffset = strlen(afterInstr);
							}
							if (isLoadTo && isWriteTo == 0) {
								sprintf(beforeInstr + beforeInstroffset,
										"loadAI r0, %d => %s\n",
										feasibleRegister->assignedRegister->offset,
										feasibleRegister->feasibleRegisterName);
								beforeInstroffset = strlen(beforeInstr);
							}
						} else {
							if (commaAfterRegister == 0)
								sprintf(currInstr + currInstroffset, "%s ",
										ptr);
							else
								sprintf(currInstr + currInstroffset, "%s, ",
										ptr);
							currInstroffset = strlen(currInstr);
							commaAfterRegister = 0;
						}
					}
					//operand register
					else {
						//if token registers is a spill register
						//and it does not have a feasible register
						//assign to it, then take the top feasible
						//register (free unused register)
						if (containRegister(*node, ptr)) {
							if ((feasibleRegister =
									containRegisterInFeasibleSet(
											feasible_reg_head_node, ptr))
									== NULL) {
								//find a free feasible register
								feasibleRegister =
										(FeasibleRegisterNode*) Dequeue(
												&feasible_reg_head_node);
								feasibleRegister->assignedRegister =
										getRegisterNode(*node, ptr);
								//feasibleRegister->inUsed = 1;
								sprintf(beforeInstr + beforeInstroffset,
										"loadAI r0, %d => %s\n",
										feasibleRegister->assignedRegister->offset,
										feasibleRegister->feasibleRegisterName);
								beforeInstroffset = strlen(beforeInstr);
								Enqueue(&feasible_reg_tail_node,
										feasibleRegister);
							} else {
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
							//feasibleRegister->inUsed = 1;
							if (commaAfterRegister == 0)
								sprintf(currInstr + currInstroffset, "%s ",
										feasibleRegister->feasibleRegisterName);
							else
								sprintf(currInstr + currInstroffset, "%s, ",
										feasibleRegister->feasibleRegisterName);
							currInstroffset = strlen(currInstr);
						} else {
							//register is not in the register list
							if (commaAfterRegister == 0)
								sprintf(currInstr + currInstroffset, "%s ",
										ptr);
							else
								sprintf(currInstr + currInstroffset, "%s, ",
										ptr);
							currInstroffset = strlen(currInstr);
						}
						commaAfterRegister = 0;
					}
				} else {
					sprintf(currInstr + currInstroffset, "%s ", ptr);
					currInstroffset = strlen(currInstr);
				}

				//go to the next token
				ptr = strtok(NULL, delim);
			}
			printf("%s", beforeInstr);

			printf("%s\n", currInstr);

			printf("%s", afterInstr);

			//toggle all feasible register to unusued
			//ToggleUnusuedFeasibleSet(feasible_reg_head_node);
			//clear the print string
			clearStr(currInstr, 2048);
			clearStr(beforeInstr, 2048);
			clearStr(afterInstr, 2048);
			currInstroffset = 0;
			beforeInstroffset = 0;
			afterInstroffset = 0;
			printf("\n");	//debug
		} else {
			printf("%s", copy);
		}
	}

	FreeFeasibleSet(feasible_reg_head_node);
}

void SimpleTopDownAllocation() {

	//simple top down relies on occurrences
	//since the whole program is assumed to
	//be the live range for all variables
	int feasibleRegister = 3;	//3 is the minimum number we can have
	//int allocatableRegister = k - feasibleRegister;
	int allocatableRegister = k - feasibleRegister;
	if (allocatableRegister < 0) {
		//cannot have allocatable register less than 0
		exit(0);
	}

	//establish link list for virtual registers
	Node * head_node = NULL;
	Node * curr_node = NULL;

	char delim[] = " \t\n";

	fp = fopen(name, "r");
	if (fp == NULL)
		exit(0);

	//read in the file and mark down all register
	TopDownMetaNode * meta = (TopDownMetaNode*) readInFile_Simple(fp, delim,
			&head_node, &curr_node);

	//we only need 3 register from k for F

	//if register count is less than
	//physical register k, no need to change
	//anything we can return
	if (meta->MAX_LIVES <= allocatableRegister) {
		fclose(fp);
		return;
	}
	//sort into priority by frequency count
	//high frequencies = allocated register
	//least frequencies = memory
	sort(head_node);

	int i;
	//dequeue k-F allocatable register
	RegisterNode * _data = NULL;
	for (i = 0; i < allocatableRegister; i++) {
		_data = (RegisterNode*) Dequeue(&head_node);
		if (_data != NULL) {
			freeRegisterNode(_data);
		}
	}

	//assign the remaining registers an offset values
	AssignOffSetValue(head_node);

	//when we do the writing
	//for every register on the right hand side
	//perform an assignment to a Feasible Register then loadAI to r0, @offset
	//for every register on the left hand side
	//perform a loadAI r0, @offset into a Feasible Register
	//then store into @offset for that register

	//printf("\n\n");

	//reset file pointer
	fseek(fp, 0, SEEK_SET);

	//print out changes
	printChanges_Simple(fp, meta, delim, &head_node, feasibleRegister);

	//debug printing out the list
	/*
	 Node * c = head_node;
	 RegisterNode * d = NULL;
	 while (c != NULL) {
	 d = (RegisterNode*) c->data;
	 if (d != NULL) {
	 printf(
	 "register name: %s\t|\tfrequency: %d\t|\tline start: %d\t|\tline end: %d\t|\toffset: %d\t\n",
	 d->registerName, d->frequency, d->lineDeclaration,
	 d->lineEnd, d->offset);

	 for (i = 0; i < strlen(d->registerName); i++) {
	 printf("%d ", d->registerName[i]);
	 }
	 printf("\n");
	 }
	 c = c->next;
	 }
	 */
	free(meta);
	FreeRegisterList(head_node);
	//close the file
	fclose(fp);
}

