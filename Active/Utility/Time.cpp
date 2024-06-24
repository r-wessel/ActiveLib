/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/Time.h"

#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/Defer.h"

#include <mutex>

using namespace active::math;
using namespace active::utility;

namespace  {
	
	constexpr double secondsToMicroseconds = 1000000.0;
	
	static std::mutex m_mutex;
		
	/*--------------------------------------------------------------------
		Get the offset of the local time to UTC
	 
		return: The utc offset in minutes
	  --------------------------------------------------------------------*/
	int32_t getLocalUTCOffset() {
		m_mutex.lock();
		auto scope = defer([&]{ m_mutex.unlock(); });
		struct tm timeLocal = {};
		time_t localT = time(nullptr);
	#ifdef WINDOWS
		struct tm timeUTC;
		time_t gmT = time(NULL);
		localtime_s(&timeLocal, &localT);
		gmtime_s(&timeUTC , &gmT);
		auto local = Time{timeLocal.tm_year + 1900, std::chrono::month{static_cast<unsigned int>(timeLocal.tm_mon + 1)}, static_cast<uint8_t>(timeLocal.tm_mday), static_cast<uint8_t>(timeLocal.tm_hour), static_cast<uint8_t>(timeLocal.tm_min)},
				utc = Time{timeUTC.tm_year + 1900, std::chrono::month{static_cast<unsigned int>(timeUTC.tm_mon + 1)}, static_cast<uint8_t>(timeUTC.tm_mday), static_cast<uint8_t>(timeUTC.tm_hour), static_cast<uint8_t>(timeUTC.tm_min)};
		return static_cast<int32_t>(utc.differenceInMinutes(local));
	#else
		localtime_r(&localT, &timeLocal);
		return static_cast<int32_t>(timeLocal.tm_gmtoff / 60);
	#endif
	} //getLocalUTCOffset

}  // namespace

#ifdef WINDOWS

template <typename TP>
std::time_t to_time_t(TP tp) {
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

#endif

/*--------------------------------------------------------------------
	Constructor
 
	isUTCTime: Construct with UTC time (false = set local time with UTC offset)
  --------------------------------------------------------------------*/
Time::Time(bool isUTCTime) {
	setToCurrent(isUTCTime);
} //Time::Time


/*--------------------------------------------------------------------
	Constructor
 
	fileTime: A file system time point
  --------------------------------------------------------------------*/
Time::Time(std::filesystem::file_time_type fileTime) :
#ifdef WINDOWS
	Time(to_time_t(fileTime))
#else
	Time(std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::file_clock::to_sys(fileTime))))
#endif
{}


/*--------------------------------------------------------------------
	Constructor
 
	fileTime: A file system time point
  --------------------------------------------------------------------*/
Time::Time(time_t time) {
#ifdef WINDOWS
	struct tm utcTime;
	gmtime_s(&utcTime , &time);
#else
	tm utcTime = *gmtime(&time);
#endif
	m_year = utcTime.tm_year + 1900;
	m_month = utcTime.tm_mon + 1;
	m_day = utcTime.tm_mday;
	m_hour = utcTime.tm_hour;
	m_minute = utcTime.tm_min;
	m_second = utcTime.tm_sec;
	m_microsecond = 0;
#ifdef WINDOWS
	m_utcOffset = getLocalUTCOffset();
#else
	m_utcOffset = utcTime.tm_gmtoff;
#endif
}

/*--------------------------------------------------------------------
	Constructor (NB: Will throw exception if an invalid date is specified)

	year: The year
	month: The month
	day: The day
	hour: The hour
	minute: The minute
	second: The second
	utcOffset: The UTC offset
  --------------------------------------------------------------------*/
Time::Time(int32_t year, std::chrono::month month, uint8_t day, uint8_t hour, uint8_t minute, double second, int16_t utcOffset) {
	m_year = year;
	m_month = static_cast<uint8_t>(std::min(std::max(minMonth, static_cast<uint8_t>(month.operator unsigned())), maxMonth));
	m_day = static_cast<uint8_t>(std::min(std::max(minDay, day), maxDay));
	setHour(hour);
	setMinute(minute);
	setSecond(static_cast<uint8_t>(second));
	m_microsecond = static_cast<uint32_t>(fmod(second, 1.0) * secondsToMicroseconds);
	m_utcOffset = utcOffset;
	testValidity();
} //Time::Time


/*--------------------------------------------------------------------
	Assignment operator
 
	source: The object to assign
 
	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::operator=(const std::chrono::system_clock::time_point& source) {
	using namespace std::chrono;
	auto dayPeriod = floor<days>(source);
	year_month_day date{dayPeriod};
	hh_mm_ss time{floor<microseconds>(source - dayPeriod)};
	m_year = date.year().operator int();
	m_month = date.month().operator unsigned int();
	m_day = date.day().operator unsigned int();
	m_hour = time.hours().count();
	m_minute = time.minutes().count();
	m_second = static_cast<uint8_t>(time.seconds().count());
	m_microsecond = static_cast<uint32_t>(time.subseconds().count());
	m_utcOffset = 0;
	return *this;
} //Time::operator=


/*--------------------------------------------------------------------
	Addition with assignment operator
 
	days: The number of days to add
 
	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::operator+=(int32_t daystoAdd) {
	using namespace std::chrono;
	year_month_day date{std::chrono::year{static_cast<int>(m_year)}/std::chrono::month{m_month}/std::chrono::day{m_day}};
	auto result = year_month_day{sys_days(date) + days{daystoAdd}};
	m_year = result.year().operator int();
	m_month = date.month().operator unsigned int();
	m_day = date.day().operator unsigned int();
	return *this;
} //Time::operator+=


/*--------------------------------------------------------------------
	Conversion operator
 
	return: An equivalent system clock time_point
  --------------------------------------------------------------------*/
Time::operator std::chrono::system_clock::time_point() const {
	return makeTimePoint();
} //Time::operator std::chrono::system_clock::time_point


/*--------------------------------------------------------------------
	Three-way comparison to another time
 
	ref: The object to compare
	epsMicrosec: Precision for comparing the microsecond values (nullopt = ignore microsecond values)
 
	return: True if this is equal to ref
  --------------------------------------------------------------------*/
std::strong_ordering Time::compare(const Time& ref, std::optional<double> epsMicrosec) const {
	auto thisPeriod = makeTimePoint(epsMicrosec), refPeriod = ref.makeTimePoint(epsMicrosec);
		//NB: The 3-way comparison is not implemented in all compilers yet - will enable the following line in future (and delete the rest)
	//return thisPeriod <=> refPeriod;
	if (thisPeriod < refPeriod)
		return std::strong_ordering::less;
	if (thisPeriod > refPeriod)
		return std::strong_ordering::greater;
	return std::strong_ordering::equivalent;
} //Time::compare


/*--------------------------------------------------------------------
	Get the UTC offset
 
	return: A pair specifying offset hours and offset minutes
  --------------------------------------------------------------------*/
std::pair<int16_t, int16_t> Time::getUTCOffset() const {
	return std::make_pair(static_cast<int16_t>(m_utcOffset / 60), static_cast<int16_t>(m_utcOffset % 60));
} //Time::getUTCOffset


/*--------------------------------------------------------------------
	Get the number of seconds elapsed since 00:00 hours, Jan 1, 1970 (Unix epoch)
 
	return: The number of seconds
  --------------------------------------------------------------------*/
double Time::secondsSince1970() const {
	using namespace std::chrono;
	if (m_year < 1970)
		return 0.0;
	system_clock::time_point timePoint{*this};
	auto result = timePoint.time_since_epoch();
	return static_cast<double>(result.count()) / secondsToMicroseconds;
} //Time::secondsSince1970


/*--------------------------------------------------------------------
	Set to the current time
 
	isUTCTime: Construct with UTC time (false = set local time with UTC offset)
	
	return A reference to this
  --------------------------------------------------------------------*/
Time& Time::setToCurrent(bool isUTCTime) {
	using namespace std::chrono;
	*this = system_clock::now();
	if (!isUTCTime) {
		auto offset = getLocalUTCOffset();
		if (offset != 0) {
			system_clock::time_point time{*this};
			time += minutes{offset};
			*this = time;
			m_utcOffset = offset;
		}
	}
	return *this;
} //Time::setToCurrent


/*--------------------------------------------------------------------
	Make the time a date only (reset all time values)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::resetTime() {
	setHour(0);
	setMinute(0);
	setSecond(0);
	setMicrosecond(0);
	setUTCOffset(0);
	return *this;
} //Time::resetTime

/*--------------------------------------------------------------------
	Make the time a time only (reset all date values)

	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::resetDate() {
	m_year = 0;
	m_month = static_cast<uint8_t>(std::chrono::January.operator unsigned int());
	m_day = minDay;
	return *this;
} //Time::resetDate


/*--------------------------------------------------------------------
	Add a specified number of days to the time
 
	daystoAdd: The number of days to add (can be negative)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::addDays(int16_t daystoAdd) {
	using namespace std::chrono;
	year_month_day date{std::chrono::year{static_cast<int>(m_year)}/std::chrono::month{m_month}/std::chrono::day{m_day}};
	auto result = year_month_day{sys_days(date) + days{daystoAdd}};
	m_year = result.year().operator int();
	m_month = date.month().operator unsigned int();
	m_day = date.day().operator unsigned int();
	return *this;
} //Time::addDays


/*--------------------------------------------------------------------
	Add a specified number of hours to the time
 
 hoursToAdd: The number of hours to add (can be negative)
 
	return: A reference to this
 --------------------------------------------------------------------*/
Time& Time::addHours(int16_t hoursToAdd) {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this};
	thisTime += hours{hoursToAdd};
	*this = thisTime;
	return *this;
} //Time::addHours


/*--------------------------------------------------------------------
	Add a specified number of minutes to the time
 
	minsToAdd: The number of minutes to add (can be negative)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::addMinutes(int16_t minsToAdd) {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this};
	thisTime += minutes{minsToAdd};
	*this = thisTime;
	return *this;
} //Time::addMinutes


/*--------------------------------------------------------------------
	Add a specified number of seconds to the time
 
	secsToAdd: The number of seconds to add (can be negative, any fractional part is assigned to milliseconds)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Time& Time::addSeconds(double secsToAdd) {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this};
	int64_t wholeSecs = static_cast<int64_t>(roundDown(secsToAdd));
	m_microsecond += static_cast<uint32_t>(secondsToMicroseconds * (secsToAdd - static_cast<double>(wholeSecs)));
	if (m_microsecond < 0) {
		wholeSecs -= 1;
		m_microsecond += static_cast<uint32_t>(secondsToMicroseconds);
	} else if (m_microsecond >= static_cast<uint32_t>(secondsToMicroseconds)) {
		wholeSecs += 1;
		m_microsecond -= static_cast<uint32_t>(secondsToMicroseconds);
	}
	thisTime += seconds{wholeSecs};
	*this = thisTime;
	return *this;
} //Time::addSeconds


/*--------------------------------------------------------------------
	Set total seconds of this date
 
	totalSeconds: The total seconds since 00:00 hours, Jan 1, 1970 UTC
  --------------------------------------------------------------------*/
void Time::setSecondsSince1970(double totalSeconds) {
	using namespace std::chrono;
	system_clock::time_point timePoint{};
	timePoint += microseconds{static_cast<uint64_t>(totalSeconds * secondsToMicroseconds)};
	*this = timePoint;
} //Time::setSecondsSince1970


/*--------------------------------------------------------------------
	Get the difference in seconds between the time in this object and the other input
	
	other: The other time to get the difference to
	
	return: The difference in seconds
  --------------------------------------------------------------------*/
double Time::differenceInSeconds(const Time& other) const {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this}, refTime{other};
	auto microSeconds = std::chrono::duration_cast<std::chrono::microseconds>(refTime - thisTime);
	return static_cast<double>(microSeconds.count()) / secondsToMicroseconds;
} //Time::differenceInSeconds


/*--------------------------------------------------------------------
	 Get the difference in minutes between the time in this object and the other input
	 
	 other: The other time to get the difference to
	 
	 return: The difference in minutes
  --------------------------------------------------------------------*/
int64_t Time::differenceInMinutes(const Time& other) const {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this}, refTime{other};
	return std::chrono::duration_cast<std::chrono::minutes>(refTime - thisTime).count();
} //Time::differenceInMinutes


/*--------------------------------------------------------------------
	 Get the difference in hours between the time in this object and the other input
	 
	 other: The other time to get the difference to
	 
	 return: The difference in hours
  --------------------------------------------------------------------*/
int64_t Time::differenceInHours(const Time& other) const {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this}, refTime{other};
	return std::chrono::duration_cast<std::chrono::hours>(refTime - thisTime).count();
} //Time::differenceInHours


/*--------------------------------------------------------------------
	 Get the difference in days between the time in this object and the other input (assuming this object is earlier than the input)
	 
	 other: The other time to get the difference to
	 
	 return: The difference in seconds
  --------------------------------------------------------------------*/
int64_t Time::differenceInDays(const Time& other) const {
	using namespace std::chrono;
	system_clock::time_point thisTime{*this}, refTime{other};
	return std::chrono::duration_cast<std::chrono::days>(refTime - thisTime).count();
} //Time::differenceInDays


/*--------------------------------------------------------------------
	Make a `system_clock::time_point` from this
 
	prec: Precision of the seconds fraction (from microseconds, e.g. 1.0 = round to nearest second. nullopt = no rounding)
  --------------------------------------------------------------------*/
std::chrono::system_clock::time_point Time::makeTimePoint(std::optional<double> prec) const {
	using namespace std::chrono;
	system_clock::time_point timePoint{sys_days{std::chrono::year{static_cast<int>(m_year)}/std::chrono::month{m_month}/std::chrono::day{m_day}}};
	auto microsecs = m_microsecond;
	if (prec)
		microsecs = static_cast<uint32_t>(secondsToMicroseconds * round(static_cast<double>(microsecs) / secondsToMicroseconds, *prec));
	timePoint += hours{m_hour} + minutes{m_minute} + seconds{m_second} + microseconds{microsecs};
	timePoint -= hours{m_utcOffset};
	return timePoint;
} //Time::makeTimePoint


/*--------------------------------------------------------------------
	Test the time validity and throw exception on failure
  --------------------------------------------------------------------*/
void Time::testValidity() {
	if (!std::chrono::year_month_day{std::chrono::year{static_cast<int>(m_year)}/std::chrono::month{m_month}/std::chrono::day{m_day}}.ok())
		throw std::out_of_range("");	//Illegal time values
} //Time::testValidity
