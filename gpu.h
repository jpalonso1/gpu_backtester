//Juan Pablo Alonso

#ifndef GPU_H_
#define GPU_H_

#include "setup.h"
#include "custom.h"
#include "xlog.h"
#include <fstream>

__device__ __host__
inline void cpyCharCustom(char* source,char* target){
	//custom character copy that works on gpu
	int cnt=0;
	while (source[cnt]!='\0'){
		target[cnt]=source[cnt];
		cnt++;
	}
}

__device__ __host__
void initExec(bt::execution& exec){
	exec.numTrades[0]=0;
    for (int sym=0;sym<DATA_ELEMENTS;sym++){
    	exec.numTrades[sym]=0;
    }
}

__device__ __host__
void clearResult(bt::result& res){
	for (int i=0;i<=DATA_ELEMENTS;i++){
		res.PnL[i]=0;
		res.sharpe[i]=0;
		res.maxDrawdown[i]=0;
		res.numTransactions[i]=0;
		res.avgDailyProfit[i]=0;
	}
}

__device__ __host__
inline void getStats(bt::execution& exec,bt::stockData* data,long dataSize){

	float netPos[DATA_ELEMENTS];
	float unallocPnL[DATA_ELEMENTS];
	long lastExec[DATA_ELEMENTS];
	long tempMaxDraw[DATA_ELEMENTS];
	long tempMaxDrawTotal=0;
	//initialize results
	clearResult(exec.result);
	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		netPos[sym]=0;
		unallocPnL[sym]=0;
		lastExec[sym]=0;
		tempMaxDraw[sym]=0;
	}

	float totalPnL=0;
	float periodPnL=0;
	//used for sharpe calculation
	float sdSum[DATA_ELEMENTS+1];
	float returnSum[DATA_ELEMENTS+1];
	for (int sym=0;sym<=DATA_ELEMENTS;sym++){
		returnSum[sym]=0;
		sdSum[sym]=0;
	}
	//loop through each record
	for (int i=1;i<dataSize;i++){
		for (int sym=0;sym<DATA_ELEMENTS;sym++){
			//get PnL based on difference from previous record
			if (netPos[sym]==0)periodPnL=0;
			else periodPnL=netPos[sym]*(data[i].d[sym]-data[i-1].d[sym]);
//			if (sym==0)testOut<<i<<",pos:,"<<netPos[sym]<<",unalloc:,"<<
//					unallocPnL[sym]<<",lastExec,"<<lastExec[sym]<<
//					",periodPnL,"<<periodPnL<<endl;
			unallocPnL[sym]+=periodPnL;
			//positions updated at the end of the day if needed
			if (i==exec.trade[sym].location[lastExec[sym]]){
//				if(sym==0)cout<<lastExec[sym]<<" net pos: "<<netPos[sym]<<endl;
				netPos[sym]+=exec.trade[sym].posSize[lastExec[sym]];
				exec.trade[sym].realPnL[lastExec[sym]]=unallocPnL[sym];
				lastExec[sym]++;
				//allocate profit
				unallocPnL[sym]=0;
			}

			exec.result.PnL[sym]+=periodPnL;
			exec.result.PnL[DATA_ELEMENTS]+=periodPnL;
			if(data[i-1].d[sym]!=0){
				returnSum[sym]+=periodPnL/data[i-1].d[sym];
				returnSum[DATA_ELEMENTS]+=periodPnL/data[i-1].d[sym];
			}
			//updateDrawdown
			tempMaxDraw[sym]-=periodPnL;
			if (tempMaxDraw[sym]<0)tempMaxDraw[sym]=0;
			if (exec.result.maxDrawdown[sym]<tempMaxDraw[sym])
				exec.result.maxDrawdown[sym]=tempMaxDraw[sym];
			tempMaxDrawTotal-=periodPnL;
			if (tempMaxDrawTotal<0)tempMaxDrawTotal=0;
			if (exec.result.maxDrawdown[DATA_ELEMENTS]<tempMaxDrawTotal)
				exec.result.maxDrawdown[DATA_ELEMENTS]=tempMaxDrawTotal;
		}
	}

	//get avg returns
	for (int sym=0;sym<=DATA_ELEMENTS;sym++){
		exec.result.avgDailyProfit[sym]=returnSum[sym]/dataSize;
	}

	float periodReturn;
	float periodReturnTotal;

	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		netPos[sym]=0;
		lastExec[sym]=0;
		tempMaxDraw[sym]=0;
	}
	//get standard deviations
	for (int i=1;i<dataSize;i++){
		periodReturnTotal=0;
		for (int sym=0;sym<DATA_ELEMENTS;sym++){
			//get PnL based on difference from previous record
			if (netPos[sym]==0)periodPnL=0;
			else periodPnL=netPos[sym]*(data[i].d[sym]-data[i-1].d[sym]);

			//positions updated at the end of the day if needed
			if (i==exec.trade[sym].location[lastExec[sym]]){
				netPos[sym]+=exec.trade[sym].posSize[lastExec[sym]];
				lastExec[sym]++;
			}


			if (data[i-1].d[sym]!=0)periodReturn=periodPnL/data[i-1].d[sym];
			else periodReturn=0;
			periodReturnTotal+=periodReturn;
			sdSum[sym]+=(periodReturn-exec.result.avgDailyProfit[sym])*
					(periodReturn-exec.result.avgDailyProfit[sym]);
		}
		sdSum[DATA_ELEMENTS]+=(periodReturnTotal-exec.result.avgDailyProfit[DATA_ELEMENTS])*
				(periodReturnTotal-exec.result.avgDailyProfit[DATA_ELEMENTS]);
	}
	float sd;
	for (int sym=0;sym<DATA_ELEMENTS;sym++){
		//get sd
		sd=sqrtf(sdSum[sym]/dataSize);
		exec.result.sharpe[sym]=sqrtf(YEAR_PERIODS)*(exec.result.avgDailyProfit[sym]/sd);
	}
	sd=sqrtf(sdSum[DATA_ELEMENTS]/dataSize);
	exec.result.sharpe[DATA_ELEMENTS]=sqrtf(YEAR_PERIODS)*(exec.result.avgDailyProfit[DATA_ELEMENTS]/sd);
}

__device__ __host__
void forceClose(bt::execution& exec,bt::stockData* data,long dataSize){
	//DES: check position at the end and create closing trade if it exists
	for (int sym=0;sym<DATA_ELEMENTS;sym++)
		{
		//current net positon
		long netPos=0;

		for (int i=0;i<exec.numTrades[sym];i++){
			netPos+=exec.trade[sym].posSize[i];
		}

		//add forced closing
		if (netPos!=0){
			float closePrice=data[dataSize-1].d[sym];
			exec.trade[sym].posSize[exec.numTrades[sym]]=-netPos;
			exec.trade[sym].price[exec.numTrades[sym]]=closePrice;
			exec.trade[sym].location[exec.numTrades[sym]]=dataSize-1;
			exec.numTrades[sym]++;
		}
	}
}

struct return_max
{
	__device__ __host__
	bool operator()(bt::result x, bt::result y)
	{
		if (x.PnL[DATA_ELEMENTS]>y.PnL[DATA_ELEMENTS])return true;
		else return false;
	}
};

struct sharpe_max
{
	__device__ __host__
	bool operator()(bt::result x, bt::result y)
	{
		if (x.sharpe[DATA_ELEMENTS]>y.sharpe[DATA_ELEMENTS])return true;
		else return false;
	}
};

//ONLY ON OPENMP
//__device__ __host__
//void printExecutions(bt::execution& exec){
//	testOut<<"symbol,location,price,size,pnl"<<endl;
//	for (int sym=0;sym<DATA_ELEMENTS;sym++){
//		for (int i=0;i<exec.numTrades[sym];i++){
//			testOut<<sym<<","<<exec.trade[sym].location[i]<<","<<
//				exec.trade[sym].price[i]<<","<<exec.trade[sym].posSize[i]<<
//				","<<exec.trade[sym].realPnL[i]<<endl;
//		}
//	}
//}

struct individual_run
{
	//hold a copy of the pointer to data
	bt::stockData* data;
	long dataSize;
    individual_run(bt::stockData* _data,long _dataSize) :
    	data(_data),dataSize(_dataSize) {}

    __device__ __host__
    bt::result operator()(const bt::parameters& par, const long& Y) const {
    	//to be run every iteration of the backtest
    	bt::execution execTemp;
    	initExec(execTemp);
    	bt::runExecution(data,dataSize,execTemp,par);
    	forceClose(execTemp,data,dataSize);
    	getStats(execTemp,data,dataSize);
//    	if(Y==0)printExecutions(execTemp);
    	return execTemp.result;
	}
};

#endif /* GPU_H_ */
