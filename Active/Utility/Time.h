/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_TIME
#define ACTIVE_UTILITY_TIME

#include <chrono>
#include <compare>
#include <filesystem>
#include <optional>

namespace active::utility {
	
	/// A class to represent a date/time
	class Time {
	public:
		
		// MARK: Constants
		
			///Minimum acceptable day value
		inline static constexpr uint8_t minDay = 1;
			///Maximum acceptable day value
		inline static constexpr uint8_t maxDay = 31;
			///Minimum acceptable month value
		inline static constexpr uint8_t minMonth = 1;
			///Maximum acceptable month value
		inline static constexpr uint8_t maxMonth = 12;
			///Maximum acceptable hour value
		inline static constexpr uint8_t maxHour = 23;
			///Maximum acceptable minute value
		inline static constexpr uint8_t maxMinute = 59;
			///Maximum acceptable second value
		inline static constexpr uint8_t maxSecond = 59;
		
		// MARK: - Types
		
			///Optional
		using Option = std::optional<Time>;

		// MARK: - Constructors
		
		/*!
			Constructor
			@param isUTCTime Construct with UTC time (false = set local time with UTC offset)
		*/
		Time(bool isUTCTime = true);
		/*!
			Constructor
		 	@param fileTime A file system time point
		*/
		Time(std::filesystem::file_time_type fileTime);
		/*!
			Constructor
		 	@param time Unix time
		*/
		Time(time_t time);
#if !defined(WINDOWS) && !defined(__linux__)
		/*!
			Constructor
		 	@param seconds Unix time
		*/
		Time(int64_t seconds) : Time{static_cast<time_t>(seconds)} {}
#endif //WINDOWS
		/*!
			Constructor
		 	@param seconds Unix time
		*/
		Time(double seconds) : Time{static_cast<time_t>(seconds)} {
			m_microsecond = static_cast<int32_t>(1e6 * fmod(seconds, 1.0));
		}
		/*!
			Constructor (NB: Will throw exception if an invalid date is specified)
			@param year The year
			@param month The month
			@param day The day
			@param hour The hour
			@param minute The minute
			@param second The second
			@param utcOffset The UTC offset
		*/
		Time(int32_t year, std::chrono::month month, uint8_t day, uint8_t hour = 0, uint8_t minute = 0, double second = 0, int16_t utcOffset = 0);
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to assign
			@return A reference to this
		*/
		Time& operator=(const std::chrono::system_clock::time_point& source);
		/*!
			Three-way comparison operator
			@param ref The object to compare
			@return The relationship between this and ref (less, equal, greater)
		*/
		std::strong_ordering operator<=>(const Time& ref) const { return compare(ref); }
		/*!
			Equality operator
			@param ref The object to compare
			@return True if this matches ref (NB: microseconds are included in this comparison - use `isEqual` to control or ignore microseconds)
		*/
		bool operator==(const Time& ref) const { return compare(ref) == std::strong_ordering::equal; }
		/*!
			Equality operator
			@param ref The object to compare
			@return True if this matches ref
		*/
		bool operator!=(const Time& ref) const { return !(*this == ref); }
		/*!
			Addition with assignment operator
			@param days The number of days to add
			@return A reference to this
		*/
		Time& operator+=(int32_t days);
		/*!
			Conversion operator
			@return An equivalent system clock time_point
		*/
		operator std::chrono::system_clock::time_point() const;
		
		// MARK: - Functions (const)
		
		/*!
			Three-way comparison to another time
			@param ref The object to compare
			@param epsMicrosec Precision for comparing the microsecond values (nullopt = use full precision)
			@return The relationship between this and ref (less, equal, greater)
		*/
		std::strong_ordering compare(const Time& ref, std::optional<double> epsMicrosec = std::nullopt) const;
		/*!
			Get the year (0 - 32768, e.g. 2011)
			@return The year
		*/
		int64_t year() const { return m_year; }
		/*!
			Get the month (1 - 12)
			@return The month
		*/
		std::chrono::month month() const { return std::chrono::month{m_month}; }
		/*!
			Get the day (1-31)
			@return The day
		*/
		uint8_t day() const { return m_day; }
		/*!
			Get the hour (0-23)
			@return The hour
		*/
		uint8_t hour() const { return m_hour; }
		/*!
			Get the minute (0-59)
			@return The minute
		*/
		uint8_t minute() const { return m_minute; }
		/*!
			Get the second (0-59)
			@return The second
		*/
		uint8_t second() const { return m_second; }
		/*!
			Get the microsecond (0-999999)
			@return The microsecond
		*/
		uint32_t microsecond() const { return m_microsecond; }
		/*!
			Get the UTC offset
		 	@return A pair specifying offset hours and offset minutes
		*/
		std::pair<int16_t, int16_t> getUTCOffset() const;
		/*!
			Get the number of seconds elapsed since 00:00 hours, Jan 1, 1970 (Unix epoch)
		 	@return The number of seconds
		*/
		double secondsSince1970() const;
		/*!
			Get the difference in seconds between the time in this object and the other input
			@param other The other time to get the difference to
			@return The difference in seconds
		*/
		double differenceInSeconds(const Time& other) const;
		/*!
			Get the difference in minutes between the time in this object and the other input
			@param other The other time to get the difference to
			@return The difference in minutes
		*/
		int64_t differenceInMinutes(const Time& other) const;
		/*!
			Get the difference in hours between the time in this object and the other input
			@param other The other time to get the difference to
			@return The difference in hours
		*/
		int64_t differenceInHours(const Time& other) const;
		/*!
			Get the difference in days between the time in this object and the other input
			@param other The other time to get the difference to
			@return The difference in days
		*/
		int64_t differenceInDays(const Time& other) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Set to the current time
		 	@param isUTCTime Construct with UTC time (false = set local time with UTC offset)
		 	return: A reference to this
		*/
		Time& setToCurrent(bool isUTCTime = true);
		/*!
			Reset all the time values to zero (leaving the calendar date untouched)
		 	return: A reference to this
		*/
		Time& resetTime();
		/*!
			Reset all the calendar date values to zero/start value (leaving the time untouched)
		 	return: A reference to this
		*/
		Time& resetDate();
		/*!
			Add a specified number of days to the time
			@param daystoAdd The number of days to add (can be negative)
			@return A reference to this
		*/
		Time& addDays(int16_t daystoAdd);
		/*!
			Add a specified number of hours to the time
			@param hourstoAdd The number of hours to add (can be negative)
			@return A reference to this
		*/
		Time& addHours(int16_t hourstoAdd);
		/*!
			Add a specified number of minutes to the time
			@param minstoAdd The number of minutes to add (can be negative)
			@return A reference to this
		*/
		Time& addMinutes(int16_t minstoAdd);
		/*!
			Add a specified number of seconds to the time
			@param secsToAdd The number of seconds to add (can be negative, any fractional part is assigned to milliseconds)
			@return A reference to this
		*/
		Time& addSeconds(double secsToAdd);
		/*!
			Set the hour (0-23)
		*/
		void setHour(uint8_t val) { m_hour = static_cast<uint8_t>(std::min(val, maxHour)); }
		/*!
			Set the minute (0-59)
		*/
		void setMinute(uint8_t val) { m_minute = static_cast<uint8_t>(std::min(val, maxMinute));}
		/*!
			Set the second (0-59)
		*/
		void setSecond(uint8_t val) { m_second = static_cast<uint8_t>(std::min(val, maxSecond)); }
		/*!
			Set the millisecond (0-999999)
		*/
		void setMicrosecond(uint32_t val) { m_microsecond = val; }
		/*!
			Set the UTC offset (value can be -ve or +ve offset from UTC time)
		 	@param offset The UTC offset in minutes (NB: all other date/time values remain unchanged, so the in effect this changes the time-point)
		*/
		void setUTCOffset(int16_t offset) { m_utcOffset = offset; }
		/*!
			Set the seconds since 00:00 hours, Jan 1, 1970 UTC
		 	@param totalSeconds The total seconds since 00:00 hours, Jan 1, 1970 UTC
		*/
		void setSecondsSince1970(double totalSeconds);
		
	private:
		/*!
			Make a `system_clock::time_point` from this
			@param prec Precision of the seconds fraction (from microseconds, e.g. 1.0 = round to nearest second. nullopt = no rounding)
		*/
		std::chrono::system_clock::time_point makeTimePoint(std::optional<double> prec = std::nullopt) const;
		/*!
			Test the time validity and throw exception on failure
		*/
		void testValidity();
		
		// MARK: - Variables
		
		int64_t m_utcOffset : 12 = 0;
		int64_t m_year : 52 = 0;
		uint8_t m_month = minMonth;
		uint8_t m_day = minDay;
		uint8_t m_hour = 0;
		uint8_t m_minute = 0;
		uint32_t m_second : 8 = 0;
		uint32_t m_microsecond : 24 = 0;
	};

}  // namespace active::utility

#endif	//ACTIVE_UTILITY_TIME

