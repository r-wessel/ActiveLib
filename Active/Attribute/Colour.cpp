/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Attribute/Colour.h"

#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/SHA256.h"

using namespace active;
using namespace active::attribute;
using namespace active::serialise;
using namespace active;

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
Colour::Colour(const string& hex) {
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
 
	is_alpha: True to include the alpha value
 
	return: The colour as hex
  --------------------------------------------------------------------*/
string Colour::hex(bool is_alpha) const {
	Memory colourOut;
	{
		BufferOut buffer{colourOut};
		buffer << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
		if (is_alpha)
			buffer << static_cast<char>(a * 255);
	}
	string result;
	HexTransport().send(BufferIn{colourOut}, BufferOut{result});
	return result;
} //Colour::hex


/*--------------------------------------------------------------------
	Get a hash value for the colour
 
	format: The required has format
 
	return: The hash formatted as specified
  --------------------------------------------------------------------*/
string Colour::hash(HashFormat format) const {
	SHA256 hasher;
	hasher << r << g << b << a;
	return hasher.product(format);
} //Colour::hash


/*--------------------------------------------------------------------
	Copy a specified colour
 
	source: The colour to copy
	isAlphaCopied: True if the alpha channel is also copied
 
	return: A reference to this
  --------------------------------------------------------------------*/
Colour& Colour::copy(const Colour& source, bool isAlphaCopied) {
	r = source.r;
	g = source.g;
	b = source.b;
	if (isAlphaCopied)
		a = source.a;
	return *this;
} //Colour::copy
