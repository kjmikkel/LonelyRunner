#ifndef NUM_H
#define NUM_H

#include "data_structure.h"

time_result* Numerical_method (int speed_array[], int length);

bool isValid(time_result* result, int* array, unsigned int number_runners);

static bool checkForSolution(int speedArray[], int length);

double closeToInteger(double x, int w);

#endif
