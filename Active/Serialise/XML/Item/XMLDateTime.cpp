/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/XML/Item/XMLDateTime.h"

#include <ctime>

using namespace active::math;
using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::utility;

using enum XMLDateTime::Content;
using enum Item::TimeFormat;

#include <regex>

namespace {
	
		//Regular expressions for parsing xs:dateTime etc
	const String dateRegex{"(-?[0-9]{4})-([0-9]{2})-([0-9]{2})"},
				timeRegex{"([0-9]{2}):([0-9]{2}):([0-9]{2}(?:\\.[0-9]+)?)"},
				zoneRegex{"(Z|[+-]{1}[0-9]{2}:[0-9]{2})?"},
				dateTimeRegex = dateRegex + "T" + timeRegex;

} // namespace

/*--------------------------------------------------------------------
	Constructor
 
	time: A time
	content: The date/time content
	prec: The seconds precision (e.g. 1e-6 for microsecond precision, 1.0 for whole seconds only)
  --------------------------------------------------------------------*/
XMLDateTime::XMLDateTime(Time& time, Content content, double prec) : Item(), m_time(time) {
	m_secsPrecision = prec;
	m_content = content;
	m_valid = true;
	m_read = false;
} //XMLDateTime::XMLDateTime


/*--------------------------------------------------------------------
	Export the object to the specified string
	
	dest: The string to write the data to
	
	return: True if the data was successfully written
  --------------------------------------------------------------------*/
bool XMLDateTime::write(String& dest) const {
		//If the preferred format is the Unix epoch, write the seconds since 1970
	if (m_format == secondsSince1970) {
		dest += String{m_time.secondsSince1970()};
		return true;
	}
		//Write the date as required
	if ((m_content != timeOnly) && (m_content != timeOnlyWithOffset)) {
			//Year
		dest += String{static_cast<int32_t>(m_time.year())}.padRight(4, "0");
			//Month
		dest += "-";
		dest += String{static_cast<int32_t>(m_time.month().operator unsigned())}.padRight(2, "0");
			//Day
		dest += "-";
		dest += String{static_cast<int32_t>(m_time.day())}.padRight(2, "0");
	}
		//Write the time as required
	if (m_content != dateOnly) {
		if (m_content == dateTime || m_content == dateTimeWithOffset)
			dest += "T";
			//Hour
		dest += String{static_cast<int32_t>(m_time.hour())}.padRight(2, "0");
			//Minute
		dest += ":";
		dest += String{static_cast<int32_t>(m_time.minute())}.padRight(2, "0");
			//Second
		dest += ":";
		dest += String{static_cast<int32_t>(m_time.second())}.padRight(2, "0");
		if (isBetween(m_secsPrecision, 0.0, 1.0, 1e-7) && (m_time.microsecond() != 0)) {
			auto secs = String{static_cast<double>(m_time.microsecond()) / 1e6, m_secsPrecision};
			if (auto pointPos = secs.find("."); pointPos)
				dest += secs.substr(*pointPos, 7);
		}
	}
		//Write a UTC offset as required
	if (m_content == dateTimeWithOffset || m_content == timeOnlyWithOffset) {
		auto offset = m_time.getUTCOffset();
		if ((offset.first == 0) && (offset.second == 0))
			dest += "Z";
		else {
			dest += offset.second < 0 ? "-" : "+";
			dest += String{abs(offset.first)}.padRight(2, "0");
			dest += ":";
			dest += String{offset.second}.padRight(2, "0");
		}
	}
	return true;
} //XMLDateTime::write


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void XMLDateTime::setDefault() {
	m_valid = true;
	m_read = false;
	m_time.setToCurrent();
	if (m_content == dateOnly)
		m_time.resetTime();
	else if (m_content == timeOnly)
		m_time.resetDate();
} //XMLDateTime::setDefault


/*--------------------------------------------------------------------
	Import the object from the specified string
	
	source: The string to read
	
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool XMLDateTime::read(const String& source) {
	bool hasDate = ((m_content != timeOnly) && (m_content != timeOnlyWithOffset)),
			hasTime = (m_content != dateOnly);
	std::string subject(source);
	std::smatch match;
	std::regex pattern{(((hasDate) ? ((hasTime) ? dateTimeRegex : dateRegex) : timeRegex) + zoneRegex).data()};
	if (!std::regex_search(subject, match, pattern) || (match.size() < 2)) {
		if (source.findFirstNotOf(String::allFloat))
			return false;
		if (auto seconds = source.toDouble(); seconds) {
			m_time = Time{*seconds};
			return true;
		}
		return false;
	}
	int32_t year = 0;
	uint8_t month = 0, day = 0, hour = 0, minute = 0;
	double second = 0.0;
	auto incoming = ++match.begin();
	if (hasDate) {
			//Year
		if (auto val = String{*(incoming++)}.toInt16(); val && (val != 0))
			year = *val;
		else
			return false;
			//Month
		if (auto val = String{*(incoming++)}.toInt16(); val && (val > 0) && (val < 13))
			month = static_cast<uint8_t>(*val);
		else
			return false;
			//Day
		if (auto val = String{*(incoming++)}.toInt16(); val && (val > 0) && (val < 32))
			day = static_cast<uint8_t>(*val);
		else
			return false;
	}
	if (hasTime) {
			//Hours
		if (auto val = String{*(incoming++)}.toInt16(); val && (val >= 0) && (val < 24))
			hour = static_cast<uint8_t>(*val);
		else
			return false;
			//Minutes
		if (auto val = String{*(incoming++)}.toInt16(); val && (val >= 0) && (val < 60))
			minute = static_cast<uint8_t>(*val);
		else
			return false;
			//Seconds (NB: Double to allow for microseconds)
		if (auto val = String{*(incoming++)}.toDouble(); val && (val >= 0.0) && (val < 60.0))
			second = *val;
		else
			return false;
	}
		//UTC offset
	int32_t utcOffset = 0;
	if (incoming != match.end() && incoming->matched) {
		if (String zone{*incoming}; !zone.empty() && (zone != "Z")) {
			int32_t sign = (zone[0] == U'-') ? -1 : 1;
			if (auto val = zone.substr(1, 2).toInt16(); val && ((*val * sign) > -13) && ((*val * sign) < 15))
				utcOffset += (*val * sign * 60);
			else
				return false;
			if (auto val = zone.substr(4, 2).toInt16(); val && (val >= 0) && (val < 60))
				utcOffset += *val;
			else
				return false;
		}
	}
	m_time = Time{year, std::chrono::month{month}, day, hour, minute, second};
	m_time.setUTCOffset(utcOffset);
		//If we get this far, a valid date/time has been read
	m_valid = true;
	return m_valid;
} //XMLDateTime::read
