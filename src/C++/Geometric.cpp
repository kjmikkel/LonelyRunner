#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <deque>
#include <vector>
#include "data_structure.h"

struct compare_event_point_pointers {
  bool operator() ( const event_point* first, const event_point* second ) const
  {

    int time1 = (first->local_position + first->rounds * (first->number_of_runners + 1)) * second->speed;
    int time2 = (second->local_position + second->rounds * (second->number_of_runners + 1)) * first->speed;;
    
    if(time1 < time2) {
      return false;
    } else if (time1 > time2) {
      return true;
    } else if(first->type == FINAL) {
      return false;
    } else if (first->type == START && second->type == END) {
      return false;
    } else if (first->runnerNumber < second->runnerNumber) {
      return false;
    }
		
    return true;
  }
};

typedef std::priority_queue<event_point*,std::vector<event_point*>, compare_event_point_pointers > event_point_priority_queue;
// use object(s) of type event_point_priority_queue where required

event_point* make_point(event_point* old_point, unsigned int position, point_type type) {
  event_point* point = new event_point;
  point->number_of_runners = old_point->number_of_runners;
  point->rounds = old_point->rounds + 1;
  point->speed = old_point->speed;
  point->runnerNumber = old_point->runnerNumber;
  point->local_position = position;
  point->type = type;
  return point;
}

void MakeTimePoints(event_point* old_end_point, event_point_priority_queue* queue) {

  event_point* start = make_point(old_end_point, 1, START);
  event_point* end = make_point(old_end_point, old_end_point->number_of_runners, END);

  queue->push(start);
  queue->push(end);
  }

void freeQueue(event_point_priority_queue* queue) {
  
  // First we remove and free all the points
  while(queue->size() > 0) {
    event_point* p = queue->top();
    queue->pop();
    
    free(p);
  }

  // Free the queue itself;
  free(queue);
}

time_result* Geometric_method (int speed_array[], const int length) {

  event_point_priority_queue* queue = new event_point_priority_queue;
  
  event_point* final_point = new event_point;
  final_point->number_of_runners = length;
  final_point->rounds = 1;
  final_point->speed = 1;
  final_point->runnerNumber = length + 1;
  final_point->local_position = length + 1;
  final_point->type = FINAL;
  queue->push(final_point);

  for(int pointIndex = 0; pointIndex < length; pointIndex++) {
    event_point point;
    point.number_of_runners = length;
    point.rounds = 0;
    point.speed = speed_array[pointIndex];
    point.runnerNumber = pointIndex;
    point.local_position = 0;
    point.type = START;
    
    MakeTimePoints(&point, queue);
  }

  int intersection = 0;
  while(queue->size() > 1) {
    // Find point and remove it from the queue
    event_point* p = queue->top();
    queue->pop();
    
    if (p->type == START) {
      intersection++;
      
      if (intersection == length) {
	
	float top = float(p->local_position + p->rounds * (length + 1)); 
	float down = float((p->speed * (length + 1)));
	
	time_result* has_result = new time_result;
	has_result->result = 1;
	has_result->result_time = top / down;
	
	freeQueue(queue); // Free the queue
	free(p); // Free the Point
	return has_result;
      }
    } else if (p->type == END) {
      intersection--;
      MakeTimePoints(p, queue);
      free(p); // free the point

    } else if (p->type == FINAL) {
      // There can be no solution, so free the entire queue
      freeQueue(queue);
      free(p); //... and the point
      
      time_result* no_result = new time_result;
      no_result->result = 0;
      no_result->result_time = 0.0;
      return no_result;
    }
  }
}
