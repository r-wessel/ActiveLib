/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_UNBOXED_TRANSPORT
#define ACTIVE_SERIALISE_UNBOXED_TRANSPORT

#include "Active/Serialise/Transport.h"

namespace active::serialise {
	
	/*!
		A placeholder for object transport in an unboxed (unserialised) form
	 
		Use for implementing templates that require a serialisation protocol in contexts transporting raw (unserialised) data
	*/
	class UnboxedTransport : public serialise::Transport {
	public:
		
		// MARK: - Constructors
		
		using Transport::Transport;

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
						  bool isTabbed = false, bool isLineFeeds = false, bool isNameSpaces = true, bool isProlog = true) const override {};
		/*!
			Receive cargo from a serialised data source
			@param cargo The cargo to receive the serialised data
			@param identity The cargo identity (name, optional namespace)
			@param source The data source (can be a wrapper for file, memory, string)
			@throw std::system_error Thrown on read errors, invalid encoding or parsing failure (e.g. ill-formed data)
		*/
		void receive(serialise::Cargo&& cargo, const serialise::Identity& identity, utility::BufferIn&& source) const override {};
	};
	
}

#endif	//ACTIVE_SERIALISE_UNBOXED_TRANSPORT
