/*
 * EM.h
 *
 *  Created on: Apr 28, 2015
 *      Author: prmathur
 */

#pragma once

#ifndef EM_H_
#define EM_H_

#include <thread>

#include <iostream>
#include <vector>
#include <string>

#include "cpmStore.h"

struct thread_specific_data{
	Code* code;
	Context* context;
	double denominator;
	boost::unordered_map<int, double> product;
};

class EM {

public:
	EM(unsigned int, unsigned int, bool);
	~EM();
	void Train();
	void PrintModel(char*);
//	void Run(){return ;}
//	bool DeleteAfterExecution(){return true;}

	void ComputeThreadSpecificCounts(thread_specific_data&);
	void CalculatePartialCounts(thread_specific_data&);

private:
	size_t m_maxIter;
	unsigned int m_topics;
	bool printModels;
	void ComputeExpectedCounts();
//	void ComputeThreadSpecificCounts(Code* code, Context* context, double &denominator);
	void MStep();
	void ResetExpectedCounts();
	void InitializeParams();
	void normalize();

// Actual model is Q where the probablities are stored,
// P is for storing expected counts, later useless.
	// p, q store partial values
	boost::unordered_map<Code*, boost::unordered_map<unsigned short int, double> > P, Q, p, q;
	boost::unordered_map<double, double> m_totp;
	boost::unordered_map<Code*, double> m_totq;
// for multi threading
	std::vector<pthread_t> threads;
};


#endif /* EM_H_ */
