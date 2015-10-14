/*
 * EM.h
 *
 *  Created on: Apr 28, 2015
 *      Author: prmathur
 */

#pragma once

#ifndef SGD_H_
#define SGD_H_

#include <iostream>
#include <vector>
#include <string>

#include "cpmStore.h"

class SGD {

	struct SoftmaxOutput{
		std::vector<double> P;
		std::vector<double> N;
		double possim;
		double negsim;
	};
	float m_lr;
	int m_negative;
	size_t m_maxIter;
	unsigned int m_topics;
	bool printModels;
	void Step();
	double HierarchicalSoftmax(Code* w, std::vector<Context*>& c, std::vector<double>& res, bool neg=false);
	SoftmaxOutput CalculateObjective(Code* w);
	void InitializeParams();
	void normalize();

// Actual model is Q where the parameters are stored,
// P is for storing expected counts, later useless.
	// p, q store partial values
	boost::unordered_map<Code*, boost::unordered_map<unsigned short int, double> > Q;
public:
	SGD(unsigned int, unsigned int, bool, float, int);
	~SGD();
	void Train();
	void PrintModel(char*);



};


#endif /* SGD_H_ */
