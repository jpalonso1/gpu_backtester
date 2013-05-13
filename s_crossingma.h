//Juan Pablo Alonso

#ifndef S_CROSSINGMA_H_
#define S_CROSSINGMA_H_


namespace bt{

//long parameters
enum {atrlen=0,fastMA=1,slowMA=2,orderSize=3};
//float parameters
enum {cutoff=0};

__device__ __host__
inline float getMA(bt::stockData* data,long maLength,
		long position, long sym){

}

__device__ __host__
inline void crossingMA(bt::stockData* data,bt::execution& exec,
		long dataSize,long sym,long orderSize,
		long fastMA,long slowMA,long atrlen,float cutoff){
	//tracks last direction. fast>slow =1, fast<slow =-1
	int lastCross=0;
	//moving averages of current point
	float slow=0,fast=0;
	float tempSlow=0,tempFast=0;
	int nextTrade=0;
	//check if MA crossed (1 cross up, -1 cross down)
	int crossCheck=0;
	int cutoffCountdown=0;
	long position,tradeCount;

	for (int i=slowMA-1;i<dataSize;i++){
		crossCheck=0;
		tempSlow=0;tempFast=0;
		//get slow MA
		for (int j=0;j<slowMA;j++){tempSlow+=data[i-j].d[sym];}
		slow=tempSlow/float(slowMA);

		//get fast MA
		for (int j=0;j<fastMA;j++){tempFast+=data[i-j].d[sym];}
		fast=tempFast/float(fastMA);

		if (fast>slow){
			//crossed up
			if (lastCross==-1)crossCheck=1;
			lastCross=1;
		}

		else if(fast<slow){
			//crossed down
			if (lastCross==1)crossCheck=-1;
			lastCross=-1;
		}

		position=exec.trade[sym].posSize[exec.numTrades[sym]];
		if (position!=0){
			cutoffCountdown--;
			//close if time limit reached
			if (cutoffCountdown==0){
				recordTrade(data,exec,sym,i,-position);
			}
			//close if MA crossed again
			else if (crossCheck!=0){
				recordTrade(data,exec,sym,i,-position);
			}
		}
		//no position, check for open
		else if (crossCheck!=0){
			recordTrade(data,exec,sym,i,orderSize*crossCheck);
			cutoffCountdown=cutoff;
		}
	}
}

inline long maParameters(thrust::host_vector<bt::parameters>& par){
	long count=0;
	//DES:defines the parameters to be tested

	bt::parameters tempPar;
	tempPar.fPar[cutoff]=0.50;
	tempPar.lPar[orderSize]=10000;

	for (int b=0;b<25;b++){
	tempPar.lPar[atrlen]=20+b;
	for (int c=0;c<20;c++){
	tempPar.lPar[fastMA]=10+c;
	for (int d=0;d<20;d++){
		tempPar.lPar[slowMA]=60+2*d;
		par.push_back(tempPar);
		count++;
	}
	}
	}

	return count;
}

//ns bt
}
#endif /* S_CROSSINGMA_H_ */
