#include <queue>
#include <deque>
#include <vector>

#include "data_structure.cpp"
  

struct compare_event_point_pointers {
  bool operator() ( const event_point* first, const event_point* second ) const
  {
    // return true if first has lower priority than second.
   //  false otherwise.
  }
};

typedef std::priority_queue<event_point*,std::vector<event_point*>, compare_event_point_pointers > event_point_priority_queue;

// use object(s) of type event_point_priority_queue where required

event_point make_point(event_point* old_point, unsigned int position, unsigned int type) {
  event_point point;
  point.number_of_runners = old_point->number_of_runners;
  point.rounds = old_point->rounds + 1;
  point.speed = old_point->speed;
  point.runnerNumber = old_point->runnerNumber;
  point.local_position = position;
  point.type = type;
  return point;
}

void MakeTimePoints(event_point* old_end_point, event_point_priority_queue* queue) {
  
  event_point start = make_point(old_end_point, 1, 0);
  event_point end = make_point(old_end_point, old_end_point->number_of_runners, 1);
  
  queue.

  }
    
    
  time_result Geometric_method () {
      


  }
