#include <stdlib.h>
#include <stdio.h>
#include <NTL/ZZ.h>
#include <queue>
#include <vector>
#include <math.h>
#include "data_structure.h"

NTL_CLIENT

struct compare_event_point_pointers {
  bool operator() ( const event_point* first, const event_point* second ) const
  {
    /*
    ZZ rounds1 = to_ZZ(first->rounds);
    ZZ rounds2 = to_ZZ(second->rounds);
    */
    
    ZZ time1 = (first->local_position +  to_ZZ(first->rounds) *  (first->number_of_runners + 1)) *  second->speed;
    ZZ time2 = (second->local_position + to_ZZ(second->rounds) * (second->number_of_runners + 1)) * first->speed;
   
    //    if (time1 > pow(2, 32) || time2 > pow(2, 32))
    // cout << "above " << time1 << ", " << time2 << "\n";

    /*
    unsigned int time1 = first->pre_computed * second->speed;
    unsigned int time2 = second->pre_computed * first->speed;;
    */
    /*
    int time1 = (first->local_position + first->rounds * (first->number_of_runners + 1)) * second->speed;
    int time2 = (second->local_position + second->rounds * (second->number_of_runners + 1)) * first->speed;;
    */
    // if (time1 < second->speed || time2 < first->speed) 
    //  printf("time1: %i and time2: %i\n", time1, time2);
    
    if(time1 < time2) {
      //      cout << "time2 larger than time1\n";
      return false;
    } else if (time1 > time2) {
      // cout << "time1 larger than time2\n";
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

static event_point* make_point(event_point* old_point, unsigned int position, point_type type) {
  event_point* point = new event_point;
  point->number_of_runners = old_point->number_of_runners;
  point->rounds = old_point->rounds + 1;
  point->speed = old_point->speed;
  point->runnerNumber = old_point->runnerNumber;
  point->local_position = position; 
  // point->pre_computed = position + (old_point->rounds + 1) * (length + 1);
  point->type = type;
  return point;
}

static void MakeTimePoints(event_point* old_end_point, event_point_priority_queue* queue, unsigned int length) {

  event_point* start = make_point(old_end_point, 1, START);
  event_point* end = make_point(old_end_point, length, END);
  
  queue->push(start);
  queue->push(end);
}

static void freeQueue(event_point_priority_queue* queue) {
  
  // First we remove and free all the points
  event_point* p;
  while(!queue->empty()) {
    p = queue->top();
    queue->pop();
    
    delete p;
  }
  
  // Free the queue itself;
  delete queue;
}

geo_time_result* Geometric_method (const int speed_array[], const int length) {
  if (length < 1) {
    std::cout << "The list of speeds is empty\n";
   
    geo_time_result* no_result = new geo_time_result;
    no_result->result = 0;
    no_result->point = NULL;
    return no_result;
  }
  event_point_priority_queue* queue = new event_point_priority_queue;
  
  event_point* final_point = new event_point;
  final_point->number_of_runners = length;
  final_point->rounds = 1;
  final_point->speed = 1;
  final_point->runnerNumber = length + 1;
  final_point->local_position = length + 1;
  //final_point->pre_computed = (length + 1) + 1 * (length + 1);
  final_point->type = FINAL;
  queue->push(final_point);

  for(int pointIndex = 0; pointIndex < length; pointIndex++) {
    event_point point;
    point.number_of_runners = length;
    point.rounds = -1; // It is important that rounds = -1, otherwise there will be a problem when the points are made
    point.speed = speed_array[pointIndex];
    point.runnerNumber = pointIndex;
    point.local_position = 0;
    point.type = START;
    
    MakeTimePoints(&point, queue, length);
  }

  int intersection = 0;
  event_point* p;
  while(!queue->empty()) {
    // Find point and remove it from the queue
    p = queue->top();
    queue->pop();
    
    if (p->type == START) {
      intersection++;
      if (intersection == length) {

	/*
	double top = p->pre_computed; 
	double down = p->speed * (length + 1.0);
	*/
	//	printf("Top: %f\nDown: %f\n", top, down);

	geo_time_result* has_result = new geo_time_result;
	has_result->result = true;
	has_result->point = p;
	
      	freeQueue(queue); // Free the queue
	return has_result;
      }
      
      delete p; // Delete the point
    } else if (p->type == END) {
      intersection--;
      // cout << "-: " << intersection << "\n";
      
      MakeTimePoints(p, queue, length);
      // delete p; // free the point

    } else if (p->type == FINAL) {
      // There can be no solution, so free the entire queue
      //cout << "***Final!***\n";
      freeQueue(queue);
      delete p; //... and the point
      
      geo_time_result* no_result = new geo_time_result;
      no_result->result = 0;
      no_result->point = NULL;
      return no_result;
    }
  }
}

/*
int main (int argc, char *argv[])
{
  if (true) {
    int testArray[6] = {12, 3, 7, 9, 55, 200};
    geo_time_result* result = Geometric_method(testArray, 6);
    
    delete result->point;
    delete result;
  }
}
*/
