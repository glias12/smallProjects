#include <iostream>

using namespace std;

void addOne(int *ptrNum)
{
  (*ptrNum)++;
};

int main()
{
  int numEntero = 0;

  cin >> numEntero;
  
  addOne(&numEntero);

  cout << numEntero << endl;

  return 0;
}
