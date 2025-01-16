/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/XML/XMLTransport.h"

#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Serialise/Item/Item.h"
#include "Active/Serialise/Item/Wrapper/ItemWrap.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/Package/Unknown.h"
#include "Active/Serialise/XML/Package/XMLProlog.h"
#include "Active/Setting/Values/StringValue.h"
#include "Active/Setting/Values/UInt32Value.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/TextEncoding.h"

#include <map>

using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::setting;
using namespace active::utility;

using enum XMLTransport::Status;

namespace {
	
		///The XML escape character
	const String escapeChar{"&"};
		///The XML excape sequence terminator
	const String terminatorChar{";"};
	
		///Category for XML processing errors
	class XMLCategory : public std::error_category {
	public:
			///Category name
		const char* name() const noexcept override {
			return "active::serialise::xml::category";
		}
		/*!
			Get a message for a specified error code
			@param errorCode A XML processing code
			@return The error message for the specified code
		*/
		std::string message(int errorCode) const override {
			switch (static_cast<XMLTransport::Status>(errorCode)) {
				case nominal:
					return "";
				case unknownEscapeChar:
					return "Found an unknown or invalid escaped character";
				case badEncoding:
					return "Found an escaped character with invalid encoding";
				case badSource:
					return "The XML source failed, e.g. corrupt file";
				case unboundedTag:
					return "The closing brace for a tag is missing (>)";
				case unknownSection:
					return "The specified section type is unknown";
				case missingTagName:
					return "The tag name is empty";
				case missingAttributes:
					return "The instruction attributes are missing";
				case attributeEqualMissing:
					return "The equals character is missing in an attribute";
				case attributeQuoteMissing:
					return "The quote character is missing in an attribute";
				case parsingError:
					return "The XML source couldn't be parsed";
				case closingTagMissing:
					return "A closing tag is missing";
				case badName:
					return "An invalid XML tag was processed";
				case badValue:
					return "A value was found, but invalid";
				case badElement:
					return "Element content was rejected as invalid";
				case badDestination:
					return "The XML write destination failed";
				case missingInventory:
					return "An object to be sent/received via XML cannot provide a content inventory";
				case unbalancedScope:
					return "A closing tag is missing";
				case inventoryBoundsExceeded:
					return "Found more instances of a named value/object than the inventory permits";
				case unknownTag:
					return "An unknown tag was found in the XML";
				default:
					return "Unknown/invalid error";
			}
		}
	};

	
		///XML processing category error instance
	static XMLCategory instance;

	
	/*!
		Make an error_code for XML processing
		@param code An XMLTransport error code
	*/
		///
	inline std::error_code makeXMLError(XMLTransport::Status code) {
		return std::error_code(static_cast<int>(code), instance);
	}

	
	/*!
		Determine is a value is within a specified range
		@param val The value to check
		@param min The minimum allowed value
		@param max The maximum allowed value
	*/
	bool isWithin(uint32_t val, uint32_t min, uint32_t max) {
		return (val >= min) && (val <= max);
	}
	
	
	/*!
		Determine if a UTF32 character is a valid leader for an XML name
		@param xmlCharCode The UTF32 character code to check
	*/
	bool isValidXMLNameLeader(uint32_t xmlCharCode) {
		if (xmlCharCode < U'\xF8')
			return (xmlCharCode == U':') || (xmlCharCode == U'_') ||
					isWithin(xmlCharCode, U'A', U'Z') || isWithin(xmlCharCode, U'a', U'z') ||
					isWithin(xmlCharCode, U'\xC0', U'\xD6') || isWithin(xmlCharCode, U'\xD8', U'\xF6');
		else if (xmlCharCode <= U'\xFFFF')
			return isWithin(xmlCharCode, U'\xF8', U'\x2FF') ||
					isWithin(xmlCharCode, U'\x370', U'\x37D') || isWithin(xmlCharCode, U'\x375', U'\x1FFF') ||
					isWithin(xmlCharCode, U'\x200C', U'\x200D') || isWithin(xmlCharCode, U'\x2070', U'\x218F') ||
					isWithin(xmlCharCode, U'\x2C00', U'\x2FEF') || isWithin(xmlCharCode, U'\x3001', U'\xD7FF') ||
					isWithin(xmlCharCode, U'\xF900', U'\xFDCF') || isWithin(xmlCharCode, U'\xFDF0', U'\xFFFD');
		return isWithin(xmlCharCode, U'\x10000', U'\xEFFFF');
	}
	
	
	/*!
		Determine if a UTF32 character is a valid for the core of an XML name (anything apart from the lead character)
		@param xmlCharCode The UTF32 character code to check
	*/
	bool isValidXMLNameCore(uint32_t xmlCharCode) {
		return isValidXMLNameLeader(xmlCharCode) ||
				(xmlCharCode == U'-') || (xmlCharCode == U'.') || isWithin(xmlCharCode, U'0', U'9') || (xmlCharCode == U'\xB7') ||
				isWithin(xmlCharCode, U'\x300', U'\x36F') || isWithin(xmlCharCode, U'\x203F', U'\x2040');
	}
	
	
	/*!
		Validate an XML name (throw exception on failure)
		@param name The name to check
	*/
	void validateXMLName(const String& name) {
		if (name.empty())
			throw std::system_error(makeXMLError(badName));
		std::u32string temp{name};
		if (!isValidXMLNameLeader(temp[0]))
			throw std::system_error(makeXMLError(badName));
		for (auto i = 1; i < temp.size(); ++i)
			if (!isValidXMLNameCore(temp[i]))
				throw std::system_error(makeXMLError(badName));
	} //validateXMLName


	
		///Identification type for XML elements
	struct XMLIdentity : Identity {
		
		// MARK: Types
		
			///Enumeration of XML element tag types
		enum class Type {
			undefined,		///<No type identified
			instruction,	///<Processing instruction, e.g. <? xml version="1.0" ?>
			startTag,		///<Start tag, e.g. <Person>
			emptyTag,		///<Empty tag, e.g. <Person name="Ralph"/>
			dataTag,		///<CDATA section, e.g. <![CDATA[UmFscGggV2Vzc2Vs]]>
			endTag,			///<End tag, e.g. </Person>
			comment,		///<Comment, e.g. <!-- About me -->
		};
		
		// MARK: Constructors
		
		/*!
			Default constructor
			@param identity The element identity
			@param tagType The tag type
		*/
		XMLIdentity(const Identity& identity = Identity(), Type tagType = Type::undefined) : Identity(identity) { type = tagType; }
		/*!
			Constructor
			@param tagType The tag type
		*/
		XMLIdentity(Type tagType) : Identity() { type = tagType; }
		
		// MARK: Public variables
		
			///The element type
		Type type = Type::undefined;
			///True if the identified element is the hierarchy root
		bool isRoot = false;
		
		// MARK: Functions (mutating)
		
		/*!
			Set the identity tag as the hierarchy root
			@return A reference to this
		*/
		XMLIdentity& asRoot() {
			isRoot = true;
			return *this;
		}
		
		/*!
			Set the identity tag type
			@param tagType The tag type
			@return A reference to this
		*/
		XMLIdentity& withType(Type tagType) {
			type = tagType;
			return *this;
		}
	};
		
	using enum XMLIdentity::Type;

	/*!
		A function class to discover XML sections within a document
	*/
	class XMLSection : protected std::map<String, std::pair<XMLIdentity::Type, String> > {
	public:
		typedef std::map<String, std::pair<XMLIdentity::Type, String> > base;
		
		/*!
			Search for an XML section in a specified expression
			@param expression The expression to search for (if a type is found, the section prefix is removed)
			@return The section type paired with the expected suffix (nullopt if not found)
		*/
		std::optional<std::pair<XMLIdentity::Type, String>> search(String& expression) {
			if (empty()) {
					//Known XML sections
				(*this)["?"] = std::make_pair(instruction, "?");
				(*this)["!--"] = std::make_pair(comment, "--");
				(*this)["![CDATA["] = std::make_pair(dataTag, "]]");
			}
			String terminator;
			for (auto& i : *this) {
				if (expression.startsWith(i.first)) {
					expression.erase(0, i.first.length());
					return i.second;
				}
			}
			return std::nullopt;
		}
	};
	

	/*!
		A glossary of reserved XML symbols and the equivalent long-form representation in plain text, e.g. '&' = '&amp'
	*/
	class XMLGlossary : public std::map<String, String> {
	public:
		typedef std::map<String, String> base;
		
		/*!
			Constructor
		*/
		XMLGlossary() : base() {
				//Standard XML entities
			(*this)["lt"] = "<";
			(*this)["amp"] = "&";
			(*this)["gt"] = ">";
			(*this)["quot"] = "\"";
			(*this)["apos"] = "'";
		}
	};
	
	
	/*!
		Utility class to write data in XML format, e.g. tags, namespaces, entity insertion
	*/
	class XMLExporter {
	public:
		
		// MARK: Constructors
		
		/*!
			Constructor
			@param dest The data destination
			@param glossary The XML entity glossary
		*/
		XMLExporter(BufferOut& dest, XMLGlossary& glossary, XMLTransport::TimeFormat format) : m_buffer(dest), m_glossary(glossary) {
			timeFormat = format;
		}
			///No copy constructor
		XMLExporter(const XMLExporter& source) = delete;
		
		// MARK: Public variables
		
			///True if the exported XML should be indented with tabs
		bool isTabbed = false;
			///True if lines in the exported XML should be terminated with line-feeds
		bool isLineFeeds = false;
			///True if exported XML tags should be prefixed with a namespace (when supplied)
		bool isNameSpaces = true;
			///The specified date/time format
		XMLTransport::TimeFormat timeFormat;
		
		// MARK: Functions (const)
		
		/*!
			Get the XML glossary
			@return The XML glossary
		*/
		XMLGlossary& glossary() const { return m_glossary; }
		/*!
			Convert a regular string to an XML string, i.e. translating special chars etc
			@param source The string to convert
			@return A reference to the converted string
		*/
		String& toXMLString(String& source) const;
		
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
			@param depth The tag depth in the XML hierarchy
		*/
		void writeTag(const String& tag, const String::Option& nameSpace, XMLIdentity::Type type, int32_t depth);
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
				throw std::system_error(makeXMLError(badDestination));
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
			///A glossary of XML entities and replacement text encountered for faster lookup
		XMLGlossary& m_glossary;
			///True if the currently writing data between start/end tags
		bool m_isTagged = false;
	};
	
	
		//Class to parse and import data from XML
	class XMLImporter {
	public:
		
		// MARK: Constructors
		
		/*!
			Constructor
			@param source The data source
			@param glossary The XML entity glossary
			@param section The current XML section
			@param isUnknownTagSkipped True if unknown names found in the JSON should be skipped
		*/
		XMLImporter(BufferIn& source, XMLGlossary& glossary, XMLSection& section, bool isUnknownTagSkipped, bool isUnknownInstructionSkipped) :
				m_buffer(source), m_glossary(glossary), m_section(section), m_isUnknownTagSkipped(isUnknownTagSkipped), m_isUnknownInstructionSkipped(isUnknownInstructionSkipped) {}
			///No copy constructor
		XMLImporter(const XMLImporter& source) = delete;
		
		// MARK: Functions (const)
		
		/*!
			Convert an XML string to a regular string, i.e. translating special chars etc
			@param source The string to convert
			@return True if the string was converted without errors
		*/
		String& fromXMLString(String& source) const;
		/*!
			Get a replacement expression from the glossary
			@param entity The entity to replace
			@return The replacement expression (nullopt = bad entity)
		*/
		String getReplacement(const String& entity) const;
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
			Get the encoding of the XML inout stream
		 	@return The XML text encoding
		*/
		TextEncoding getEncoding() const { return m_buffer.textEncoding(); }
		/*!
			Determine if the imported is at the end of the data source
		 	@return True if eof has been reached
		*/
		bool eof() const { return m_buffer.eof(); }
		
		// MARK: Functions (mutating)
		
		/*!
			Get the importer glossary
			@return The importer glossary
		*/
		XMLGlossary& glossary() { return m_glossary; }
		/*!
			Get the importer section dictionary
			@return The importer section dictionary
		*/
		XMLSection& section() { return m_section; }
		/*!
			Get an element identity from the data source
			@return The element identity
		*/
		XMLIdentity getIdentity();
		/*!
			Get content from the data source, e.g. the data between the opening and closing tags
			@return The XML content
		*/
		String::Option getContent();
		/*!
			Add an entity to the glossary
			@param entity The entity to add
			@param text The replacement text for the entity
		*/
		void addEntity(const String& entity, const String text) { m_glossary[entity] = text; }
		/*!
			Set the encoding of the XML inout stream
		 	@param encoding The source text encoding
		*/
		void setEncoding(TextEncoding encoding) { m_buffer.setSourceEncoding(encoding); }
		/*!
			Set the encoding of the XML inout stream
		 	@param format The source data format
		*/
		void setFormat(DataFormat format) { m_buffer.setFormat(format); }
		/*!
			Determine if unknown tags should be skipped
		 	@return True if unknown tags should be skipped
		*/
		bool isUnknownTagSkipped() const { return m_isUnknownTagSkipped; }
		/*!
			Determine if unknown instructions should be skipped
		 	@return True if unknown instructions should be skipped
		*/
		bool isUnknownInstructionSkipped() const { return m_isUnknownInstructionSkipped; }
		
	private:
			///The XML source buffer
		BufferIn& m_buffer;
			///Glossary of XML entities
		XMLGlossary& m_glossary;
			///Current XML section
		XMLSection& m_section;
			//True if unknown tags should be skipped over
		bool m_isUnknownTagSkipped = false;
			//True if unknown instructions should be skipped over
		bool m_isUnknownInstructionSkipped = true;
	};
	
	
	/*--------------------------------------------------------------------
		Convert an XML string to a regular string, i.e. translating special chars etc

		source: The string to convert

		return: True if the string was converted without errors
	  --------------------------------------------------------------------*/
	String& XMLImporter::fromXMLString(String& source) const {
		String::sizeOption index = 0;
		if (!source.find(escapeChar, *index))
			return source;
		BufferIn sourceBuffer{source};
		String output;
		output.reserve(source.dataSize());
		while (sourceBuffer.find(escapeChar, &output, true)) {
			String entity;
			if (!sourceBuffer.find(terminatorChar, &entity, true))
				throw std::system_error(makeXMLError(badEncoding));
			output += getReplacement(entity);
		}
		source = std::move(output);
		return source;
	} //XMLImporter::fromXMLString

	
	/*--------------------------------------------------------------------
		Get a replacement expression from the glossary
	 
		entity: The entity to replace
	 
		return: The replacement expression (nullopt = bad entity)
	  --------------------------------------------------------------------*/
	String XMLImporter::getReplacement(const String& entity) const {
		auto i = m_glossary.find(entity);
		if (i != m_glossary.end())
			return i->second;
		uint32_t charCode = 0;
		if (entity.find("#x") == 0) {	//Hex char code
			if (auto entityCode = HexTransport().receive(entity.substr(2)); entityCode)
				charCode = *entityCode;
			else
				throw std::system_error(makeXMLError(unknownEscapeChar));
		} else if (entity.find("#") == 0) {	//Decimal char code
			auto code = UInt32Value{entity.substr(1)};
			if (!code)
				throw std::system_error(makeXMLError(badEncoding));
			charCode = code.data;
		} else
			throw std::system_error(makeXMLError(unknownEscapeChar));
		String result{reinterpret_cast<char32_t*>(&charCode), 1};
		if (result.empty())
			throw std::system_error(makeXMLError(badEncoding));
		return result;
	} //XMLImporter::getReplacement
	

	/*--------------------------------------------------------------------
		Get an element identity from the data source
	 
		return: The element identity
	  --------------------------------------------------------------------*/
	XMLIdentity XMLImporter::getIdentity() {
		if (!m_buffer.findFirstOf("<", nullptr, false, true, true))
			return undefined;
		if (!m_buffer)
			throw std::system_error(makeXMLError(badSource));	//The input stream has failed
		XMLIdentity identity;
		if (!m_buffer.findFirstOf(">", &identity.name, false, true, true))
			throw std::system_error(makeXMLError(unboundedTag));		//Failure to find a closing char is a fatal error
		if (auto section = m_section.search(identity.name); section) {
			if (!section->second.empty()) {
				if (auto sectionEnd = identity.name.rfind(section->second); sectionEnd)
					identity.name.erase(*sectionEnd);
				else
					throw std::system_error(makeXMLError(unknownSection));		//Failure to find the section terminator is a fatal error
			}
			identity.type = section->first;
		} else {
			if (auto tagEnd = identity.name.findLastNotOf(String::allWhiteSpace); tagEnd && (*tagEnd < (identity.name.length() - 1)))
				identity.name.erase(*tagEnd + 1);
			if (identity.name.empty())
				throw std::system_error(makeXMLError(missingTagName));	//A tag with no content is a fatal error
				//Search for a tag closure
			if (auto closingPos = identity.name.rfind("/"); closingPos) {
					//If the tag closure is at the end, this is an empty tag
				if (closingPos == (identity.name.length() - 1)) {
					identity.name.erase(*closingPos, 1);
					if (identity.name.empty())
						throw std::system_error(makeXMLError(missingTagName));
					return identity.withType(emptyTag);
				} else {
						//Otherwise, a tag closure at the start is an end tag (anything else is probably just content)
					if (auto closingPos = identity.name.find("/"); closingPos == 0) {
						identity.name.erase(0, 1);
						identity.type = endTag;
					} else
						identity.type = startTag;
				}
			} else
				identity.type = startTag;
				//Check if the tag includes a namespace
			if (auto spacePosition = identity.name.findFirstOf(String::allWhiteSpace), dividerPos = identity.name.rfind(":");
					dividerPos && spacePosition && (dividerPos < spacePosition)) {
				identity.group = identity.name.substr(0, *dividerPos);
				identity.name.erase(0, *dividerPos + 1);
			}
		}
		return identity;
	} //XMLImporter::getIdentity
	
	
	/*--------------------------------------------------------------------
		Get content from the data source, e.g. the data between the opening and closing tags
	 
		return: The XML content (nullopt on failure)
	  --------------------------------------------------------------------*/
	String::Option XMLImporter::getContent() {
		String content;
		if (!m_buffer.findIf([](char32_t uniChar){ return uniChar == U'<'; }, &content))
			return std::nullopt;	//Not finding a tag is not necessarily an error - the caller will determine
		return fromXMLString(content);
	} //XMLImporter::getContent
	
	
	/*--------------------------------------------------------------------
		Convert an XML string to a regular string, i.e. translating special chars etc

		source: The string to convert

		return: A reference to the converted string
	  --------------------------------------------------------------------*/
	String& XMLExporter::toXMLString(String& source) const {
		for (auto& i : m_glossary)
			source.replaceAll(i.second, "&" + i.first + ";");
		return source;
	} //XMLExporter::toXMLString

	
	/*--------------------------------------------------------------------
		Write the specified string
	 
		toWrite: The string to write
	  --------------------------------------------------------------------*/
	void XMLExporter::write(const String& toWrite) {
		String::size_type exportLength = toWrite.size();
		if (exportLength == 0)
			return;
		if (!m_buffer.write(toWrite))
			throw std::system_error(makeXMLError(badDestination));
	} //XMLExporter::write

	
	/*--------------------------------------------------------------------
		Write a tag to the data destination
		
		tag: The tag to write
		namespace: The tag namespace
		type: The tag type
		depth: The tag depth in the XML hierarchy
	  --------------------------------------------------------------------*/
	void XMLExporter::writeTag(const String& tag, const String::Option& nameSpace, XMLIdentity::Type type, int32_t depth) {
		String xmlStr(tag), prefix;
		if (m_isTagged && isLineFeeds && (type != endTag))
			prefix.append("\n");
		if (isTabbed && ((type != endTag) || ((type == endTag) && !m_isTagged)))
			prefix.append(String(depth, "\t"));
		prefix.append("<");
		if (type == endTag)
			prefix.append("/");
		else if (type == instruction)
			prefix.append("?");
		if (nameSpace && !nameSpace->empty())
			prefix.append(*nameSpace).append(":");
		xmlStr = prefix + xmlStr;
		if (type == startTag) {
			m_isTagged = true;
			xmlStr.append(">");
		} else {
			if (type == emptyTag)
				xmlStr.append("/");
			else if (type == instruction)
				xmlStr.append("?");
			m_isTagged = false;
			xmlStr.append(">");
			if (isLineFeeds)
				xmlStr.append("\n");
		}
		if (!m_buffer.write(xmlStr))
			throw std::system_error(makeXMLError(badDestination));
	} //XMLExporter::writeTag
	
	
	/*--------------------------------------------------------------------
		Write a phrase to the data destination
		
		phrase: The phrase to write
	  --------------------------------------------------------------------*/
	void XMLExporter::writePhrase(const String& phrase) {
		String xmlStr(phrase);
		write(toXMLString(xmlStr));
	} //XMLExporter::writePhrase

	
	/*!
		Import the contents of the specified cargo from XML
		@param container The cargo container to receive the imported data
		@param containerIdentity The container identity
		@param importer The XML data importer
		@param depth The recursion depth into the XML hierarchy
	*/
	void doXMLImport(Cargo& container, const XMLIdentity& containerIdentity, XMLImporter& importer, int32_t depth = 0);
	

	/*--------------------------------------------------------------------
		Read the attributes of a start tag (as a string)

		startTag: The start tag (which may contain an attributes section - removed on return)
	  --------------------------------------------------------------------*/
	String readAttributes(String& startTag) {
		String attributes;
		BufferIn processor(startTag);
		String tagOut;
		processor.findFirstOf(String::allWhiteSpace, &tagOut, true, true, true);
			//Skip to the start of the first attribute (where available)
		while (processor.findFirstNotOf(String::allWhiteSpace)) {
				//Find the attribute tag
			String tag;
			if (!processor.findFirstOf("=", &tag, false, false, true) || tag.empty())
				throw std::system_error(makeXMLError(attributeEqualMissing));	//An attribute must split the tag and value with "="
			String quoteChar, value;
			if (!processor.get(quoteChar) || ((quoteChar != "\"") && (quoteChar != "'")) || !processor.findFirstOf(quoteChar, &value, false, false, true))
				throw std::system_error(makeXMLError(attributeEqualMissing));	//Value not enclosed with quotes
			attributes += "<" + tag + ">" + value + "</" + tag + ">";
		}
		startTag = std::move(tagOut);
		validateXMLName(startTag);
		return attributes;
	} //readAttributes
	
	
	/*--------------------------------------------------------------------
		Import a cargo item from a serialised XML phrase

		cargo: A cargo item to import the phrase
		phrase: The XML phrase to be imported
	  --------------------------------------------------------------------*/
	void doXMLItemImport(Cargo& cargo, String& phrase) {
		if (!cargo.readSetting(StringValue{phrase}))
			throw std::system_error(makeXMLError(badValue));
	} //doXMLItemImport
	
	
	/*--------------------------------------------------------------------
		Process an XML instruction

		importer: The XML data importer
		dataIn: The imported data
		instruction: The incoming instruction
	 	attributes: The instruction attributes
	  --------------------------------------------------------------------*/
	void processInstruction(XMLImporter& importer, Cargo& dataIn, const XMLIdentity& instruction, const String& attributes) {
			//Note - this method currently looks for the xml instruction only
			//It should be expanded in future to pass instructions to the XML reader
		if (instruction.name.lowercase() != "xml") {
			if (!importer.isUnknownInstructionSkipped())
				throw std::system_error(makeXMLError(unknownInstruction));
			return;
		}
		if (attributes.empty())
			throw std::system_error(makeXMLError(missingAttributes));
		BufferIn stringBuffer(attributes);
		XMLImporter instructionImporter(stringBuffer, importer.glossary(), importer.section(), importer.isUnknownTagSkipped(), false);
		instructionImporter.setEncoding(importer.getEncoding());
		XMLProlog declaration;
		doXMLImport(declaration, instruction, instructionImporter);
		declaration.validate();
		importer.setEncoding(declaration.encoding);
	} //processInstruction
	
	
	/*--------------------------------------------------------------------
		Return specifed cargo in a wrapper

		cargo: The cargo to be wrapped

		return: The wrapped cargo
	  --------------------------------------------------------------------*/
	Cargo::Unique wrapped(Cargo& cargo) {
		if (auto* package = dynamic_cast<Package*>(&cargo); package != nullptr)
			return std::make_unique<PackageWrap>(*package);
		if (auto* item = dynamic_cast<Item*>(&cargo); item != nullptr)
			return std::make_unique<ItemWrap>(*item);
		throw std::out_of_range("");	//Illegal cargo type
	} //wrapped
	
	
	/*--------------------------------------------------------------------
		Import the contents of the specified cargo from XML
	 
		container: The cargo container to receive the imported data
		containerIdentity: The container identity
		importer: The XML data importer
		depth: The recursion depth into the XML hierarchy
	  --------------------------------------------------------------------*/
	void doXMLImport(Cargo& container, const XMLIdentity& containerIdentity, XMLImporter& importer, int32_t depth) {
			//Find out what the container can hold
		Inventory inventory;
		if (!container.fillInventory(inventory) && !container.isItem())
			throw std::system_error(makeXMLError(missingInventory));
		inventory.resetAvailable();	//Reset the availability of each entry to zero so we can count incoming items
		for (;;) {	//We break out of this loop when an error occurs or we run out of data
			String phrase;
			if (auto newContent = importer.getContent(); newContent)
				phrase = std::move(*newContent);	//If there is an inline text, check if the container wants to import it
			else if (!importer.eof())	//If we can't read a valid character but haven't reached the end of the source, something's broken
				throw std::system_error(makeXMLError(parsingError));
			if (!phrase.empty())
				doXMLItemImport(container, phrase);
			auto identity = importer.getIdentity();	//Get the identity of the next item in the XML source
			bool isEmpty = false;
			switch (identity.type) {
				case undefined:	//End of file
					if (depth != 0)	//Failure if tags haven't been balanced correctly
						throw std::system_error(makeXMLError(unbalancedScope));
					return;
				case instruction: {
					auto attributes = readAttributes(identity.name);
					importer.fromXMLString(phrase);
					processInstruction(importer, container, identity, attributes);
					break;
				}
				case comment:
					break;	//Not processing these yet
				case endTag:
					if ((identity.name != containerIdentity.name) ||	//Check this tag pairs with the opening tag
						(containerIdentity.group && (identity.group != containerIdentity.group))) //…and namespace when specified
						throw std::system_error(makeXMLError(unbalancedScope));
					if (!container.validate()) //And the received cargo is valid
						throw std::system_error(makeXMLError(badElement));
					return;
				case dataTag:
					doXMLItemImport(container, phrase);
					break;
				case emptyTag:
					isEmpty = true;	//NB: Deliberate fall-through to startTag (they are essentially the same, just no end tag for an empty tag)
				case startTag: {
					auto attributes = readAttributes(identity.name);
					Cargo::Unique cargo;
					Inventory::iterator incomingItem = inventory.end();
					if (containerIdentity.isRoot) {	//Inventory is for contained items - the outermost transport wrapper (or root) is not included
						if (containerIdentity.name != identity.name)	//Check the root element tag matches the root container identity
							throw std::system_error(makeXMLError(unbalancedScope));
						cargo = wrapped(container);	//The next iteration will inspect the inventory of the root container
					} else {
						incomingItem = inventory.registerIncoming(identity);	//Seek the incoming element in the inventory
						if ((incomingItem == inventory.end()) && inventory.isEveryItemAccepted) {
							if (auto* package = dynamic_cast<Package*>(&container); package != nullptr)
								incomingItem = package->allocate(inventory, identity, containerIdentity);
						}
						cargo = (incomingItem == inventory.end()) ? nullptr : container.getCargo(*incomingItem);
						if (cargo != nullptr) {
							if (!incomingItem->bumpAvailable()) {
								do {
									if (inventory.isEveryItemAccepted) {	//A package with dynamic content might transform an item to an array
										if (auto* package = dynamic_cast<Package*>(&container); package != nullptr) {
											if (incomingItem = package->allocateArray(inventory, incomingItem); incomingItem != inventory.end()) {
												cargo = container.getCargo(*incomingItem);
												incomingItem->bumpAvailable();
												break;	//Continue when transformed
											}
										}
									}
									throw std::system_error(makeXMLError(inventoryBoundsExceeded));
								} while (false);
							}
						} else {
							if (isEmpty)
								break;	//Just skip an unknown empty tag
							else if (importer.isUnknownTagSkipped())
								cargo = std::make_unique<Unknown>();	//Otherwise allow the parser to move beyond unknown/unwanted elements
							else
								throw std::system_error(makeXMLError(unknownTag));
						}
					}
					cargo->setDefault();
					if (!attributes.empty()) {
						BufferIn stringBuffer(attributes);
						XMLImporter elementImporter(stringBuffer, importer.glossary(), importer.section(), importer.isUnknownTagSkipped(), false);
						elementImporter.setEncoding(importer.getEncoding());
						doXMLImport(*cargo, identity, elementImporter);
						if (auto* package = dynamic_cast<Package*>(cargo.get()); (package != nullptr) && !package->finaliseAttributes())
							throw std::system_error(makeXMLError(badElement));
					}
					if (isEmpty) {
						if (!cargo->validate())
							throw std::system_error(makeXMLError(badElement));
					} else
						doXMLImport(*cargo, identity, importer);
					if (incomingItem != inventory.end()) {
						if (incomingItem->isRepeating()) {
							if (auto package = dynamic_cast<Package*>(&container);
									(package != nullptr) && !package->insert(std::move(cargo), *incomingItem))
								throw std::system_error(makeXMLError(badElement));
						}
					}
					break;
				}
			}
		}
	} //doXMLImport
	
	
	/*--------------------------------------------------------------------
		Get the XML attributes for the specified cargo

		exporter: The destination for outgoing XML
		cargo: The cargo
		sequence: The inventory handling sequence
		startTag: The cargo start tag (attributes will be appended)

		return: True if the cargo contains only attributes
	  --------------------------------------------------------------------*/
	bool getContainerAttributes(XMLExporter& exporter, const Cargo& cargo, const Inventory::Sequence& sequence, String& startTag) {
		bool isAttributes = true;
		for (const auto& entry : sequence) {
			const auto& item{*entry.second};
			if (!item.required)
				continue;
			if (!item.isAttribute()) {
				isAttributes = false;
				continue;
			}
			auto content = cargo.getCargo(item);
			if (!content || !content->isItem())
				continue;
			String value;
			if (!content->write(value))
				continue;
			String attribute{" "};
			if (exporter.isNameSpaces && item.identity().group && !item.identity().group->empty())
				attribute.append(*item.identity().group).append(":");
			attribute.append(item.identity().name).append("=\"").append(exporter.toXMLString(value)).append("\"");
			startTag += attribute;
		}
		return isAttributes;
	} //getContainerAttributes
	
	
	/*--------------------------------------------------------------------
		Export cargo to XML
	
		cargo: The cargo to export
		identity: The cargo identity
		exporter: The XML exporter
	
		return: True if the export process completed without errors
	  --------------------------------------------------------------------*/
	void doXMLExport(const Cargo& cargo, const XMLIdentity& identity, XMLExporter& exporter, int32_t depth = 0) {
		validateXMLName(identity.name);
		Inventory inventory;
			//Single-value items won't specify an inventory (no point)
		if (!cargo.fillInventory(inventory) || (inventory.empty())) {
			if (!cargo.isItem())
				throw std::system_error(makeXMLError(badValue));
			String outgoing;
			if (!cargo.write(outgoing))
				throw std::system_error(makeXMLError(badValue));
			exporter.writeTag(identity.name, exporter.isNameSpaces ? identity.group.value_or(String()) : String(), startTag, depth);
			exporter.writePhrase(outgoing);
			exporter.writeTag(identity.name, identity.group, endTag, depth);
			return;
		}
		bool isWrapperTag = true;
		if (cargo.isItem()) {
			if (inventory.size() != 1)
				throw std::system_error(makeXMLError(badValue));
		}
		if (inventory.size() == 1) {
			auto itemIdentity = inventory.front().identity();
			isWrapperTag = !identity.name.empty() && !itemIdentity.name.empty() && (itemIdentity != identity);
		}
		auto sequence = inventory.sequence();
		auto itemDepth = depth;
		if (isWrapperTag) {
			String openingTag(identity.name);
			auto isAttributes = getContainerAttributes(exporter, cargo, sequence, openingTag) || (identity.type == instruction);
			auto tagType = (isAttributes) ? ((identity.type == instruction) ? instruction : emptyTag) : startTag;
			exporter.writeTag(openingTag, identity.group, tagType, depth);
			if (isAttributes)
				return;
			++itemDepth;
		}
		for (auto& entry : sequence) {
			auto item = *entry.second;
			if ((!item.required) || (item.isAttribute()))
				continue;
			auto entryNameSpace{item.identity().group.value_or(String())};
				//Each cargo container may contain multiple export items
			auto limit = item.available;
			for (item.available = 0; item.available < limit; ++item.available) {
				if (auto content = cargo.getCargo(item); content) {
					doXMLExport(*content, isWrapperTag ? item.identity() : identity, exporter, itemDepth);
				} else
					break;	//Discontinue an inventory item when the supply runs out
			}
		}
			//Non-items write a closing tag
		if (isWrapperTag)
			exporter.writeTag(identity.name, identity.group, endTag, depth);
	} //doXMLExport

}  // namespace

/*--------------------------------------------------------------------
	Send cargo as XML to a specified destination
 
	cargo: The cargo to be sent as XML
	identity: The cargo identity (name, optional namespace)
	destination: The destination buffer for the exported XML (can target file, memory, string)
	isTabbed: True to indent lines with tabs
	isLineFeeds: True to add line-feeds
	isNameSpaces: True to include namespaces (where specified)
	isProlog: True if an XML prolog should be written, e.g. <?xml encoding="utf-8" version="1.0"?>
 
	return: True if the cargo was successfully sent as XML
  --------------------------------------------------------------------*/
void XMLTransport::send(Cargo&& cargo, const Identity& identity, BufferOut&& destination,
						bool isTabbed, bool isLineFeeds, bool isNameSpaces, bool isProlog) const {
	if (!isLineFeeds)
		isTabbed = false;	//Tabs would be pointless without line-feeds
	XMLGlossary glossary;
	XMLExporter exporter(destination, glossary, getTimeFormat());
	exporter.isTabbed = isTabbed;
	exporter.isLineFeeds = isLineFeeds;
	exporter.isNameSpaces = isNameSpaces;
	if (isProlog)
		doXMLExport(XMLProlog(), XMLIdentity(XMLProlog::tag, instruction), exporter);
	doXMLExport(cargo, identity, exporter);
	exporter.flush();
} //XMLTransport::send


/*--------------------------------------------------------------------
	Receive cargo from a specified XML source
 
	cargo: The cargo to receive the XML data
	identity: The cargo identity (name, optional namespace)
	source: The XML source (can be a wrapper for file, memory, string)
 
	return: True if the cargo was successfully received
  --------------------------------------------------------------------*/
void XMLTransport::receive(Cargo&& cargo, const Identity& identity, BufferIn&& source) const {
	XMLGlossary glossary;
	XMLSection section;
	XMLImporter importer(source, glossary, section, isUnknownNameSkipped(), m_isUnknownInstructionSkipped);
	try {
		doXMLImport(cargo, XMLIdentity(identity).asRoot(), importer);
	} catch(...) {
			//In the event of an error, capturing the row/column where parsing ended can help disgnostics
		setLastRow(source.getLastRow());
		setLastColumn(source.getLastColumn());
		throw;
	}
} //XMLTransport::receive
