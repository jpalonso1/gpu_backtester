Project: GPU based backtesting framework
Author: Juan Pablo Alonso

Brute force backtesting.

This program runs a series of backtesting simulations based
on specified paramenters and sorts the results according to 
sharpe ratio (can be modified to Max drawdown or return as 
objective function).

Results on crosssing MA test: 22,500 simulations in 6 seconds

REQUIREMENTS: 
-Linux
-Nvidia Cuda framework
-for gpu backend: Nvidia card compatible with CUDA 5.0

Compile with:
$make 
to run the gpu based version, or
$make -f makeomp
to run using openMP backend (slower)