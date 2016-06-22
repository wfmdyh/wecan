// **************************************************************************
// editfilters.h
//
// Description:
//	Defines several special purpose edit box classes. 
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


#ifndef _SUBEDIT_H
#define _SUBEDIT_H


// **************************************************************************
// Base edit control filter
// **************************************************************************
class CEditBase : public CEdit
	{
	public:
		CEditBase () 
			{
			m_bEmpty = true;
			}
		virtual ~CEditBase () {}

	protected:
		virtual BOOL ProcessChar (UINT nChar)
			{
			ASSERT (FALSE);	// Must be provided by derived class
			return (FALSE);
			}

		bool m_bEmpty;

	protected:
		afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg long OnPaste (WPARAM wNotUsed, LPARAM lNotUsed);

		DECLARE_MESSAGE_MAP ();
	};

// **************************************************************************
// Edit control that allows A-Z, 0-9 and _ (Used for object names)
// **************************************************************************
class CNameEdit	: public CEditBase
	{
	public:
		CNameEdit () {}
	protected:
		BOOL ProcessChar (UINT nChar);
	};

// **************************************************************************
// Edit control that allows integers in various formats
// **************************************************************************
class CNumericEdit : public CEditBase
	{
	public:
		CNumericEdit () 
			{
			m_bInternal = false; 
			m_eFormat = tUnsignedDecimal;
			}
		
		typedef enum _tagINTEGERFORMAT
			{
			tSignedDecimal,
			tUnsignedDecimal,
			tOctal,
			tHex
			} INTEGERFORMAT;

		void SetFormat (INTEGERFORMAT eFormat);
		void SetValue (DWORD dwVal);
		DWORD GetValue ();

	protected:
		BOOL ProcessChar (UINT nChar);
		INTEGERFORMAT m_eFormat; 

		// These members are implemented to support use of spin buttons.
		// Because we support octal and spin buttons do not, we need to
		// always get and set text in decimal so the base of the spin
		// button can remain in decimal regardless of the selected
		// format for the edit control.
		bool m_bInternal;
		afx_msg LRESULT OnGetText (WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnSetText (WPARAM /*wParam*/, LPARAM lParam);

		DECLARE_MESSAGE_MAP ();
	};

// **************************************************************************
// Edit control that allows 0-9,-,. and E
// **************************************************************************
class CRealNumEdit : public CEditBase
	{
	public:
		CRealNumEdit () {}

	protected:
		BOOL ProcessChar (UINT nChar);
	};

// **************************************************************************
// Edit control that allows valid file name chars
// **************************************************************************
class CFileNameEdit : public CEditBase
	{
	public:
		CFileNameEdit () {}

	protected:
		static LPCTSTR sm_szFileChars;
		BOOL ProcessChar (UINT nChar);
	};


#endif // _SUBEDIT_H
