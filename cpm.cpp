//============================================================================
// Name        : cpm.cpp
// Author      : Prashant Mathur
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "cpmStore.h"
#include "EM.h"

int main(int argc, char*argv[]) {
	// parse arguments
	char *modelFile;
	char *filename="CPM-model.txt";
	cpmStore *cpm;
	unsigned int topics=50, iterations=30;
	int window_size = 5;
	unsigned int DUP = 30000;
	bool printIntermediate = false, prune=false;
	for(size_t i=0; i<argc; i++){
		if(strcmp(argv[i],"--train")==0){
			modelFile = argv[i+1];
		} else if(strcmp(argv[i], "--window")==0){
			window_size = atoi(argv[i+1]);
		} else if(strcmp(argv[i], "--DUP")==0){
			DUP = atoi(argv[i+1]);
		} else if(strcmp(argv[i], "--topics")==0){
			topics = atoi(argv[i+1]);
		} else if(strcmp(argv[i], "--iterations")==0){
			iterations = atoi(argv[i+1]);
		} else if(strcmp(argv[i], "--output")==0){
			filename = argv[i+1];
		} else if(strcmp(argv[i], "--printIntermediateModels")==0){
			printIntermediate = true;
		} else if(strcmp(argv[i], "--prune")==0){
			prune=true;
		}
	}
	if(modelFile == NULL) {
		std::cerr<< "No model file\n"; exit(0);
	}
	cpm = new cpmStore(modelFile, window_size, DUP, prune);
	EM *em = new EM(topics, iterations, printIntermediate);
	em->Train();
	em->PrintModel(filename);
	return 0;
}
