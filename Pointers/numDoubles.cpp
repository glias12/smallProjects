#include <iostream>

using namespace std;

int main()
{
  int arraySize = 0;
  double *array = NULL;
  int addTotal = 0;
  
  cin >> arraySize;

  if (arraySize <= 0)
    {
      cout << "Not a valid number" << endl;
      return 1;
    };
  
  array = new double[arraySize];

  for (int i = 0; i < arraySize; i++)
    cin >> array[i];

  for (int i = 0; i < arraySize; i++)
    addTotal = addTotal + array[i];

  cout << "Media:" << addTotal / arraySize << endl;

  delete [] array;
  return 0;
}
