#include "blocxx/BLOCXX_config.h"
#include "TimeUtils.hpp"
#include "TimeDuration.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Time
	{
		bool isSpecial(const Time::TimeDuration& td)
		{
			return td.isSpecial();
		}

		bool isInfinite(const Time::TimeDuration& td)
		{
			return td.isInfinite();
		}

		bool isInvalid(const Time::TimeDuration& td)
		{
			return td.isInvalid();
		}


		bool isInvalid(const TimePeriod& p)
		{
			if( isInvalid(p.begin()) )
			{
				return true;
			}
			else if( isInvalid(p.end()) )
			{
				return true;
			}
			else if( isPosInfinity(p.begin()) )
			{
				return true;
			}
			return false;
		}

		bool isInfinite(const TimePeriod& p)
		{
			return timeBetween(p.begin(), p.end()).isInfinite();
		}

		DateTime infiniteTime()
		{
			return DateTime::getPosInfinity();
		}

		DateTime invalidTime()
		{
			return DateTime(E_TIME_NADT);
		}

		TimePeriod infiniteTimePeriod(const DateTime& start)
		{
			return TimePeriod(start, infiniteTime());
		}

		TimePeriod invalidTimePeriod()
		{
			return TimePeriod(invalidTime(), invalidTime());
		}
	}
}
