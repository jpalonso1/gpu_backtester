#ifndef XLOG_H_
#define XLOG_H_

#include <string>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sys/time.h>
#include <stdio.h>
#include <iomanip>

class XLog {
private:
	char logName[20];
	std::ostream* logFile;
	double timeStart;
	double getTimeDiff();
public:
	//log single string of characters using current time
	void log(const char*input);
	//print end message
	void end();
	//print start message
	void start();
	//use a number or other iostream compatible argument
	template <class T>
	void log(const char* input,T inputT);
	XLog(const char* _logName);
    ~XLog();
};

template<class T>
void XLog::log(const char* input, T inputT) {
	std::cout << std::fixed;
	std::cout<<std::setprecision(3);
	std::cout<<"Seconds: "<<getTimeDiff();
	std::cout << std::scientific;
	std::cout<<" "<<logName<<" || "<<input<<" "<<inputT<<std::endl;
}

#endif /* XLOG_H_ */
