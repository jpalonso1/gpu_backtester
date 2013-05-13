//Juan Pablo Alonso

#ifndef SETUP_H_
#define SETUP_H_

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/transform.h>
#include <thrust/sequence.h>
#include <thrust/sort.h>

using std::cout;
using std::endl;

const int DATA_ELEMENTS=5;
const int LONG_PARAMETERS=5;
const int FLOAT_PARAMETERS=5;
const int YEAR_PERIODS=252;
const int MAX_ORDERS=100;
//only "neutral" strategies implemented for sharpe
const float BENCHMARK=0;

namespace bt{

struct parameters{
	//DES:holds floating (fPar) and long (lPar)
	//parameters for EACH iteration
	float fPar[FLOAT_PARAMETERS];
	long lPar[LONG_PARAMETERS];
};

struct stockData{
	//DES: hold each line of the stock data.
	//date as string
	char date[20];
	//each of the optional elements: prices of multiple stocks,
	//volumes, custom indicators, etc.
	float d[DATA_ELEMENTS];
};

struct trade{
	//negative for short/sell
	long posSize[MAX_ORDERS];
	//"time" (vector element location) of execution relative to data
	long location[MAX_ORDERS];
	float price[MAX_ORDERS];

	//pnl for closing trades
	float realPnL[MAX_ORDERS];
};

struct result{
	float PnL[DATA_ELEMENTS+1];
	float sharpe[DATA_ELEMENTS+1];
	float maxDrawdown[DATA_ELEMENTS+1];
	float numTransactions[DATA_ELEMENTS+1];
	float avgDailyProfit[DATA_ELEMENTS+1];
};

struct execution{
	trade trade[DATA_ELEMENTS];
	result result;
	char symbol[DATA_ELEMENTS][20];
	long numTrades[DATA_ELEMENTS];
//	execution();
};

void extractRawData(char* filename,thrust::host_vector<bt::stockData>& data,bool header=false);

__device__ __host__
inline void recordTrade(bt::stockData* data,bt::execution& exec,
		long sym,long location,long amount){
	//get number of shares (rounded down) to get amount
	long adjAmount=amount/data[location].d[sym];
	exec.trade[sym].posSize[exec.numTrades[sym]]=adjAmount;
	exec.trade[sym].price[exec.numTrades[sym]]=data[location].d[sym];
	exec.trade[sym].location[exec.numTrades[sym]]=location;
	exec.numTrades[sym]++;
}


}//namespace bt

#endif /* SETUP_H_ */
