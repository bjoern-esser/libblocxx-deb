/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* array.cpp
**
** This demonstrates how to use the various collections classes of BloCxx
*  Collections classes demonstrated include:
*
*  Other classes demonstrated include:
*
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/String.hpp>
#include <blocxx/Array.hpp>   // array is actually a class that implements an stl vector

#include <iostream>

using namespace BLOCXX_NAMESPACE;
using namespace std;

#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"


//typedef Array<String> StringArray;  // note: this is declared in CommonFwd.hpp, but we did it here to demonstrate it
typedef Array<String> HolidaysArray;  // this is the one we'll actually use

HolidaysArray usHolidays;
//StringArray months(12); // or I could construct it with an initial size

void printArray(String header)
{
   HolidaysArray::iterator it;

   // print out the entire Array, in the order the Array has them
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
      // you can add to the end of the array
      usHolidays += String("*** Begin 1 Adding (+=) ***");
      usHolidays += String("1: New Years Day");
      usHolidays += String("2: Valentine's Day");
      usHolidays += String("*** End 1 Adding (+=) ***");

      // you can push_back to the end of the array
      usHolidays.append("*** Begin 2 Appending ***");
      usHolidays.append("3: St. Patrick's Day");
      usHolidays.append("4: Easter");
      usHolidays.append("*** End 2 Appending ***");

      // or on the front of the Array
      usHolidays.push_back("*** Begin 3 Pushing Back ***");
      usHolidays.push_back("5: Memorial Day");
      usHolidays.push_back("6: Independence Day");
      usHolidays.push_back("*** End 3 Pushing Back ***");

      // you CAN't push_front

      // I can Insert at a position
      // Notice that by inserting them all at position 3, they'll be inserted in reverse order,
      // because the second insert at position 3 will push the first insert at 3 back one
      usHolidays.insert(3, "*** Begin 4 Insert at pos 3***");
      usHolidays.insert(3, "7: Labor Day");
      usHolidays.insert(3, "8: Halloween");
      usHolidays.insert(3, "*** End 4 Insert at pos 3***");

      // I can add to usHolidays from another HolidaysArray
      HolidaysArray moreHolidays;

      moreHolidays += String("*** Begin 5 MoreHolidaysArray - first append***");
      moreHolidays += String("9: Thanksgiving");
      moreHolidays += String("10:Christmas Eve");
      moreHolidays += String("*** End 5 MoreHolidaysArray - first append***");
      usHolidays.appendArray(moreHolidays);

      //moreHolidays += String("11:Christmas Day");
      //moreHolidays += String("12:New Years Eve");
      //moreHolidays += String("*** Ending of MoreHolidaysArray");

      cout << "Array size: " << usHolidays.size() << endl;;
      cout << "Array MaxSize: " << usHolidays.max_size() << endl;;
      cout << "Array Capacity: " << usHolidays.capacity() << endl;;

      printArray("All holidays: ");


      // now remove element at index 5
      cout << "element at index 5 prior to removal: " << usHolidays[5] << endl;;
      usHolidays.remove(5);
      cout << "element at index 5 after removal: " << usHolidays[5] << endl;;
      printArray("All holidays after removing element at index 5");

      // now remove elements 1-3
      cout << "element at index 0 prior to removing elements 1-3: " << usHolidays[0] << endl;;
      cout << "element at index 1 prior to removing elements 1-3: " << usHolidays[1] << endl;;
      cout << "element at index 2 prior to removing elements 1-3: " << usHolidays[2] << endl;;
      cout << "element at index 3 prior to removing elements 1-3: " << usHolidays[3] << endl;;
      cout << "element at index 4 prior to removing elements 1-3: " << usHolidays[4] << endl;;
      usHolidays.remove(1,3); // notice that this means 1-inclusive through 3-exclusive, or 1-2
      cout << "element at index 0 after removing elements 1-3: " << usHolidays[0] << endl;;
      cout << "element at index 1 after removing elements 1-3: " << usHolidays[1] << endl;;
      cout << "element at index 2 after removing elements 1-3: " << usHolidays[2] << endl;;
      cout << "element at index 3 after removing elements 1-3: " << usHolidays[3] << endl;;
      cout << "element at index 4 after removing elements 1-3: " << usHolidays[4] << endl;;
      cout << "Notice calling remove(1,3) actually removed 1-inclusive through 3-exclusive" << endl;

      // pop_back - can't pop_front of an array
      usHolidays.pop_back();
      printArray("All holidays after pop_back");

      // remove all holidays that have the word "Day" in them
      HolidaysArray::iterator it;
      it = usHolidays.begin();
      while( it != usHolidays.end() )
      {
         if ((*it).endsWith("***"))
         {
            it = usHolidays.erase(it);
         }
         else
         {
            // only increment if we didn't remove, because remove returns the next element
            it++;
         }
      }
      printArray("List after removing all comment lines (***)");


      // now clear the Array
      usHolidays.clear();
      printArray("Entire Array - after Clear");
      cout << "Done printing the entire Array after clear. " << endl;

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
