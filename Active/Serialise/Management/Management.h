/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_MANAGEMENT
#define ACTIVE_SERIALISE_MANAGEMENT

#include "Active/Serialise/Management/Manager.h"

namespace active::serialise {
	
	/*!
	 Serialisation management
	 
	 A collection point for serialisation managers. There can be any number involved at one time and should not typically be coupled. The management
	 allows independent access to specific managers
	 */
	class Management : public std::vector<Manager*> {
	public:
		
			///Shared pointer
		using Shared = std::shared_ptr<Management>;

		// MARK: - Functions (const)
		
		/*!
		 Get a manager of a specific type
		 @return The requested manager (nullptr on failure)
		 @tparam T The required manager type
		 */
		template<typename T> requires std::is_base_of_v<Manager, T>
		T* get() const {
			for (auto manager : *this)
				if (auto result = dynamic_cast<T*>(manager); result)
					return result;
			return nullptr;
		}

		// MARK: - Functions (const)
		
		/*!
		 Add any management provided directly by a cargo object
		 @param cargo The source cargo object. NB: The manager resources are not owned by this object - don't use beyond the cargo lifespan
		 */
		void add(Cargo& cargo) {
			if (auto management = dynamic_cast<Management*>(&cargo)) {
				for (auto manager : *management)
					push_back(manager);
			}
			if (auto manager = dynamic_cast<Manager*>(&cargo))
				push_back(manager);
			if (auto management = cargo.management(); management != nullptr)
				for (auto manager : *management)
					push_back(manager);
		}
	};
	
}

#endif	//ACTIVE_SERIALISE_MANAGEMENT
