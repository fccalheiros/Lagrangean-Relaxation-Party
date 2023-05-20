#include "Time.h"

long int TempoAtual() {
	struct timeb tp;
	ftime(&tp); 
	long int res = (long int)tp.millitm + (long int)tp.time * 1000;
	return ( res );
}

float TempoPassado(long int tempo) {
	return ( ((float)(TempoAtual() - tempo))/(float)1000 );
}

clock_t TempoCpuAtual() {
	return clock();
}

float TempoCpuPassado(clock_t tempo){
	clock_t dif  = TempoCpuAtual() - tempo;
	double total = (double)dif;
	total = total / (double) CLOCKS_PER_SEC;
	return ( (float) total );
}

float ElapsedTime(long int InitialTime, long int FinalTime) {
	return (((float)(FinalTime - InitialTime)) / (float)1000);
}

float CPUElapsedTime(clock_t InitialTime, clock_t FinalTime) {
	clock_t dif = FinalTime - InitialTime;
	double total = (double)dif;
	total = total / (double)CLOCKS_PER_SEC;
	return ((float)total);
}