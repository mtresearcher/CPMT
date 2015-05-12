/*
 * cpmStore.cpp
 *
 *  Created on: Apr 27, 2015
 *      Author: prmathur
 */
#pragma once

#define MAXW 50
#define MAXSIZE 1000

#include "cpmStore.h"
#include<map>

cpmStore *cpmStore::instance=NULL;

cpmStore::cpmStore(char *filename, int window, unsigned long DUP, bool prune)  {
	// TODO Auto-generated constructor stub
	instance = this;
	m_context_size = window;
	m_dup=DUP;
	m_prune=prune;
	LoadTrainingCorpus(filename);
}

cpmStore::~cpmStore() {
	// TODO Auto-generated destructor stub
}

bool cpmStore::LoadTrainingCorpus(char* filename){
//	std::cerr<<filename<<std::endl;
	boost::unordered_map<std::string, unsigned long> freqMap;
	// do one pass ... encode all words .. prune to top N words ..
	std::ifstream file(filename);
	std::string line;
	if(file.is_open())
	{
		while(getline(file, line)){
			std::vector<std::string> vecStr;
			std::string del = " ";
			split_marker_perl(line, del, vecStr);
			std::string temp("<s>");
			freqMap[temp]=1;
			for(size_t i=0; i<vecStr.size(); i++){
				freqMap[vecStr[i]]++;
			}
		}
	}
	file.close();

	// Encoding
	for(boost::unordered_map<std::string, unsigned long>::iterator itr=freqMap.begin();
			itr!=freqMap.end(); itr++){
		if(m_prune){
			if(itr->second >= 5){
				Code* c=EncodeWord(itr->first);
				c->SetCount(itr->second);
			}
		} else {
			Code* c=EncodeWord(itr->first);
			c->SetCount(itr->second);
		}
	}

	// storing context
	std::ifstream file1(filename);
	if(file1.is_open())
	{
		while(getline(file1, line)){
			chop(line);
			std::vector<std::string> vecStr;
			std::string del = " ";
			split_marker_perl(line, del, vecStr);
			for(size_t i=0; i<vecStr.size(); i++){
				Code *c = getCode(vecStr[i]);
				if(c!=NULL){
					Context *con = (Context*)calloc(1, sizeof(Context));
					con = new Context();
					for(int j = i - (floor(m_context_size/2)) ; j <= i + (floor(m_context_size/2)); j++ ){
						if(j == i) continue;
						std::string con_word="<s>";
						if(j >= 0 && j < vecStr.size()) con_word = vecStr[j];
						Code *con_word_code = getCode(con_word);
						if(con_word_code != NULL){
							con->add(con_word_code);
						}
					}
					c->addContext(con);
				}
			}
		}
	}
	file1.close();
	std::cerr<<"Vocabulary Size : "<<m_vocab.size()<<std::endl;
	// finished storing context

	return true;
}

Code *cpmStore::getCode(std::string& s){
	Code *c = NULL;
	if(m_vocab.find(s)!=m_vocab.end()){
		c = m_vocab[s];
	}
	return c;
}

Code* cpmStore::EncodeWord(const std::string& s){
	Code *c = NULL;
	if(m_vocab.find(s)!=m_vocab.end()){
		return m_vocab[s];
	}
	c = (Code*)calloc(1, sizeof(Code));
	c = new Code(m_words, s);
	m_vocab[s] = c;
	m_words++;
	return c;
}

Code::Code(unsigned long c, std::string s){
	c_code = c;
	c_counts=0;
	c_string=s;
	m_map.clear();
}

Code::~Code(){
	c_code=0;
	c_counts=0;
	c_string="";
	m_map.clear();
}

void cpmStore::chop(std::string &str) {
	int i = 0;
	while (isspace(str[i]) != 0) {
		str.replace(i, 1, "");
	}
	while (isspace(str[str.length() - 1]) != 0) {
		str.replace(str.length() - 1, 1, "");
	}
	return;
}

unsigned short int cpmStore::split_marker_perl(std::string& str, std::string& marker,
		std::vector<std::string> &array) {
	int found = str.find(marker), prev = 0;
	while (found != std::string::npos) // warning!
	{
		array.push_back(str.substr(prev, found - prev));
		prev = found + marker.length();
		found = str.find(marker, found + marker.length());
	}
	array.push_back(str.substr(prev));
	return array.size() - 1;
}

Context::Context(){
	bow.clear();
}

Context::~Context(){
	bow.clear();
}

bool Context::add(Code* indx){
	for(std::vector<Code*>::iterator itr=bow.begin(); itr!=bow.end(); itr++){
		if(**itr == *indx) return true;
	}
	bow.push_back(indx);
	return true;
}

bool Context::remove(Code* indx){
	for(std::vector<Code*>::iterator itr=bow.begin(); itr!=bow.end(); itr++){
		if(**itr == *indx){
			bow.erase(itr);
			return true;
		}
	}
	return false;
}

bool Context::operator==(Context& b){
	for(unsigned long i = 0; i < bow.size(); i++){
		if(bow[i] != b.bow[i]) return false;
	}
	return true;
}
