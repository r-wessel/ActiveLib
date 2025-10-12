/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_RPC_FUNCTION
#define ACTIVE_RPC_FUNCTION

#include "Active/RemoteProc/Binding.h"
#include "Active/RemoteProc/NamedFunction.h"
#include "Active/Serialise/Inventory/Identity.h"
#include "Active/Serialise/Item/Wrapper/ItemWrap.h"
#include "Active/Serialise/Package/NullPackage.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/String.h"

#include <functional>
#include <type_traits>
#include <utility>

namespace active::rpc {
	
	/*!
	 Interface for a C++ function implementation binding to a remote procedure call
	 @tparam Argument The function argument type
	 @tparam Return The function return type
	 @tparam Binding The protocal/data binding
	*/
	template<typename Argument, typename Return, typename Binding>
	class Function : public NamedFunction<Argument, Return>, public Binding {
	public:

		// MARK: - Types
		
		using base = NamedFunction<Argument, Return>;
			//A bridged C++ function
		using Function = base::Function;

		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param jsName The function name (as called from JavaScript)
		 @param function The C++ function bound to the name
		 */
		Function(const active::utility::String& jsName, Function function) : base{jsName, function} {}
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		Function(const Function& source) = default;

		// MARK: - Functions (const)
		
		/*!
		 Determine if the function returns void (no return)
		 @return True if the function returns void
		 */
		bool isVoid() const override;
		/*!
		 Get the bound function name
		 @return The bound function name
		 */
		active::utility::String getName() const override { return base::getName(); }
		/*!
		 Execute the function
		 @param param The input parameter
		 @return The function result
		 */
		std::unique_ptr<typename Binding::ValueType> executeRPC(const typename Binding::ValueType& param) const override;
	};
	
	
	/*--------------------------------------------------------------------
		Determine if the function returns void (no return)
	 
		return: True if the function returns void
	  --------------------------------------------------------------------*/
	template<typename Argument, typename Return, typename Binding>
	bool Function<Argument, Return, Binding>::isVoid() const {
		if constexpr(std::is_same<Return, void>::value)
			return true;
		else
			return false;
	} //Function<Argument, Return, Binding>::isVoid

	
	/*--------------------------------------------------------------------
		Execute the function
	 
		param: The input parameter
	 
		return: The function result
	  --------------------------------------------------------------------*/
	template<typename Argument, typename Return, typename Binding>
	std::unique_ptr<typename Binding::ValueType> Function<Argument, Return, Binding>::executeRPC(const typename Binding::ValueType& bindingParameters) const {
		typename Binding::Transport transport;
			//Process any returned result into the binding value type
		auto processResult = [&, transport](Return* outgoing) -> std::unique_ptr<typename Binding::ValueType> {
			if constexpr(!std::is_same<Return, void>::value) {
				if (outgoing == nullptr)
					throw;	//NB: Throw a system exception here in future with a defined error
				auto result = std::make_unique<typename Binding::ValueType>();
				transport.send(std::move(*outgoing), active::serialise::Identity{}, *result);
				return result;
			}
			return nullptr;
		};
			//Ensure void parameters and/or return values are handled correctly
		if constexpr(std::is_same<Argument, void>::value) {
			active::serialise::NullPackage null;
			if constexpr(std::is_same<Return, void>::value)
				base::execute(null);	//No parameters and no return value
			else {
				auto result = base::execute(null);	//No parameters with return value
				return processResult(dynamic_cast<Return*>(result.get()));
			}
		} else {
			Argument argument;
			if constexpr (std::is_base_of<active::serialise::Package, Argument>::value)
				transport.receive(active::serialise::PackageWrap{argument}, active::serialise::Identity{}, bindingParameters);
			else
				transport.receive(active::serialise::ItemWrap{argument}, active::serialise::Identity{}, bindingParameters);
			if constexpr(std::is_same<Return, void>::value)
				base::execute(argument);	//Parameters and no return value
 			else {
				auto result = base::execute(argument);	//Parameters with return value
				return processResult(dynamic_cast<Return*>(result.get()));
			}
		}
		return nullptr;
	} //Function<Argument, Return, Binding>::execute

}

#endif	//ACTIVE_RPC_FUNCTION
