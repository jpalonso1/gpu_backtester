//Juan Pablo Alonso

#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "setup.h"
//#include "s_momentum.h"
#include "s_crossingma.h"

static char* dataFile={"multiStock.csv"};
//static std::ofstream testOut("testOut.csv");

namespace bt{

long setParameters(thrust::host_vector<bt::parameters>& par){
	return maParameters(par);
}

//This function is called in every thread. DO NOT modify function name or arguments
__device__ __host__
inline void runExecution(bt::stockData* data,long dataSize,
		bt::execution& exec,const bt::parameters& par){
	//modify this line to call custom function:
	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		crossingMA(data,exec,dataSize,sym,par.lPar[bt::orderSize],
				par.lPar[bt::fastMA],par.lPar[bt::slowMA],
				par.lPar[bt::atrlen],par.fPar[bt::cutoff]);
	}
//	executeMomentum(data,exec,dataSize);
}

//namespace bt
}

#endif /* CUSTOM_H_ */
