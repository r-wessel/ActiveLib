/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_VALUE_SETTING
#define ACTIVE_SETTING_VALUE_SETTING

#include "Active/Container/Vector.h"
#include "Active/Setting/Setting.h"
#include "Active/Setting/Values/Value.h"
#include "Active/Setting/Values/ValueBase.h"

#include <set>
#include <unordered_set>

namespace active::setting {

		///Setting containing any number of values, e.g. bool, string etc.
	class ValueSetting : public setting::Setting, public container::Vector<Value> {
	public:
		
		// MARK: - Types
		
			//The base class of this setting
		using base = container::Vector<Value>;
			///Unique pointer
		using Unique = std::unique_ptr<ValueSetting>;
			///Shared pointer
		using Shared = std::shared_ptr<ValueSetting>;
			///Optional
		using Option = std::optional<ValueSetting>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		ValueSetting() {}
		/*!
			Constructor
			@param nameID The setting identifier
		*/
		explicit ValueSetting(const utility::NameID& nameID) : Setting{nameID} {}
		/*!
			Constructor
			@param value A value to populate into the setting
			@param rows The number of rows of this value
			@param cols The number of columns of this value
		*/
		ValueSetting(const Value& value, size_t rows = 1, size_t cols = 1);
		/*!
			Constructor
			@param value A value to populate into the setting
			@param nameID The setting identifier
		*/
		ValueSetting(Value&& value, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param values An array of values to populate into the setting
			@param nameID The setting identifier
		*/
		template<typename T> requires std::copyable<T>
		ValueSetting(const std::vector<T>& values, const utility::NameID::Option nameID = std::nullopt) : setting::Setting(nameID) {
			for (auto& value: values)
				emplace_back(ValueBase<T>(value));
		}
		/*!
			Constructor
			@param values An array of values to populate into the setting
			@param nameID The setting identifier
		*/
		template<typename T> requires std::copyable<T>
		ValueSetting(const std::set<T>& values, const utility::NameID::Option nameID = std::nullopt) : setting::Setting(nameID) {
			for (auto& value: values)
				emplace_back(ValueBase<T>(value));
		}
		/*!
			Constructor
			@param values An array of values to populate into the setting
			@param nameID The setting identifier
		*/
		template<typename T> requires std::copyable<T>
		ValueSetting(const std::unordered_set<T>& values, const utility::NameID::Option nameID = std::nullopt) : setting::Setting(nameID) {
			for (auto& value : values)
				emplace_back(ValueBase<T>{value});
		}
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(bool val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(int32_t val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(uint32_t val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(int64_t val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(double val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(const utility::Guid& val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(const utility::String& val, const utility::NameID::Option nameID = std::nullopt);
		/*!
			Constructor
			@param val The value
			@param nameID The setting identifier
		*/
		explicit ValueSetting(const utility::Time& val, const utility::NameID::Option nameID = std::nullopt);

			///Use default copy constructor
		ValueSetting(const ValueSetting& source) = default;
			///Use default copy constructor
		ValueSetting(ValueSetting&& source) noexcept;

		/*!
			Destructor
		*/
		virtual ~ValueSetting() = default;

		/*!
			Clone method
			@return A clone of this object
		*/
		ValueSetting* clonePtr() const override	{ return new ValueSetting(*this); }

		// MARK: - Operators (const)
		
		/*!
			Subscript operator (const)
			@param row The value row
			@param col The value column
		 	@return The specified value
		*/
		const Value& operator()(size_t row = 0, size_t col = 0) const { return *(*this)[getValueIndex(row, col)]; }
		/*!
			Equality operator
			@param ref The value to compare
		 	@return True if the values are identical
		*/
		bool operator==(const Setting& ref) const override;
		/*!
			Less-than operator
			@param ref The value to compare
		 	@return True if this is less than ref
		*/
		bool operator<(const Setting& ref) const override;

		// MARK: - Operators (mutating)
		
		/*!
			Subscript operator (mutating)
			@param row The value row
			@param col The value column
		 	@return The specified value
		*/
		Value& operator()(size_t row = 0, size_t col = 0) { return *(*this)[getValueIndex(row, col)]; }
		/*!
			Assignment operator
			@param source The object to copy
		 	@return A reference to this
		*/
		Setting& operator=(const Setting& source) override;
		ValueSetting& operator=(ValueSetting&& source) = default;

		// MARK: - Conversion operators
		
		/*!
			Get a boolean value
			@return A boolean value
		*/
		operator bool() const { return getValue<bool>(); }
		/*!
			Get a 32-bit integer value
			@return A 32-bit integer value
		*/
		operator int32_t() const { return getValue<int32_t>(); }
		/*!
			Get a 32-bit integer value
			@return A 32-bit integer value
		*/
		operator uint32_t() const { return getValue<uint32_t>(); }
		/*!
			Get a 64-bit integer value
			@return A 64-bit integer value
		*/
		operator int64_t() const { return getValue<int64_t>(); }
		/*!
			Get a double precision value
			@return A double precision value
		*/
		operator double() const { return getValue<double>(); }
		/*!
			Get a string value
			@return A string value
		*/
		operator active::utility::String() const { return getValue<utility::String>(); }
		/*!
			Get a guid value
			@return A guid value
		*/
		operator active::utility::Guid() const { return getValue<utility::Guid>(); }
		/*!
			Get a time value
			@return A time value
		*/
		operator active::utility::Time() const { return getValue<utility::Time>(); }
		
		/*!
			Get an array of values
			@return An array of values
		*/
		template<typename T>
		operator std::vector<T>() const {
			std::vector<T> result;
			for (const auto& value : *this)
				result.push_back(*value);
			return result;
		}

		// MARK: - Functions (const)
		
		/*!
			Get the default value type
			@return The default value type (nullopt = undefined)
		*/
		std::optional<Value::Type> getDefaultType() const { return m_type; }
		/*!
			Get the number of columns the values are notionally arranged in
			@return The number of columns
		*/
		size_t getColumnSize() const { return m_columns; }
		
			///True if the setting is empty
		bool empty() const override	{ return base::empty(); }
		/*!
			Determine if a value is good
			@param row the value row
			@param col the value column
			@return True if the value is good (status == good)
		*/
		bool isGood(size_t row = 0, size_t col = 0) const { return (*this)[getValueIndex(row, col)]->isGood(); }
		/*!
			Get the status of a value
			@param row the value row
			@param col the value column
			@return The value status
		*/
		Value::Status status(size_t row = 0, size_t col = 0) const { return (*this)[getValueIndex(row, col)]->status; }
		/*!
			Get a boolean value
			@return A boolean value
		*/
		bool boolVal(size_t row = 0, size_t col = 0) const { return getValue<bool>(row, col); }
		/*!
			Get a 32-bit integer value
			@return A 32-bit integer value
		*/
		int32_t int32Val(size_t row = 0, size_t col = 0) const { return getValue<int32_t>(row, col); }
		/*!
			Get a 32-bit integer value
			@return A 32-bit integer value
		*/
		uint32_t uint32Val(size_t row = 0, size_t col = 0) const { return getValue<uint32_t>(row, col); }
		/*!
			Get a 64-bit integer value
			@return A 64-bit integer value
		*/
		int64_t int64Val(size_t row = 0, size_t col = 0) const { return getValue<int64_t>(row, col); }
		/*!
			Get a double precision value
			@return A double precision value
		*/
		double doubleVal(size_t row = 0, size_t col = 0) const { return getValue<double>(row, col); }
		/*!
			Get a string value
			@return A string value
		*/
		active::utility::String stringVal(size_t row = 0, size_t col = 0) const { return getValue<utility::String>(row, col); }
		/*!
			Get a guid value
			@return A guid value
		*/
		active::utility::Guid guidVal(size_t row = 0, size_t col = 0) const { return getValue<utility::Guid>(row, col); }
		/*!
			Get a time value
			@return A time value
		*/
		active::utility::Time timeVal(size_t row = 0, size_t col = 0) const { return getValue<utility::Time>(row, col); }

		// MARK: - Functions (mutating)
		
		/*!
			Set the default value type
			@param type The default value type (nullopt = undefined)
		*/
		void setDefaultType(std::optional<Value::Type> type = std::nullopt) { m_type = type; }
		/*!
			Set the number of columns the values are notionally arranged in
			@param size The number of columns
		*/
		void setColumnSize(size_t size) { m_columns = std::max(base::size(), std::min(size_t(1), size)); }
		
		/*!
			Clear the setting (remove any values)
		*/
		void clear() override	{ base::clear(); }
		/*!
			Assign a default to the setting
		*/
		void setDefault() override;
		
	protected:
			///Get the index of a value at a specified row/column (NB: index must be in range or an exception will be thrown)
		size_t getValueIndex(size_t row = 0, size_t col = 0) const {
			auto result = row * m_columns + col;
			if (result >= base::size())
				throw std::out_of_range("");
			return result;
		}
			///Get the value at a specified row/column (NB: index must be in range or an exception will be thrown)
		template<typename T>
		T getValue(size_t row = 0, size_t col = 0) const {
			return *(*this)[getValueIndex(row, col)];
		}
		
	private:
			///A default type for the setting values (NB: the actual stored values do not need to adhere to this type)
		std::optional<Value::Type> m_type;
			///The number of columns the values are notionally arranged in (has no impact on vector size)
		size_t m_columns = 1;
	};
	
}

#endif	//ACTIVE_SETTING_VALUE_SETTING
