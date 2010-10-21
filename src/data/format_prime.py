import json
import os, glob

import re, StringIO

prime = "prime_number_raw.txt"

def saveFile(filename, data):
	file = open(filename, "w")
	file.write(data)
	file.close()	

def fixPrimes(number_to_find):
	file = open(prime, "r")

	strFile = file.read()	
	file.close()	
	
	matcher = re.compile('\s*([0-9]+)\s+')
	
	primeList = matcher.findall(strFile)	
	
	primes_we_want = primeList[0:number_to_find -1]

	json_primes = json.dumps(primes_we_want)
	
	saveFile("prime_numbers.json", json_primes)

		
fixPrimes(50000)
