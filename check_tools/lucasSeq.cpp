#include <stdlib.h>
#include <iostream>


using namespace std;

int luca(int p,int q,int k)
{
  int v;
  int v0 = 2;
  int v1 = p;

  if(k==0)
  {
    return v0;
  }
  else if(k==1)
  {
    return v1;
  }
  else
  {
      return v = p*luca(p,q,k-1) - q * luca(p,q,k-2);
  }

}

int main()
{
  std::cout << "Hello" << '\n';
  //std::cout << luca(2,1,2) << '\n';
  std::cout << luca(1,-1,6) << '\n';
}
