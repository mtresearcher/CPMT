/*
 * cpmStore.h
 *
 *  Created on: Apr 27, 2015
 *      Author: prmathur
 */
#pragma once

#ifndef CPMSTORE_H_
#define CPMSTORE_H_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"

typedef unsigned long Vocab;

// there exist one context object for every word in the vocabulary
class Code;

class Context {
	friend class Code;
public:
	Context();
	~Context();
	bool add(Code*);
	bool remove(Code*);
	std::vector<Code*> getCodes() {return bow;}
	typedef std::vector<Code*>::iterator iterator;
	typedef std::vector<Code*>::const_iterator const_iterator;
	bool operator==(Context& b);

private:
	std::vector<Code*> bow; // map of context and their counts
};

class Code {
public:
	Code();
	Code(unsigned long, std::string);
	~Code();
	std::string Decode(){return c_string;}
	unsigned long getCode(){return c_code;}
	void SetCount(unsigned long c){c_counts=c; return;}
	unsigned long IncrementCount(){c_counts++; return c_counts;}
	unsigned long getCounts(){return c_counts;}
	bool operator==(Code& code){if(c_code == code.c_code) return true; else return false;}
	std::vector<Context*> getContexts(){return m_map;}
	void addContext(Context* con){ m_map.push_back(con); }
private:
	std::vector<Context*> m_map;
	std::string c_string;
	unsigned long c_code;
	double c_counts;
};

class cpmStore {
	friend class EM;
public:

	cpmStore();
	cpmStore(char *, int window=5, unsigned long DUP=5000000, bool prune=false);
	~cpmStore();
	static cpmStore& Instance(){ return *instance;}
	const static cpmStore& InstanceConst() { return *instance;}
	unsigned long getVocabSize(){return m_words;}
	unsigned short getContextSize(){return m_context_size; }
	boost::unordered_map<std::string, Code*> getVocab(){return m_vocab;}
	// iters
	typedef boost::unordered_map<std::string, Code*>::iterator iterator;
	iterator begin() {
		return m_vocab.begin();
	}
	iterator end() {
		return m_vocab.end();
	}
	Code* getNode(unsigned long c){return m_revmap[c];}

private:
	boost::unordered_map<std::string, Code*> m_vocab;
	bool LoadTrainingCorpus(char*);
	unsigned long m_words, m_dim, m_dup;
	unsigned short int m_context_size;
	bool m_prune;
	static cpmStore *instance;
	boost::unordered_map<unsigned long, Code*> m_revmap;
	void chop(std::string &);
	Code* EncodeWord(const std::string&);
	Code* getCode(std::string&);
	unsigned short int split_marker_perl(std::string&, std::string&,
			std::vector<std::string> &);
};


#endif /* CPMSTORE_H_ */
