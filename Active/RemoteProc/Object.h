/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_RPC_OBJECT
#define ACTIVE_RPC_OBJECT

#include "Active/Container/Vector.h"
#include "Active/RemoteProc/DefaultBinding.h"

namespace active::rpc {
	
	template<typename FunctionBinding>
	class ServerPortal;
	
	/*!
	 Interface for an object binding remote procedure calls to C++ functions
	 @tparam FunctionBinding The platform serialisation protocal/data binding
	*/
	template<typename FunctionBinding = DefaultBinding>
	class Object : public std::vector<std::shared_ptr<FunctionBinding>> {
	public:

		// MARK: - Types
		
		using base = std::vector<std::shared_ptr<FunctionBinding>>;
		using size_type = base::size_type;

		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param name The object name
		 @param toReserve The number of items to reserve space for in the vector
		 */
		Object(const active::string& name, size_type toReserve = 0) : base{toReserve}, m_name{name} {}
		/*!
		 Constructor
		 @param name The object name
		 @param items The items to insert in the array
		*/
		template<class Derived>
		explicit Object(const active::string& name, const std::initializer_list<Derived>& items) : base{items}, m_name{name} {}
		virtual ~Object() {}
		// MARK: - Functions (const)
		
		/*!
		 Get the object name (as expressed to to the server portal)
		 @return The object name
		 */
		const auto& getName() const { return m_name; }

		// MARK: - Functions (mutating)
		
		/*!
		 Set the parent portal for this object
		 @param portal The server portal
		 */
		void setPortal(const ServerPortal<FunctionBinding>& portal) { m_portal = &portal; }
		
	protected:
			///The parent server portal for this object
		const ServerPortal<FunctionBinding>* m_portal = nullptr;
		
	private:
			///The object name
		active::string m_name;
	};
	
}

#endif	//ACTIVE_RPC_OBJECT
