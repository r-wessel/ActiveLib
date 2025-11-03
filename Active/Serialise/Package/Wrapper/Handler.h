/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_HANDLER
#define ACTIVE_SERIALISE_HANDLER

#include "Active/Setting/ValueSetting.h"

namespace active::setting {
	class SettingList;
}

namespace active::serialise {
	
		///Concept for packages constructed using an input parameter list
	template<typename Obj>
	concept MadeWithParams = requires(Obj& obj, const setting::SettingList* params) {
		{ Obj{params} };
	};
	
	
	/*!
		Maker function to reconstruct an object instance
		@return A new instance
	*/
	template<typename T> requires std::is_base_of_v<Package, T>
	Package* makeFunc(const setting::SettingList* params) {
		if constexpr (MadeWithParams<T>)
			return new T{params};
		else
			return new T{};
	}
	
	/*!
	 A class for initialising serialised packages
	 
	 The primary role of this class is to construct class instancees using a serialised attribute, e.g. object name. An instance of this class
	 should be populated with the identifying attribute values and maker functions for all relevant object types
	 Objects can be paired with a reconstruction method by the expected serialisation tag/type name and optionally input settings A custom
	 filter can be used if a simple comparison of the serialised type name is insufficient, e.g. if the type name is a compound type or the
	 precise type can only be discovered using the settings.
	*/
	class Handler {
	public:
		
		// MARK: Types
		
			///Shared pointer
		using Shared = std::shared_ptr<Handler>;
		
			//Factory function for reconstructing a Package
		using Reconstruction = std::function<Package*(const setting::SettingList*)>;

			//Filter for picking reconstruction method based on an object tag/type name and optional settings
		using Filter = std::function<bool(const active::utility::String&, const setting::SettingList*)>;
	
		// MARK: Constructors
		
		/*!
		 Constructor
		 @param attributeTag The tag identifying a package type
		 @param parameter Any tags for optional parameter attributes required for instantiating a package
		*/
		Handler(const utility::String& attributeTag, std::initializer_list<active::utility::String> const& parameter = {}) :
				m_attributeTag{attributeTag}, m_parameterTags{parameter} {}

		/*!
		 Handler factory
		 @param attributeTag The tag identifying a package type
		 @param tag a package type tag
		 @param parameter Any tags for optional parameter attributes required for instantiating a package
		 @return A new handler
		*/
		template<typename T> requires std::is_base_of_v<Package, T>
		static Handler makeHandler(const utility::String& attributeTag, const utility::String& tag, std::initializer_list<active::utility::String> const& parameter = {}) {
			Handler result{attributeTag, parameter};
			result.add<T>(tag);
			return result;
		}
		
		// MARK: Functions (const)
		
		/*!
			Determine if the handler is empty (no defined object types)
			@return True if no object types are defined
		*/
		bool empty() const { return filteredReconstruction.empty() && filteredReconstruction.empty(); }
		/*!
			Reconstruct a package instance based on the attached tag
			@param tag The attached tag
			@param parameters Any parameters supplied for the package construction (nullptr = none)
			@return A new package (nullptr on failure)
		*/
		Package* reconstruct(const utility::String& tag, const setting::SettingList* parameters = nullptr) const {
				//Exact tag/type name matches are searched first
			if (auto maker = reconstruction.find(tag); (maker != reconstruction.end()))
				return maker->second.second(parameters);
				//Then more general filters are used if no exact name match is found
			if (auto maker = std::find_if(filteredReconstruction.begin(), filteredReconstruction.end(),
										  [&tag, parameters](const auto& entry){ return entry.filter(tag, parameters); }); maker != filteredReconstruction.end())
				return maker->reconstruction(parameters);
			return nullptr;
		} //reconstruct
		/*!
		 Get the attribute tag for object types
		 @return The attribute tage
		 */
		const auto& attributeTag() const {
			return m_attributeTag;
		} //attributeTag
		/*!
		 Get any parameter tags for reading package construction parameters
		 @return The parameter tage
		 */
		const auto& parameterTags() const {
			return m_parameterTags;
		} //parameterTags
		/*!
			Find the tag associated with a specified object type
			@param info The object type info
			@return The serialisation tag associated with this type (unknown type = nullopt)
		*/
		std::optional<utility::String> findTagFor(const std::type_info& info) const {
			if (auto maker = std::find_if(reconstruction.begin(), reconstruction.end(),
										  [&info](const auto& entry){ return entry.second.first == &info; }); maker != reconstruction.end())
				return maker->first;
			if (auto maker = std::find_if(filteredReconstruction.begin(), filteredReconstruction.end(),
										  [&info](const auto& entry){ return entry.typeInfo == &info; }); maker != filteredReconstruction.end())
				return maker->tag;
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
			@param tag A tag/type name used to identify the object type
		*/
		template<typename T> requires std::is_base_of_v<Package, T>
		void add(const active::utility::String& tag) {
			reconstruction[tag] = std::make_pair(&typeid(T), makeFunc<T>);
		}
		/*!
			Add a method of reconstructing an object of a specified type
			@param filter A filter used to identify the object type
		*/
		template<typename T> requires std::is_base_of_v<Package, T>
		void add(const Filter& filter) {
			filteredReconstruction.push_back({filter, &typeid(T), makeFunc<T>});
		}
		/*!
		 Merge the reconstruction functions of another handler into this
		 @param handler The handler to merge
		 */
		void merge(const Handler& handler) {
			for (const auto& item : handler.reconstruction)
				reconstruction[item.first] = item.second;
		}
		
	private:
			///A reconstructor pairing package identification with a reconstruction method
		struct Reconstructor {
			Reconstructor(active::utility::String tg, const std::type_info* info, const Reconstruction& construction) :
					tag{tg}, typeInfo{info}, reconstruction{construction} {}
			Reconstructor(const Filter& filt, const std::type_info* info, const Reconstruction& construction) :
					filter{filt}, typeInfo{info}, reconstruction{construction} {}
			active::utility::String tag;
			const std::type_info* typeInfo;
			Filter filter;
			Reconstruction reconstruction;
		};
		
			///Factory functions to construct packages from serialisation tags paired with the original type info and reconstruction function
		std::unordered_map<active::utility::String, std::pair<const std::type_info*, Reconstruction>> reconstruction;
			///Factory functions to construct packages from a filter utilising type names and/or object settings
		std::vector<Reconstructor> filteredReconstruction;
			///The tag of the attribute identifying package type
		utility::String m_attributeTag;
			///Secondary tags required to instantiate a package type
		std::vector<utility::String> m_parameterTags;
	};

}
	
#endif //ACTIVE_SERIALISE_HANDLER
