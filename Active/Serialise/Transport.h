/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_TRANSPORT
#define ACTIVE_SERIALISE_TRANSPORT

#include "Active/Serialise/XML/Item/XMLDateTime.h"
#include "Active/Utility/Memory.h"

namespace active::utility {
	
	class BufferIn;
	class BufferOut;

}

namespace active::serialise {
	
	class Cargo;
	struct Identity;

}

namespace active::serialise {
	
	/*!
		Interface for objects that send/receive data in a serialised form
	*/
	class Transport {
	public:
		
		// MARK: - Types
			
		using TimeFormat = std::optional<xml::XMLDateTime::Format>;
		using size_type = utility::Memory::size_type;
		using enum xml::XMLDateTime::Format;
		
			///Policy for adherence to schema
		enum Policy {
			relaxed,	///<No constraints, skips unknown items and does not impose content requirements
			moderate,	///<Unknown items not accepted, but does not impose content requirements
			strict,	///<Schema adherence fully implemented - no unknown instances and all required instances must be found
			verbose,	///<Schema adherence fully implemented - no unknown instances and requirement imposed on all instances
		};
		
		// MARK: - Constructors
		
		/*!
			Default constructor
			@param policy Content policy, i.e. unknown instances and requirements policy
			@param timeFormat The serialisation date/time format (nullopt = use the format specified in each outgoing item)
		*/
		Transport(Policy policy = relaxed, TimeFormat timeFormat = iso8601) noexcept {
			switch (policy) {
				case verbose:
					m_isEveryEntryRequired = true;
					[[fallthrough]];
				case strict:
					m_isMissingEntryFailed = true;
					[[fallthrough]];
				case moderate:
					m_isUnknownNameSkipped = false;
					break;
				default:
					break;	//NB: Default member variable settings equate to relaxed policy
			}
			m_timeFormat = timeFormat;
		}
		/*!
			Destructor
		*/
		virtual ~Transport() noexcept {}
		
		
		// MARK: - Functions (const)
		
		/*!
			Send serialised cargo to a specified destination
			@param cargo The cargo to be serialised
			@param identity The cargo identity (name, optional namespace)
			@param destination The destination buffer for the exported serialised data (can target file, memory, string)
			@param isTabbed True to indent lines with tabs
			@param isLineFeeds True to add line-feeds
			@param isNameSpaces True to inclued namespaces (where specified)
			@param isProlog True if an serialisation prolog should be written
			@throw std::system_error Thrown on write errors, serialisation failure (e.g. invalid names, missing inventory etc)
		*/
		virtual void send(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferOut&& destination,
						  bool isTabbed = false, bool isLineFeeds = false, bool isNameSpaces = true, bool isProlog = true) const = 0;
		/*!
			Receive cargo from a serialised data source
			@param cargo The cargo to receive the serialised data
			@param identity The cargo identity (name, optional namespace)
			@param source The data source (can be a wrapper for file, memory, string)
			@throw std::system_error Thrown on read errors, invalid encoding or parsing failure (e.g. ill-formed data)
		*/
		virtual void receive(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferIn&& source) const = 0;
		/*!
			Set the preferred serialisation date/time format
			@return The preferred date/time format
		*/
		TimeFormat getTimeFormat() const noexcept { return m_timeFormat; }
		/*!
			Get the last received character row position of the data source (after calling receive, for error diagnostics)
			@return The last row position received from the data source
		*/
		size_type getLastRow() const noexcept { return m_lastRow; }
		/*!
			Get the last received character column position of the data source (after calling receive, sfor error diagnostics)
			@return The last column position received from the data source
		*/
		size_type getLastColumn() const noexcept { return m_lastColumn; }
		/*!
			Determine if unknown names are skipped
			@return True if unknown names are skipped
		*/
		bool isUnknownNameSkipped() const noexcept { return m_isUnknownNameSkipped; }
		/*!
			Determine if all inventory entries should be treated as 'required'
			@return True if all inventory entries should be treated as 'required'
		*/
		bool isEveryEntryRequired() const noexcept { return m_isEveryEntryRequired; }
		/*!
			Determine if a receive operation should be failed if an entry marked 'required' is not found
			@return True if a receive operation should be failed if an entry marked 'required' is not found
		*/
		bool isMissingEntryFailed() const noexcept { return m_isMissingEntryFailed; }
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the preferred date/time format for serialisation
			@param format The preferred date/time format (nullopt = use the format specified in each outgoing item)
		*/
		void setTimeFormat(TimeFormat format) noexcept { m_timeFormat = format; }
		/*!
			Set whether unknown names are skipped
			@param state True if unknown names are skipped
		*/
		void setUnknownNameSkipped(bool state) noexcept { m_isUnknownNameSkipped = state; }
		/*!
			Set whether all inventory entries should be treated as 'required'
			@param state True if all inventory entries should be treated as 'required'
		*/
		void setEveryEntryRequired(bool state) noexcept { m_isEveryEntryRequired = state; }
		/*!
			Set whether a receive operation should be failed if an entry marked 'required' is not found
			@param state True if a receive operation should be failed if an entry marked 'required' is not found
		*/
		void setMissingEntryFailed(bool state) noexcept { m_isMissingEntryFailed = state; }
		
	protected:
		/*!
			Set the last received character row position of the data source
			@param row The last row position received from the data source
		*/
		void setLastRow(size_type row) const noexcept { m_lastRow = row; }
		/*!
			Set the last received character column position of the data source
			@param col The last column position received from the data source
		*/
		void setLastColumn(size_type col) const noexcept { m_lastColumn = col; }
		
	private:
			//The preferred date/time format
		TimeFormat m_timeFormat = iso8601;
			//The last row read from the data source (can be useful for error diagnostics)
		mutable size_type m_lastRow = 0;
			//The last column read from the data source (can be useful for error diagnostics)
		mutable size_type m_lastColumn = 0;
			//True if unknown tags should be skipped over
		bool m_isUnknownNameSkipped = true;
			//True if all inventory entries should be treated as 'required'
		bool m_isEveryEntryRequired = false;
			//True if a receive operation should be failed if an entry marked 'required' is not found
		bool m_isMissingEntryFailed = false;
	};
	
}

#endif	//ACTIVE_SERIALISE_TRANSPORT
