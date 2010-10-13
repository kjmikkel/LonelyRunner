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

#include <stddef.h>
#include <stdlib.h>
#include <json/json.h>

struct time_record {
  unsigned int geo_result;
  unsigned int num_result;
  unsigned int speed;
};

using namespace std;

void array_to_json_array(unsigned int* array, json_object* json_array, int array_length) {
  for(int array_index = 0; array_index < array_length; array_index++) {
    json_object *jint = json_object_new_int(array[array_index]);
    json_object_array_add(json_array, jint);
  }
}

void doTest(int* runners, int* speeds, int* actual_speeds, int runner_num, int speed_num) {  
  for(int runner_index = 0; runner_index < runner_num; runner_index++) {
    
    stringstream ss;
    ss << runners[runner_index];
    
    
    string filename = "../data/test" + ss.str() + "runners.json";
    
    int start_speed_index = 0;
    int num_speeds = 0;
    
    // We find out how many cells of the array have speeds that are greater or equal to the numbero 
    for(int speed_index = 0; speed_index < speed_num; speed_index++) {
      if (speeds[speed_index] >= runners[runner_index]) {
	start_speed_index = speed_index;
	break;
      }
    }
    
    // I allocate the needed memory for the results
    int needed_mem = speed_num - start_speed_index;
    // time_record* records[needed_mem];
    
    unsigned int geo_results[needed_mem];
    unsigned int num_results[needed_mem];
    unsigned int speed_results[needed_mem];
    
    // I allocate the memory for the speeds we are going to use as input
    int num_runners = runners[runner_index];
    int runner_speeds[num_runners];
    
    for(int speed_index = start_speed_index; speed_index < speed_num; speed_index++) {
      
      int real_index = speed_index - start_speed_index;
      speed_results[real_index] = speeds[speed_index];
      
      printf("%d, %d\n", speeds[speed_index], num_runners); 
      
      int prime_real_index = 0;
      for(int copy_index = speeds[speed_index] - num_runners; copy_index < speeds[speed_index]; copy_index++) {
	runner_speeds[prime_real_index] = actual_speeds[copy_index];
	prime_real_index++;
      }
      /*
	for(int print_index = 0; print_index < num_runners; print_index++) {
	printf("runner speeds: %d\n", runner_speeds[print_index]);
	}
	printf("\n");
	break;
      */
      
      // The tests themselves
      struct timeval start;
      struct timeval end;
      
      struct timezone tz;
      struct tm *tm;
      
      gettimeofday(&start, &tz);
      time_result* geo_result = Geometric_method(runner_speeds, num_runners);
      gettimeofday(&end, &tz); 
      geo_results[real_index] = end.tv_usec - start.tv_usec;
      printf("Geo done\n");

      gettimeofday(&start, &tz);
      time_result* num_result = Numerical_method(runner_speeds, num_runners);
      gettimeofday(&end, &tz);
      num_results[real_index] = end.tv_usec - start.tv_usec;

      printf("geo: %d, num: %d\n", geo_results[real_index], num_results[real_index]);
    }
    
    // Create the json object we are going to store the tests in
    json_object *testInstance = json_object_new_object();
    
    /* Create the json arrays we are going to store the results in*/
    json_object *json_geo_results = json_object_new_array();
    array_to_json_array(geo_results, json_geo_results, needed_mem);
    json_object_object_add(testInstance ,"Geometrical results", json_geo_results);

    json_object *json_num_results = json_object_new_array();
    array_to_json_array(num_results, json_num_results, needed_mem);
    json_object_object_add(testInstance ,"Numerical results", json_num_results);

    json_object *json_speeds = json_object_new_array();
    array_to_json_array(speed_results, json_speeds, needed_mem);
    json_object_object_add(testInstance ,"speeds used", json_speeds);
    
    printf ("The json object created: %s\n",json_object_to_json_string(testInstance));
    
    ofstream out(filename.c_str());
    out << json_object_to_json_string(testInstance);
    out.close();
  }
}

int main (int argc, char *argv[]) {
  
  const int runner_num = 5;
  const int speed_num = 20;
  const int max_number = 50000;
  
  int runners[runner_num] = {10, 50, 100, 500, 1000};
  int speeds[speed_num]; 
  for(int speed_index = 0; speed_index < speed_num; speed_index++) {
    speeds[speed_index] = (speed_index + 1) * 100;
  }

  // We find the primes which are going as the speeds
  int* primes = findPrimes(max_number);
  doTest(runners, speeds, primes, runner_num, speed_num);
  
  int sequential_numbers[max_number];
  for(int seq_index = 1; seq_index <= max_number; seq_index++) {
    sequential_numbers[seq_index - 1] = seq_index;
  }
  
  doTest(runners, speeds, sequential_numbers, runner_num, speed_num);
    
}
