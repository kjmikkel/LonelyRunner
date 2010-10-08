#ifndef D_STRUCT_H
#define D_STRUCT_H

enum point_type { START, END, FINAL };

struct event_point {
  unsigned int local_position;
  unsigned int number_of_runners;
  unsigned int rounds;
  unsigned int speed;
  unsigned int runnerNumber;
  point_type type;
};

struct time_result {
  // the time we are returning
  double result_time;

  // whether we could find a solution to equation (1)
  bool result;
};



#endif
