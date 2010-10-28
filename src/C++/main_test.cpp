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

void recursive_array(int* array, int* number_array, 
		     int count_up, int last_index, 
		     int array_index, int max_number, const int num_runners) {

  for(int index = last_index + 1; index < count_up + 1; index++) { 
    array[array_index] = number_array[index];
    /*  
cout << "Index: " << index << ", " << count_up << ", array_index: " << array_index  << ", array_val" << array[array_index]  << "\n";
    int val2;
    cin >> val2;
    */
    if(array_index < num_runners) {
      if (array_index == 0) {
	printf("The first Index is at %d\n", array[array_index]);
      }
      
      recursive_array(array, number_array, 
		      count_up + 1, last_index + 1, 
		      array_index + 1, max_number,
		      num_runners);
      
    } else {
           
      //    geo_time_result* geo_result = NULL;

      
      geo_time_result* geo_result = Geometric_method(array,		     
					   	     num_runners);
      
      
      
      
      
      // We check for errors
      if(geo_result != NULL && (!geo_result->result || !isValid(geo_result, array))) {
	  
	printf("error: %d, %d\n", geo_result->result, isValid(geo_result, array));
	printf("For the values: [");
	//stringstream ss;
	
	for(int index = 0; index < num_runners; index++) {  
	  printf(", %d", array[index]);
	  /*
	  ss.seekp(0);
	  ss.str("");
	  */
	}
	printf("]\n\n");
      }
      delete geo_result->point;
      delete geo_result;
      
    }
  }
}

// This will test every single possible combination of speeds under or equal to 100 with 10 runners 
void ultimateTest() {
  struct timeval start;
  struct timeval end;
  
  struct timezone tz;
  struct tm *tm;

  // The array which are going to contain all the different permutations of speeds below 100
  int array_number = 10;
  int test_array[array_number];

  // The array that is going to contain the values we are going to check
  int max_number = 100;
  int real_number_array[max_number];
  
  for(int index = 0; index < max_number; index++) {
    real_number_array[index] = 2 + index;
  }
 
  FILE *stream ;
  if((stream = freopen("output", "w", stdout)) == NULL)
    exit(-1);

  // Now to populate the array with 10 values
  gettimeofday(&start, &tz);
  
  recursive_array(test_array, real_number_array, 
		  max_number - array_number - 1, 
		  -1,
		  0,
		  max_number, array_number);
  
  gettimeofday(&end, &tz);
  cout << "\nDone. Making this took ";
  
  stringstream ss;
  ss << (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
  cout << ss.str() << " microseconds.\n";
  //delete start;
  //delete tz;
  
  // delete end;
  // delete tm;
}

void doTest(int* runners, int* speeds, int* actual_speeds, int runner_num, int speed_num, int offset, int times_to_do_test, bool randomize, string name) {  
  for(int runner_index = 0; runner_index < runner_num; runner_index++) {
    
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
    
    // We find out how many cells of the array have speeds that are greater or equal to the number
    for(int speed_index = 0; speed_index < speed_num; speed_index++) {
      if (speeds[speed_index] >= runners[runner_index]) {
	start_speed_index = speed_index;
	break;
      }
    }
    // I allocate the needed memory for the results
    int needed_mem = speed_num - start_speed_index;
    
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

    for(int speed_index = start_speed_index; speed_index < speed_num; speed_index++) {
      
      int real_index = speed_index - start_speed_index;
      speed_results[real_index] = speeds[speed_index];
      
      int prime_real_index = 0;
      for(int copy_index = speeds[speed_index] - num_runners; copy_index < speeds[speed_index]; copy_index++, prime_real_index++) {
	runner_speeds[prime_real_index] = actual_speeds[copy_index];
      }
      
      // The tests themselves
      struct timeval start;
      struct timeval end;
      
      struct timezone tz;
      struct tm *tm;
      
      cout << "before geo\n";
      //    unsigned long micro_seconds = 0;
      unsigned long seconds = 0; 
      
      unsigned long time_test_array[times_to_do_test];
      unsigned long seconds_time_test_array[times_to_do_test];
      for(int time_test_index = 0; time_test_index < times_to_do_test; time_test_index++) {
	gettimeofday(&start, &tz);
	geo_time_result* geo_result = Geometric_method(runner_speeds, num_runners);
	gettimeofday(&end, &tz); 
	
	time_test_array[time_test_index] = (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
	seconds_time_test_array[time_test_index] = (end.tv_sec - start.tv_sec);
	
	// We check for errors
	if(geo_result != NULL && (!geo_result->result || !isValid(geo_result, runner_speeds))) {	  
	  printf("error: %d, %d\n", geo_result->result, isValid(geo_result, runner_speeds));
	  
	  num_time_result* num_result = Numerical_method(runner_speeds, num_runners, true, false);
	  printf("error num: %d, %d\n", num_result->result, isValid(num_result, runner_speeds, num_runners));
	  
	  return;
	  // If there is an error the we record it
	  b_geo_error = true;
	  // We record the time it happened
	  geo_error[real_index] = runner_speeds[real_index];
	} else {
	  // to be able to differentiate it from all the other values we set it to 0
	  geo_error[real_index] = 0;
	}
	if (geo_result != NULL)
	  delete geo_result->point;
	delete geo_result;
      }
      cout << "after geo\n";

      // I record the values
      long_pair pair = find_spread(time_test_array, times_to_do_test);
      geo_results[real_index] = pair.average;
      geo_spread[real_index] = pair.spread;
      seconds = 0;
      for(int index = 0; index < times_to_do_test; index++) {
	seconds += seconds_time_test_array[index];
      }
      geo_seconds[real_index] = seconds / times_to_do_test;

      cout << "before num\n";
      for(int time_test_index = 0; time_test_index < times_to_do_test; time_test_index++) {
	cout << "Index of num: " << (time_test_index + 1) << "\n";
	gettimeofday(&start, &tz);
	num_time_result* num_result = Numerical_method(runner_speeds, num_runners, randomize, false);
	gettimeofday(&end, &tz);
	time_test_array[time_test_index] = (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
	seconds_time_test_array[time_test_index] = (end.tv_sec - start.tv_sec);

	if(!num_result->result || !isValid(num_result, runner_speeds, num_runners)) {
	  printf("*error*: %d, %d\n", num_result->result, isValid(num_result, runner_speeds, num_runners));
	 
	  b_num_error = true;
	  num_error[real_index] = runner_speeds[real_index];
	} else {
	  num_error[real_index] = 0;
	}	
	delete num_result;
      }
      cout << "after num\n";
      
      pair = find_spread(time_test_array, times_to_do_test);
      num_results[real_index] = pair.average;
      num_spread[real_index] = pair.spread;

      seconds = 0;
      for(int index = 0; index < times_to_do_test; index++) {
	seconds += seconds_time_test_array[index];
      }
      num_seconds[real_index] = seconds / times_to_do_test;
      
      cout << "geo: " <<  geo_results[real_index] << ", num: " << num_results[real_index] << "\n";
    }
      
    // Create the json object we are going to store the tests in
    json_object *testInstance = json_object_new_object();
    
    /* Create the json arrays we are going to store the results in*/
    add_to_json_object(testInstance, geo_results, needed_mem, "Geometrical results");
    add_to_json_object(testInstance, geo_spread, needed_mem, "Geometrical spread");
    add_to_json_object(testInstance, geo_seconds, needed_mem, "Seconds used");
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


void sequential_prime_test() {
  const int runner_num = 8;
  const int offset = 0;
  const int speed_num = 80;
  int max_number = 500000;
  int times_to_do_tests = 10;
  
  // The number of runners
  int runners[runner_num] = {10, 50, 100, 500, 1000, 2000, 4000, 8000};
  int speeds[speed_num]; 
  
  // The array contains the speeds we are going to test
  for(int speed_index = 0; speed_index < speed_num; speed_index++) {
    speeds[speed_index] = (speed_index + 1) * 100 + offset;
  }

  // We find the primes which are going as the speeds
  len_array primes = findPrimes(max_number);
   doTest(runners, speeds, primes.array, primes.len, speed_num, offset, times_to_do_tests, false, "Primes");
  doTest(runners, speeds, primes.array, primes.len, speed_num, offset, times_to_do_tests, true, "Primes-Random");
  printf("done prime\n");
  
  delete primes.array;
  
  int sequential_numbers[max_number];
  for(int seq_index = 1; seq_index <= max_number - offset; seq_index++) {
    sequential_numbers[seq_index - 1] = seq_index;
  }

  doTest(runners, speeds, sequential_numbers, runner_num, speed_num, offset, times_to_do_tests, true, "Sequential-Random");
  doTest(runners, speeds, sequential_numbers, runner_num, speed_num, offset, times_to_do_tests, false, "Sequential");
  
  srand(time(0));
  for(int random_index = 0; random_index < max_number; random_index++) {
    sequential_numbers[random_index] = rand() + 2;
  }
  doTest(runners, speeds, sequential_numbers, runner_num, speed_num, 0, times_to_do_tests, false, "Random");
  //  doTest(runners, speeds, sequential_numbers, runner_num, speed_num, 0, times_to_do_tests, false,  "Random");
}

int main (int argc, char *argv[]) {
  sequential_prime_test();
  //ultimateTest();
}
