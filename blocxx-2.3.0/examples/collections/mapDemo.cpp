/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* map.cpp
**
** This demonstrates how to use the various collections classes of BloCxx
*  Collections classes demonstrated include:
*
*  Other classes demonstrated include:
*
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/String.hpp>
#include <blocxx/Map.hpp>

#include <iostream>

using namespace BLOCXX_NAMESPACE;
using namespace std;

#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"


typedef Map<String, String> HolidaysMap;
typedef HolidaysMap::value_type holiday;
HolidaysMap usHolidays;

void printMap(String header)
{
   HolidaysMap::iterator it;
   cout << "==== " << header << " ====" << endl;
   it = usHolidays.begin();
   while (it != usHolidays.end())
   {
      holiday hol = (holiday)*it;
      cout << "   " << hol.first << ":  " << hol.second << endl;
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
      HolidaysMap::iterator it;


      // you can insert using array index
      usHolidays["New Years Day"] = "Jan 1";
      usHolidays["Valentine's Day"] = "Feb 14";
      usHolidays["St. Patrick's Day"] = "Mar 17";
      usHolidays["Easter"] = "1st Sunday after 1st Full Moon after March 21";
      usHolidays["Memorial Day"] = "Last Monday in May";
      usHolidays["Independence Day"] = "July 4";

      // or you can insert using .insert method
      usHolidays.insert(HolidaysMap::value_type("Labor Day", "First Monday in September"));

      // make it easier by typedef - see the typedef above:  typedef HolidaysMap::value_type holiday;
      usHolidays.insert(holiday("Halloween", "October 31"));
      usHolidays.insert(holiday("Thanksgiving", "Last Thursday in November"));
      usHolidays.insert(holiday("Christmas", "December 25"));
      usHolidays.insert(holiday("New Years Eve", "December 31"));

      cout << "Map size: " << usHolidays.size() << endl;;
      cout << "Map MaxSize: " << usHolidays.max_size() << endl;;

      // print out the entire list, alphabetically (because that's the default comparator)
      // get an iterator and loop through it
      printMap("All holidays, alphabetically: ");

      // lookup a specific entry - this will output the value only
      cout << "Looking up date of 'Easter': " << usHolidays["Easter"] << endl;


      // lookup a specific non-existent entry: Warning - this will put an empty-valued entry in the map
      cout << "Looking up date of '4th of July': " << usHolidays["4th of July"] << endl;
      cout << "NOTE: Didn't find '4th of July', but '4th of July' now exists as an empty entry in the map" << endl;
      // try to find 5th of July with the find method, see below


      // Halloween isn't really a holiday, so erase it
      // NOTE:   This erases the value, doesn't remove the key - it should, but it doesn't
      cout << "Should be here:  Looking up date of 'Halloween': " << usHolidays["Halloween"] << endl;;
      usHolidays.erase("Halloween");
      cout << "Should be gone:  Looking up date of 'Halloween': " << usHolidays["Halloween"] << endl;;


      // print out the entire list, alphabetically (because that's the default comparator)
      // get an iterator and loop through it
      printMap("All holidays, alphabetically: Notice that Halloween is still here, with an empty value, as is '4th of July'");


      // what happens if I add two with the same key
      cout << "Looking up date of 'Independence Day': " << usHolidays["Independence Day"] << endl;;
      usHolidays["Independence Day"] = "4th of July";
      cout << "Looking up date of 'Independence Day': " << usHolidays["Independence Day"] << endl;;


      // find returns an iterator to the found element.  That iterator can be incremented to walk through the list
      cout << "Finding the holiday that comes after (alphabetically) Memorial Day: " << endl;
      it = usHolidays.find("Memorial Day");
      it++;
      holiday afterMemDay = (holiday)*it;
      cout << " The holiday that comes after (alphabetically) Memorial Day: " << afterMemDay.first << endl;
      cout << "      It's date is: " << afterMemDay.second << endl;

      // I can continue to walk through the rest of the list
      cout << " Continuing through the rest of the list" << endl;
      while (it != usHolidays.end())
      {
         holiday hol = (holiday)*it;
         cout << "   " << hol.first << ":  " << hol.second << endl;
         it++;
      }

      // I can also go to the element BEFORE Memorial Day
      cout << "Finding the holiday that comes before (alphabetically) Memorial Day: " << endl;
      it = usHolidays.find("Memorial Day");
      it--;
      holiday beforeMemDay = (holiday)*it;
      cout << " The holiday that comes before (alphabetically) Memorial Day: " << beforeMemDay.first << endl;
      cout << "      It's date is: " << beforeMemDay.second << endl;

      // now try to find a non-existent entry using the find method
      it = usHolidays.find("5th of July");
      if (it != usHolidays.end())
      {
         holiday hol = (holiday)*it;
         cout << " Found a value: " << hol.first << ":  " << hol.second << endl;
      }
      else
      {
         cout << " Didn't find value for 5th of July." << endl;
      }


      printMap("Printing the entire list again, before we clear it: Is '5th of July' in it? ");

      // now clear the list
      usHolidays.clear();

      printMap("Printing the entire list again, after we clear it: ");
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
