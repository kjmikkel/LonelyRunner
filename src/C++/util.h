#ifndef UTIL_H
#define UTIL_H

#include "data_structure.h"
int extern stringToInt(std::string str);
std::string extern intToString(int i);
std::string extern string_speeds(const int speeds[], int length);
len_array extern read_json_file_array(char* filename);
void extern write_to_file(char* filename, std::string data);

#endif
