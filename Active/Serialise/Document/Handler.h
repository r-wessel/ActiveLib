/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_DOC_HANDLER
#define ACTIVE_SERIALISE_DOC_HANDLER

#include "Active/Serialise/Document/Transportable.h"

#include <typeinfo>
#include <functional>
#include <memory>

namespace active::serialise::doc {
	
	/*!
		Factory function to reconstruct an object instance based on an incoming serialised document object
		@param incoming The incoming document object
		@return A new instance reconstructed from the document object (nullptr on failure)
	*/
	template<typename T> requires Reconstructable<T>
	void* reconstructFunc(const Object& incoming) {
		try {
			return new T(incoming);
		} catch(...) {
			return nullptr;	//Object constructors should throw an exception if incoming data isn't viable (NB: only use for unrecoverable problems)
		}
	}
	
	/*!
		A class for shipping (sending/receiving) objects in a serialised document
	
		The primary role of this class is to reconstruct class instances from the serialised document objects. An instance of this class should
		be populated with the identifying tags and reconstruction functions for all required document object types
	*/
	class Handler {
	public:
		
		// MARK: Types
		
			///Shared pointer
		using Shared = std::shared_ptr<Handler>;
		
			//Factory function for reconstructing instances from a serialised document object
		using Reconstruction = std::function<void*(const Object& object)>;
		
		// MARK: Constructors
		
		/*!
			Default constructor
		*/
		Handler() {}
		
		// MARK: Functions (const)
		
		/*!
			Reconstruct a class instance from an incoming serialised document object
			@param incoming The incoming serialised document object
			@return The unwrapped object (nullptr on failure)
		*/
		template<typename T, class Obj> requires Reconstructable<T> && Typed<Obj>
		T* reconstruct(const Obj& incoming) const {
			if (auto maker = reconstruction.find(incoming.docType); (maker != reconstruction.end()) && (maker->second.first == &typeid(T)))
				return reinterpret_cast<T*>(maker->second.second(incoming));
			return nullptr;
		} //reconstruct
		
		// MARK: Functions (mutating)
		
		/*!
			Add a method of reconstructing objects of a specified type
			@param tags Tag(s) that may be used to identify an object of this type
		*/
		template<typename T> requires Transportable<T>
		void add(std::initializer_list<active::utility::String> const& tags) {
			for (const auto& tag : tags) {
				add<T>(tag);
			}
		}
		
		/*!
			Add a method of reconstructing an object of a specified type
			@param tag A tag that may be used to identify the object type
		*/
		template<typename T> requires Transportable<T>
		void add(const active::utility::String& tag) {
			reconstruction[tag] = std::make_pair( &typeid(T), reconstructFunc<T>);
		}
		
	private:
			///Factory functions to construct objects from received serialised data paired with the typename and type info of the original class
		std::unordered_map<active::utility::String, std::pair<const std::type_info*, Reconstruction>> reconstruction;
	};

}  // namespace active::serialise::doc

#endif	//ACTIVE_SERIALISE_DOC_HANDLER
