
#include "mods/utils/integerFactorization.hpp"

namespace mods
{
   namespace utils
     {
	namespace
	  {
	     void factorize(int n, std::vector<int>* factors)
	       {
		  int currentValue = 2;
		  while((currentValue * currentValue) <= n)
		    {
		       if((n % currentValue) == 0)
			 {
			    factors->push_back(currentValue);
			    factorize(n / currentValue, factors);
			    return;
			 }
		       ++currentValue;
		    }
		  factors->push_back(n);
	       }
	  }
	
	std::vector<int> integerFactorization(int n)
	  {
	     std::vector<int> v;
	     factorize(n, &v);
	     return v;
	  }
     } // namespace utils
} // namespace mods