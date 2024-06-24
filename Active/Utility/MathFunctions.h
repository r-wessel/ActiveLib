/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MATH_FUNCTIONS
#define ACTIVE_MATH_FUNCTIONS

#include <cmath>
#include <numbers>

/*!
	A collection of commonly used maths contants and functions
*/

namespace active::math {
	
	//MARK: - Common maths constants
	
	constexpr double pi = std::numbers::pi_v<double>;
		///Default length precision (0.01mm)
	constexpr double eps = 1e-5;
		///Default angle precision (0.1 degrees)
	constexpr double epsAngle = pi / 1800;

	//MARK: - Common unit conversions

		///Angle unit conversions
	constexpr double radianToDegree = 180.0 / pi;
	constexpr double degreeToRadian = 1.0 / radianToDegree;
	constexpr double radianToMinute = radianToDegree * 60;
	constexpr double minuteToRadian = 1.0 / radianToMinute;
	constexpr double radianToSecond = radianToMinute * 60;
	constexpr double secondToRadian = 1.0 / radianToSecond;
	constexpr double radianToGrad = radianToDegree * 10.0 / 9.0;
	constexpr double gradToRadian = 1.0 / radianToGrad;
	
		///Length unit conversions
	constexpr double inchToMillimetre = 25.4;
	constexpr double pointToMillimetre = 0.3527777777778;
	constexpr double footToInch = 12;
	constexpr double yardToFoot = 3;
	constexpr double metreToKilometre = 1000;
	constexpr double metreToDecimetre = 10;
	constexpr double metreToCentimetre = 1e2;
	constexpr double metreToMillimetre = 1e3;
	constexpr double metreToInch = metreToMillimetre / inchToMillimetre;
	constexpr double metreToFoot = metreToInch / footToInch;
	constexpr double metreToYard = metreToFoot / yardToFoot;
	constexpr double metreToMile = 0.0006213712;
	
		///Area unit conversions
	constexpr double metre2ToMillimetre2 = metreToMillimetre * metreToMillimetre;
	constexpr double metre2ToCentimetre2 = metreToCentimetre * metreToCentimetre;
	constexpr double metre2ToAre = 0.01;
	constexpr double metre2ToHectare = 0.0001;
	constexpr double metre2ToInch2 = metreToInch * metreToInch;
	constexpr double metre2ToFoot2 = metreToFoot * metreToFoot;
	constexpr double metre2ToYard2 = metreToYard * metreToYard;
	constexpr double metre2ToMile2 = metreToMile * metreToMile;
	
		///Volume unit conversions
	constexpr double metre3ToLitre = 1000.0;
	constexpr double metre3ToCentimetre3 = metre2ToCentimetre2 * metreToMillimetre;
	constexpr double metre3ToMillimetre3 = metre2ToMillimetre2 * metreToCentimetre;
	constexpr double metre3ToInch3 = metre2ToInch2 * metreToInch;
	constexpr double metre3ToFoot3 = metre2ToFoot2 * metreToFoot;
	constexpr double metre3ToYard3 = metre2ToYard2 * metreToYard;
	constexpr double metre3ToGallon = 264.172051;
	
		///Mass unit conversions
	constexpr double kilogramToMilligram = 1000000.0;
	constexpr double kilogramToGram = 1000.0;
	constexpr double kilogramToTonne = 0.001;
	constexpr double kilogramToOunce = 35.2739619;
	constexpr double kilogramToPound = 2.204622622;
	constexpr double kilogramToStone = 0.15747304;
	constexpr double kilogramToTon = 0.001102311;
	
	//MARK: - Common floating point comparisons
	
	/*!
	 	Determine if one floating point value is less than another (with precision)
		@param val1 The first value
		@param val2 The second value
		@param prec The comparison precision (tolerance)
		@return True if the first value is less than the second
	*/
	inline bool isLess(double val1, double val2, double prec = eps)
		{ return (val2 - val1 > prec); }
	
	/*!
	 	Determine if two floating point values are identical (with precision)
		@param val1 The first value
		@param val2 The second value
		@param prec The comparison precision (tolerance)
		@return True if the two values are identical
	*/
	inline bool isEqual(double val1, double val2, double prec = eps)
		{ return (fabs(val1 - val2) <= prec); }
	/*!
	 	Determine if one floating point value is greater than another (with precision)
		@param val1 The first value
		@param val2 The second value
		@param prec The comparison precision (tolerance)
		@return True if the first value is greater than the second
	*/
	inline bool isGreater(double val1, double val2, double prec = eps)
		{ return (val1 - val2 > prec); }
	/*!
	 	Compare two floating point values (with precision)
		@param val1 The first value
		@param val2 The second value
		@param prec The comparison precision (tolerance)
		@return -1 if the first value is less than the second, 0 if the values are equal, 1 if the first value is greater than the second
	*/
	inline int compare(double val1, double val2, double prec = eps)
		{ return isEqual(val1, val2, prec) ? 0 : (isLess(val1, val2, prec) ? -1 : 1); }
	
	inline bool isLessOrEqual(double val1, double val2, double prec = eps)
		{ return (isEqual(val1, val2, prec) || isLess(val1, val2, prec)); }
	
	inline bool isGreaterOrEqual(double val1, double val2, double prec = eps)
		{ return (isEqual(val1, val2, prec) || isGreater(val1, val2, prec)); }
	
	inline bool isLessZero(double val, double prec = eps)
		{ return isLess(val, 0.0, prec); }
	
	inline bool isLessOrEqualZero(double val, double prec = eps)
		{ return isLessOrEqual(val, 0.0, prec); }
	
	inline bool isZero(double val, double prec = eps)
		{ return isEqual(val, 0.0, prec); }
	
	inline bool isGreaterOrEqualZero(double val, double prec = eps)
		{ return isGreaterOrEqual(val, 0.0, prec); }
	
	inline bool isGreaterZero(double val, double prec = eps)
		{ return isGreater(val, 0.0, prec); }
	
	inline bool isBetween(double val, double lower, double upper, double prec = eps)
		{ return (isGreater(val, lower, prec) && isLess(val, upper, prec)); }
	
	inline bool isWithin(double val, double lower, double upper, double prec = eps)
		{ return (isGreaterOrEqual(val, lower, prec) && isLessOrEqual(val, upper, prec)); }
	
	inline double fMod(double val1, double val2, double prec = eps) {
		val1 = fmod(val1, val2);
		return (isZero(val1, prec) || isEqual(fabs(val1), fabs(val2), prec)) ? 0.0 : val1;
	}

	//MARK: - Common math functions
	
	template <class T>
	inline const T& minVal(const T& val1, const T& val2) { return (val1 < val2) ? val1 : val2; }
	template <class T>
	inline const T& maxVal(const T& val1, const T& val2) { return (val1 > val2) ? val1 : val2; }

		///Rounding functions
	inline double round(const double& val, double module = eps)
		{ return floor((val / module) + 0.5) * module; }

	inline double roundUp(const double& val, double module = eps, double tolerance = eps) {
			//If tolerances differ from the module size and the current value is within the tolerances of a module, we accept that module size
		if (module > tolerance) {
			double rounded = round(val, module);
			if (isEqual(val, rounded, tolerance))
				return rounded;
		}
		return ceil(val / module) * module;
	} //roundUp

	inline double roundDown(const double& val, double module = eps, double tolerance = eps) {
			//If tolerances differ from the module size and the current value is within the tolerances of a module, we accept that module size
		if (module > tolerance) {
			double rounded = round(val, module);
			if (isEqual(val, rounded, tolerance))
				return rounded;
		}
		return floor(val / module) * module;
	} //roundDown


	inline double sign(const double& val, double prec = eps)
		{ return isZero(val, prec) ? 0.0 : ((val < 0) ? -1.0 : 1.0); }
	
	
	template <class T>
	inline T sgn(const T& val)
		{ return (val == T()) ? T() : T((val > 0.0) ? 1.0 : -1.0); }
	template <class T>
	inline T flow_sgn(const T& val)
		{ return T((val >= T()) ? 1 : -1); }

	//MARK: - Common angle functions
	
		///Calculate an angle as a value between 0 to 2.pi radians
	inline double angleMod(double angle) {
		angle = fmod(angle, 2.0 * pi);
		if (angle < 0.0)
			angle += (2.0 * pi);
		return angle;
	}
		///Determine if an angle divides cleanly into another angle
	inline bool dividesInto(double val, double div, double prec = eps)
		{ return isZero(fMod(val, div, prec), prec); }
		///Determine if an angle is bounded between two other angles
	inline bool isAngleBetween(double val, double lower, double upper, double prec = eps) {
		double minA = angleMod(lower), maxA = angleMod(upper);
		if (maxA < minA) {
			if (val > minA)
				maxA += (2.0 * pi);
			else
				minA -= (2.0 * pi);
		}
		return isBetween(val, minA, maxA, prec);
	}
		///Determine if an angle is bounded within two other angles
	inline bool isAngleWithin(double val, double lower, double upper, double prec = eps) {
		double minA = angleMod(lower), maxA = angleMod(upper);
		if (maxA < minA) {
			if (val > minA)
				maxA += (2.0 * pi);
			else
				minA -= (2.0 * pi);
		}
		return isWithin(val, minA, maxA, prec);
	}
		///Determine if two angles are equal
	inline bool isEqualAngle(double angle1, double angle2, double prec = eps) {
		angle1 = angleMod(angle1);
		angle2 = angleMod(angle2);
		if (isEqual(angle1, angle2, prec))
			return true;
		if (isEqual(angle1, 2.0 * pi, prec))
			angle1 = 0.0;
		if (isEqual(angle2, 2.0 * pi, prec))
			angle2 = 0.0;
		return (isEqual(angle1, angle2, prec));
	}
		///Determine if two angles are parallel
	inline bool isParallelAngle(double angle1, double angle2, double prec = eps) {
		angle1 = angleMod(angle1 - angle2);
		return (isEqual(angle1, 0.0, prec) || isEqual(angle1, pi, prec) || isEqual(angle1, 2.0 * pi, prec));
	}
		///Determine if two angles are perpendicular
	inline bool isPerpendicularAngle(double angle1, double angle2, double prec = eps) {
		return isParallelAngle(angle1 - pi / 2.0, angle2, prec);
	}
		///Get a change in angle from 180 to -180
	inline double angleDelta(double angle1, double angle2) {
		double delta = angle2 - angle1;
		if (delta <= -pi)
			delta += 2.0 * pi;
		else if (delta > pi)
			delta = 2.0 * pi - delta;
		return delta;
	}

}

#endif	//ACTIVE_MATH_FUNCTIONS
