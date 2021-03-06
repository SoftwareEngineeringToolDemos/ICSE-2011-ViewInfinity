/*
 * $Revision: 1.17 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-12 15:50:08 +0100 (Mo, 12 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration and implementation of Array class and
 * Array algorithms
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_ARRAY_H
#define OGDF_ARRAY_H


#include <ogdf/basic/basic.h>


namespace ogdf {

//! Threshold used by \a quicksort() such that insertion sort is
//! called for instances smaller than \a maxSizeInsertionSort.
const int maxSizeInsertionSort = 40;


//! Iteration over all indices \a i of an array \a A.
/**
 * Note that the index variable \a i has to be defined prior to this macro
 * (just as for \c #forall_edges, etc.).
 * <h3>Example</h3>
 * 
 *   \code
 *   Array<double> A;
 *   ...
 *   int i;
 *   forall_arrayindices(i, A) {
 *     cout << A[i] << endl;
 *   }
 *   \endcode
 * 
 *   Note that this code is equivalent to the following tedious long version
 * 
 *   \code
 *   Array<double> A;
 *   ...
 *   int i;
 *   for(i = A.low(); i <= A.high(); ++i) {
 *     cout << A[i] << endl;
 *   }
 *   \endcode
 */
#define forall_arrayindices(i, A) \
   for(i = (A).low(); i<=(A).high(); ++i)

//! Iteration over all indices \a i of an array \a A, in reverse order.
/**
 * Note that the index variable \a i has to be defined prior to this macro
 * (just as for \c #forall_edges, etc.).
 * See \c #forall_arrayindices for an example
 */
#define forall_rev_arrayindices(i, A) \
   for(i = (A).high(); i>=(A).low(); --i)


//! The parameterized class \a Array<E,INDEX> implements dynamic arrays of type \a E.
/**
 * The template parameter \a E denotes the element type and the parameter
 * \a INDEX denotes the index type. The index type must be chosen such that
 * it can express the whole index range of the array instance, as well as
 * its size. The default index type is \c int, other possible types are
 * \c short and <code>long long<code> (on 64-bit systems).
 */
template<class E, class INDEX = int> class Array {
public:
	//! Creates an array with empty index set.
	Array() { construct(0,-1); }

	//! Creates an array with index set [0..\a s-1].
	explicit Array(INDEX s) {
		construct(0,s-1); initialize();
	}

	//! Creates an array with index set [\a a..\a b].
	Array(INDEX a, INDEX b) {
		construct(a,b); initialize();
	}

	//! Creates an array with index set [\a a..\a b] and initializes each element with \a x.
	Array(INDEX a, INDEX b, const E &x) {
		construct(a,b); initialize(x);
	}

	//! Creates an array that is a copy of \a A.
	Array(const Array<E> &A) {
		copy(A);
	}

	// destruction
	~Array() {
		deconstruct();
	}

	//! Returns the minimal array index.
	INDEX low() const { return m_low; }

	//! Returns the maximal array index.
	INDEX high() const { return m_high; }

	//! Returns the size (number of elements) of the array.
	INDEX size() const { return m_high - m_low + 1; }

	//! Returns a pointer to the first element.
	E *begin() { return m_pStart; }

	//! Returns a pointer to the first element.
	const E *begin() const { return m_pStart; }

	//! Returns a pointer to one past the last element.
	E *end() { return m_pStop; }

	//! Returns a pointer to one past the last element.
	const E *end() const { return m_pStop; }

	//! Returns a pointer to the last element.
	E *rbegin() { return m_pStop-1; }

	//! Returns a pointer to the last element.
	const E *rbegin() const { return m_pStop-1; }

	//! Returns a pointer to one before the first element.
	E *rend() { return m_pStart-1; }

	//! Returns a pointer to one before the first element.
	const E *rend() const { return m_pStart-1; }

	//! Returns a reference to the element at position \a i.
	const E &operator[](INDEX i) const {
		OGDF_ASSERT(m_low <= i && i <= m_high)
		return m_vpStart[i];
	}

	//! Returns a reference to the element at position \a i.
	E &operator[](INDEX i) {
		OGDF_ASSERT(m_low <= i && i <= m_high)
		return m_vpStart[i];
	}

	//! Swaps the elements at position \a i and \a j.
	void swap(INDEX i, INDEX j) {
		OGDF_ASSERT(m_low <= i && i <= m_high)
		OGDF_ASSERT(m_low <= j && j <= m_high)

		ogdf::swap(m_vpStart[i], m_vpStart[j]);
	}

	//! Reinitializes the array to an array with empty index set.
	void init() { init(0,-1); }
	
	//! Reinitializes the array to an array with index set [0..\a s-1].
	/**
	 * Notice that the elements contained in the array get discarded!
	 */
	void init(INDEX s) { init(0,s-1); }

	//! Reinitializes the array to an array with index set [\a a..\a b].
	/**
	 * Notice that the elements contained in the array get discarded!
	 */
	void init(INDEX a, INDEX b) {
		deconstruct();
		construct(a,b);
		initialize();
	}

	//! Reinitializes the array to an array with index set [\a a..\a b] and sets all entries to \a x.
	void init(INDEX a, INDEX b, const E &x) {
		deconstruct();
		construct(a,b);
		initialize(x);
	}

	//! Assignment operator.
	Array<E,INDEX> &operator=(const Array<E,INDEX> &array2) {
		deconstruct();
		copy(array2);
		return *this;
	}

	//! Sets all elements to \a x.
	void fill(const E &x) {
		E *pDest = m_pStop;
		while(pDest > m_pStart)
			*--pDest = x;
	}

	//! Sets elements in the intervall [\a i..\a j] to \a x.
	void fill(INDEX i, INDEX j, const E &x) {
		OGDF_ASSERT(m_low <= i && i <= m_high)
		OGDF_ASSERT(m_low <= j && j <= m_high)

		E *pI = m_vpStart + i, *pJ = m_vpStart + j+1;
		while(pJ > pI)
			*--pJ = x;
	}

	//! Enlarges the array by \a add elements and sets new elements to \a x.
	/**
	 *  Note: address of array entries in memory may change!
	 * @param add is the number of additional elements; \a add can be negative in order to shrink the array.
	 * @param x is the inital value of all new elements.
	 */
	void grow(INDEX add, const E &x);

	//! Enlarges the array by \a add elements.
	/**
	 *  Note: address of array entries in memory may change!
	 * @param add is the number of additional elements; \a add can be negative in order to shrink the array.
	 */
	void grow(INDEX add);

	//! Sorts the array using Quicksort.
	void quicksort();

	//! Sorts the subarray with index set [\a l..\a r] using Quicksort.
	void quicksort(INDEX l, INDEX r);

	//! Sorts the subarray with index set [\a l..\a r] using Quicksort and a user-defined comparer.
	void quicksort(INDEX l, INDEX r, Comparer<E> &comp);

	//! Sorts the array using Quicksort and a user-defined comparer.
	void quicksort(Comparer<E> &comp);

	//! Sorts the array using Quicksort and a user-defined comparer.
	/**
	 * @param comp is a user-defined comparer; \a C must be a class providing a \a less(x,y) method.
	 */
	template<class C>
	void quicksortCT(C &comp) {
		quicksortCT(m_pStart,m_pStop-1,comp);
	}

	//! Sorts the subarray with index set [\a l..\a r] using Quicksort and a user-defined comparer.
	/**
	 * @param l is the left-most position in the range to be sorted.
	 * @param r is the right-most position in the range to be sorted.
	 * @param comp is a user-defined comparer; \a C must be a class providing a \a less(x,y) method.
	 */
	template<class C>
	void quicksortCT(INDEX l, INDEX r, C &comp) {
		OGDF_ASSERT(low() <= l && l <= high())
		OGDF_ASSERT(low() <= r && r <= high())

		quicksortCT(m_vpStart+l,m_vpStart+r,comp);
	}

	//! Randomly permutes the subarray with index set [\a l..\a r].
	void permute (INDEX l, INDEX r);

	//! Randomly permutes the array.
	void permute() {
		permute(low(), high());
	}
	
	//! Performs a binary search for element \a x.
	/**
	 * \pre The array must be sorted!
	 * \return the index of the found element, and low()-1 if not found.
	 */
	int binarySearch (const E& x) const;

	//! Performs a binary search for element \a x with comparer \a comp.
	/**
	 * \pre The array must be sorted according to \a comp!
	 * \return the index of the found element, and low()-1 if not found.
	 */
	int binarySearch (const E& x, Comparer<E> &comp) const;

	//! Performs a linear search for element \a x.
	/**
	 * Warning: linear running time!
	 * Note that the linear search runs from back to front.
	 * \return the index of the found element, and low()-1 if not found.
	 */
	int linearSearch (const E& x) const;

	//! Performs a linear search for element \a x with comparer \a comp.
	/**
	 * Warning: linear running time!
	 * Note that the linear search runs from back to front.
	 * \return the index of the found element, and low()-1 if not found.
	 */
	int linearSearch (const E& x, Comparer<E> &comp) const;

	template<class F, class I> friend class ArrayBuffer; // for efficient ArrayBuffer::compact-method

private:
	E *m_vpStart; //!< The virtual start of the array (address of A[0]).
	E *m_pStart;  //!< The real start of the array (address of A[m_low]).
	E *m_pStop;   //!< Successor of last element (address of A[m_high+1]).
	INDEX m_low;    //!< The lowest index.
	INDEX m_high;   //!< The highest index.

	//! Allocates new array with index set [\a a..\a b].
	void construct(INDEX a, INDEX b);

	//! Initializes elements with default constructor.
	void initialize();

	//! Initializes elements with \a x.
	void initialize(const E &x);

	//! Deallocates array.
	void deconstruct();

	//! Constructs a new array which is a copy of \a A.
	void copy(const Array<E,INDEX> &A);

	//! Internal Quicksort implementation.
	void quicksort(E *pL, E *pR);
	//! Internal Quicksort implementation with user-defined comparer.
	void quicksort(E *pL, E *pR, Comparer<E> &comp);
	
	//! Internal Quicksort implementation with comparer template.
	template<class C>
	void quicksortCT(E *pL, E *pR, C &comp) {
		INDEX s = (INDEX)(pR-pL);

		// use insertion sort for small instances
		if (s < maxSizeInsertionSort) {
			for (E *pI = pL+1; pI <= pR; pI++) {
				E v = *pI; 
				E *pJ = pI;
				while (--pJ >= pL && comp.less(v,*pJ)) {
					*(pJ+1) = *pJ;
				}
				*(pJ+1) = v;
			}
			return;
		}

		E *pI = pL, *pJ = pR;
		E x = *(pL+(s>>1));

		do {
			while (comp.less(*pI,x)) pI++;
			while (comp.less(x,*pJ)) pJ--;
			if (pI <= pJ) ogdf::swap(*pI++,*pJ--);
		} while (pI <= pJ);

		if (pL < pJ) quicksortCT(pL,pJ,comp);
		if (pI < pR) quicksortCT(pI,pR,comp);
	}

	OGDF_NEW_DELETE
}; // class Array




// enlarges array by add elements and sets new elements to x
template<class E, class INDEX>
void Array<E,INDEX>::grow(INDEX add, const E &x)
{
	OGDF_ASSERT(m_pStart != 0 /*&& add > 0*/)

	INDEX sOld = size(), sNew = sOld + add;

	// expand allocated memory block
	m_pStart = (E *)realloc(m_pStart, sNew*sizeof(E));

	m_vpStart = m_pStart-m_low;
	m_pStop   = m_pStart+sNew;
	m_high   += add;

	// initialize new array entries
	for (E *pDest = m_pStart+sOld; pDest < m_pStop; pDest++)
		new (pDest) E(x);
}

// enlarges array by add elements and sets new elements to x
template<class E, class INDEX>
void Array<E,INDEX>::grow(INDEX add)
{
	OGDF_ASSERT(m_pStart != 0 /*&& add > 0*/)

	INDEX sOld = size(), sNew = sOld + add;

	// expand allocated memory block
	m_pStart = (E *)realloc(m_pStart, sNew*sizeof(E));

	m_vpStart = m_pStart-m_low;
	m_pStop   = m_pStart+sNew;
	m_high   += add;

	// initialize new array entries
	for (E *pDest = m_pStart+sOld; pDest < m_pStop; pDest++)
		new (pDest) E;
}

template<class E, class INDEX>
void Array<E,INDEX>::construct(INDEX a, INDEX b)
{
	m_low = a; m_high = b;
	INDEX s = b-a+1;
	
	if (s < 1) {
		m_pStart = m_vpStart = m_pStop = 0;

	} else {
		m_pStart = (E *)malloc(s*sizeof(E));
		if (m_pStart == 0) THROW(InsufficientMemoryException);

		m_vpStart = m_pStart - a;
		m_pStop = m_pStart + s;
	}
}


template<class E, class INDEX>
void Array<E,INDEX>::initialize()
{
	E *pDest = m_pStart;
	try {
		for (; pDest < m_pStop; pDest++)
			new(pDest) E;
	} catch (...) {
		while(--pDest >= m_pStart)
			pDest->~E();
		free(m_pStart);
		throw;
	}
}


template<class E, class INDEX>
void Array<E,INDEX>::initialize(const E &x)
{
	E *pDest = m_pStart;
	try {
		for (; pDest < m_pStop; pDest++)
			new(pDest) E(x);
	} catch (...) {
		while(--pDest >= m_pStart)
			pDest->~E();
		free(m_pStart);
		throw;
	}
}


template<class E, class INDEX>
void Array<E,INDEX>::deconstruct()
{
	if (doDestruction((E*)0)) {
		for (E *pDest = m_pStart; pDest < m_pStop; pDest++)
			pDest->~E();
	}
	free(m_pStart);
}


template<class E, class INDEX>
void Array<E,INDEX>::copy(const Array<E,INDEX> &array2)
{
	construct(array2.m_low, array2.m_high);

	if (m_pStart != 0) {
		E *pSrc = array2.m_pStop;
		E *pDest = m_pStop;
		while(pDest > m_pStart)
			//*--pDest = *--pSrc;
 			new (--pDest) E(*--pSrc);
	}
}


//---------------------------------------------------------
// quicksort
//---------------------------------------------------------

//#define MAX_SORT 40

// apply quicksort
// using default ordering defined by <, <=, ... operators (fast)
template<class E, class INDEX>
void Array<E,INDEX>::quicksort(E *pL, E *pR)
{
	INDEX s = INDEX(pR-pL);

	// use insertion sort for small instances
	if (s < maxSizeInsertionSort) {
		for (E *pI = pL+1; pI <= pR; pI++) {
			E v = *pI; 
			E *pJ = pI;
			while (--pJ >= pL && *pJ > v) {
				*(pJ+1) = *pJ;
			}
			*(pJ+1) = v;
		}
		return;
	}

	E *pI = pL, *pJ = pR;
	E x = *(pL+(s>>1));

	do {
		while (*pI < x) pI++;
		while (*pJ > x) pJ--;
		if (pI <= pJ) ogdf::swap(*pI++,*pJ--);
	} while (pI <= pJ);

	if (pL < pJ) quicksort(pL,pJ);
	if (pI < pR) quicksort(pI,pR);
}

// sort array a from A[l] to A[r] using quicksort
template<class E, class INDEX>
inline void Array<E,INDEX>::quicksort(INDEX l, INDEX r)
{
	OGDF_ASSERT(low() <= l && l <= high())
	OGDF_ASSERT(low() <= r && r <= high())

	if (l > r) return;
	quicksort(m_vpStart+l,m_vpStart+r);
}

// sort array a using quicksort
template<class E, class INDEX>
inline void Array<E,INDEX>::quicksort()
{
	if (low() > high()) return;
	quicksort(m_pStart,m_pStop-1);
}


// apply quicksort
// using a generic compare object for type E
// slightly slower, but just one instantiation for type E
template<class E, class INDEX>
void Array<E,INDEX>::quicksort(E *pL, E *pR, Comparer<E> &comp)
{
	INDEX s = INDEX(pR-pL);

	// use insertion sort for small instances
	if (s < maxSizeInsertionSort) {
		for (E *pI = pL+1; pI <= pR; pI++) {
			E v = *pI; 
			E *pJ = pI;
			while (--pJ >= pL && comp.compare(*pJ,v) > 0) {
				*(pJ+1) = *pJ;
			}
			*(pJ+1) = v;
		}
		return;
	}

	E *pI = pL, *pJ = pR;
	E x = *(pL+(s>>1));

	do {
		while (comp.compare(*pI,x) < 0) pI++;
		while (comp.compare(*pJ,x) > 0) pJ--;
		if (pI <= pJ) ogdf::swap(*pI++,*pJ--);
	} while (pI <= pJ);

	if (pL < pJ) quicksort(pL,pJ,comp);
	if (pI < pR) quicksort(pI,pR,comp);
}


// sort array a from A[l] to A[r] using quicksort and compare element comp
template<class E, class INDEX>
inline void Array<E,INDEX>::quicksort(INDEX l, INDEX r, Comparer<E> &comp)
{
	OGDF_ASSERT(low() <= l && l <= high())
	OGDF_ASSERT(low() <= r && r <= high())

	quicksort(m_vpStart+l,m_vpStart+r,comp);
}

// sort array a using quicksort and compare element comp
template<class E, class INDEX>
inline void Array<E,INDEX>::quicksort(Comparer<E> &comp)
{
	if (size() >= 2)
		quicksort(m_pStart,m_pStop-1,comp);
}


// apply quicksort
// using a parameterized compare object
// fast, but instantiated for each compare type C of type E
/*template<class E>template<class C>
void Array<E>::quicksortCT(E *pL, E *pR, C &comp)
{
	int s = pR-pL;

	// use insertion sort for small instances
	if (s < maxSizeInsertionSort) {
		for (E *pI = pL+1; pI <= pR; pI++) {
			E v = *pI; 
			E *pJ = pI;
			//while (--pJ >= pL && comp.compare(*pJ,v) > 0) {
			while (--pJ >= pL && comp.less(v,*pJ)) {
				*(pJ+1) = *pJ;
			}
			*(pJ+1) = v;
		}
		return;
	}

	E *pI = pL, *pJ = pR;
	E x = *(pL+(s>>1));

	do {
		while (comp.less(*pI,x)) pI++;
		while (comp.less(x,*pJ)) pJ--;
		if (pI <= pJ) swap(*pI++,*pJ--);
	} while (pI <= pJ);

	if (pL < pJ) quicksortCT(pL,pJ,comp);
	if (pI < pR) quicksortCT(pI,pR,comp);
}*/


// sort array a from A[l] to A[r] using quicksort and
// parameterized compare object comp
/*template<class E, class C>
inline void Array<E>::quicksortCT(int l, int r, C &comp)
{
	OGDF_ASSERT(low() <= l && l <= high())
	OGDF_ASSERT(low() <= r && r <= high())

	quicksortCT(m_vpStart+l,m_vpStart+r,comp);
}*/

// sort array a using quicksort and parameterized compare object comp
/*template<class E, class C>
inline void Array<E>::quicksortCT(C &comp)
{
	quicksortCT(m_pStart,m_pStop-1,comp);
}*/



// permutes array a from a[l] to a[r] randomly
template<class E, class INDEX>
void Array<E,INDEX>::permute (INDEX l, INDEX r)
{
	OGDF_ASSERT(low() <= l && l <= high())
	OGDF_ASSERT(low() <= r && r <= high())

	E *pI = m_vpStart+l, *pStart = m_vpStart+l, *pStop = m_vpStart+r;
	while(pI <= pStop)
		ogdf::swap(*pI++,*(pStart+randomNumber(0,r-l)));
}

template<class E, class INDEX>
int Array<E,INDEX>::binarySearch(const E& e) const {
	if(size() < 2)
		return low() - ( (size() == 1 && e == m_vpStart[low()]) ? 0 : 1 );
	int l = low();
	int r = high();
	do {
		int m = (r + l)/2;
		if(e > m_vpStart[m])
			l = m+1;
		else
			r = m;
	} while(r>l);
	return e == m_vpStart[l] ? l : low()-1;
}

template<class E, class INDEX>
int Array<E,INDEX>::binarySearch(const E& e, Comparer<E> &comp) const {
	if(size() < 2) {
		if(size() == 1 && comp.equal(e, m_vpStart[low()]))
			return low();
		return low()-1;
	}
	int l = low();
	int r = high();
	do {
		int m = (r + l)/2;
		if(comp.greater(e, m_vpStart[m]))
			l = m+1;
		else
			r = m;
	} while(r>l);
	return comp.equal(e, m_vpStart[l]) ? l : low()-1;
}

template<class E, class INDEX>
int Array<E,INDEX>::linearSearch(const E& e) const {
	int i;
	for(i = size(); i-->0;)
		if(e == m_pStart[i]) break;
	return i+low();
}

template<class E, class INDEX>
int Array<E,INDEX>::linearSearch(const E& e, Comparer<E> &comp) const {
	int i;
	for(i = size(); i-->0;)
		if(comp.equal(e, m_pStart[i])) break;
	return i+low();
}


// prints array a to output stream os using delimiter delim
template<class E, class INDEX>
void print(ostream &os, const Array<E,INDEX> &a, char delim = ' ')
{
	for (int i = a.low(); i <= a.high(); i++) {
		if (i > a.low()) os << delim;
		os << a[i];
	}
}


// output operator
template<class E, class INDEX>
ostream &operator<<(ostream &os, const ogdf::Array<E,INDEX> &a)
{
	print(os,a);
	return os;
}

} // end namespace ogdf


#endif
