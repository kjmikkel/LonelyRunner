#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

enum point_type { START, END, FINAL };

struct event_point  {
  unsigned int local_position;
  unsigned int number_of_runners;
  unsigned int rounds;
  unsigned int speed;
  unsigned int runnerNumber;
  point_type type;
};
		    
struct geo_time_result {
  // the time we are returning
  event_point point;

  // whether we could find a solution to equation (1)
  bool result;
};

struct len_array {
  int* array;  
  unsigned int len;
};

struct num_time_result {
  // the time we are returning
  unsigned int k1;
  unsigned int k2;
  unsigned int a;
  
  // whether we could find a solution to equation (1)
  bool result;
};
		  
#endif
