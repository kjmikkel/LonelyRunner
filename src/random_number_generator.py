#    This file is part of The Lonely Runner Checker.
#
#    The Lonely Runner Checker is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    The Lonely Runner Checker is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with The Lonely Runner Checker.  If not, see <http://www.gnu.org/licenses/>.

import json, random, sys
import os, glob, re

max_number = 500000

def loadJsonFile(filename):
	file = open(filename, "r")
	return json.load(file)	

def saveFile(filename, data):
	file = open(filename, "w")
	file.write(data)
	file.close()	

def makeData():
	filename = "./data_input/"
	num_dict = {}
	random.seed(os.urandom(128))
	num_list = []
	while len(num_list) < max_number:
		int_candidate = random.randint(1, sys.maxint - 1)
		if not num_dict.has_key(int_candidate):
			num_list.append(str(int_candidate))
			num_dict[int_candidate] = True

	
	saveFile(filename + "random_numbers.json", json.dumps(num_list))	
	num_list.sort()
	saveFile(filename + "random_numbers_sorted.json", json.dumps(num_list))	
	
makeData()

