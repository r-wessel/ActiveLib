/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef BIM_INTERFACE_FUNCTIONAL
#define BIM_INTERFACE_FUNCTIONAL

#include "Active/Serialise/Cargo.h"
#include "BIMLib/Utility/String.h"

namespace active::rpc {
	
	/*!
	 Interface for objects providing an executable function
	 @tparam Packaging The packaging type for transporting input parameters and results (Cargo is used as the default for serialisation)
	*/
	template<typename Packaging = active::serialise::Cargo>
	class Functional {
	public:
		/*!
		 Destructor
		 */
		virtual ~Functional() {};

		// MARK: - Functions (const)
		
		/*!
		 Get the function name
		 @return The function name
		 */
		virtual active::utility::String getName() const = 0;
		/*!
		 Execute the function
		 @param param The transported input parameter
		 @return The data to transport back to the caller (nullptr if the result is void)
		 */
		virtual std::unique_ptr<Packaging> execute(const Packaging& param) const = 0;
	};

}

#endif	//BIM_INTERFACE_FUNCTIONAL
