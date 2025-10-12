/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_RPC_SERVER_PORTAL
#define ACTIVE_RPC_SERVER_PORTAL

#include "Active/RemoteProc/DefaultBinding.h"
#include "Active/RemoteProc/Object.h"
#include "Active/Utility/String.h"

namespace active::rpc {
	
	/*!
	 Interface for a server portal receiving remote procedure calls
	 @tparam FunctionBinding The platform serialisation protocal/data binding
	*/
	template<typename FunctionBinding = DefaultBinding>
	class ServerPortal : protected std::vector<std::shared_ptr<Object<FunctionBinding>>> {
	public:
		
		// MARK: - Types
		
		using base = std::vector<std::shared_ptr<Object<FunctionBinding>>>;
		
		// MARK: - Constructors
		
		/*!
		 Destructor
		 */
		virtual ~ServerPortal() {}

		// MARK: - Functions (mutating)
		
		/*!
		 Install a JS function object
		 @param object The object to install
		 @return True if the object was successfully installed
		 */
		std::shared_ptr<Object<FunctionBinding>> install(std::shared_ptr<Object<FunctionBinding>> object) = 0;
		/*!
		 Install a JS function object
		 @return True if the object was successfully installed
		 @tparam T The type of object to install
		 */
		template<typename T> requires std::is_base_of_v<Object<FunctionBinding>, T>
		std::shared_ptr<Object<FunctionBinding>> install() { return install(std::make_shared<T>()); }
	};
	
}

#endif	//ACTIVE_RPC_SERVER_PORTAL
