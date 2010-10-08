#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <math.h>
#include "data_structure.h"

double closeToInteger(double x, int w) {
  double input = fmod(x * w, 1.0);
  
  return fmin(1.0 - input, input);  
}

time_result* Numerical_method (int speed_array[], const int length) {
  for(int first_index = 0; first_index < length - 1; first_index++) {
    int first_speed = speed_array[first_index];
    
    for(int second_index = first_index + 1; second_index < length; second_index++) {
      int second_speed = speed_array[second_speed];
      int k = first_speed + second_speed;

      for(int a = 1; a < k; a++) {
	bool testValid = true;
	float x = float (a) / float(k);

	for(int speed_index = 0; speed_index < length; speed_index++) {
	  
	  testValid &= closeToInteger(x, speed_array[speed_index]) >= (1.0 / (length + 1));
	  
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
