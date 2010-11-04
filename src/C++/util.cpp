#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>

#include <json/json.h>

#include "data_structure.h"

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

