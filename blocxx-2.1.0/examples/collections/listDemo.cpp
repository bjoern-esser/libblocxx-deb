/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* list.cpp
**
** This demonstrates how to use the various collections classes of BloCxx
*  Collections classes demonstrated include:
* 
*  Other classes demonstrated include:
*
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/String.hpp>
#include <blocxx/List.hpp>

#include <iostream>

using namespace BLOCXX_NAMESPACE;
using namespace std;

#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"


typedef List<String> HolidaysList;

HolidaysList usHolidays;


void printList(String header)
{
   HolidaysList::iterator it;
   
   // print out the entire list, in the order the list has them
   // get an iterator and loop through it
   cout << "==== " << header << " ====" << endl;
   it = usHolidays.begin();
   while (it != usHolidays.end())
   {
      cout << "   " << *it << endl;
      it++;
   }
}


// ********************************************************************
// The main function... This demonstrates the Collections classes
// ********************************************************************
int main(int argc, char* argv[])
{
   try
   {
      // you can push on the back of the list
      usHolidays.push_back("*** Begin 1 Pushing Back ***");
      usHolidays.push_back("1: New Years Day");
      usHolidays.push_back("2: Valentine's Day");
      usHolidays.push_back("3: St. Patrick's Day");
      usHolidays.push_back("*** End 1 Pushing Back ***");
      
      // or on the front of the list
      usHolidays.push_front("*** Begin 2 Pushing Front ***");
      usHolidays.push_front("4: Easter");
      usHolidays.push_front("5: Memorial Day");
      usHolidays.push_front("6: Independence Day");
      usHolidays.push_front("*** End 2 Pushing Front ***");
      
      // or insert
      usHolidays.insert(++usHolidays.begin(), "*** Begin 3 Insert @ ++ begin ***");
      usHolidays.insert(++usHolidays.begin(), "7: Labor Day");
      usHolidays.insert(++usHolidays.begin(), "8: Halloween");
      usHolidays.insert(++usHolidays.begin(), "9: Thanksgiving");
      usHolidays.insert(++usHolidays.begin(), "*** End 3 Insert @ ++ begin ***");
      usHolidays.insert(usHolidays.begin(), "*** Begin 4 Insert @ begin ***");
      usHolidays.insert(usHolidays.begin(), "10:Christmas Eve");
      usHolidays.insert(usHolidays.begin(), "11:Christmas Day");
      usHolidays.insert(usHolidays.begin(), "12:New Years Eve");
      usHolidays.insert(usHolidays.begin(), "*** End 4 Insert @ begin ***");
      
      cout << "List size: " << usHolidays.size() << endl;;
      cout << "List MaxSize: " << usHolidays.max_size() << endl;;

      printList("All holidays: ");

      // add a duplicate, print the list, then call unique to remove duplicates
      usHolidays.push_back("duplicate");
      usHolidays.push_back("duplicate");
      usHolidays.push_front("duplicate");

      printList("All holidays after duplicates added (1 at beginning, 2 at end): ");
      usHolidays.unique();
      printList("All holidays after unique: Notice it only removes consecutive duplicates");
      
      // now remove "duplicate" - does it remove both or just the first one it finds?
      usHolidays.remove("duplicate");
      printList("All holidays after removing 'duplicate': notice it removes BOTH instances of 'duplicate'");
      
      // reverse the list
      usHolidays.reverse();
      printList("All holidays after reverse");

      // now pop the front and the back
      cout << "Referencing front: " << usHolidays.front() << endl;
      cout << "Referencing back: " << usHolidays.back() << endl;
      printList("All holidays after referencing front and back");
      cout << "Popping from front and back " << endl;
      usHolidays.pop_front();
      usHolidays.pop_back();
      printList("All holidays after popping front and back");


      // now clear the list
      usHolidays.clear();
      printList("Entire List - after Clear");
      cout << "Done printing the entire list after clear. " << endl;
      
      return 0;
   }
   catch(Exception& e)
   {
      cerr << "ERROR:   Exception:" << e << endl;
   }
   catch(std::exception& e)
   {
      cerr << "ERROR:   sdtException:" << e.what() << endl;
   }
   catch(...)
   {
      cerr << "ERROR:   UnknownException." << endl;
   }

   return 1;
}
