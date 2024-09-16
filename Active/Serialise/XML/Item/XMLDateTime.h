/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_DATE_TIME
#define ACTIVE_SERIALISE_XML_XML_DATE_TIME

#include "Active/Serialise/Item/Item.h"
#include "Active/Utility/Time.h"

namespace active::serialise::xml {

		///Class for transporting a data/time via XML
	class XMLDateTime : public Item {
	public:
		
		// MARK: - Types
		
			///Preferred JSON date/time format
		enum class Format {
			iso8601,			///< ISO 8601
			secondsSince1970,	///< Unix (posix) epoch
		};
		
			///The date/time content, i.e. just a date, just a time, date & time etc. Ignored for secondsSince1970 format
		enum class Content {
			dateTime,
			dateOnly,
			timeOnly,
			dateTimeWithOffset,
			timeOnlyWithOffset,
		};
		
		// MARK: - Constructors
		
		/*!
			Constructor
		 	@param time A time
			@param content The date/time content
			@param prec The seconds precision (e.g. 1e-6 for microsecond precision, 1.0 for whole seconds only)
		*/
		XMLDateTime(utility::Time& time, Content content = Content::dateTimeWithOffset, double prec = 1e-6);
		/*!
			Constructor
		 	@param time A time
			@param content The date/time content
			@param prec The seconds precision (e.g. 1e-6 for microsecond precision, 1.0 for whole seconds only)
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		XMLDateTime(const utility::Time& time, Content content = Content::dateTimeWithOffset, double prec = 1e-6) :
				XMLDateTime(const_cast<utility::Time&>(time), content, prec) {}
		
		// MARK: - Functions (const)
		
		/*!
			Get the time
			@return The time
		*/
		virtual const utility::Time& getTime() const { return m_time; }
		/*!
			Get the precision for seconds (e.g. 1e-6 for microsecond precision, 1.0 for whole seconds only)
			@return The seconds precision
		*/
		double getSecondsPrecision() const { return m_secsPrecision; }
		/*!
			Get the date/time format
			@return The date/time format
		*/
		virtual Format getFormat() const { return m_format; }
		/*!
			Get the content of the date/time
			@return The content of the date/time
		*/
		virtual Content getContent() const { return m_content; }
		/*!
			Determine if a time has been read
			@return True if a time has been read
		*/
		virtual bool isRead() const { return m_read; }
		/*!
			Write the item data to a string
			@param dest The string to write the const data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override;
		/*!
			Get the serialisation type for the item value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Type> type() const override { return (m_format == Format::iso8601) ? text : number; }
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the precision for seconds (e.g. 1e-6 for microsecond precision, 1.0 for whole seconds only)
			@param prec The seconds precision
		*/
		void setSecondsPrecision(double prec) { m_secsPrecision = prec; }
		/*!
			Set the date/time format
			@param format The date/time format
		*/
		virtual void setFormat(Format format) { m_format = format; }
		/*!
			Set the content of the date/time
			@param content The content of the date/time
		*/
		virtual void setContent(Content content) { m_content = content; }
		/*!
			Set to the default package content
		*/
		void setDefault() override;
		/*!
			Validate the transported data
			@return True if the data is valid
		*/
		bool validate() override	{ return m_valid; }
		/*!
			Read the item data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override;
		
	private:
		utility::Time& m_time;
		utility::Time m_buffer;
		double m_secsPrecision = 1e-6;
		Format m_format = Format::iso8601;
		Content m_content = Content::dateTime;
		bool m_read;
		bool m_valid;
	};
	
}

#endif	//ACTIVE_SERIALISE_XML_XML_DATE_TIME
