INCLUDES=-I.

#set up compiler and options
CXX = nvcc
CXXFLAGS = $(INCLUDES)

SRC=backmain.cu setup.cu xlog.cpp

EXEC=backtest

$(EXEC):
	$(CXX) -o $@ $(SRC) $(CXXFLAGS)
        
clean:
	rm -f $(EXEC)
