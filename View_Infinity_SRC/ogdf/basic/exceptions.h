/*
 * $Revision: 1.13 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-08 16:10:37 +0100 (Do, 08 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Definition of exception classes
 * 
 * \author Carsten Gutwenger, Markus Chimani
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

#include <stdio.h>

#ifndef OGDF_EXCEPTIONS_H
#define OGDF_EXCEPTIONS_H


namespace ogdf {

#ifdef OGDF_DEBUG
/**
 * If this flag is set the #THROW macros pass the location where the
 * exception is thrown (file name and line number) to the exception
 * constructor, otherwise not.
 */
#define OGDF_THROW_WITH_INFO
#endif

#ifdef THROW
#undef THROW
#endif

#ifndef OGDF_THROW_WITH_INFO
#define THROW_PARAM(CLASS, PARAM) throw CLASS ( PARAM )
#define THROW(CLASS)              throw CLASS ( )
#else
//! Replacement for \c throw.
/**
 * This macro is used to throw an exception and pass the file name
 * and line number of the location in the source file.
 * @param CLASS is the name of the exception class.
 * @param PARAM is an additional parameter (like the error code) required
 *        by the exception calls.
 */
#define THROW_PARAM(CLASS, PARAM) throw CLASS ( PARAM , __FILE__ , __LINE__ )
//! Replacement for \c throw.
/**
 * This macro is used to throw an exception and pass the file name
 * and line number of the location in the source file.
 * @param CLASS is the name of the exception class.
 */
#define THROW(CLASS)              throw CLASS ( __FILE__ , __LINE__ )
#endif


	//! Error code for a violated precondition.
	/**
	 * \see PreconditionViolatedException
	 */
	enum PreconditionViolatedCode {
		pvcUnknown,
		pvcSelfLoop,          //!< graph contains a self-loop
		pvcTreeHierarchies,   //!< hierarchies are not only trees
		pvcAcyclicHierarchies,//!< hierarchies are not acyclic
		pvcSingleSource,      //!< graph has not a single source
		pvcUpwardPlanar,      //!< graph is not upward planar
		pvcTree,              //!< graph is not a rooted tree
		pvcForest,            //!< graph is not a rooted forest
		pvcOrthogonal,        //!< layout is not orthogonal
		pvcPlanar,            //!< graph is not planar
		pvcClusterPlanar,     //!< graph is not c-planar
		pvcNoCopy,            //!< graph is not a copy of the corresponding graph
		pvcConnected,         //!< graph is not connected
		pvcBiconnected,         //!< graph is not twoconnected
		pvcSTOP               // INSERT NEW CODES BEFORE pvcSTOP!
	}; // enum PreconditionViolatedCode


	//! Code for an internal failure condition
	/**
	 * \see AlgorithmFailureException
	 */
	enum AlgorithmFailureCode {
		afcUnknown,
		afcIllegalParameter, //!< function parameter is illegal
		afcNoFlow,           //!< min-cost flow could not find a legal flow
		afcSort,             //!< sequence not sorted
		afcLabel,            //!< labelling failed
		afcExternalFace,     //!< external face not correct
		afcForbiddenCrossing,//!< crossing forbidden but necessary
		afcTimelimitExceeded,//!< it took too long
		afcNoSolutionFound,  //!< couldn't solve the problem
		afcSTOP              // INSERT NEW CODES BEFORE afcSTOP!
	}; // enum AlgorithmFailureCode



	//! Code for the library which was intended to get used, but its use is not supported.
	/**
	 * \see LibraryNotSupportedException
	 */
	enum LibraryNotSupportedCode {
		lnscUnknown,
		lnscCoin,                          //!< COIN not supported
		lnscAbacus,                        //!< ABACUS not supported
		lnscFunctionNotImplemented,        //!< the used library doesn't support that function
		lnscMissingCallbackImplementation, //
		lnscSTOP                           // INSERT NEW CODES BEFORE nscSTOP!
	}; // enum AlgorithmFailureCode



	//! Base class of all ogdf exceptions.
	class Exception { 

	private:

		const char *m_file; //!< Source file where exception occurred.
		int         m_line; //!< Line number where exception occurred.

	public:
		//! Constructs an exception.
		/**
		 * @param file is the name of the source file where exception was thrown.
		 * @param line is the line number in the source file where the exception was thrown.
		 */
		Exception(const char *file = NULL, int line = -1) : 
		  m_file(file),
		  m_line(line)
		{}

		//! Returns the name of the source file where exception was thrown.
		/**
		 * Returns a null pointer if the name of the source file is unknown.
		 */
		const char *file() { return m_file; }
		
		//! Returns the line number where the exception was thrown.
		/**
		 * Returns -1 if the line number is unknown.
		 */
		int line() { return m_line; }
	};


	//! %Exception thrown when result of cast is 0.
	class DynamicCastFailedException : public Exception {

	public:
		//! Constructs a dynamic cast failed exception.
		DynamicCastFailedException(const char *file = NULL, int line = -1) : Exception(file, line) {}
	};


	//! %Exception thrown when not enough memory is available to execute an algorithm.
	class InsufficientMemoryException : public Exception {

	public:
		//! Constructs an insufficient memory exception.
		InsufficientMemoryException(const char *file = NULL, int line = -1) : Exception(file, line) {}
	};


	//! %Exception thrown when preconditions are violated.
	class PreconditionViolatedException : public Exception 
	{
	public:
		//! Constructs a precondition violated exception.
		PreconditionViolatedException(PreconditionViolatedCode code,
			const char *file = NULL,
			int line = -1) :
		Exception(file, line),
		m_exceptionCode(code)
		{}

		//! Constructs a precondition violated exception.
		PreconditionViolatedException(
			const char *file = NULL,
			int line = -1) :
		Exception(file, line),
		m_exceptionCode(pvcUnknown)
		{}

		//! Returns the error code of the exception.
		PreconditionViolatedCode exceptionCode() const { return m_exceptionCode; }

	private:
		PreconditionViolatedCode m_exceptionCode; //!< The error code specifying the exception.
	}; // class PreconditionViolatedException



	//! %Exception thrown when an algorithm realizes an internal bug that prevents it from continueing.
	class AlgorithmFailureException : public Exception
	{
	public:

		//! Constructs an algorithm failure exception.
		AlgorithmFailureException(AlgorithmFailureCode code,
			const char *file = NULL,
			int line = -1) :
		Exception(file, line),
		m_exceptionCode(code)
		{}

		//! Constructs an algorithm failure exception.
		AlgorithmFailureException(
			const char *file = NULL,
			int line = -1) :
		Exception(file, line),
		m_exceptionCode(afcUnknown)
		{}

		//! Returns the error code of the exception.
		AlgorithmFailureCode exceptionCode() const { return m_exceptionCode; }

	private:
		AlgorithmFailureCode m_exceptionCode; //!< The error code specifying the exception.
	}; // class AlgorithmFailureException



	//! %Exception thrown when an external library shall be used which is not supported.
	class LibraryNotSupportedException : public Exception {
		public:
		//! Constructs a library not supported exception.
			LibraryNotSupportedException(LibraryNotSupportedCode code,
				const char *file = NULL,
				int line = -1) :
			Exception(file, line),
			m_exceptionCode(code)
			{}
				
		//! Constructs a library not supported exception.
			LibraryNotSupportedException(
				const char *file = NULL,
				int line = -1) :
			Exception(file, line),
			m_exceptionCode(lnscUnknown)
			{}

		//! Returns the error code of the exception.
		LibraryNotSupportedCode exceptionCode() const { return m_exceptionCode; }

	private:
		LibraryNotSupportedCode m_exceptionCode; //!< The error code specifying the exception.
	}; // class LibraryNotSupportedException

} // end namespace ogdf


#endif
