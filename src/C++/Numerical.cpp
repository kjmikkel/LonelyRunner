/*
    This file is part of The Lonely Runner Verifier.

    The Lonely Runner Verifier is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Lonely Runner Verifier is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Lonely Runner Verifier.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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
    num -= mod;
  }
  return num;
}

bool isValidInternal(ZZ P, ZZ Q, const int array[], unsigned int number_runners) {
  
  ZZ compare;
  ZZ distance_from_start;
  ZZ distance_to_end;
  bool valid = true;
  
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
      cout << "fail: compare: " << compare << ", " << compare * (number_runners + 1) << ", " << Q << ", " << array[index]  << "\n";
      break;
  }

  return valid;
}

bool isValid(num_time_result* result, const int array[], unsigned int number_runners) {
  
  ZZ P = to_ZZ(result->a);    
  ZZ Q = to_ZZ(result->k1 + result->k2);
  bool b_result = isValidInternal(P, Q, array, number_runners);
  if(!b_result) {
    std::cout << "P: " << P << ", Q: " << Q << "\n";
  }
  return b_result;
}

bool isValid(geo_time_result* result, const int array[]) {
  event_point point = result->point;
  //  if (point == NULL) return false;
  
  ZZ P = point.local_position + to_ZZ(point.rounds) * (point.number_of_runners + 1);
  ZZ Q = to_ZZ(point.speed) * (point.number_of_runners + 1);
  
  bool b_result = isValidInternal(P, Q, array, point.number_of_runners);
  if (!b_result) {
    cout << "Local position: " << point.local_position << ", rounds: " << point.rounds << " number of runners: " << point. number_of_runners << ", speed: " << point.speed << "\n";
  }
  return b_result;
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

num_time_result* Numerical_method (int speed_array[], const int length, bool randomPermutate, bool reverse, bool check_for_max_solution) {
  
  
  //std::cout << "\n";

  if (length == 1) {
    std::cout << "Only one runner\n";
    // If there is only one runner then I might as well use the Geometrical trick
    num_time_result* num_result = new num_time_result;
    
    num_result->a = 1;
    num_result->k1 = speed_array[0] * (length + 1);

    num_result->k2 = 0;
       
    num_result->result = true;
    return num_result;
  }

  // Randomize the order of the numbers
  if (randomPermutate) {
    srand ( time(NULL) );
    
    int temp;
    int random_to_index;
    
    for(int randomize_index = 0; randomize_index < length; randomize_index++) {
      random_to_index = rand() % length;

      temp = speed_array[randomize_index];
      speed_array[randomize_index] = speed_array[random_to_index];
      speed_array[random_to_index] = temp;
      
    }
  }
  

  if (reverse) {
    int temp;

    for(int reverse_index = 0; reverse_index < length / 2; reverse_index++) {
      temp = speed_array[length - 1 - reverse_index];

      speed_array[length - 1 - reverse_index] = speed_array[reverse_index];

      speed_array[reverse_index] = temp;
    }
  }
  
  unsigned int candidate_k1 = -1;
  unsigned int candidate_k2 = -1;
  unsigned int candidate_a = 0;
  ZZ candidate_k = to_ZZ(-1);
  bool no_candidate = true;
  for(int first_index = 0; first_index < length - 1; first_index++) {
    int first_speed = speed_array[first_index];
      if (first_index % 100 == 0 && first_index > 0)
	cout << "First index: " << first_index << " out of " << length << "\n"; 
    // cout << "first index: " << first_speed << ", " << first_index << "\n";
    //    printf("Num first index: %d\n", first_index);
    
    bool cal_first_div = false;
    ZZ first_div;
    
    
    for(int second_index = first_index + 1; second_index < length; second_index++) {
      if (second_index % 100 == 0)
	cout << "second index: " << second_index << " out of " << length << "\n"; 
      
      int second_speed = speed_array[second_index];
      ZZ k = to_ZZ(first_speed) + second_speed;
     
      int start = 0;
      
      if (!cal_first_div) {
	cal_first_div = true;
	first_div = (first_speed * (length + 1)) ;
      }
            
      ZZ start_candidate_1 = to_ZZ(1); //k / first_div - 1;
      ZZ start_candidate_2 = to_ZZ(1); //k / (second_speed * (length + 1)) - 1;
      ZZ smallest;

      if (start_candidate_1 > start_candidate_2) {
	smallest = start_candidate_2;
      }
      else {
	smallest = start_candidate_1;
      }

      while(start < smallest) {
	start++;     
      }
      
      ZZ distance_from_start;
      ZZ distance_to_end;
      ZZ compare;      
      ZZ zz_a;
      for(int a = start; a < k; a++) {
	bool testValid = true;
	zz_a = to_ZZ(a);
	
	for(int speed_index = 0; speed_index < length; speed_index++) {
	  distance_from_start = ZZ_mod(zz_a * speed_array[speed_index], k);
	  distance_to_end = k - distance_from_start;
	  
	  if (distance_from_start > distance_to_end) {
	    compare = distance_to_end;
	  } else {
	    compare = distance_from_start;
	  }
	  
	  testValid &= compare * (length + 1) >= k;	  
	  if(!testValid) 
	    break;
	}
	
	if (testValid) {
	  bool candidate_comparison = k * candidate_a < candidate_k * a;  

	  if(no_candidate || candidate_comparison) {
	    no_candidate = false;
	    candidate_k1 = first_speed;
	    candidate_k2 = second_speed;
	    candidate_a = a;
	  }
	}
	
	if(!check_for_max_solution && !no_candidate) {
	  num_time_result* result = new num_time_result;	  
	  result->result = true;
	  result->k1 = candidate_k1;
	  result->k2 = candidate_k2;
	  result->a = candidate_a;
	  return result;
   	}
      }
    }
    
    num_time_result* result = new num_time_result;
    if (candidate_k1 > 0 && candidate_k2 > 0 && candidate_a > 0) {	
      cout << "Got it\n";	  
      result->result = true;
      result->k1 = candidate_k1;
      result->k2 = candidate_k2;
      result->a = candidate_a;
      return result;
      
    } 
     
  }

  cout << "Return no solution";
  num_time_result* result = new num_time_result;
  result->result = false;
  result->k1 = 0;
  result->k2 = 0;
  result->a = 0;
  
  return result;
}
