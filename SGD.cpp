/*
 * SGD.cpp
 *
 *  Created on: Apr 28, 2015
 *      Author: prmathur
 */
#pragma once

#include "SGD.h"
#include "cpmStore.h"
#include <cmath>
#include <math.h>


SGD::SGD(unsigned int topics, unsigned int iterations, bool printVerbose, float learningrate, int negSamples) {
	m_maxIter=iterations;
	m_topics=topics;
	m_lr=learningrate;
	printModels=printVerbose;
	m_negative=negSamples;
	srandom(20);
}

SGD::~SGD() {
	// TODO Auto-generated destructor stub
}

void SGD::InitializeParams(){
	cpmStore* cpm = &cpmStore::Instance();
	for(cpmStore::iterator i = cpm->begin();
			i != cpm->end(); i++){
		for(unsigned short int j = 0;
				j < m_topics; j++){
			float noise = ((double) random() / (RAND_MAX)) + 1;
			Q[i->second][j] = 1/(m_topics + noise);
		}
	}
	char filename[20];
	sprintf(filename, "init.model");
	PrintModel(filename);
}

double SGD::HierarchicalSoftmax(Code* w, std::vector<Context*>& contexts, std::vector<double>& res, bool neg){
	// calculate 1/1+ e^(-1 * v_c * v_w)
	// calculate v_c*v_w
	double num=0;
	size_t numContexts=contexts.size();
	if(numContexts>10) numContexts=10;
	for(size_t i=0; i<numContexts; i++){ // for all contexts
		std::vector<Code*> c = contexts[i]->getCodes();
		for(int j=0; j < c.size(); j++){ // for all codes in context
			for(unsigned short int i=0; i<m_topics; i++){ // dot product of context and word
				num += Q[c[j]][i] * Q[w][i];
				res[i] += Q[c[j]][i];
			}
			std::transform(res.begin(), res.end(), res.begin(), std::bind1st(std::divides<double>(),c.size()));
		}
		if (neg==true) num *= -1;	// if its negative sample
	}
	std::transform(res.begin(), res.end(), res.begin(), std::bind1st(std::divides<double>(),numContexts));
	num /= numContexts;
	return log(1.0 / (1.0 + exp(-1.0 * num)));
}

SGD::SoftmaxOutput SGD::CalculateObjective(Code* w){
	// calculate negation contexts for w
	cpmStore *cpm = &cpmStore::Instance();
	unsigned long word_code = w->getCode();
	unsigned long total_codes = cpm->getVocabSize();
	std::vector<Context*> negContexts,posContexts;
	posContexts = w->getContexts();
	double posContextSize = (posContexts.size()>5) ? 5 : posContexts.size()-1;
	posContexts.erase(posContexts.begin()+posContextSize, posContexts.end());
	for(int i=0; i<m_negative; i++){
		unsigned long random_code = (rand()+1)%(total_codes);
		Code* negCode=NULL;
		if(random_code < total_codes && random_code != word_code) // condition for negative sample
			negCode=cpm->getNode(random_code);
		if(negCode!=NULL){
			negContexts.push_back(negCode->getContexts()[0]);
		} else{
			i--;
		}
	}
	std::vector<double> negVec(m_topics), posVec(m_topics);
	double negSim = HierarchicalSoftmax(w, negContexts, negVec, true);
	double posSim = HierarchicalSoftmax(w, posContexts, posVec, false);
	SGD::SoftmaxOutput S = {posVec, negVec, posSim, negSim};

	return S;
}

//int numDigits(size_t i){
//	return i > 0 ? (int) log10 ((double) i) + 1 : 1;
//}

void SGD::Step(){
	cpmStore *cpm = &cpmStore::Instance();
	if(cpm==NULL) {std::cerr<<"Corpus was not loaded properly!\n";exit(0);}
	size_t count=0;
	for(boost::unordered_map<std::string, Code*>::iterator itr=cpm->begin();
			itr!=cpm->end(); itr++){
		printf("%lldK%c", count / 1000, 13);
		fflush(stdout);
		SoftmaxOutput S = CalculateObjective(itr->second);
		for(unsigned short int i=0; i<m_topics; i++){
			Q[itr->second][i] += (S.possim > S.negsim) ?
					m_lr*(S.P[i] - Q[itr->second][i]) :
					2 * m_lr * (S.P[i] - Q[itr->second][i]);	// SGD update step for positive words
			Q[itr->second][i] -= (S.negsim > S.possim) ?
					m_lr*(S.N[i] - Q[itr->second][i]) :
					2 * m_lr * (S.N[i] - Q[itr->second][i]);	// SGD update step for negative words
 		}
		count++;
	}
	printf("\n");
	fflush(stdout);
}

void SGD::Train(){
	std::cerr<<"\nInitializing Parameters\n";
	InitializeParams();
	for(size_t i=1; i <= m_maxIter; i++){
		std::cerr<<"\nIteration "<<i<<std::endl;
		Step();
		if(printModels){
			char filename[20];
			sprintf(filename, "%d.iter.model", i);
			PrintModel(filename);
		}
	}
	return;
}

void SGD::PrintModel(char* filename){
	std::ofstream ofs (filename, std::ofstream::out);
	boost::unordered_map<Code*, boost::unordered_map<unsigned short int, double> >::iterator itr=Q.begin();
	while(itr!=Q.end()){
		ofs << itr->first->Decode() << " ";
		for(unsigned short int j = 0; j != m_topics; j++){
			ofs << Q[itr->first][j] << " ";
		}
		ofs <<std::endl;
		itr++;
	}
	return;
}
