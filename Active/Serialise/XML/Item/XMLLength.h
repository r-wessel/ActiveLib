/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_LENGTH
#define ACTIVE_SERIALISE_XML_XML_LENGTH

#include "Active/Serialise/XML/Item/XMLNumber.h"

namespace active::serialise::xml {
	
	/*!
		Class for XML serialisation of lengths (3 decimal places, i.e. nearest mm)
	*/
	class XMLLength : public XMLNumber {
	public:
		/*!
			Constructor
			@param val The data value
		*/
		XMLLength(double& val) : XMLNumber(val) {}

		/*!
			Get the require floating point precision
			@return The require floating point precision
		*/
		virtual double getPrecision() const { return math::eps; }
	};
	
}

#endif	//ACTIVE_SERIALISE_XML_XML_LENGTH
