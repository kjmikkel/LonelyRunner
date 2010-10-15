#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <math.h>
#include <algorithm>
#include "data_structure.h"

long double closeToInteger(long double x, int w) {
  long double input = fmod(x * w, 1.0);
  
  return fmin(1.0 - input, input);  
}

bool isValid(time_result* result, int* array, unsigned int number_runners) {
  bool valid = true;
  double compareTo = 1.0 / (number_runners + 1.0);
  double nudge = 0.0000000001;
  for(int index = 0; index < number_runners; index++) {
    double left_compare = closeToInteger(result->result_time, array[index]) + nudge;
    valid = left_compare >= compareTo;
    
    if (!valid)
      break;
  }
  return valid;
}

static bool checkForSolution(int speedArray[], int length) {
  
  for(int numberIndex = 2; numberIndex < length + 2; numberIndex++) {
    // get a number in the the set {2, ..., n+1}
    int number = numberIndex;
    
    bool doesDevide = false;
    
    for(int speedIndex = 0; speedIndex < length; speedIndex++) {
      doesDevide |= (speedArray[speedIndex] % number == 0);
      
      // If the number does devide one of the speeds, then we can move on to the next number
      if(doesDevide) {
	break;
      }
    }
    
    if (!doesDevide) {
      return true;
    }
  }
  
  return false;
}

time_result* Numerical_method (const int speed_array[], const int length) {
  
  double compare_to = (1.0 / (length + 1.0));
  double nudge = 0.0000000001;
  for(int first_index = 0; first_index < length - 1; first_index++) {
    int first_speed = speed_array[first_index];
    
    //    printf("Num first index: %d\n", first_index);
    
    for(int second_index = first_index + 1; second_index < length; second_index++) {
      // if (second_index % 100 == 0)
	//	printf("Num second index: %d\n", second_index);

      int second_speed = speed_array[second_index];
      int k = first_speed + second_speed;

      // int start = k / (length+1);
      // Look into this code - it might lead to extreme speedups, but currently there is a problem 
      
      int start = 0;
      int start_candidate_1 = k / (first_speed * (length + 1)) - 1;
      int start_candidate_2 = k / (second_speed * (length + 1)) - 1;
      if (start_candidate_1 > start_candidate_2)
	start = start_candidate_1;
      else
	start = start_candidate_2;
      //     printf("index_1: %d, index_2: %d\n",start_candidate_1, start_candidate_2);
      
      
      for(int a = start; a < k; a++) {
	bool testValid = true;
	double d_a = double(a);
	double d_k = double(k);
	double x = d_a / d_k;

	
	for(int speed_index = 0; speed_index < length; speed_index++) {
	  double close = closeToInteger(x, speed_array[speed_index]);
	  testValid &= close >= compare_to;
	  
	  if(!testValid) 
	    break;
	}
	
	if (testValid) {
	  time_result* result = new time_result;
	  result->result = true;
	  result->result_time = x;
	  return result;
	}
      }
    }
  }

  time_result* result = new time_result;
  result->result = false;
  result->result_time = 0.0;
  return result;
}
