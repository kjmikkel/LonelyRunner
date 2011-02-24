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

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <assert.h>

#include "data_structure.h"
#include "util.h"

// The prime numbers were found at: http://www.troubleshooters.com/codecorn/primenumbers/primenumbers.htm

len_array findPrimes(int upTo)
{
  std::string filename = "../prime_numbers.json";
  char* file = const_cast<char *>(filename.c_str());
  len_array arr = read_json_file_array(file);
  return arr;
}

  
int* primeAlgo(int upTo) {
  int *array = new int[upTo + 1];
  assert(array != NULL);

  /* SET ALL BUT 0 AND 1 TO PRIME STATUS */
  int ss;
  for(ss = 0; ss <= upTo+1; ss++)
    array[ss] = ss;
  array[0] = 0;
  array[1] = 0;
  
  /* MARK ALL THE NON-PRIMES */
  int thisFactor = 2;
  int lastSquare = 0;
  int thisSquare = 0;
  while(thisFactor * thisFactor <= upTo)
    {
      /* MARK THE MULTIPLES OF THIS FACTOR */
      int mark = thisFactor + thisFactor;
      while(mark <= upTo)
	{
	  array[mark] = 0;
	  mark += thisFactor;
	}
      
      /* PRINT THE PROVEN PRIMES SO FAR */
      thisSquare = thisFactor * thisFactor;
      for(;lastSquare < thisSquare; lastSquare++)
	{
	  //  if(array[lastSquare]) printPrime(lastSquare);
	}
      
      /* SET thisFactor TO NEXT PRIME */
      thisFactor++;
      while(array[thisFactor] == 0) thisFactor++;
      assert(thisFactor <= upTo);
    }
  
  /* PRINT THE REMAINING PRIMES */
  for(;lastSquare <= upTo; lastSquare++)
    {
      //    if(array[lastSquare]) printPrime(lastSquare);
    }
  
  
  int prime_found = 0;
  int prime_index = 0;
  while(prime_index < upTo) {
    while(array[prime_index] == 0) prime_index++;
    prime_found++;
    prime_index++;
  }
  
  int *return_primes = new int[prime_found];
  
  int primes_inserted = 0;
  prime_index = 0;
  while(primes_inserted < prime_found) {
    
    while(array[prime_index] == 0) prime_index++;
    return_primes[primes_inserted] = array[prime_index];
    primes_inserted++;
    prime_index++;
  }
  return return_primes;
}
