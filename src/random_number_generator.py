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

