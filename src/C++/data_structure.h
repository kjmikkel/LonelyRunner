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
  event_point* point;

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
