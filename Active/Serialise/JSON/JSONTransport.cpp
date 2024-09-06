/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/JSON/JSONTransport.h"

#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Serialise/Item/Item.h"
#include "Active/Serialise/Item/UnknownItem.h"
#include "Active/Serialise/Item/Wrapper/ItemWrap.h"
#include "Active/Serialise/Null.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/Package/Unknown.h"
#include "Active/Setting/Values/BoolValue.h"
#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Setting/Values/Int64Value.h"
#include "Active/Setting/Values/NullValue.h"
#include "Active/Setting/Values/StringValue.h"
#include "Active/Setting/Values/Value.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/Defer.h"
#include "Active/Utility/TextEncoding.h"

#include <unordered_map>

using namespace active::serialise;
using namespace active::serialise::json;
using namespace active::setting;
using namespace active::utility;

using enum JSONTransport::Status;

namespace {
	
		//Leading characters for an object
	constexpr char32_t objectLeader{U'{'};
		//Leading characters for an array
	constexpr char32_t arrayLeader{U'['};
		//Leading characters for a text value
	constexpr char32_t textLeader{U'\"'};
		//Leading string for a text value
	const String textLeaderStr{std::u32string{textLeader}};
		//Possible leading characters for a numeric value
	const std::u32string numberLeader{U"-0123456789"};
		//Possible content characters for a numeric value
	const std::u32string numberContent{numberLeader + U"+.eE"};
		//Possible leading characters for a boolean value
	const std::u32string boolLeader{U"tf"};
		//Leading characters for a null value
	constexpr char32_t nullLeader{U'n'};
		//A JSON null value
	String nullValue{"null"};
		//All possible value leaders
	const std::u32string valueLeaders{std::u32string{textLeader} + std::u32string{numberLeader} +
			std::u32string{boolLeader} + std::u32string{nullLeader}};
		//Value delimiter character
	constexpr char32_t valueDelimiter{U','};
		//Object terminator
	constexpr char32_t objectTerminator{U'}'};
		//Array terminator
	constexpr char32_t arrayTerminator{U']'};
		//All possible JSON value terminators
	String valueTerminators{std::u32string{valueDelimiter} + std::u32string{objectTerminator} + std::u32string{arrayTerminator}};
		//The JSON escape character
	constexpr char32_t escapeChar{U'\\'};
		//The JSON escape string
	const String escapeStr{std::u32string{escapeChar}};
		//The JSON replacement for an escape character
	String escapeCharSymbol{"\\\\"};

		///Determine if a specified char is a value terminator
	bool isValueTerminator(char32_t uniChar) {
		switch (uniChar) {
			case valueDelimiter: case objectTerminator: case arrayTerminator:
				return true;
			default:
				return false;
		}
	}
	
		///Category for JSON processing errors
	class JSONCategory : public std::error_category {
	public:
			///Category name
		const char* name() const noexcept override {
			return "active::serialise::json::category";
		}
		/*!
			Get a message for a specified error code
			@param errorCode A JSON processing code
			@return The error message for the specified code
		*/
		std::string message(int errorCode) const override {
			switch (static_cast<JSONTransport::Status>(errorCode)) {
				case nominal:
					return "";
				case unknownEscapeChar:
					return "Found an unknown or invalid escaped character";
				case badEncoding:
					return "Found an escaped character with invalid encoding";
				case badSource:
					return "The JSON source failed, e.g. corrupt file";
				case nameMissing:
					return "Found an object with no name";
				case incompleteContext:
					return "A scope has been started but not closed";
				case parsingError:
					return "The JSON source couldn't be parsed";
				case closingQuoteMissing:
					return "Found a text value with no closing quote";
				case valueMissing:
					return "A value was required but not found";
				case badValue:
					return "An invalid value was found";
				case badDestination:
					return "The JSON write destination failed";
				case missingInventory:
					return "An object to be sent/received via JSON cannot provide a content inventory";
				case unbalancedScope:
					return "A scope has been opened but not closed";
				case badDelimiter:
					return "A value delimiter (,) has been found in the wrong context";
				case inventoryBoundsExceeded:
					return "Found more instances of a named value/object than the inventory permits";
				case invalidObject:
					return "An invalid object instance was found";
				case unknownName:
					return "An unknown name was found in the JSON";
				case instanceMissing:
					return "A required JSON instance value is missing";
				default:
					return "Unknown/invalid error";
			}
		}
	};


	/*!
		A glossary of reserved JSON symbols and the equivalent long-form representation in plain text, e.g. '&' = '&amp'
	*/
	class JSONGlossary : public std::unordered_map<String, String> {
	public:
		typedef std::unordered_map<String, String> base;
		
		/*!
			Default constructor
		*/
		JSONGlossary();
		
		/*!
			Get a replacement for a specified entity
			@param entity The entity to replace
			@return The replacement
		*/
		String getReplacement(const String& entity) const;
	};
	
	
		///JSON processing category error instance
	static JSONCategory instance;

	
		///Make an error code for JSON processing
	inline std::error_code makeJSONError(JSONTransport::Status code) {
		return std::error_code(static_cast<int>(code), instance);
	}
	
	
	/*--------------------------------------------------------------------
		Convert a string to a JSON string, i.e. translating special chars etc

		source: The string to convert
	 
		return: A reference to the converted string
	  --------------------------------------------------------------------*/
	String& toJSONString(String& source, JSONGlossary& glossary) {
			//We need to replace the JSON escape char first (and separately) to ensure subsequent escaped chars aren't affected
		source.replaceAll(escapeStr, escapeCharSymbol);
		for (auto& i : glossary)
			if (i.second != escapeStr)
				source.replaceAll(i.second, i.first);
		return source;
	} //toJSONString
	
	
	/*--------------------------------------------------------------------
		Convert an JSON string to a regular string, i.e. translating special chars etc

		source: The string to convert
	 
		return: A reference to the converted string
	  --------------------------------------------------------------------*/
	String& fromJSONString(String& source, JSONGlossary& glossary) {
		String::sizeOption index = 0;
		if (!source.find(escapeStr, *index))
			return source;
		BufferIn sourceBuffer{source};
		String output;
		output.reserve(source.dataSize());
		while (sourceBuffer.find(escapeChar, &output, true)) {
			String entity;
			sourceBuffer.getString(entity, 1);
			if ((entity == "u") && !sourceBuffer.getString(entity, 4))	//Hex char code
				throw std::system_error(makeJSONError(badEncoding));
			output += glossary.getReplacement(entity);
		}
		source = std::move(output);
		return source;
	} //fromJSONString
	
	
		///Identification type for JSON elements
	struct JSONIdentity : Identity {
		
		// MARK: Types
		
			///Enumeration of JSON element tag types
		enum class Type {
			undefined,		///<No type identified
			objectStart,	///<Object start brace, i.e. {
			arrayStart,		///<Array start brace, [
			valueStart,		///<An item value, e.g. "Ralph"
			nullItem,		///<A 'null' for value/object/array content
			delimiter,		///<Value delimiter, i.e. ,
			objectEnd,		///<Object end brace, i.e. }
			arrayEnd,		///<Array end brace, i.e. ]
		};

			///Enumeration of JSON parsing stages
		enum class Stage {
			root,		///<A new element is expected, either a new object, array or (unnamed) value
			array,		///<Within an array - same as root condition, but different terminator expected
			object,		///<Within an object - a named value is expected
			complete,	///<An element has been read - either a terminator or delimiter for the next value is expected
		};
	
		// MARK: Constructors
		
		/*!
			Default constructor
			@param identity The element identity
			@param tagType The tag type
		*/
		JSONIdentity(const Identity& identity = Identity(), Type tagType = Type::undefined) : Identity(identity) {
			if (const auto* jsonIdentity = dynamic_cast<const JSONIdentity*>(&identity); jsonIdentity != nullptr) {
				type = jsonIdentity->type;
				stage = jsonIdentity->stage;
			} else
				type = tagType;
		}
		/*!
			Constructor
			@param tagType The tag type
		*/
		JSONIdentity(Type tagType) : Identity() { type = tagType; }
		
		// MARK: Public variables
		
			///The element type
		Type type = Type::undefined;
			///The stage at which the identity is found
		Stage stage = Stage::root;
		
		// MARK: Functions (mutating)
		
		/*!
			Set the identity tag as the hierarchy root
			@return A reference to this
		*/
		JSONIdentity& atStage(Stage newStage) {
			stage = newStage;
			return *this;
		}
		
		/*!
			Set the identity tag type
			@param tagType The tag type
			@return A reference to this
		*/
		JSONIdentity& withType(Type tagType) {
			type = tagType;
			return *this;
		}
	};
	
	using enum JSONIdentity::Type;
	using enum JSONIdentity::Stage;

	
	/*!
		Utility class to write data in JSON format, e.g. tags, namespaces, entity insertion
	*/
	class JSONExporter {
	public:
		
		// MARK: Constructors
		
		/*!
			Constructor
			@param dest The data destination
			@param glossary The JSON entity glossary
		*/
		JSONExporter(BufferOut& dest, JSONGlossary& glossary, JSONTransport::TimeFormat format, bool everyEntryRequired) :
				m_buffer(dest), m_glossary(glossary), timeFormat{format}, isEveryEntryRequired{everyEntryRequired} {
		}
			///No copy constructor
		JSONExporter(const JSONExporter& source) = delete;
		
		// MARK: Public variables
		
		bool isEveryEntryRequired = false;
			///True if the exported JSON should be indented with tabs
		bool isTabbed = false;
			///True if lines in the exported JSON should be terminated with line-feeds
		bool isLineFeeds = false;
			///True if exported JSON tags should be prefixed with a namespace (when supplied)
		bool isNameSpaces = true;
			///The specified date/time format
		JSONTransport::TimeFormat timeFormat;
		
		// MARK: Functions (const)
		
		/*!
			Get the JSON glossary
			@return The JSON glossary
		*/
		JSONGlossary& glossary() const { return m_glossary; }
		
		// MARK: Functions (mutating)
		
		/*!
			Write the specified string
			@param toWrite The string to write
		*/
		void write(const String& toWrite);
		/*!
			Write a tag to the data destination
			@param tag The tag to write
		 	@param nameSpace The tag namespace.
			@param type The tag type
			@param depth The tag depth in the JSON hierarchy
		*/
		void writeTag(const String& tag, const String::Option& nameSpace, JSONIdentity::Type type, int32_t depth);
		/*!
			Write a phrase to the data destination
			@param phrase The phrase to write
		*/
		void writePhrase(const String& phrase);
		/*!
			Flush the buffer to the destination
		*/
		void flush() {
			if (!m_buffer.flush())
				throw std::system_error(makeJSONError(badDestination));
		}
		/*!
			Add an entity to the glossary
			@param entity The entity to add
			@param text The replacement text for the entity
		*/
		void addEntity(const String& entity, const String text);
		
	private:
			///A buffer for the exported data (wraps the export destination)
		BufferOut& m_buffer;
			///A glossary of JSON entities and replacement text encountered for faster lookup
		JSONGlossary& m_glossary;
	};
	
	
		//Class to parse and import data from JSON
	class JSONImporter {
	public:
		
		// MARK: Constructors
		
		/*!
			Constructor
			@param source The data source
			@param glossary The JSON entity glossary
			@param isUnknownNameSkipped True if unknown names found in the JSON should be skipped
		*/
		JSONImporter(BufferIn& source, JSONGlossary& glossary,
					 bool isUnknownNameSkipped, bool isEveryEntryRequired, bool isMissingEntryFailed) :
				m_buffer(source), m_glossary(glossary), m_isUnknownNameSkipped(isUnknownNameSkipped),
				m_isEveryEntryRequired(isEveryEntryRequired), m_isMissingEntryFailed(isMissingEntryFailed) {}
			///No copy constructor
		JSONImporter(const JSONImporter& source) = delete;
		
		// MARK: Functions (const)
		
		/*!
			Determine if unknown names should be skipped
		 	@return True if unknown names should be skipped
		*/
		bool isUnknownSkipped() const { return m_isUnknownNameSkipped; }
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
		/*!
			Determine if an error has occurred
			@return True if an error has occurred
		*/
		bool isError() const noexcept { return m_status != nominal; }
		/*!
			Get the current read position in the source data (not the read position in the buffer)
			@return The read position (e.g. the read position in a source file)
		*/
		Memory::size_type getPosition() const { return m_buffer.getPosition(); }
		/*!
			Get the last received character row position of the data source
			@return The last row position received from the data source
		*/
		Memory::size_type getLastRow() const { return m_buffer.getLastRow(); }
		/*!
			Get the last received character column position of the data source
			@return The last column position received from the data source
		*/
		Memory::size_type getLastColumn() const { return m_buffer.getLastColumn(); }
		/*!
			Get the encoding of the JSON inout stream
		 	@return The JSON text encoding
		*/
		TextEncoding getEncoding() const { return m_buffer.textEncoding(); }
		/*!
			Set the current read position in the source data (not the read position in the buffer)
			@param pos The read position (e.g. the read position in a source file)
		*/
		void setPosition(Memory::size_type pos) const { m_buffer.setPosition(pos); }
		/*!
			Get the transport status
			@return The transport status (nominal = no errors)
		*/
		JSONTransport::Status getStatus() const { return m_status; }
		
		// MARK: Functions (mutating)
		
		/*!
			Get the importer glossary
			@return The importer glossary
		*/
		JSONGlossary& glossary() { return m_glossary; }
		/*!
			Get an element identity from the data source
			@param stage The JSON import stage
			@return The element identity
		*/
		JSONIdentity getIdentity(JSONIdentity::Stage stage);
		/*!
			Get a value from the data source, e.g. the data between quotes
			@return The JSON value
		*/
		String getValue();
		/*!
			Get item content from the data source
			@param item The item to receive the content
		*/
		void getContent(Item& item);
		/*!
			Add an entity to the glossary
			@param entity The entity to add
			@param text The replacement text for the entity
		*/
		void addEntity(const String& entity, const String text) { m_glossary[entity] = text; }
		/*!
			Set the encoding of the JSON inout stream
		 	@param format The source data format
		*/
		void setFormat(DataFormat format) { m_buffer.setFormat(format); }
		/*!
			Set the transport status
			@param status The transport status (nominal = no errors)
		*/
		void setStatus(JSONTransport::Status status) { m_status = status; }
		
	private:
			///The JSON source buffer
		BufferIn& m_buffer;
			///Glossary of JSON entities
		JSONGlossary& m_glossary;
			//The current transport status
		JSONTransport::Status m_status = nominal;
			//True if unknown tags should be skipped over
		bool m_isUnknownNameSkipped = false;
			//True if all inventory entries should be treated as 'required'
		bool m_isEveryEntryRequired = false;
			//True if a receive operation should be failed if an entry marked 'required' is not found
		bool m_isMissingEntryFailed = false;
	};
	
	
	/*--------------------------------------------------------------------
		Default constructor
	  --------------------------------------------------------------------*/
	JSONGlossary::JSONGlossary() : base() {
			//Standard JSON entities
		(*this)["\\\\"] = "\\";
		(*this)["\\\""] = "\"";
		(*this)["\\/"] = "/";
		(*this)["\\b"] = "\b";
		(*this)["\\f"] = "\f";
		(*this)["\\n"] = "\n";
		(*this)["\\r"] = "\r";
		(*this)["\\t"] = "\t";
	} //JSONGlossary::JSONGlossary
	
	
	/*--------------------------------------------------------------------
		Get a replacement for a specified entity
	 
		entity: The entity to replace
	 
		return: The replacement
	  --------------------------------------------------------------------*/
	String JSONGlossary::getReplacement(const String& entity) const {
		if (auto i = find(escapeStr + entity); i != end())
			return i->second;
		uint32_t charCode = 0;
		if (entity[0] == U'u') {	//Hex char code
			if (auto entityCode = HexTransport().receive(entity.substr(1)); entityCode)
				charCode = *entityCode;
			else
				throw std::system_error(makeJSONError(unknownEscapeChar));
		}
		String result{reinterpret_cast<char16_t*>(&charCode), 1};
		if (result.empty())
			throw std::system_error(makeJSONError(badEncoding));
		return result;
	} //JSONGlossary::getReplacement
	

	/*--------------------------------------------------------------------
		Get an element identity from the data source
	
		stage: The JSON import stage
	 
		return: The element identity
	  --------------------------------------------------------------------*/
	JSONIdentity JSONImporter::getIdentity(JSONIdentity::Stage stage) {
		if (!m_buffer.findIf([](char32_t uniChar){ return !isWhiteSpace(uniChar); }))
			return undefined;
		auto leader = m_buffer.getEncodedChar();
		if (leader.second == 0)
			return undefined;
		if (!m_buffer)
			throw std::system_error(makeJSONError(badSource));
		switch (stage) {
			case root: case array:	//Either at the document root or in an array
				switch (leader.first) {
					case objectLeader:
						return objectStart;
					case arrayLeader:
						return arrayStart;
					case arrayTerminator:
						return arrayEnd;
					case valueDelimiter:
						return delimiter;
				}
				if (valueLeaders.find(leader.first) == std::u32string::npos)
					throw std::system_error(makeJSONError(badValue));
					//Check for a null item
				if (leader.first == nullLeader) {
					String text{"n"};
					m_buffer.findIf([](char32_t uniChar){ return isValueTerminator(uniChar); }, &text);
					if (text != nullValue)
						throw std::system_error(makeJSONError(badValue));
					return nullItem;
				}
				m_buffer.rewind(leader.second);	//Put the leading value back into the buffer
				return valueStart;
			case object: {	//In an object
				if (leader.first == valueDelimiter)
					return delimiter;
				if (leader.first == objectTerminator)
					return objectEnd;
				if (leader.first != textLeader)
					throw std::system_error(makeJSONError(nameMissing));
				JSONIdentity identity;
				if (!m_buffer.findFirstOf("\"", &identity.name, false, false, true, false, escapeChar) || identity.name.empty() ||
						!m_buffer.findFirstOf(":", nullptr, false, false, true))
					throw std::system_error(makeJSONError(nameMissing));
				fromJSONString(identity.name, m_glossary);
					//Check if the tag includes a namespace
				if (auto dividerPos = identity.name.rfind(":"); dividerPos) {
					identity.group = identity.name.substr(0, *dividerPos);
					identity.name.erase(0, *dividerPos + 1);
				}
				auto valueIdentity = getIdentity(root);
				identity.type = valueIdentity.type;
				return identity;
			}
			case complete:	//A value has been completed
				if (leader.first == objectTerminator)
					return objectEnd;
				if (leader.first == arrayTerminator)
					return arrayEnd;
				if (leader.first == valueDelimiter)
					return delimiter;
				throw std::system_error(makeJSONError(incompleteContext));
		}
		throw std::system_error(makeJSONError(parsingError));
	} //JSONImporter::getIdentity
	
	
	/*--------------------------------------------------------------------
		Get item content from the data source
	 
		item: The item to receive the content
	  --------------------------------------------------------------------*/
	void JSONImporter::getContent(Item& item) {
		m_buffer.findIf([](char32_t uniChar){ return !isWhiteSpace(uniChar); });
			//First attempt to find a valid JSON value, determining the type according to JSON conventions
		auto content = m_buffer.getEncodedChar();	//Get the first character from the buffer
		if (content.second == 0)
			throw std::system_error(makeJSONError(valueMissing));
		std::unique_ptr<Value> value;
		String text;
			//If we have an opening quote, this must be a text value
		if (content.first == textLeader) {
				//Search for the closing quotes and extract string content
			if (!m_buffer.findFirstOf(textLeaderStr, &text, false, false, true, false, escapeChar))
				throw std::system_error(makeJSONError(closingQuoteMissing));
			value = std::make_unique<StringValue>(fromJSONString(text, m_glossary));
		} else {
			text.append(content.first);
			m_buffer.findIf([](char32_t uniChar){ return isValueTerminator(uniChar); }, &text);
				//Trim trailing white-space chars
			auto lastChar = text.findLastNotOf(String::allWhiteSpace);
			if (!lastChar)
				throw std::system_error(makeJSONError(valueMissing));
			text = text.substr(0, *lastChar + 1);
				//Check for an incoming bool value
			if (text == "true")
				value = std::make_unique<BoolValue>(true);
			else if (text == "false")
				value = std::make_unique<BoolValue>(false);
				//Check for an incoming null value
			else if (text == "null")
				value = std::make_unique<NullValue>();
			else {
					//Finally check for an incoming numeric value - test for chars not complying with an integer
				if (text.findIf([](char32_t uniChar){ return !isNumeric(uniChar); }))
					value = std::make_unique<DoubleValue>();	//Assume a double
				else
					value = std::make_unique<Int64Value>();	//Assume an integer
				*value = text;
				if (value->status == Value::bad)
					throw std::system_error(makeJSONError(badValue));
			}
		}
		if (!value)
			throw std::system_error(makeJSONError(valueMissing));
			//Once a value has been retrieved with a type based on the JSON encoding, we can assign that to the receiving item
		if (!item.read(*value))
			throw std::system_error(makeJSONError(badValue));
	} //JSONImporter::getContent

	
	/*--------------------------------------------------------------------
		Write a tag to the data destination
		
		tag: The tag to write
		namespace: The tag namespace
		type: The tag type
		depth: The tag depth in the JSON hierarchy
	  --------------------------------------------------------------------*/
	void JSONExporter::writeTag(const String& tag, const String::Option& nameSpace, JSONIdentity::Type type, int32_t depth) {
		String jsonStr;
		bool isClosing = (type == objectEnd) || (type == arrayEnd);
		if ((depth > 0) || isClosing) {
			if (isLineFeeds)
				jsonStr.append("\n");
			if (isTabbed)
				jsonStr.append(String(depth, "\t"));
		}
		if (!isClosing) {
				//Write a name when specified
			if (!tag.empty()) {
				jsonStr.append("\"");
				if (nameSpace && !nameSpace->empty()) {
					auto namespaceOut = *nameSpace;
					jsonStr.append(toJSONString(namespaceOut, m_glossary)).append(":");
				}
				auto tagOut = tag;
				jsonStr.append(toJSONString(tagOut, m_glossary)).append("\": ");
			}
		}
		switch (type) {
			case objectStart:
				jsonStr.append("{");
				break;
			case arrayStart:
				jsonStr.append("[");
				break;
			case nullItem:
				jsonStr.append(nullValue);
				break;
			case objectEnd:
				jsonStr.append("}");
				break;
			case arrayEnd:
				jsonStr.append("]");
				break;
			default:
				break;
		}
		if (!m_buffer.write(jsonStr))
			throw std::system_error(makeJSONError(badDestination));
	} //JSONExporter::writeTag

	
	/*--------------------------------------------------------------------
		Write the specified string
	 
		toWrite: The string to write
	  --------------------------------------------------------------------*/
	void JSONExporter::write(const String& toWrite) {
		if (toWrite.empty())
			return;	//No data is not an error
		if (!m_buffer.write(toWrite))
			throw std::system_error(makeJSONError(badDestination));
	} //JSONExporter::write
	
	
	/*--------------------------------------------------------------------
		Write a phrase to the data destination
		
		phrase: The phrase to write
	  --------------------------------------------------------------------*/
	void JSONExporter::writePhrase(const String& phrase) {
		String jsonStr(phrase);
		write(toJSONString(jsonStr, m_glossary));
	} //JSONExporter::writePhrase

	
	/*!
		Import the contents of the specified cargo from JSON
		@param container The cargo container to receive the imported data
		@param containerIdentity The container identity
		@param importer The JSON data importer
		@param depth The recursion depth into the JSON hierarchy
	*/
	void doJSONImport(Cargo& container, const JSONIdentity& containerIdentity, JSONImporter& importer, int32_t depth = 0);
	
	
	/*--------------------------------------------------------------------
		Get the identity of an incoming array item

		inventory: The cargo inventory to find the item from
		identity: The identity of the item (set from the inventory details)

		return: The wrapped cargo
	  --------------------------------------------------------------------*/
	void getArrayIdentity(Cargo& container, const Inventory& inventory, const JSONIdentity& containerIdentity, JSONIdentity& identity) {
		if (!identity.name.empty())
			return;	//It already has a name
		auto type = identity.type;	//Preserve the original type
		if ((containerIdentity.type == arrayStart) && !containerIdentity.name.empty())
				//If the outer container is named, use that
			identity = containerIdentity;
		else {
				//Seek an inventory item suited to an array (almost always the sole entry for a typical array container)
			auto iter = std::find_if(inventory.begin(), inventory.end(), [&](auto& i) {
				return i.isRepeating();
			});
			if (iter != inventory.end())
				identity = iter->identity();
		}
		identity.type = type;
	} //getArrayIdentity
	
	
	/*--------------------------------------------------------------------
		Make a wrapper for the  specifed cargo

		cargo: The cargo to be wrapped
		containerIdentity: The cargo identity
		identity: The identity for the wrapped cargo (set from the container)

		return: The wrapped cargo
	  --------------------------------------------------------------------*/
	Cargo::Unique makeWrapper(Cargo& cargo, const JSONIdentity& containerIdentity, const Inventory& inventory, JSONIdentity& identity) {
		if (auto* package = dynamic_cast<Package*>(&cargo); package != nullptr)
			return std::make_unique<PackageWrap>(*package);
		if (auto* item = dynamic_cast<Item*>(&cargo); item != nullptr)
			return std::make_unique<ItemWrap>(*item);
		throw std::out_of_range("");	//Illegal cargo type
	} //makeWrapper
	
	
	/*--------------------------------------------------------------------
		Make a cargo object to represent an unknown, incoming item type
	 
		identity: The item identity
	 
		return: A suitable cargo object
	  --------------------------------------------------------------------*/
	Cargo::Unique makeUnknown(const JSONIdentity& identity) {
		if (identity.type == valueStart)
			return std::make_unique<UnknownItem>();
		return std::make_unique<Unknown>();
	} //makeUnknown
	
	
	/*--------------------------------------------------------------------
		Get the inventory for a container to receive imported data
	 
		container: The cargo container
		isEveryEntryRequired: True if all inventory items should be marked as 'required'
	 
		return: The completed inventory
	  --------------------------------------------------------------------*/
	Inventory getImportInventoryFor(Cargo& container, bool isEveryEntryRequired) {
		Inventory inventory;
		if (!container.fillInventory(inventory) && (dynamic_cast<Item*>(&container) == nullptr))
			throw std::system_error(makeJSONError(missingInventory));
		inventory.resetAvailable();	//Reset the availability of each entry to zero so we can count incoming items
		if (isEveryEntryRequired)
			inventory.setAllRequired();
		return inventory;
	} //getImportInventoryFor
	
	
	/*--------------------------------------------------------------------
		Import the contents of the specified cargo from JSON
	 
		container: The cargo container to receive the imported data
		containerIdentity: The container identity
		importer: The JSON data importer
		depth: The recursion depth into the JSON hierarchy
	  --------------------------------------------------------------------*/
	void doJSONImport(Cargo& container, const JSONIdentity& containerIdentity, JSONImporter& importer, int32_t depth) {
		Inventory inventory = getImportInventoryFor(container, importer.isEveryEntryRequired());
		auto attributesRemaining = inventory.attributeSize(true);	//This is tracked where the container requires attributes first
		auto parsingStage = containerIdentity.stage;
		auto* package = dynamic_cast<Package*>(&container);
		auto isReadingAttribute = (package != nullptr) && package->isAttributeFirst();
		std::optional<Memory::size_type> restorePoint;
		auto loopScope = defer([&importer, &inventory]{
			if (importer.isMissingEntryFailed() && (inventory.countRequired() > 0))
				importer.setStatus(instanceMissing);
		});
		for (;;) {	//We break out of this loop when an error occurs or we run out of data
			Memory::size_type readPoint = importer.getPosition();
			auto identity = importer.getIdentity(parsingStage);	//Get the identity of the next item in the JSON source
			switch (identity.type) {
				case undefined:	//End of file
					if (depth != 0)	//Failure if tags haven't been balanced correctly
						throw std::system_error(makeJSONError(unbalancedScope));
					return;
				case nullItem:
					parsingStage = complete;	//We're going to skip the null items completely
					continue;
				case delimiter:
					if (parsingStage != complete)	//A delimiter has been found before anything was read
						throw std::system_error(makeJSONError(unbalancedScope));
					parsingStage = containerIdentity.stage;
					continue;	//Move onto the next item
				case objectStart: case valueStart: case arrayStart: {
					if (parsingStage == complete)	//An element has been read, but no delimiter reached - expected a closing symbol
						throw std::system_error(makeJSONError(unbalancedScope));
					Cargo::Unique cargo;
					Inventory::iterator incomingItem = inventory.end();
					if (parsingStage == array)
						getArrayIdentity(container, inventory, containerIdentity, identity);
					if ((parsingStage == root) || (identity.type == arrayStart))	//Att root/array we're importing to the container we already have
						cargo = makeWrapper(container, containerIdentity, inventory, identity);
					else {
						incomingItem = inventory.registerIncoming(identity);	//Seek the incoming element in the inventory
						if (incomingItem != inventory.end()) {
							if (isReadingAttribute && !incomingItem->isAttribute())
								incomingItem = inventory.end();
							else {
								if (!incomingItem->bumpAvailable())
									throw std::system_error(makeJSONError(inventoryBoundsExceeded));
								incomingItem->required = false;	//Does not change import behaviour - flags that we have found at least one instance
								if ((attributesRemaining > 0) && incomingItem->isAttribute() && incomingItem->required)
									--attributesRemaining;
								cargo = (incomingItem == inventory.end()) ? nullptr : container.getCargo(*incomingItem);
							}
							if (cargo)
								cargo->setDefault();
						}
					}
					bool isKnown = true;
					if (!cargo) {	//Allow the parser to move beyond unknown/unwanted elements
						if (importer.isUnknownSkipped() || isReadingAttribute) {
							isKnown = false;
							cargo = makeUnknown(identity);
							if (isReadingAttribute && !restorePoint)	//If not all attributes read, parse data twice (first for attributes only)
								restorePoint = readPoint;	//If this is the first instance, set a restore point so reading can resume here
						} else
							throw std::system_error(makeJSONError(unknownName));
					}
					do {	//Fake loop context allows first condition to escape when true
						if (identity.type == valueStart) {
							if (auto* item = dynamic_cast<active::serialise::Item*>(cargo.get()); item != nullptr) {
								importer.getContent(*item);
								break;
							}
						}
						doJSONImport(*cargo, JSONIdentity{identity}.atStage((identity.type == arrayStart) ? array : object), importer, depth + 1);
					} while (false);
					if (incomingItem != inventory.end()) {
						if (incomingItem->isRepeating() && (package != nullptr) && !package->insert(std::move(cargo), *incomingItem))
							throw std::system_error(makeJSONError(invalidObject));
					} else if (isKnown && (identity.type != arrayStart))
						return;	//If there is no defined item, we're in an array or the root - we need to return the imported element now
					parsingStage = complete;	//An element has been parsed - we either expect a delimiter or a terminator
					break;
				}
				case objectEnd: case arrayEnd:
					if (containerIdentity.stage != (identity.type == objectEnd ? object : array))
						throw std::system_error(makeJSONError(unbalancedScope));	//The scope end couldn't be paired with the atart
					if (restorePoint) {
						isReadingAttribute = false;
						importer.setPosition(*restorePoint);	//Move the read position back to the first non-attribute
						restorePoint.reset();
						attributesRemaining = 0;	//It may not be an error is this is not already zero - the container will validate the result
						if (!package->finaliseAttributes())
							throw std::system_error(makeJSONError(invalidObject));
						inventory = getImportInventoryFor(container, importer.isEveryEntryRequired());	//The inventory will probably change here
						parsingStage = object;	//Resuming reading at non-attributes is always in the context of an object
						break;
					}
					if (!container.validate())
						throw std::system_error(makeJSONError(invalidObject));	//The incoming data was rejected as invalid
					return;
			}
		}
	} //doJSONImport
	
	
	/*--------------------------------------------------------------------
		Export cargo to JSON
	
		cargo: The cargo to export
		identity: The cargo identity
		exporter: The JSON exporter
	  --------------------------------------------------------------------*/
	void doJSONExport(const Cargo& cargo, const JSONIdentity& identity, JSONExporter& exporter, int32_t depth = 0) {
		using enum JSONIdentity::Type;
		String tag, nameSpace;
		if (identity.stage != root) {
			if (identity.name.empty())	//Non-root values, i.e. values embedded in an object, must have an identifying name
				throw std::system_error(makeJSONError(nameMissing));
				//Formulate and write the identifying name
			tag = identity.name;
			if (exporter.isNameSpaces && identity.group)
				nameSpace = *identity.group;
		}
		const auto* item = dynamic_cast<const Item*>(&cargo);
		Inventory inventory;
			//Single-value items won't specify an inventory (no point)
		if (!cargo.fillInventory(inventory) || (inventory.empty())) {
			exporter.writeTag(tag, nameSpace, valueStart, depth);
			if ((item == nullptr) || item->isNull()) {
				if ((item == nullptr) && (dynamic_cast<const Null*>(&cargo) == nullptr))
					throw std::system_error(makeJSONError(badValue));	//If anything other than a single-value item lands here, it's an error
				exporter.write(nullValue);
				return;
			}
			String outgoing;
				//Check for a time item not matching the current output spec
			if (const auto* timeItem = dynamic_cast<const xml::XMLDateTime*>(item);
						exporter.timeFormat && (timeItem != nullptr) && (timeItem->getFormat() != *exporter.timeFormat)) {
				xml::XMLDateTime formattedTimeItem{*timeItem};
				formattedTimeItem.setFormat(*exporter.timeFormat);	//Set the specified format
				if (!formattedTimeItem.write(outgoing))
					throw std::system_error(makeJSONError(badValue));
			} else if (!item->write(outgoing))
				throw std::system_error(makeJSONError(badValue));
			if (item->type() == Item::text)
				outgoing = "\"" + toJSONString(outgoing, exporter.glossary()) + "\"";
			exporter.write(outgoing);
			return;
		}
		if ((item != nullptr) && (inventory.size() != 1))	//An item can have multiple values but they must all be a homogenous type, e.g. an array
			throw std::system_error(makeJSONError(badValue));
			//Determine if this element acts as an object/array wrapper for values
			//The package will have an outer object wrapper (even if an array) if the outer element has a name that differs from the inner item
		bool isWrapper = (inventory.size() > 1) || (identity.stage == root) ||
				(!identity.name.empty() && !inventory.begin()->identity().name.empty() && (inventory.begin()->identity() != identity));
			//An array package will have a single item within more than one possible value
		bool isArray = (inventory.size() == 1) && !(inventory.begin()->maximum() == 1),
			 isFirstItem = true;
		if (cargo.isNull()) {
			exporter.writeTag(tag, nameSpace, nullItem, depth);
			return;
		}
		if (isArray)
			exporter.writeTag(tag, nameSpace, arrayStart, depth);
		else if (isWrapper)
			exporter.writeTag(tag, nameSpace, objectStart, depth++);
		auto sequence = inventory.sequence();
		for (auto& entry : sequence) {
			auto item = *entry.second;
			if (!exporter.isEveryEntryRequired && (!item.required || (item.available == 0)))
				continue;
			if (isFirstItem)
				isFirstItem = false;
			else
				exporter.write(",");
			auto entryNameSpace{item.identity().group.value_or(String())};
				//Each package item may have multiple available cargo items to export
			auto limit = item.available;
			bool isItemArray = item.isRepeating() && !isArray,
				 isFirstValue = true;
			if (isItemArray)
				exporter.writeTag(item.identity().name, entryNameSpace, arrayStart, depth);
			for (item.available = 0; item.available < limit; ++item.available) {
				auto content = cargo.getCargo(item);
				if (!content)
					break;	//Discontinue an inventory item when the supply runs out
				if (isFirstValue)
					isFirstValue = false;
				else
					exporter.write(",");
				doJSONExport(*content, isItemArray || isArray ? item.identity() : JSONIdentity{item.identity()}.atStage(object),
							 exporter, (dynamic_cast<Package*>(content.get()) == nullptr) ? depth : depth + ((identity.stage == root) ? 0 : 1));
			}
			if (isItemArray)
				exporter.writeTag(String{}, String{}, arrayEnd, depth);
		}
		if (isArray)
			exporter.writeTag(String{}, String{}, arrayEnd, depth);
		else if (isWrapper)
			exporter.writeTag(String{}, String{}, objectEnd, --depth);
	} //doJSONExport

}  // namespace

/*--------------------------------------------------------------------
	Convert a regular string to an JSON string, i.e. translating special chars etc
 
	source: The string to convert
 
	return: The converted string
  --------------------------------------------------------------------*/
String JSONTransport::convertToJSONString(const String& source) {
	JSONGlossary glossary;	
	String result{source};
	return toJSONString(result, glossary);
} //JSONTransport::convertToJSONString


/*--------------------------------------------------------------------
	Convert an JSON string to a regular string, i.e. translating special chars etc
 
	source: The string to convert
 
	return: The converted string
  --------------------------------------------------------------------*/
String JSONTransport::convertFromJSONString(const String& source) {
	JSONGlossary glossary;
	String result{source};
	return fromJSONString(result, glossary);
} //JSONTransport::convertFromJSONString


/*--------------------------------------------------------------------
	Send cargo as JSON to a specified destination
 
	cargo: The cargo to be sent as JSON
	identity: The cargo identity (name, optional namespace)
	destination: The destination buffer for the exported JSON (can target file, memory, string)
	isTabbed: True to indent lines with tabs
	isLineFeeds: True to add line-feeds
	isNameSpaces: True to inclued namespaces (where specified)
  --------------------------------------------------------------------*/
void JSONTransport::send(Cargo&& cargo, const Identity& identity, BufferOut&& destination,
						 bool isTabbed, bool isLineFeeds, bool isNameSpaces, bool isProlog) const {
	if (!isLineFeeds)
		isTabbed = false;	//Tabs would be pointless without line-feeds
	JSONGlossary glossary;
	JSONExporter exporter(destination, glossary, getTimeFormat(), isEveryEntryRequired());
	exporter.isTabbed = isTabbed;
	exporter.isLineFeeds = isLineFeeds;
	exporter.isNameSpaces = isNameSpaces;
	doJSONExport(cargo, JSONIdentity(identity).atStage(root), exporter);
	exporter.flush();
} //JSONTransport::send


/*--------------------------------------------------------------------
	Receive cargo from a specified JSON source
 
	cargo: The cargo to receive the JSON data
	identity: The cargo identity (name, optional namespace)
	source: The JSON source (can be a wrapper for file, memory, string)
  --------------------------------------------------------------------*/
void JSONTransport::receive(Cargo&& cargo, const Identity& identity, BufferIn&& source) const {
	JSONGlossary glossary;
	JSONImporter importer(source, glossary, isUnknownNameSkipped(), isEveryEntryRequired(), isMissingEntryFailed());
	try {
		doJSONImport(cargo, JSONIdentity(identity).atStage(root), importer);
		if (importer.isError())
			throw std::system_error(makeJSONError(importer.getStatus()));
	} catch(...) {
			//In the event of an error, capturing the row/column where parsing ended can help disgnostics
		setLastRow(source.getLastRow());
		setLastColumn(source.getLastColumn());
		throw;
	}
} //JSONTransport::receive
