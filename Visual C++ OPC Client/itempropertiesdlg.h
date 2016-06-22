// **************************************************************************
// itempropertiesdlg.h
//
// Description:
//	Defines a dialog class for assigning OPC item properties.
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


#ifndef _ITEMPROPERTIESDLG_H
#define _ITEMPROPERTIESDLG_H

class CKItem;
class CKServer;


// **************************************************************************
class CKItemPropertiesDlg : public CDialog
	{
	// Construction
	public:
		CKItemPropertiesDlg (CObArray &cItemList, 
			DWORD cdwItems, 
			IOPCItemMgt *pIItemMgt,
			CKServer *pServer,
			CWnd *pParent = NULL);   // standard constructor

		bool IsModified () {return (m_bModified);}

	// Dialog Data
		//{{AFX_DATA(CKItemPropertiesDlg)
		enum { IDD = IDD_ITEM_PROPERTIES };
		CString	m_strAccessPath;
		BOOL	m_bActive;
		CString	m_strItemID;
		//}}AFX_DATA
		int m_vtDataType;

	private:
		CObArray *m_pItemList;
		int m_cnItems;

		IOPCItemMgt *m_pIItemMgt;
		IOPCItemProperties *m_pIItemProps;

		int m_nSelIndex;
		CImageButton m_cNext;
		CImageButton m_cPrev;
		CSmartToolTip m_cToolTip;

		CKServer *m_pServer;

		bool m_bModified;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKItemPropertiesDlg)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		void UpdateStatus ();
		void SelectItem (int nIndex);
		void GetValue (VARIANT &vtVal, TCHAR *pBuffer, int nBufLen);

		CKItem* GetSelectedItem ();

		void OnApply ();

		// Generated message map functions
		//{{AFX_MSG(CKItemPropertiesDlg)
		virtual BOOL OnInitDialog ();
		afx_msg void OnNext ();
		afx_msg void OnPrevious ();
		afx_msg void OnActive ();
		afx_msg void OnChangeDataType ();
		virtual void OnOK ();
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP ()
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _ITEMPROPERTIESDLG_H
