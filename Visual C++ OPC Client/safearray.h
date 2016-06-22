// **************************************************************************
// safearray.h
//
// Description:
//	Defines a SAFEARRAY wrapper class.
//
// DISCLAIMER:
//	This programming example is provided "AS IS".  As such Kepware, Inc.
//	makes no claims to the worthiness of the code and does not warranty
//	the code to be error free.  It is provided freely and can be used in
//	your own projects.  If you do find this code useful, place a little
//	marketing plug for Kepware in your code.  While we would love to help
//	every one who is trying to write a great OPC client application, the 
//	uniqueness of every project and the limited number of hours in a day 
//	simply prevents us from doing so.  If you really find yourself in a
//	bind, please contact Kepware's technical support.  We will not be able
//	to assist you with server related problems unless you are using KepServer
//	or KepServerEx.
// **************************************************************************


#ifndef _SAFEARRAY_H
#define _SAFEARRAY_H


// **************************************************************************
class CSafeArray : public SAFEARRAY
	{
	public:
		CSafeArray (const CSafeArray &cSrc);
		CSafeArray (WORD wRows, WORD wCols, WORD wBytesPerElement, void *pData = NULL);
		CSafeArray (DWORD cdwElements, WORD wBytesPerElement, void *pData = NULL);
		~CSafeArray ();

		// Returns the length of the array in bytes
		DWORD GetByteLength () const;

		// Returns the number of elements in the array
		DWORD GetNumElements () const;
		DWORD GetNumRows () const;
		DWORD GetNumCols () const;

		// So we can send to a CF_TEXT client
		void Format (CString &str, VARTYPE vt) const;

		// Assigment as a vector
		void operator = (const SAFEARRAY &cSrc);

		// Overloaded operators to construct/destroy
		void *operator new (size_t s);
		void operator delete (void *p);

		// Comparison
		bool operator == (const SAFEARRAY &cSrc) const;
		bool operator != (const SAFEARRAY &cSrc) const {return !(*this == cSrc);}

	protected:
		void Alloc (DWORD dwBytes);
	};


#endif	// _SAFEARRAY_H
