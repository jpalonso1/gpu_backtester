//Juan Pablo Alonso
//GPU Backtester 1.0

#include "setup.h"
#include "gpu.h"
#include "xlog.h"

using namespace std;

void runBacktest(thrust::device_vector<bt::stockData>& data,
		thrust::device_vector<bt::parameters>& par, thrust::device_vector<bt::result>& res,
		long vecSize){
	//create "dummy" vector sequence. Only used to track position
	thrust::device_vector<long> Y(vecSize);
    thrust::sequence(Y.begin(),Y.end());
    //wrap data in device pointer
    bt::stockData* dataPtr=thrust::raw_pointer_cast(&data[0]);
    //transform the vector using the specified function
    thrust::transform(par.begin(), par.end(), Y.begin(), res.begin(),
			individual_run(dataPtr,data.size()));

}

void optimizeParameters(thrust::device_vector<bt::result>& res){
	thrust::sort(res.begin(),res.end(),sharpe_max());
}

int main(){
	XLog logMain("Total time");

	//get data
	XLog logExtract("Extracting data");
	logExtract.start();
	thrust::host_vector<bt::stockData> datah;
	bt::extractRawData(dataFile,datah,true);
	thrust::device_vector<bt::stockData>datad(datah.size());
//	thrust::device_vector<bt::stockData> datad=datah;
	thrust::copy(datah.begin(), datah.end(), datad.begin());
	logExtract.log("Lines: ",datah.size());
	logExtract.end();

	//create vector of parameters to be tested
	XLog logPar("Setting parameters");
	logPar.start();
	thrust::host_vector<bt::parameters> parh;
	long VEC_SIZE=setParameters(parh);
	thrust::device_vector<bt::parameters> pard(VEC_SIZE);
	thrust::copy(parh.begin(), parh.end(), pard.begin());
	//    thrust::device_vector<bt::parameters> pard=parh;
    thrust::device_vector<bt::result> resd(VEC_SIZE);


    //run the backtesting on gpu
    XLog logBacktest("Run backtest");
    logBacktest.start();
    logBacktest.log("Total simulations to run: ",VEC_SIZE);
    runBacktest(datad
    		,pard,resd,VEC_SIZE);
    logBacktest.end();

    //sort on gpu
    XLog logSort("Sorting");
    logSort.start();
    optimizeParameters(resd);
    thrust::host_vector<bt::result> resh(resd.size());
    thrust::copy(resd.begin(), resd.end(), resh.begin());
    logSort.end();

    //sample output
    for (int i=0;i<10;i++){
		cout<<i<<" - Sum PnL: "<<resh[i].PnL[DATA_ELEMENTS];
		cout<<" sharpe: "<<resh[i].sharpe[DATA_ELEMENTS];
		cout<<" Max Drawdown: "<<resh[i].maxDrawdown[DATA_ELEMENTS]<<endl;
    }
    logMain.end();
	return 0;
}
