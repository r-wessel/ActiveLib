/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Attribute/Finish.h"

#include "Active/Utility/SHA256.h"

using namespace active;
using namespace active::attribute;
using namespace active::utility;

/*--------------------------------------------------------------------
	Get a hash value for the finish
 
	format: The required has format
 
	return: The hash formatted as specified
  --------------------------------------------------------------------*/
String Finish::hash(HashFormat format) const {
	if (!id.empty())
		return id;
	SHA256 hasher;
	hasher << name << colour.r << colour.g << colour.b << colour.a <<
			emissionColour.r << emissionColour.g << emissionColour.b << emissionColour.a <<
			specularColour.r << specularColour.g << specularColour.b << specularColour.a <<
			ambientReflection << diffuseReflection << metalness << roughness << specularReflection;
	id = hasher.product(format);
	return id;
} //Finish::hash
