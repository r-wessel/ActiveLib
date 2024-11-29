/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_MANAGER
#define ACTIVE_SERIALISE_MANAGER

#include <memory>

namespace active::serialise {
	
	/*!
	 Base class for a serialisation manager
	 
	 A serialisation manager is intended to assist with reorganistion or broad management of data, e.g. caching objects during complex migration
	 processes until they can be appropriately handled
	 */
	class Manager {
	public:
		using Shared = std::shared_ptr<Manager>;
		
		virtual ~Manager() {}
	};
	
}

#endif	//ACTIVE_SERIALISE_MANAGER
