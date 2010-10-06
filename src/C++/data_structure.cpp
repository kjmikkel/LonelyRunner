
struct event_point {
  unsigned int local_position;
  unsigned int number_of_runners;
  unsigned int rounds;
  unsigned int speed;
  unsigned int runnerNumber;
  unsigned int type;
};

struct time_result {
  // the time we are returning
  float result_time;

  // whether we could find a solution to equation (1)
  bool result;
};
  
