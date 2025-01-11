/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_ENTRY
#define ACTIVE_SERIALISE_ENTRY

#include "Active/Serialise/Inventory/Identity.h"

#include <cstdint>
#include <optional>

namespace active::serialise {
	
	class Inventory;
	class Management;

	/*!
		An entry in the inventory of a serialisable package
	*/
	struct Entry {
		
		using enum Identity::Role;
		
		class ID {
		public:
			ID(Identity&& identity) : m_identity{std::move(identity)} {}
			ID(const Identity& identity) : m_wrapped{std::reference_wrapper<const Identity>{identity}} {}

				///Get the wrapped identity
			const Identity& identity() const { return m_wrapped ? m_wrapped->get() : m_identity; }

		private:
			Identity m_identity;
			std::optional<std::reference_wrapper<const Identity>> m_wrapped;
		};
		
		// MARK: - Constructors
		
		/*!
			Constructor  (Single-instance items)
			@param ident The entry identity
			@param ind An index for the entry
			@param role Entry role, i.e. attribute/element
			@param mustHave True if required for export
			@param owner The owner type
		*/
		Entry(const Identity& ident, int16_t ind, Identity::Role role, bool mustHave = true, const std::type_info* owner = nullptr) :
				Entry{ID{ident}, ind, role, mustHave, owner} {}
		Entry(Identity&& ident, int16_t ind, Identity::Role role, bool mustHave = true, const std::type_info* owner = nullptr) :
				Entry{ID{std::move(ident)}, ind, role, mustHave, owner} {}
		/*!
			Constructor  (Single-instance items)
			@param ident The entry identity
			@param ind An index for the entry
			@param howMany How many items are currently available
			@param mustHave True if required for export
			@param owner The owner type
		*/
		Entry(const Identity& ident, int16_t ind, size_t howMany, std::optional<size_t> max = 1, bool mustHave = true, const std::type_info* owner = nullptr) :
				Entry{ID{ident}, ind, howMany, max, mustHave, owner} {}
		Entry(Identity&& ident, int16_t ind, size_t howMany, std::optional<size_t> max = 1, bool mustHave = true, const std::type_info* owner = nullptr) : 	Entry{ID{std::move(ident)}, ind, howMany, max, mustHave, owner} {}
		Entry(const Entry& source) = default;
		Entry& operator=(const Entry& source) = default;

		// MARK: - Variables
		
			///The type info of the owner package
		const std::type_info* ownerType = nullptr;
			///The inventory containing this entry
		Inventory* inventory = nullptr;
			///How many are currently available (also acts as the requested item on import/export in 'getCargo')
		size_t available = 0;
			///The entry index
		int16_t index = 0;
			///True if required for export
		bool required = true;
		
		// MARK: - Functions (const)
		
			///Get the wrapped identity
		const Identity& identity() const { return m_id.identity(); }
			///Get any management applied to this item (nullptr = unmanaged)
		Management* management() const;
		/*
			Determine if the entry is an attribute
			@return True if the entry is repeating
		*/
		bool isAttribute() const { return (m_role == attribute); }
		/*!
			Determine if the entry items can repeat, e.g. are stored in an array
			@return True if the entry is repeating
		*/
		bool isRepeating() const { return (!m_maximum || (m_maximum > 1)); }
		/*!
			Determine if the entry is managed
			@return True if the entry is managed
		*/
		bool isManaged() const;
		/*!
			The maximum number of instances against this entry (nullopt = unlimited)
			@return The maximum number of instances
		*/
		std::optional<size_t> maximum() const { return isAttribute() ? 1 : m_maximum; }

		// MARK: - Functions (mutating)
		
		/*!
			Set the entry identity
			return: True if the availability was bumped (false indicates no additional items are allowed)
		*/
		void setIdentity(const Identity& ident) { m_id = ID{ident}; }
		/*!
			Set the entry identity
			return: True if the availability was bumped (false indicates no additional items are allowed)
		*/
		void setIdentity(Identity&& ident) { m_id = ID{std::move(ident)}; }
		/*!
			Set the entry owner type
			@param role The The entry role
			@return A reference to this
		*/
		Entry& withRole(Identity::Role role) {
			m_role = role;
			return *this;
		}
		/*!
			Set the entry owner type
			@param owner The owner type ID
			@return A reference to this
		*/
		Entry& withOwner(const std::type_info* owner) {
			ownerType = owner;
			return *this;
		}
		/*!
			Bump the entry availability (where allowed)
			return: True if the availability was bumped (false indicates no additional items are allowed)
		*/
		bool bumpAvailable() {
			if (!maximum() || (available < maximum())) {
				++available;
				return true;
			}
			return false;
		}
		/*!
			Set the maximum number of instances against this entry (nullopt = unlimited)
			@param max The new maximum
		*/
		void setMaximum(std::optional<size_t> max = std::nullopt) { m_maximum = max; }
		
	private:
		/*!
			Constructor  (Single-instance items)
			@param ind An index for the entry
			@param role Entry role, i.e. attribute/element
			@param mustHave True if required for export
			@param owner The owner type
		*/
		Entry(ID&& id, int16_t ind, Identity::Role role, bool mustHave, const std::type_info* owner) : m_id{id} {
			index = ind;
			m_role = role;
			required = mustHave;
			if (required)
				available = 1;
			ownerType = owner;
		}
		/*!
			Constructor (for elements with potentially more than 1 instance)
			@param ind An index for the entry
			@param howMany How many items are currently available
			@param max The maximum number of instances that can be imported (nullopt = unlimited)
			@param mustHave True if required for export
			@param owner The owner type
		*/
		Entry(ID&& id, int16_t ind, size_t howMany, std::optional<size_t> max, bool mustHave, const std::type_info* owner) : m_id{id} {
			index = ind;
			m_maximum = max;
			if (m_maximum.has_value() && (*m_maximum <= 0))
				m_maximum = 1;
			available = howMany;
			required = mustHave;
			ownerType = owner;
			m_role = element;
		}
		
			///The type identity
		ID m_id;
			///The maximum number that can be imported (nullopt = unlimited; greater than 1 means imported cargo will be inserted)
		std::optional<size_t> m_maximum = 1;
			///The entry type
		Identity::Role m_role = element;
	};
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_ENTRY
