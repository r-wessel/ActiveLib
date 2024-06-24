/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_FACETER
#define ACTIVE_GEOMETRY_FACETER

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/PolyEdge.h"

namespace active::geometry {

	///A class to divide an arc into facets
	class Faceter {
	public:
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param orig The arc origin
			@param end The arc end
			@param isStart Include a vertex at the start angle
			@param isEnd Include a vertex at the end angle
			@param toler The arc tolerance
		*/
		Faceter(const Point& orig, const PolyPoint& end, bool isStart = false, bool isEnd = true, double toler = 0.002);
		/*!
			Constructor
			@param orig The arc origin
			@param end The arc end
		 	@param step The step value along the path
		*/
		Faceter(const Point& orig, const PolyPoint& end, double step);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Faceter(const Faceter& source);
		/*!
			Destructor
		*/
		~Faceter() {}
		
		// MARK: - Operators
		
		/*!
			Increment operator
		*/
		void operator++ ();
		/*!
			Postfix increment operator
			@return A copy of this
		*/
		Faceter operator++ (int);
		
		// MARK: - Functions (const)
		
		/*!
			Get the current vertex
			@return The current vertex
		*/
		Point getVertex() const;
		/*!
			Determine if the current position is at the start
			@return True if the current position is at the start
		*/
		bool isAtStart() const { return (m_currentStep == 0); }
		/*!
			Determine if the end angle has been reached
			@return True if the end angle has been reached
		*/
		bool isAtEnd() const { return (m_currentStep == (m_endStep - m_startStep)); }
		/*! Get the remaining length of the edge after the last vertex
		 	@return The remainder
		*/
		double getRemainder() const { return m_remainder; }
		
	private:
		bool m_isAlong;
		PolyEdge m_edge;
		double m_step;
		double m_remainder;
		Arc m_arc;
		double m_incAngle;
		double m_incHeight;
		vertex_index m_currentStep;
		vertex_index m_startStep;
		vertex_index m_endStep;
	};
	
}

#endif	//ACTIVE_GEOMETRY_FACETER
