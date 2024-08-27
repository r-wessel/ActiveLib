/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_INVENTORY
#define ACTIVE_SERIALISE_INVENTORY

#include "Active/Serialise/Inventory/Entry.h"

#include <vector>

namespace active::serialise {
	
	/*!
		Interface for the inventory of a package (describing package/item content)
	*/
	class Inventory : public std::vector<Entry> {
	public:
		
		// MARK: - Types
		
			///Inventory value type, i.e. entries
		using value = Entry;
			///Base storage class - entry keyed by ID/name
		using base = std::vector<value>;
			///Inventory size_type
		using size_type = base::size_type;
			///Base storage class - entry keyed by ID/name
		using iterator = typename base::iterator;
			///An optional inventory
		using Option = std::optional<Inventory>;
			///An inventory item pairing an entry with its ID/name
		using Item = value;
			///An optional inventory item
		using ItemOption = std::optional<Inventory::Item>;
			///Inventory contents sequenced by the entry index (NB: invalidated if the inventory changes)
		using Sequence = std::vector<std::pair<int16_t, const_iterator>>;

		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		Inventory() : base() {}
		/*!
			Constructor
			@param entries The inventory entries
		*/
		Inventory(base entries) : base(entries) { m_isFilled = true; }
		
		// MARK: - Variables
		
			///True if cargo with any item ID can be accepted by the package inventory
		bool isEveryItemAccepted = false;
		
		// MARK: - Functions (const)
		
			///True if entries have been filled into the inventory
		bool isFilled() const { return m_isFilled; }
		/*!
			Count the number of attributes in the inventory
			@param isRequiredOnly True if only required attributes should be counted
			@return The number of attributes */
		size_type attributeSize(bool isRequiredOnly = false) const;
		/*!
			Count the number of required entries in the inventory
			@return The number of required entries
		*/
		size_type countRequired() const;
		/*!
			Get the inventory handling sequence, e.g. to package in the correct order for transport
			@return The inventory handling sequence (ordered by entry.index). Invalidated if the inventory changes
		*/
		Sequence sequence() const;

		// MARK: - Functions (mutating)
		
		/*!
			Set the owner type for the inventory entries
			@param ownerType The owner type (typically used to link entries to objects within a hierarchical structure)
			@return A reference to this
		*/
		Inventory& withType(const std::type_info* ownerType);
		/*!
			Merge another inventory with this
			@param inventory The inventory to merge
			@return A reference to this
		*/
		Inventory& merge(const Inventory& inventory);
		/*!
			Merge an entry with this
			@param entry The entry to merge
			@return A reference to this
		*/
		Inventory& merge(const Entry& entry);
		/*!
			Register an incoming item in the inventory
			@param identity The item identity
 			@return The registered item (nullopt == not found)
		*/
		iterator registerIncoming(const Identity& identity);
		/*!
			Reset the availability per entry to zero (typically just prior to importing)
		*/
		void resetAvailable();
		/*!
			Find an entry by identity and optionally owner type
			@param identity The entry identity
			@param owner The entry owner type
			@return An iterator at the requested entry (end() on failure)
		*/
		iterator findEntry(const Identity& identity, const std::type_info* owner = nullptr);
		/*!
			Find an entry by field index and optionally owner type
			@param ind The field index
			@param owner The entry owner type
			@return An iterator at the requested entry (end() on failure)
		*/
		iterator findEntry(int16_t ind, const std::type_info* owner = nullptr);
		/*!
			Set the required state of a specified field (for export)
			@param mustHave True if the field is required for export
			@param ind The field index
			@param owner The entry owner type
			@return True if the field was found
		*/
		bool setRequired(bool mustHave, int16_t ind, const std::type_info* owner = nullptr);
		/*!
			Mark all the entries as 'required'
		*/
		void setAllRequired();
		
	private:
			///True if entries have been filled into the inventory
		bool m_isFilled = false;
	};
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_INVENTORY
