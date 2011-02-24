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

#ifndef NUM_H
#define NUM_H

#include "data_structure.h"

num_time_result* Numerical_method (int speed_array[], const int length, bool randomize_order, bool reverse, bool check_for_max_solution);

bool isValid(num_time_result* result, const int array[], unsigned int number_runners);

bool isValid(geo_time_result* result, const int array[]);

static bool checkForSolution(int speedArray[], int length);

double closeToInteger(double x, int w);

#endif
