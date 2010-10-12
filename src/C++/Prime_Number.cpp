#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// Found here: http://www.troubleshooters.com/codecorn/primenumbers/primenumbers.htm

void printPrime(unsigned int bn)
	{
	static char buf[1000];

	sprintf(buf, "%ull", bn);
	buf[strlen(buf) - 2] = '\0';
	printf("%s\n", buf);
	}

int* findPrimes(int upTo)
{
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
