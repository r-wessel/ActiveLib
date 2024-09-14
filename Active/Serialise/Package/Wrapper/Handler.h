/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_HANDLER
#define ACTIVE_SERIALISE_HANDLER

namespace active::serialise {
	
	/*!
		Maker function to reconstruct an object instance
		@return A new instance
	*/
	template<typename T> requires std::is_base_of_v<Package, T>
	Package* makeFunc() {
		return new T{};
	}
	
	/*!
	 A class for initialising serialised packages
	 
	 The primary role of this class is to construct class instances using a serialised attribute, e.g. object name. An instance of this class
	 should be populated with the identifying attribute values and maker functions for all relevant object types
	*/
	class Handler {
	public:
		
		// MARK: Types
		
			///Shared pointer
		using Shared = std::shared_ptr<Handler>;
		
			//Factory function for reconstructing a Package
		using Reconstruction = std::function<Package*()>;
		
		// MARK: Constructors
		
		/*!
		 Constructor
		 @param attributeTag The tag identifying a package type
		*/
		Handler(const utility::String& attributeTag) : m_attributeTag{attributeTag} {}
		
		// MARK: Functions (const)
		
		/*!
			Determine if the handler is empty (no defined object types)
			@return True if no object types are defined
		*/
		bool empty() const { return reconstruction.empty(); }
		/*!
			Reconstruct a package instance based on the attached tag
			@param tag The attached tag
			@return A new package (nullptr on failure)
		*/
		Package* reconstruct(const utility::String& tag) const {
			if (auto maker = reconstruction.find(tag); (maker != reconstruction.end()))
				return maker->second.second();
			return nullptr;
		} //reconstruct
		/*!
			Get the attribute tag for object types
			@return The attribute tage
		*/
		const utility::String& attributeTag() const {
			return m_attributeTag;
		} //reconstruct
		/*!
			Find the tag associated with a specified object type
			@param info The object type info
			@return The serialisation tag associated with this type (unknown type = nullopt)
		*/
		utility::String::Option findTagFor(const std::type_info& info) const {
			if (auto maker = std::find_if(reconstruction.begin(), reconstruction.end(),
										  [&info](const auto& entry){ return entry.second.first == &info; }); maker != reconstruction.end())
				return maker->first;
			return std::nullopt;
		}

		// MARK: Functions (mutating)
		
		/*!
			Add a method of reconstructing packages
			@param tags Tag(s) that may be used to identify a package of this type
		*/
		template<typename T> requires std::is_base_of_v<Package, T>
		void add(std::initializer_list<active::utility::String> const& tags) {
			for (const auto& tag : tags)
				add<T>(tag);
		}
		
		/*!
			Add a method of reconstructing an object of a specified type
			@param tag A tag that may be used to identify the object type
		*/
		template<typename T> requires std::is_base_of_v<Package, T>
		void add(const active::utility::String& tag) {
			reconstruction[tag] = std::make_pair(&typeid(T), makeFunc<T>);
		}
		
	private:
			///Factory functions to construct packages from serialisation tags paired with the original type info and reconstruction function
		std::unordered_map<active::utility::String, std::pair<const std::type_info*, Reconstruction>> reconstruction;
			///The tag of the attribute identifying package type
		utility::String m_attributeTag;
	};

}
	
#endif //ACTIVE_SERIALISE_HANDLER
