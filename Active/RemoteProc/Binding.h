/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_RPC_BINDING
#define ACTIVE_RPC_BINDING

#include "Active/Utility/Cloner.h"
#include "Active/string/string_utf8.h"

namespace active::rpc {
	
	/*!
	 Interface for binding a serialisation data type and transport protocol for a remote procedure call
	 @tparam Type The data serialisation type
	 @tparam Trans The transport protocol to transform the serialised data to C++ objects
	*/
	template<typename Type, typename Trans>
	class Binding {
	public:
		
		// MARK: - Types
		
		using DataType = Type;
		using Transport = Trans;
		
		// MARK: - Constructors
		
		virtual ~Binding() {}
		
		// MARK: - Functions (const)
		
		/*!
		 Get the bound function name
		 @return The bound function name
		 */
		virtual string getName() const = 0;
		/*!
		 Determine if the function returns void (no return)
		 @return True if the function returns void
		 */
		virtual bool isVoid() const = 0;
		/*!
		 Execute a function
		 @param param The input parameter
		 @return The function result (nullptr on failure)
		 */
		virtual std::unique_ptr<DataType> executeRPC(const DataType& param) const = 0;
	};
	
}

#endif	//ACTIVE_RPC_BINDING
