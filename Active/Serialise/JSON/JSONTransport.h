/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_JSON_JSON_TRANSPORT
#define ACTIVE_SERIALISE_JSON_JSON_TRANSPORT

#include "Active/Serialise/Transport.h"
#include "Active/Utility/String.h"

namespace active::serialise::json {
	
	/*!
		A serialisation tool class to send/receive data serialised via JSON
	*/
	class JSONTransport : public serialise::Transport {
	public:
		
		// MARK: - Types
		
			///Status of of the JSON transport
		enum class Status {
			nominal,	///<No errors logged
			unknownEscapeChar,	///<The character following an escape is unknown, e.g. not \n, \r, \u etc
			badEncoding,	///<The encoding of an escaped character is invalid, e.g. \uXXXX
			badSource,	///<The JSON source failed, e.g. corrupt file
			nameMissing,	///<The name of a value/object is missing
			incompleteContext,	///<A structure (object, array etc) has been started but not closed
			parsingError,	///<The JSON source couldn't be parsed - likely corrupt or not well-formed
			closingQuoteMissing,	///<The closing quote is missing in a text value
			valueMissing,	///<A value was required but not found
			badValue,	///<A value was found, but invalid, e.g. text where a number was expected or a number that couldn't be parsed
			badDestination,	///<The JSON destination failed, e.g. full storage, corrupt FS etc prevented writing
			missingInventory,	///<An object has been provided to send/receive data, but it cannot provide an inventory for its content
			unbalancedScope,	///<A scope has been opened but not closed, e.g. an object started with { but not closed with }
			badDelimiter,	///<A value delimiter, i.e. a comma, has been found in the wrong context
			inventoryBoundsExceeded,	///<The source contains more items of a specific type than the inventory maximum permits
			invalidObject,	///<Insertion of an incoming object was rejected, indicating invalid content
			unknownName,	///<An unknown name was found in the JSON
			instanceMissing,	///<A required instance value is missing
		};
		
		// MARK: - Static functions
		
		/*!
			Convert a regular string to an JSON string, i.e. translating special chars etc
			@param source The string to convert
			@return The converted string
		*/
		static utility::String convertToJSONString(const utility::String& source);
		/*!
			Convert an JSON string to a regular string, i.e. translating special chars etc
			@param source The string to convert
			@return The converted string
		*/
		static utility::String convertFromJSONString(const utility::String& source);
		
		// MARK: - Constructors
		
		using Transport::Transport;

		// MARK: - Functions (const)
		
		/*!
			Send cargo as JSON to a specified destination
			@param cargo The cargo to be sent as JSON
			@param identity The cargo identity (name, optional namespace)
			@param destination The destination buffer for the exported JSON (can target file, memory, string)
			@param isTabbed True to indent lines with tabs
			@param isLineFeeds True to add line-feeds
			@param isNameSpaces True to inclued namespaces (where specified)
			@param isProlog True if an serialisation prolog should be written (not currently used for JSON)
			@throw std::system_error Thrown on write errors, serialisation failure (e.g. invalid names, missing inventory etc)
		*/
		virtual void send(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferOut&& destination,
						  bool isTabbed = false, bool isLineFeeds = false, bool isNameSpaces = true, bool isProlog = true) const override;
		/*!
			Receive cargo from a specified JSON source
			@param cargo The cargo to receive the JSON data
			@param identity The cargo identity (name, optional namespace)
			@param source The JSON source (can be a wrapper for file, memory, string)
			@throw std::system_error Thrown on read errors, invalid encoding or parsing failure (e.g. ill-formed JSON)
		*/
		void receive(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferIn&& source) const override;
	};
	
}

#endif	//ACTIVE_SERIALISE_JSON_JSON_TRANSPORT
