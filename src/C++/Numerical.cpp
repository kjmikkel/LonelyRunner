#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <NTL/ZZ.h>
#include <NTL/ZZ_pXFactoring.h>
#include "data_structure.h"

NTL_CLIENT

long double closeToInteger(long double x, int w) {
  long double sum = x * w;
  long double one = 1.0;
  long double input = std::fmod(sum, one);
  
  return fmin(1.0 - input, input);  
}

ZZ ZZ_mod(ZZ num, ZZ mod) {
  while (num >= mod) {
    num = num - mod;
  }
  return num;
}

bool isValidInternal(ZZ P, ZZ Q, int* array, unsigned int number_runners) {
  
  bool valid = true;
  ZZ compare;
  ZZ distance_from_start;
  ZZ distance_to_end;

  for(int index = 0; index < number_runners; index++) {
    // Look into getting the mod code working
    distance_from_start = ZZ_mod(array[index] * P, Q);
    distance_to_end = Q - distance_from_start;
    
    if (distance_from_start > distance_to_end) {
      compare = distance_to_end;
    } else {
      compare = distance_from_start;
    }

    valid = compare * (number_runners + 1) >=  Q;
    
    if (!valid)
      break;
  }
  return valid;
}

bool isValid(num_time_result* result, int* array, unsigned int number_runners) {
  
  ZZ P = to_ZZ(result->a);    
  ZZ Q = to_ZZ(result->k1 + result->k2);
  return isValidInternal(P, Q, array, number_runners);
}

bool isValid(geo_time_result* result, int* array, unsigned int number_runners) {
  event_point* point = result->point;

  ZZ P = to_ZZ(point->local_position + point->rounds * (point->number_of_runners + 1));
  ZZ Q = to_ZZ(point->speed * (point->number_of_runners + 1));
  return isValidInternal(P, Q, array, number_runners);
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

num_time_result* Numerical_method (const int speed_array[], const int length) {
  
  //  double compare_to = (1.0 / (length + 1.0));
  for(int first_index = 0; first_index < length - 1; first_index++) {
    int first_speed = speed_array[first_index];
    
    printf("Num first index: %d\n", first_index);
    bool cal_first_div = false;
    ZZ first_div;

    for(int second_index = first_index + 1; second_index < length; second_index++) {
      if (second_index % 25 == 0)
	 printf("Num second index: %d\n", second_index);
      
      int second_speed = speed_array[second_index];
      ZZ k = to_ZZ(first_speed + second_speed);
      //      cout << "k: " << k << ", num1: " << first_speed << ", num2: " << second_speed << "\n";
      // int start = k / (length+1);
      // Look into this code - it might lead to extreme speedups, but currently there is a problem 
      
      ZZ start;
      
      if (!cal_first_div) {
	cal_first_div = true;
	first_div = (first_speed * (length + 1)) ;
	//	cout << "f1: " << first_div << "\n";
      }
      
      if (k > 1000000) {
	cout << "K: " << k << "\n";
      }

      ZZ start_candidate_1 = k / first_div - 1;
      ZZ start_candidate_2 = k / (second_speed * (length + 1)) - 1;
      
      //      cout << "f2: " << (second_speed * (length + 1)) - 1 << "\n";


      if (start_candidate_1 > start_candidate_2)
	start = start_candidate_2;
      else
	start = start_candidate_1;
      //     printf("index_1: %d, index_2: %d\n",start_candidate_1, start_candidate_2);
      
      
      ZZ distance_from_start;
      ZZ distance_to_end;
      ZZ compare;

      for(ZZ a = start; a < k; a++) {
	bool testValid = true;
	for(int speed_index = 0; speed_index < length; speed_index++) {
	  distance_from_start = ZZ_mod(a * speed_array[speed_index], k);
	  distance_to_end = k - distance_from_start;
	  
	  if (distance_from_start > distance_to_end) {
	    compare = distance_to_end;
	  } else {
	    compare = distance_from_start;
	  }
	  
	  testValid &= compare * (length + 1) >= 1;	  
	  if(!testValid) 
	    break;
	}
	
	if (testValid) {
	  num_time_result* result = new num_time_result;
	  result->result = true;
	  result->k1 = first_speed;
	  result->k2 = second_speed;
	  result->a = 2;
	  return result;
	}
      }
    }
  }

  num_time_result* result = new num_time_result;
  result->result = false;
  result->k1 = 0;
  result->k2 = 0;
  result->a = 0;
  
  return result;
}
