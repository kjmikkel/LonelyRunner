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

#include "Prime_Number.h"
#include "Geometric.h"
#include "Numerical.h"

#include "data_structure.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <cmath>
#include <ctime>
#include <cstdlib>

#include <stddef.h>
#include <stdlib.h>
#include <json/json.h>
//#include <NTL/ZZ.h>

#include "util.h"

using namespace std;

struct long_pair {
  unsigned long average;
  unsigned long spread;
};

void array_to_json_array(unsigned long* array, json_object* json_array, int array_length) {
  for(int array_index = 0; array_index < array_length; array_index++) {
    json_object *jint = json_object_new_int(array[array_index]);
    json_object_array_add(json_array, jint);
  }
}

void add_to_json_object(json_object* json_obj, unsigned long* array, int needed_mem, string name) {
  json_object* json_array = json_object_new_array();
  array_to_json_array(array, json_array, needed_mem);
  json_object_object_add(json_obj, name.c_str(), json_array);
}

long_pair find_spread(unsigned long* time_array, int number_of_times) {
  unsigned long long  average = 0;
  for(int index = 0; index < number_of_times; index++) {
    average += time_array[index];
  }
  average /= number_of_times;
  
  long temp;
  unsigned long long result = 0;
  for(int index = 0; index < number_of_times; index++) {
    temp = time_array[index] - average;
    result += temp * temp;
  }
  
  result = sqrt(result / number_of_times);
  long_pair pair;

  pair.average = (unsigned long)average;
  pair.spread = (unsigned long)result;
  return pair;
} 


void appendValueToFile(string data) {
  ofstream fout;
  fout.open("output", ios::app);
  fout << data;
  fout.close();
} 

void doTest(
	    // The list containing the number of runners we are going to test for
	    int* runners, 
	    // The number of different kinds of speeds contained in runners
	    int runner_num,

	    // The different max speeds
	    int* speeds, 
	    // The number of different speeds
	    int speed_num, 

	    // The stored speeds we are going to test
	    int* actual_speeds, 
	    // The total number of speeds in actual_speeds
	    int actual_speeds_num,

	    // The offset that we are going to use to for the speeds
	    int offset, 
	    // The number of times we are going to do the test
	    int times_to_do_test, 
	    // Whether or not we are going to randomize the numerical solution
	    bool randomize,
	    // Whether or not we are going to use indexes instead of numbers
	    //  bool index,
	    // The name of the test
	    string name) {  
  for(int runner_index = 0; runner_index < runner_num; runner_index++) {
  
    // We find the name to save under
    stringstream ss;
    ss << runners[runner_index];
    
    string filename = "../data/test_" + name + "_" + ss.str() + "_runners";
    if (offset == 0) {
      filename += ".json";
    } else {
      ss << offset;
      filename += "_with_offset_" + ss.str() + ".json";
    }

    int start_speed_index = 0;
    int num_speeds = 0;
    
    // We find the first speed that is above the number of runners
    for(int speed_index = 0; (speed_index < speed_num) && (speeds[speed_index] < actual_speeds_num); speed_index++) {
      cout << speed_index << "\n";
      if (speeds[speed_index] >= runners[runner_index]) {
	start_speed_index = speed_index;
	break;
      }
    }

    // If we cannot find enough numbers, then we stop the test
    if (speeds[start_speed_index] > actual_speeds_num) {
      cout << "Not so many speeds\n";
      return;
    }

    int end_speed_index = speed_num - 1;
    for(int index = start_speed_index; start_speed_index < speed_num; index++) {
      if (speeds[index] >= actual_speeds_num) {
	end_speed_index = index;
	break;
      }
    }
    
    // I allocate the needed memory for the results
    int needed_mem = speed_num - start_speed_index - (speed_num - end_speed_index);
    // The geo results and the array to contain any error
    unsigned long geo_results[needed_mem];
    

    unsigned long geo_seconds[needed_mem];
    unsigned long geo_spread[needed_mem];
    unsigned long geo_error[needed_mem];
   
    // The num results and the array to contain any error
    unsigned long num_results[needed_mem];
    unsigned long num_seconds[needed_mem];
    unsigned long num_spread[needed_mem];
    unsigned long num_error[needed_mem];
    
    unsigned long speed_results[needed_mem];
   
    // I allocate the memory for the speeds we are going to use as input
    int num_runners = runners[runner_index];
    int runner_speeds[num_runners];
    
    // The bools used to indicate whether any error at all happened
    bool b_geo_error = false;
    bool b_num_error = false;

    for(int speed_index = start_speed_index; speed_index < end_speed_index; speed_index++) {
      
      int real_index = speed_index - start_speed_index;
      speed_results[real_index] = speeds[speed_index];
            
      int prime_real_index = 0;
      for(int copy_index = speeds[speed_index] - num_runners; copy_index < speeds[speed_index]; copy_index++, prime_real_index++) {
	runner_speeds[prime_real_index] = actual_speeds[copy_index];
	//cout <<  runner_speeds[prime_real_index]  << "\n";
      }
      
 
      // The tests themselves
      struct timeval start;
      struct timeval end;
      
      struct timezone tz;
      struct tm *tm;
      
      cout << "Runners: " << runners[runner_index] << ", Max number: " << actual_speeds[speeds[speed_index]] << "\n";
      cout << "Before Geo: [";
      //    unsigned long micro_seconds = 0;
      unsigned long seconds = 0; 
      
      unsigned long time_test_array[times_to_do_test];
      unsigned long seconds_time_test_array[times_to_do_test];
      for(int time_test_index = 0; time_test_index < times_to_do_test; time_test_index++) {
	cout << ".";
	gettimeofday(&start, &tz);
	geo_time_result* geo_result = Geometric_method(runner_speeds, num_runners);
	gettimeofday(&end, &tz); 
	
	time_test_array[time_test_index] = (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
	seconds_time_test_array[time_test_index] = (end.tv_sec - start.tv_sec);
	
	// We check for errors
	if(geo_result != NULL && (!geo_result->result || !isValid(geo_result, runner_speeds))) {	  
	  cout << "Geo Error\n";
	  cout << "Name: " << name << "\n";
	  cout << "The number of speeds to test: " << speeds[start_speed_index] << ", the actual number of speeds: " << actual_speeds_num << "\n";
	  printf("error: %d, %d\n", geo_result->result, isValid(geo_result, runner_speeds));
	  for(int index = 0; index < num_runners; index++) {
	    cout << runner_speeds[index] << "\n";
	    //    if(runner_speeds[index] < 2)
	    //cout << "Speed less than 2: " << runner_speeds[index] << "\n";
	  }
	  num_time_result* num_result = Numerical_method(runner_speeds, num_runners, true, false, false);
	  printf("error num: %d, %d\n", num_result->result, isValid(num_result, runner_speeds, num_runners));
	  
	  // If there is an error the we record it
	  b_geo_error = true;
	  // We record the time it happened
	  geo_error[real_index] = runner_speeds[real_index];
	} else {
	  // to be able to differentiate it from all the other values we set it to 0
	  geo_error[real_index] = 0;
	}
	if (geo_result != NULL)
	  //	  delete geo_result->point;
	delete geo_result;
      }
      cout << "]\n";
      // I record the values
      long_pair pair = find_spread(time_test_array, times_to_do_test);
      geo_results[real_index] = pair.average;
      geo_spread[real_index] = pair.spread;
      seconds = 0;
      for(int index = 0; index < times_to_do_test; index++) {
	seconds += seconds_time_test_array[index];
      }
      geo_seconds[real_index] = seconds / times_to_do_test;

      cout << "Before Num: [";
      for(int time_test_index = 0; time_test_index < times_to_do_test; time_test_index++) {
	cout << ".";
	gettimeofday(&start, &tz);
	num_time_result* num_result = Numerical_method(runner_speeds, num_runners, randomize, false, false);
	gettimeofday(&end, &tz);
	time_test_array[time_test_index] = (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
	seconds_time_test_array[time_test_index] = (end.tv_sec - start.tv_sec);

	if(!num_result->result || !isValid(num_result, runner_speeds, num_runners)) {
	  cout << "Num error\n";
	  cout << "Name: " << name << "\n";
	  cout << "The number of speeds to test: " << speeds[start_speed_index] << ", the actual number of speeds: " << actual_speeds_num << "\n";
	  printf("*error*: %d, %d\n", num_result->result, isValid(num_result, runner_speeds, num_runners));
	 
	  b_num_error = true;
	  num_error[real_index] = runner_speeds[real_index];
	} else {
	  num_error[real_index] = 0;
	}	
	delete num_result;
      }
      cout << "]\n";
      
      pair = find_spread(time_test_array, times_to_do_test);
      num_results[real_index] = pair.average;
      num_spread[real_index] = pair.spread;

      seconds = 0;
      for(int index = 0; index < times_to_do_test; index++) {
	seconds += seconds_time_test_array[index];
      }
      num_seconds[real_index] = seconds / times_to_do_test;
      
      cout << "Time spent on Geo: " <<  geo_results[real_index] << ", Time spent on Num: " << num_results[real_index] << "\n";
    }
      
    // Create the json object we are going to store the tests in
    json_object *testInstance = json_object_new_object();
    
    /* Create the json arrays we are going to store the results in*/
    add_to_json_object(testInstance, geo_results, needed_mem, "Geometrical results");
    add_to_json_object(testInstance, geo_spread, needed_mem, "Geometrical spread");
    add_to_json_object(testInstance, geo_seconds, needed_mem, "Geometrical Seconds used");
    if (b_geo_error)
      add_to_json_object(testInstance, geo_error, needed_mem, "Geometrical errors");
    
    add_to_json_object(testInstance, num_results, needed_mem, "Numerical results");
    add_to_json_object(testInstance, num_spread, needed_mem, "Numerical spread");
    add_to_json_object(testInstance, num_seconds, needed_mem, "Numerical seconds");
    if (b_num_error)
      add_to_json_object(testInstance, num_error, needed_mem, "Numerical errors");
    
    add_to_json_object(testInstance, speed_results, needed_mem, "Speeds used");
    
    printf ("The json object created: %s\n",json_object_to_json_string(testInstance));
    
    
    ofstream out(filename.c_str());
    out << json_object_to_json_string(testInstance);
    out.close();
  }
  cout << "done\n";
}

len_array file_data(std::string filename)
{
  char* file = const_cast<char *>(filename.c_str());
  len_array arr = read_json_file_array(file);
  return arr;
}


void sequential_prime_test() {
  const int runner_num = 10;
  const int offset = 0;
  const int speed_num = 50000;
  int max_number = 500000;
  int times_to_do_tests = 10;
  
  // The number of runners
  int runners[runner_num] = {100, 500, 1000, 2000, 4000, 5000, 8000, 12000, 30000, 50000};
  int speeds[speed_num]; 
 
  // The array contains the speeds we are going to test
  for(int speed_index = 0; speed_index < speed_num; speed_index++) {
    speeds[speed_index] = (speed_index + 1) * 100 + offset;
  }
  
  cout << "before prime\n";
  // We find the primes which are going as the speeds
  len_array primes = findPrimes(max_number);
  doTest(runners, runner_num, 
	 speeds, speed_num,
	 primes.array, primes.len, 
	 offset, times_to_do_tests, false, "Primes");
  doTest(runners, runner_num,
	 speeds, speed_num,
	 primes.array, primes.len, 
	 offset, times_to_do_tests, true, "Primes-Random");
  
  printf("done prime\n");
  delete primes.array;
  
  int sequential_numbers[max_number];
  for(int seq_index = 1; seq_index <= max_number - offset; seq_index++) {
    sequential_numbers[seq_index - 1] = seq_index;
  }

  doTest(runners, runner_num,
	 speeds, speed_num,
	 sequential_numbers, max_number - offset,
	 offset, times_to_do_tests, true, "Sequential-Random");
  doTest(runners, runner_num, 
	 speeds, speed_num,
	 sequential_numbers, max_number - offset,
	 offset, times_to_do_tests, false, "Sequential");
  
  std::string random = "../data_input/random_numbers.json";
  std::string random_sorted = "../data_input/random_numbers_sorted.json";
  
  len_array random_arr = file_data(random);
  len_array random_sorted_arr = file_data(random_sorted);
  
  cout << "Random\n";
  doTest(runners, runner_num,
	 speeds, speed_num,
	 random_arr.array, random_arr.len,
	 offset, times_to_do_tests, false, "Random");
  cout << "Done first random\n";
  delete random_arr.array;
  
  doTest(runners, runner_num,
	 speeds, speed_num,
	 random_sorted_arr.array, random_sorted_arr.len, 
	 offset, times_to_do_tests, false, "Random-Sorted");
  cout << "Done second random\n";
  delete random_sorted_arr.array;
}

int main (int argc, char *argv[]) {
  sequential_prime_test();
  //  range_test(1, 13, 10);
}
