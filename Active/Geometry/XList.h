/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_X_LIST
#define ACTIVE_GEOMETRY_X_LIST

#include "Active/Geometry/XPoint.h"

#include "Active/Container/List.h"

namespace active::geometry {
	
	class PolyEdge;
	
	/*!
		Class to represent a list of intersection points
	 
		NB: The word "intersection" is generally abbreviated to "X" in class names, e.g. `XPoint`, `XInfo` etc.
		Refer to the documentation for `XInfo` and `Xpoint` for a better understanding of the terminology and management of intersections, e.g.
		`blade` and `target` (defined in `XPoint`).
	*/
	class XList {
	public:

		// MARK: - Types
		
			///The list container type
		typedef container::List<XPoint> base;
			///The list iterator type
		typedef base::iterator iterator;
			///The list const_iterator type
		typedef base::const_iterator const_iterator;
			///The list size type
		typedef base::size_type size_type;

		// MARK: - Constructors
		
		/*!
			Constructor
			@param targetFilter A filter for intersections with the target
			@param bladeFilter A filter for intersections with the blade
		*/
		XList(const XInfo& targetFilter = XInfo(), const XInfo& bladeFilter = XInfo());

		// MARK: - Functions (const)
		
		/*!
			Get an iterator at the first intersection
			@return An iterator at the first intersection
		*/
		const_iterator begin() const { return m_intersect.begin(); }
		/*!
			Get an iterator one beyond the last intersection
			@return An iterator one beyond the last intersection
		*/
		const_iterator end() const { return m_intersect.end(); }
		/*!
			Get the front intersection in the list
			@return The front intersection
		*/
		const XPoint* front() const { return m_intersect.front().get(); }
		/*!
			Get the back intersection in the list
			@return The back intersection
		*/
		const XPoint* back() const { return m_intersect.back().get(); }
		/*!
			Get the list size
			@return The list size
		*/
		size_type size() const { return m_intersect.size(); }
		/*! Determine if the intersection list is empty
			@return True if the list is empty
		*/
		bool empty() const  { return m_intersect.empty(); }
		/*!
			Get the intersection filter for a specified role
			@return The requested intersection filter
		*/
		const XInfo& getFilter(XPoint::Role role) const { return m_filter[getPart(role)]; }
		/*!
			Determine if the list accepts intersections having a relative position to a component with a specified role
			@param role The component role
			@param pos The relative position of the point with respect to the component
			@return True if the list accepts this relative position for the role
		*/
		bool withPos(XPoint::Role role, Position pos) const;
		/*!
			Determine if the list filters intersections for a specified role based on its relative position
			@param role The component role
			@return True if the list has a defined filter for this role
		*/
		bool isPos(XPoint::Role role) const;
		/*!
			Set the current target intersection filter
			@param filter The target intersection filter
		*/
		void setFilter(XPoint::Role role, const XInfo& filter) { m_filter[getPart(role)] = filter; }

		// MARK: - Functions (mutating)
		
		/*!
			Get an iterator at the first intersection
			@return An iterator at the first intersection
		*/
		iterator begin() { return m_intersect.begin(); }
		/*!
			Get an iterator one beyond the last intersection
			@return An iterator one beyond the last intersection
		*/
		iterator end() { return m_intersect.end(); }
		/*!
			Get the front intersection in the list
			@return The front intersection
		*/
		XPoint* front() { return m_intersect.front().get(); }
		/*!
			Get the back intersection in the list
			@return The back intersection
		*/
		XPoint* back() { return m_intersect.back().get(); }
		/*!
			Insert an intersection point
			@param pt A new intersection point
			@return True if the point was inserted
		*/
		bool insert(XPoint&& pt);
		/*!
			Insert an intersection point
			@param pt A new intersection point
			@return True if the point was inserted
		*/
		bool insert(XPoint::Unique&& pt);
		/*!
			Erase the intersection at the specified iterator
			@param iter The iterator pointing to the intersection to erase
			@return A interator at the next intersection in the list
		*/
		iterator erase(iterator iter) { return m_intersect.erase(iter); }
		/*!
			Release the intersection at the specified iterator
			@param iter The iterator pointing to the intersection to release
			@return The release intersection
		*/
		auto release(iterator& iter) { return m_intersect.release(iter); }
		/*!
			Clear the list of intersections
		*/
		void clear() { m_intersect.clear(); }
		/*!
			Remove duplicate intersections
		 	@param prec The comparison precision
		*/
		void removeDuplicates(double prec = math::eps);
		/*!
			Template method to sort the list of intersections
			@param comp The object to make the sort comparisons
		*/
		template <class Compare>
		void sort(Compare comp) { m_intersect.sort(comp); }
		/*!
			Reverse the order of intersections in the list
		*/
		void reverse() { m_intersect.reverse(); }
		/*!
			Add an intersection type to the target filter
			@param role The component role
			@param pos The relative position of the point with respect to the component
		*/
		void addPos(XPoint::Role role, Position pos);
		/*!
			Set the current target vertex index
			@param vertexIndex The current target vertex index
		*/
		void setVertex(XPoint::Role role, vertOption vertexIndex) { m_filter[getPart(role)].vertexIndex = vertexIndex; }
		/*!
			Set the current blade object ID
			@param partIndex The current blade part index
		*/
		void setPart(XPoint::Role role, partOption partIndex) { m_filter[getPart(role)].partIndex = partIndex; }
		/*!
			Swap the target and blade filters
		*/
		void swapFilters();
		
	private:
		XPoint::Role getPart(XPoint::Role role) const {
			return (m_isSwapped) ? ((role == XPoint::target) ? XPoint::blade : XPoint::target) : role;
		}
		
			///The list of intersection points
		base m_intersect;
			///Filter for acceptable intersection position relative to the target and blade
		XInfo m_filter[2];
			///True if the relative position filters have been swapped
		bool m_isSwapped;
	};
	
	
		//Compare points on the basis of position in the x-y plane
	struct ComparePosition : public std::greater<XPoint::Unique> {
	public:
		/*!
			Constructor
			@param prec The comparison precision
		*/
		ComparePosition(double prec = math::eps) { m_prec = prec; }
		/*!
			Copy constructor
			@param source The object to copy
		*/
		ComparePosition(const ComparePosition& source) { m_prec = source.m_prec; }
		/*!
			Functional operator to compare 2 points
			@param pos1 The first point to compare
			@param pos2 The second point to compare
			@return True if pos1 < pos2
		*/
		bool operator() (XPoint::Unique& pos1, XPoint::Unique& pos2) const
				{ return (math::isLess(pos1->x, pos2->x, m_prec) ||
						(math::isEqual(pos1->x, pos2->x, m_prec) && math::isLess(pos1->y, pos2->y, m_prec))); }
	private:
			///The comparison precision
		double m_prec;
	};
	
	
		//Class to sort points along the length of a poly-edge
	struct AlongLengthOf {
	public:
		/*!
			Constructor
			@param edge The poly-edge to sort the point along
		*/
		AlongLengthOf(const PolyEdge& edge);
		/*!
			Functional operator to compare 2 points along the length of a poly-edge
			@param pos1 The first point to compare
			@param pos2 The second point to compare
			@return True if pos1 < pos2
		*/
		bool operator() (XPoint::Unique& pos1, XPoint::Unique& pos2) const;
		
	private:
			
		Point m_origin;
		double m_radius;
		bool m_isClockwise;
	};
	
}

#endif	//ACTIVE_GEOMETRY_X_LIST
