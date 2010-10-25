#ifndef NUM_H
#define NUM_H

#include "data_structure.h"

num_time_result* Numerical_method (int speed_array[], const int length, bool randomize_order, bool reverse);

bool isValid(num_time_result* result, const int array[], unsigned int number_runners);

bool isValid(geo_time_result* result, const int array[]);

static bool checkForSolution(int speedArray[], int length);

double closeToInteger(double x, int w);

#endif
