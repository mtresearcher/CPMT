/*
 * EM.cpp
 *
 *  Created on: Apr 28, 2015
 *      Author: prmathur
 */
#pragma once

#include "EM.h"
#include "cpmStore.h"


EM::EM(unsigned int topics, unsigned int iterations, bool printVerbose) {
	m_maxIter=iterations;
	m_topics=topics;
	printModels=printVerbose;
	srandom(20);
}

EM::~EM() {
	// TODO Auto-generated destructor stub
}

void EM::InitializeParams(){
	cpmStore* cpm = &cpmStore::Instance();
	for(cpmStore::iterator i = cpm->begin();
			i != cpm->end(); i++){
		for(unsigned short int j = 0;
				j < m_topics; j++){
			float noise = ((double) random() / (RAND_MAX)) + 1;
			P[i->second][j] = 1 / (cpm->getVocabSize() + noise);
			noise = ((double) random() / (RAND_MAX)) + 1; // different noise
			Q[i->second][j] = 1/(m_topics + noise);
		}
	}
	normalize();
	char filename[20];
	sprintf(filename, "init.model");
	PrintModel(filename);
	threads.clear();
}

void EM::normalize(){
	cpmStore *cpm = &cpmStore::Instance();
	m_totp.clear();
	m_totq.clear();
	if(cpm==NULL) return;
	for(cpmStore::iterator itr = cpm->begin(); itr != cpm->end(); itr++){ // itr->first = word
		for(unsigned short int j = 0; j < m_topics; j++){
			m_totq[itr->second] += Q[itr->second][j];
			m_totp[j] += P[itr->second][j];
		}
	}
	for(cpmStore::iterator itr = cpm->begin(); itr != cpm->end(); itr++){ // itr->first = word
		for(unsigned short int j = 0; j < m_topics; j++){
			P[itr->second][j] /= m_totp[j];
			Q[itr->second][j] /= m_totq[itr->second];
		}
	}
}

void EM::ComputeThreadSpecificCounts(thread_specific_data& data){
	for(unsigned short int j = 0; j < m_topics; j++){
		data.product[j] = 1;
		bool flag = false;
		std::vector<Code*> cont_codes = data.context->getCodes();
		for(size_t indx=0; indx < cont_codes.size(); indx++){
			data.product[j] *= P[cont_codes[indx]][j];
			if(data.product[j] < 1e-16) {data.product[j]=0; flag=true; break;}
		}
		if(flag==true) continue;
		data.denominator += (data.product[j] * Q[data.code][j]);
	}
}

void EM::CalculatePartialCounts(thread_specific_data& data){
	double temp=0;
	for(unsigned short int j = 0; j < m_topics; j++){
		temp = (data.product[j] * Q[data.code][j]) / data.denominator;
		std::vector<Code*> cont_codes = data.context->getCodes();
		for(size_t indx=0; indx < cont_codes.size(); indx++){
			p[cont_codes[indx]][j] += temp;
			m_totp[j] += temp;
		}
		q[data.code][j]+=temp;
		m_totq[data.code] += temp;
	}
}

void EM::ComputeExpectedCounts(){
	std::vector<std::thread> threads;
	cpmStore *cpm = &cpmStore::Instance();
	if(cpm==NULL) {std::cerr<<"Corpus was not loaded properly!\n";exit(0);}
	size_t count=0;
	for(boost::unordered_map<std::string, Code*>::iterator itr=cpm->begin();
			itr!=cpm->end(); itr++){
		count++;
		if(count%1000==0) std::cerr<<".";
		boost::unordered_map<unsigned short int, double> product; // \pi on topics
		double denominator=0;
		std::vector<Context*> cont_vec = itr->second->getContexts();
		for(size_t i=0; i<cont_vec.size(); i++){
			for(unsigned short int j = 0; j < m_topics; j++){
				if(Q[itr->second][j] == 0) continue;
				product[j] = 1;
				bool flag = false;
				std::vector<Code*> cont_codes = cont_vec[i]->getCodes();
				for(size_t indx=0; indx < cont_codes.size(); indx++){
					if(P[cont_codes[indx]][j] > 0 && product[j] > 0){
						product[j] *= P[cont_codes[indx]][j];
					}
					if(product[j] < 1e-16) {product[j]=0; flag=true; break;}
				}
				if(flag==true) continue;
				denominator += (product[j] * Q[itr->second][j]);
			}
			if(denominator==0) {
				continue;
			}
			double temp=0;
			for(unsigned short int j = 0; j < m_topics; j++){
				if(Q[itr->second][j] == 0 || product[j]==0) continue;
				temp = (product[j] * Q[itr->second][j]) / denominator;
				std::vector<Code*> cont_codes = cont_vec[i]->getCodes();
				for(size_t indx=0; indx < cont_codes.size(); indx++)
					p[cont_codes[indx]][j] += temp;
				m_totp[j] += cont_codes.size()*temp;
				q[itr->second][j]+=temp;
				m_totq[itr->second] += temp;
			}
		}
	}
}

void EM::MStep(){
	// normalize to probabilities
	cpmStore *cpm = &cpmStore::Instance();
	size_t count=0;
	for(cpmStore::iterator itr = cpm->begin();
				itr != cpm->end(); itr++){ // itr->first = word
		count++;
		if(count%1000==0) std::cerr<<".";
		for(unsigned short int j = 0; j < m_topics; j++){
			if(m_totq[itr->second] > 0){
				Q[itr->second][j] = q[itr->second][j] / m_totq[itr->second];
				if(Q[itr->second][j] < 1e-10){
					Q[itr->second][j]=0;
					std::cerr<<"0\b";
				}
			}
			if(m_totp[j] > 0){
				P[itr->second][j] = p[itr->second][j] / m_totp[j];
				if(P[itr->second][j] < 1e-10){
					P[itr->second][j]=0;
					std::cerr<<"0\b";
				}
			}
		}
	}
}

void EM::ResetExpectedCounts(){
	p.clear();
	q.clear();
	m_totp.clear();
	m_totq.clear();
	return;
}

void EM::Train(){
	std::cerr<<"\nInitializing Parameters\n";
	InitializeParams();
	for(size_t i=1; i <= m_maxIter; i++){
		std::cerr<<"\nIteration "<<i<<std::endl;
		ResetExpectedCounts();
		// compute expected counts
		std::cerr<<"\nE Step\n";
		ComputeExpectedCounts();
		// maximise probabilities
		std::cerr<<"\nM Step\n";
		MStep();
		if(printModels){
			char filename[20];
			sprintf(filename, "%d.iter.model", i);
			PrintModel(filename);
		}
	}
	return;
}

void EM::PrintModel(char* filename){
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
