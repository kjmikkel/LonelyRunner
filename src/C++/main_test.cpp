#include "Prime_Number.h"
#include "Geometric.h"
#include "Numerical.h"

#include "data_structure.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <json/json.h>

int main (int argc, char *argv[]) {
  
  const int runner_num = 5;
  int speed_num = 20;
  
  int runners[runner_num] = {10, 50, 100, 500, 1000};
  int speeds[speed_num]; 
  for(int speed_index = 0; speed_index < speed_num; speed_index++) {
    speeds[speed_index] = (speed_index + 1) * 100;
  }

  // We find the primes which are going as the speeds
  int* primes = findPrimes(50000);

  printf("prime: %d \n", primes[1000]);
  
  for(int runner_index = 0; runner_index < runner_num; runner_index++) {
  
    std::stringstream ss;
    ss << runners[runner_index];
    
    
    std::string filename = "test" + ss.str() + "runners";
    
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
    int geo_results[speed_num - start_speed_index];
    int num_results[speed_num - start_speed_index];
    int speed_results[speed_num - start_speed_index];
    
    // I allocate the memory for the speeds we are going to use as input
    int num_runners = runners[runner_index];
    int runner_speeds[num_runners];
    
    for(int speed_index = start_speed_index; speed_index < speed_num; speed_index++) {
      
      int real_index = speed_index - start_speed_index;
      speed_results[real_index] = speeds[speed_index];
      
      printf("%d, %d\n", speeds[speed_index], num_runners);
      printf("%d\n", *(primes + sizeof(int) * (speeds[speed_index] - num_runners)));
      //continue;



      //      memcpy(runner_speeds, primes + sizeof(int) * (speeds[speed_index] - num_runners), sizeof(runner_speeds));
      int prime_real_index = 0;
      for(int copy_index = speeds[speed_index] - num_runners; copy_index < speeds[speed_index]; copy_index++) {
	runner_speeds[prime_real_index] = primes[copy_index];
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
      
      gettimeofday(&start, &tz);
      time_result* num_result = Numerical_method(runner_speeds, num_runners);
      gettimeofday(&end, &tz);
      num_results[real_index] = end.tv_usec - start.tv_usec;

      //char * string = "{\"name\" : \"joys of programming\"}";
      //json_object * jobj = json_tokener_parse(string);
      
      // ofstream result_file;
      
  
  
      //  json_object *container = json_object_new_object ();
      
      //  json_object *j = json_object_new_array();
      
      ///  result_file.open(".\");
      // result_file << 
      //result_file.close();
    }
  }
}
