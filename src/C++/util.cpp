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

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>

#include <sstream>

#include <json/json.h>

#include <boost/lexical_cast.hpp>

#include "data_structure.h"

std::string intToString(int i)
{
    return boost::lexical_cast<std::string>(i);
}

int stringToInt(std::string str) {
    using boost::bad_lexical_cast;

    try {
       return boost::lexical_cast<int>(str);
    } catch (bad_lexical_cast &) {
       return -1;
    }
}

std::string string_speeds(const int speed_array[], int length) {
    std::string buf;
    buf += "[";
    for(int i = 0; i < length; i++) {
        buf += intToString(speed_array[i]);
	if (i != length - 1)
            buf += ", ";
	}
    buf += "]";
    return buf;
}

std::string read_entire_file(char* filename) {
  std::string buf;
  std::string line;
      
  std::ifstream in(filename);
  while(std::getline(in,line))
    buf += line;
  
  return buf;
}

void write_to_file(char* filename, std::string data) {
  std::ofstream File(filename);

  File << data;

  File.close();
}

len_array read_json_file_array(char* filename) {
  std::string content = read_entire_file(filename);
  
  json_object * jobj = json_tokener_parse(content.c_str());
  
  int speed_number = 0;   
  int length = json_object_array_length(jobj);
  int* array = NULL;
  array = new int[length];
  
  json_object * jvalue;
  
  for (int index = 0; index < length; index++){
    jvalue = json_object_array_get_idx(jobj, index); /*Getting the array element at position index*/
    array[index] = json_object_get_int(jvalue); 
  }
  
  len_array result;
  result.array = array;
  result.len = length;

  return result;
}

