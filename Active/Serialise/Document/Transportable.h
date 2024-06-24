/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_DOC_TRANSPORTABLE
#define ACTIVE_SERIALISE_DOC_TRANSPORTABLE

#include "Active/Utility/String.h"

#include <concepts>

namespace active::setting {

	class SettingList;
	
}

namespace active::serialise::doc {

	class Object;

		///Concept for classes that can be serialised into a document
	template<typename T>
	concept Serialisable = requires(const T& t, const setting::SettingList* spec) {
		{ t.send(spec) } -> std::same_as<Object>;
	};
	
		///Concept for classes that can reconstruct an instance from a serialised document object
	template<typename T>
	concept Reconstructable = std::constructible_from<T, const Object&, const setting::SettingList*>;
	
		///Concept for classes that can be transported in a serialised document
	template<typename T>
	concept Transportable = Serialisable<T> && Reconstructable<T>;

		///Concept for classes that specify a document type name to reconstruct objects from serialised data
	template <typename T>
	concept Typed = requires(T t) {
		{ t.docType } -> std::same_as<active::utility::String&>;
	};
	
}

#endif	//ACTIVE_SERIALISE_DOC_TRANSPORTABLE
