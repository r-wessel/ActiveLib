/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_MEASURED_VALUE
#define ACTIVE_SETTING_MEASURED_VALUE

#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Utility/BufferIn.h"

#include <functional>

namespace active::setting {

	///Template specialisation of DoubleValue for values of measurement
	/*!
		Targets measurement properties (dimensions) like length, volume, area etc. where the formatting is expected to be a numeric value suffixed
		with an abbreviation of the unit of measurement, e.g. 100mm. The measurement formatting will specify the unit type and
		precision. Refer to the `Unit` template for more detail.
		
		Restrict use of this template to values that express measured properties rather than as a catch-all for formatted quantities. It is
		assumed (for example) that there is a fixed conversion factor between any measurement unit of the same kind, so values can be stored
		internally as a homogenous unit type and converted on the fly for string representations.
		
		Conversion of measured values should only be done for the purposes of string representation, e.g. for the user to read/edit. Otherwise, all
		coding should assume a fixed unit type, e.g. lengths are always in metres. Interactions with external software should convert to/from that
		software's native units only when reading/writing data from/to that software, and knowledge of the conversion factor should as limited as
		possible. This approach keeps code managing measurements as simple as possible (and will elminate the possibility of a host of errors).
		
		Some coding examples follow using `LengthValue` to illustrate how measurements should be handled in practice. `AreaValue` and `VolumeValue`
		work in exactly the same way (with relevant units).
		
		LengthValue holds values in metres, so can be constructed with a length of 3 metres as follows:
		
			LengthValue width{3.0};
		
		A string can be extracted from the value:
		
			String output{width};	//output becomes 3m
		
		The measurement unit can be changed to suit user requirements:
	 
			width.setUnit({foot, inch, 6, true});	//Set the unit to foot/inch, precision 1/64", fractional output
			output = width;	//output becomes 9' 10 7/64"
	
		A custom unit can also be used to retrieve a value without knowing or changing the formatting it contains:
	
			LengthValue width{3.1234};	//Default unit is metres to 4 decimal places
			String output = width;	//…which produces: 3.1234m
			output = width({foot, inch, 6, true});	//But with a custom unit: 10' 2 31/32"
			output = width({mm, 1});	//Or alternately: 3123.4mm
		
		The unit can have some bearing on the conversion of text to a measurement, but only when no units are explicitly stated. For example,
		if a `LengthValue` unit is decimal feet and text is assigned to it with no units, it will assume the figure is decimal feet:

			LengthValue width{{foot, 4}};	//Length unit is decimal feet
			width = "3";	//Text has no unit, so assumed to be decimal feet
			String output{width({metre, 4})};	//But can then be output as metres (or any other unit); output is 0.9144m (= 3 feet)
	 	
		However, the member unit is ignored if the text has explicit units:
		
			LengthValue width{{metre, 4}};	//Length unit is metres
			width = "3' 6 1/2\"";	//This text is formatted as feet and fractional inches, and is interpreted as such
			String output{width};	//So the resulting output is 1.0795m (= 3' 6 1/2")
		
		Units (of the same type, e.g. length) can be freely mixed in input, e.g. 3m 4" will be intrepreted as "3 metres + 4 inches" = 3.1016m. Any
		series of numbers (separated by white space) will be interpeted as being the same unit type and added together, so 4 6" will be read
		as "4 inches + 6 inches", i.e. it will not assume that the space implies feet. If the number is intended to be "4 feet + 6 inches" it should
		be explicitly entered as 4'6".
		
		The unit for a MeasuredValue can be sourced from the member variable or (optionally) a specified retrieval function. The latter could be used
		in cases where multiple instances must independently react to a common change, e.g. the UI refreshing displayed measurements in response to
		the user changing their preferred unit/formatting, e.g. if we have some function that returns the users formatting preferences:
		
			LengthUnit preferredLengthUnit() {
				...Some process to retrieve and return the preferred length unit
			}
		
		…it can be used to ensure a refresh always reflects the current preferences:
		
			LengthValue width{preferredLengthUnit};
	*/
	template<class T>
	class MeasuredValue : public DoubleValue {
	public:
		
		// MARK: - Types
		
			//Measurement unit type
		using Type = typename T::Type;
			///Function retrieving a measument unit
		using UnitRetrieval = std::function<T()>;
		
		// MARK: - Constructors

		/*!
			Default constructor
		*/
		MeasuredValue() {}
		/*!
			Constructor
			@param val The measurement value
		*/
		explicit MeasuredValue(double val) : DoubleValue(val) {}
		/*!
			Constructor
			@param unit The measurement unit
			@param val The measurement value
		*/
		explicit MeasuredValue(const T& unit, double val = 0.0) : DoubleValue(val), m_unit{unit} {}
		/*!
			Constructor
			@param unitFinder A function to retrieve the preferred unit
			@param val The measurement value
		*/
		MeasuredValue(UnitRetrieval unitFinder, double val = 0.0) : DoubleValue(val), m_unitFinder{unitFinder} {}
		
		// MARK: - Operators
		
		using DoubleValue::operator=;
		
		/*!
			Assignment operator
			@param val A string value to assign
			@return A reference to this
		*/
		Value& operator=(const utility::String& val) override {
			return assign(val, getUnit());
		}

		/*!
			Get a string value
			@return A string value
		*/
		operator utility::String() const override {
			return (*this)(getUnit());
		}
		/*!
			Get a string value
			@return A string value
		*/
		virtual utility::String operator()(const T& unit) const {
				//Pair a value with a unit type
			using UnitValue = std::pair<double, Type>;
				//Collect the string from a value (as an integer) before replacing it with the remaining fraction of a specified type
			auto collect = [](UnitValue& value, const T& unit, Type type) {
				UnitValue otherValue{0.0, type};
				auto total = value.first;
				value.first = math::roundDown(total, 1.0);
				otherValue.first = unit.conversion(otherValue.second, unit.conversion(value.second, total - value.first, true));
				utility::String result;
				if (unit.isLeadingZero || !math::isZero(value.first, 1.0))
					result = utility::String{value.first, 1.0} + unit.suffix(value.second);
				value = otherValue;
				return result;
			};

			bool isSuffix = unit.isUnitSuffix || unit.secondary;
			UnitValue value{unit.conversion(unit.primary, data), unit.primary};
			utility::String result;
				//Split primary and second value when a second unit is specified
			if (unit.secondary) {
				result = collect(value, unit, *unit.secondary);
					//Split secondary and tertiary value when a third unit is specified
				if (unit.tertiary)
					result += " " + collect(value, unit, *unit.tertiary);
			}
				//Floating point output
			if (unit.isDecimal()) {
				if (!result.empty())
					result += " ";
				result += utility::String{value.first, unit.eps()};
			} else {
					//Fractional output
				auto wholePart = math::roundDown(value.first, 1.0);
				if (!math::isZero(wholePart)) {
					value.first -= wholePart;
					if (!result.empty())
						result += " ";
					result += utility::String{wholePart, 1.0};
				}
				auto dividend = static_cast<uint64_t>(math::round(fabs(value.first) / unit.eps(), 1.0));
				if (dividend != 0) {
					auto divisor = static_cast<uint64_t>(unit.divisor());
					while ((dividend % 2) == 0) {
						dividend /= 2;
						divisor /= 2;
					}
					if (!result.empty())
						result += " ";
					result += utility::String{dividend} + "/" + utility::String{divisor};
				}
				if (result.empty())
					result = "0";
			}
			if (isSuffix)
				result += unit.suffix(value.second);
			return result;
		}
		
		// MARK: - Functions (const)

		/*!
			Get the measurement unit
			@return The measurement unit
		*/
		virtual T getUnit() const { return m_unitFinder ? m_unitFinder() : m_unit; }
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the measurement unit
			@param unit The measurement unit (NB: A specified unitFinder function will take precedence over this setting)
		*/
		virtual void setUnit(const T& unit) { m_unit = unit; }
		/*!
			Set a unit retrieval function
			@param unitFinder The measurement unit retrieval function (nullptr = use member unit)
		*/
		virtual void setUnit(UnitRetrieval unitFinder) { m_unitFinder = unitFinder; }
		/*!
			Assign a measurement value as a string
			@param val A string value to assign
			@param unit The input unit
			@return A reference to this
		*/
		virtual Value& assign(const utility::String& val, const T& unit) {
				//Mark the value as bad until we establish a valid measurement from the text
			data = 0.0;
			status = bad;
				//Find any explicit units in the words to create a list of measurement expressions
			using UnitExpression = std::pair<utility::String, Type>;
			std::vector<UnitExpression> measureExpressions;
			utility::String::size_type start = 0;
			while (start < val.size()) {
				if (auto match = unit.findSuffix(val, start); match) {
					if (match->second == 0)
						return *this;	//A unit is leading the expression - not accepted as valid input
					measureExpressions.push_back(UnitExpression(val.substr(start, match->second - start), match->first));
					start = match->second + unit.suffix(match->first).size();
				} else {
					measureExpressions.push_back(UnitExpression(val.substr(start), unit.primary));	//Use the value unit unit as a default
					break;
				}
			}
				//Get the localised thousands separator
			const auto& numPunct{std::use_facet<std::numpunct<char>>(std::locale{})};
			utility::String thousandsSep{numPunct.thousands_sep()};
				//Extract measurement values from each expression
			for (auto& expression : measureExpressions) {
					//Break the expression into whitespace separated words
				auto words = utility::BufferIn{expression.first}.readWords();
				if (words.empty())
					continue;
				for (auto& word : words) {
						//Strip out the thousands separator
					word.replaceAll(thousandsSep, utility::String{});
						//Values can be expressed as a fraction - allow for dividend/divisor
					auto dividend = 0.0, divisor = 1.0;
					DoubleValue number;
						//Attempt to extract a divisor
					if (auto divisionPos = word.find("/"); divisionPos) {
						if (number = word.substr(*divisionPos + 1); (number.status == good) && !math::isZero(number.data))
							divisor = number;
						else
							return *this;	//invalid divisor value
						word = word.substr(0, divisionPos);
					}
					number = word;
					if (number.status == good)
						dividend = number;
					else
						return *this;	//invalid dividend value
					status = good;
					data += unit.conversion(expression.second, dividend / divisor, true);
				}
			}
			return *this;
		}
		
	private:
			///The measurement unit (primarily for string formatting
		T m_unit;
			///Optional unit retrieval function (nullptr = use member unit)
		UnitRetrieval m_unitFinder;
	};
	
}

#endif //ACTIVE_SETTING_MEASURED_VALUE
