/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Attribute/Colour.h"

#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"

using namespace active;
using namespace active::attribute;
using namespace active::serialise;
using namespace active::utility;

namespace {

	char8_t getComponent(uint32_t& source) {
		char8_t result = source & 0xFF;
		source >>= 8;
		return result;
	} //getComponent
	
}

/*--------------------------------------------------------------------
	Constructor
 
	hex: A colour in hex digits
  --------------------------------------------------------------------*/
Colour::Colour(const String& hex) {
	if (hex.length() < 2)
		return;
	if (auto colourCode = HexTransport().receive(hex); colourCode) {
		if (hex.length() > 6)
			a = static_cast<float>(getComponent(*colourCode)) / 255;
		if (hex.length() > 4)
			b = getComponent(*colourCode);
		if (hex.length() > 2)
			g = getComponent(*colourCode);
		r = getComponent(*colourCode);
	}
} //Colour::Colour


/*--------------------------------------------------------------------
	Get the colour in hex digits
 
	isAlpha: True to include the alpha value
 
	return: The colour as hex
  --------------------------------------------------------------------*/
String Colour::hex(bool isAlpha) const {
	Memory colourOut;
	{
		BufferOut buffer{colourOut};
		buffer << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
		if (isAlpha)
			buffer << static_cast<char>(a * 255);
	}
	String result;
	HexTransport().send(BufferIn{colourOut}, BufferOut{result});
	return result;
} //Colour::hex
