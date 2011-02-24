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
	
#	primes_we_want = primeList[0:number_to_find -1]

	primes_we_want = primeList

	json_primes = json.dumps(primes_we_want)
	
	saveFile("prime_numbers.json", json_primes)

		
fixPrimes(50000)
