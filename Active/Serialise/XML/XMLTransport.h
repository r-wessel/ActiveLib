/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_TRANSPORT
#define ACTIVE_SERIALISE_XML_XML_TRANSPORT

#include "Active/Serialise/Transport.h"
#include "Active/Utility/String.h"

namespace active::serialise::xml {
	
	/*!
		A serialisation tool class to send/receive data serialised via XML
	*/
	class XMLTransport : public serialise::Transport {
	public:
		
		// MARK: - Types
		
			///Status of of the XML transport
		enum class Status {
			nominal,	///<No errors logged
			unknownEscapeChar,	///<The character following an escape is unknown, e.g. not \n, \r, \u etc
			badEncoding,	///<The encoding of an escaped character is invalid, e.g. \uXXXX
			badSource,	///<The XML source failed, e.g. corrupt file
			unboundedTag,	///<The closing brace for a tag is missing (>)
			unknownSection,	///<The specified section type is unknown
			unknownInstruction,	///<The specified instruction type is unknown
			missingTagName,	///<The tag name is empty
			missingAttributes,	///<The instruction attributes are missing
			attributeEqualMissing,	///<The equals character is missing in an attribute
			attributeQuoteMissing,	///<The quote character is missing in an attribute
			parsingError,	///<The XML source couldn't be parsed - likely corrupt or not well-formed
			closingTagMissing,	///<A closing tag is missing
			badName,	///<A value was found, but invalid
			badValue,	///<A value was found, but invalid
			badElement,	///<Element content was rejected as invalid
			badDestination,	///<The XML destination failed, e.g. full storage, corrupt FS etc prevented writing
			missingInventory,	///<An object has been provided to send/receive data, but it cannot provide an inventory for its content
			unbalancedScope,	///<The closing tag cannot be paired with the opening
			inventoryBoundsExceeded,	///<The source contains more items of a specific type than the inventory maximum permits
			unknownTag,	///<An unknown tag was found in the XML
		};
		
		// MARK: - Constructors
		
		using Transport::Transport;
		
		// MARK: - Functions (const)
		
		/*!
			Send cargo as XML to a specified destination
			@param cargo The cargo to be sent as XML
			@param identity The cargo identity (name, optional namespace)
			@param destination The destination buffer for the exported XML (can target file, memory, string)
			@param isTabbed True to indent lines with tabs
			@param isLineFeeds True to add line-feeds
			@param isNameSpaces True to inclued namespaces (where specified)
			@param isProlog True if an serialisation prolog should be written
			@throw std::system_error Thrown on write errors, serialisation failure (e.g. invalid XML names, missing inventory etc)
		*/
		virtual void send(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferOut&& destination,
				  bool isTabbed = false, bool isLineFeeds = false, bool isNameSpaces = true, bool isProlog = true) const override;
		/*!
			Receive cargo from a specified XML source
			@param cargo The cargo to receive the XML data
			@param identity The cargo identity (name, optional namespace)
			@param source The XML source (can be a wrapper for file, memory, string)
			@throw std::system_error Thrown on read errors, invalid encoding or parsing failure (e.g. ill-formed XML)
		*/
		void receive(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferIn&& source) const override;
		/*!
			Determine if unknown instructions are skipped
			@return True if unknown instructions are skipped
		*/
		bool isUnknownInstructionSkipped() const noexcept { return m_isUnknownInstructionSkipped; }
		
		// MARK: - Functions (mutating)
		
		/*!
			Set whether unknown instructions are skipped
			@param state True if unknown instructions are skipped
		*/
		void setUnknownInstructionSkipped(bool state) noexcept { m_isUnknownInstructionSkipped = state; }
		
	private:
			//True if unknown instructions should be skipped over
		bool m_isUnknownInstructionSkipped = true;
};
	
}

#endif	//ACTIVE_SERIALISE_XML_XML_TRANSPORT
