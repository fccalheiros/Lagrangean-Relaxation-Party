#ifndef __TIME_H
#define __TIME_H

#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <math.h> 
#include <stdio.h>



long int TempoAtual();
clock_t TempoCpuAtual();

float TempoPassado(long int tempo);
float TempoCpuPassado(clock_t tempo);

float ElapsedTime(long int InitialTime, long int FinalTime);
float CPUElapsedTime(clock_t InitialTime, clock_t FinalTime);

#endif
