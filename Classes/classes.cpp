#include <iostream>

using namespace std;

class DayOfYear
{
public:
  void output();
  int month;
  int day;
};

int main()
{
  DayOfYear today, birthday;

  cout << "Enter today's date:" << endl;
  cout << "Enter Month as a number: ";
  cin >> today.month;

  cout << "Enter the day of the month ";
  cin >> today.day;

  cout << "Enter your birthday:" << endl;
  cout << "Enter month as a number: ";
  cin >> birthday.month;

  cout << "Enter the day of the month: ";
  cin >> birthday.day;

  cout << "Today's date is ";
  today.output();

  cout << "Your birthday is ";
  birthday.output();

  if (today.month == birthday.month && today.day == birthday.day)
    cout << "Happy Birthday!\n";
  else
    cout << "Happy Unbirthday\n";

  return 0;
}

void DayOfYear :: output ()
{
  cout << "month = " << month
       << ", day = " << day << endl;
}


