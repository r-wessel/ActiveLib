/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_RPC_DEFAULT_BINDING
#define ACTIVE_RPC_DEFAULT_BINDING

#include "Active/RemoteProc/Binding.h"
#include "Active/Serialise/JSON/JSONTransport.h"

namespace active::rpc {
	
		///Define the default binding for rpc protocal and data serialisation
	using DefaultBinding = Binding<active::string, active::serialise::json::JSONTransport>;
	
}

#endif //ACTIVE_RPC_DEFAULT_BINDING
