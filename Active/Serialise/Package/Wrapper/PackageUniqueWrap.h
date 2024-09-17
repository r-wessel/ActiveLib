/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_UNIQUE_WRAPPER
#define ACTIVE_SERIALISE_UNIQUE_WRAPPER

#include "Active/Serialise/Package/Package.h"
#include "Active/Utility/Concepts.h"

namespace active::serialise {
	
	/*!
	 Wrapper for packages held in a unique ptr
	 
	 These are a special case for serialisation - the value might be null on read or write. On read, a new instance should be created. This wrapper
	 is capable of making one if the object can only be a single type. For polymorphic objects, use a Handler to reconstruct the correct type.
	 
	 Use this wrapper in conjunction with Mover (or a derived class) wherever serialisation of a member object held in a unique pointer is required
	 */
	class PackageUniqueWrap {
	public:
		
		// MARK: - Types
		
			///A function type to read the object from the unique pointer
		using Reader = std::function<Package*()>;
		using Writer = std::function<void(std::unique_ptr<Package>)>;
		using Maker = std::function<std::unique_ptr<Package>()>;
		
		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param var A reference to a unique pointer package member variable
		 */
		template<class T> requires std::is_base_of_v<Package, T>
		PackageUniqueWrap(std::unique_ptr<T>& var) :
				m_reader{[&](){ return var.get(); }},
				m_writer{
					[&](std::unique_ptr<Package> incoming) {
						auto temp = dynamic_cast<T*>(incoming.get());
						if (temp != nullptr) {
							var.reset(temp);
							incoming.release();
						}
					}
				} {
					if constexpr (!utility::DefaultConstructable<T>) {
						m_maker = [&](){ return std::make_unique<T>(); };
					}
				}
		/*!
		 Constructor
		 @param var A reference to a unique pointer package member variable
		 */
		template<class T> requires std::is_base_of_v<Package, T>
		PackageUniqueWrap(const std::unique_ptr<T>& var) : PackageUniqueWrap{const_cast<std::unique_ptr<T>&>(var)} {}
			
		// MARK: - Functions (const)

		/*!
		 Get a pointer to the wrapped object
		 @return A pointer to the object (nullptr if unallocated)
		 */
		Package* get() const { return m_reader(); }
		/*!
		 Set the object held in the unique pointer
		 @param incoming The new object
		 */
		void set(std::unique_ptr<Package> incoming) const { m_writer(std::move(incoming)); }
		/*!
		 Determine if this package can make new object instances
		 @return True if this package can make new object instances
		 */
		bool canMake() const { return m_maker.operator bool(); }
		/*!
		 Make a new instance of the object type
		 @return The new object instance
		 */
		std::unique_ptr<Package> make() const { return m_maker(); }

	private:
			///The variable reader
		Reader m_reader;
			///The variable writer
		Writer m_writer;
			///A factory to make new objects
		Maker m_maker;
	};

}
	
#endif //ACTIVE_SERIALISE_UNIQUE_WRAPPER
