/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_NUMBER
#define ACTIVE_SERIALISE_XML_XML_NUMBER

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

namespace active::serialise::xml {
	
	/*!
		Class for XML serialisation of numbers
	*/
	class XMLNumber : public ValueWrap<double> {
	public:
		
		// MARK: - Types
		
		typedef ValueWrap<double> base;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param val The data value
		*/
		XMLNumber(double& val) : base(val) {}
		/*!
			Destructor
		*/
		virtual ~XMLNumber() = default;
		
		// MARK: - Functions (const)
		
		/*!
			Get the require floating point precision
			@return The require floating point precision
		*/
		virtual double getPrecision() const = 0;
		/*!
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		virtual bool write(utility::String& dest) const { dest = utility::String{get(), getPrecision()}; return true; }
	};
	
}

#endif	//ACTIVE_SERIALISE_XML_XML_NUMBER
