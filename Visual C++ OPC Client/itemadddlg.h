// **************************************************************************
// itemadddlg.h
//
// Description:
//	Defines a dialog class for adding OPC items.  Allows user to browse for
//	items and other handy things.
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


#ifndef _ITEMADDDLG_H
#define _ITEMADDDLG_H

class CKGroup;
class CKItem;


// **************************************************************************
class CKItemAddDlg : public CDialog
	{
	// Construction
	public:
		CKItemAddDlg (CKGroup *pGroup, IOPCBrowseServerAddressSpace *pIBrowse, CWnd *pParent = NULL);   // standard constructor
		~CKItemAddDlg ();

	// Dialog Data
		//{{AFX_DATA(CKItemAddDlg)
		enum { IDD = IDD_GROUP_ADDITEMS };
		CString	m_strAccessPath;
		BOOL	m_bActive;
		CString	m_strItemID;
		int	m_vtDataType;
		//}}AFX_DATA
		BOOL m_bAutoValidate;
		BOOL m_bBrowseFlat;

	protected:
		bool m_bModified;
		
		CKGroup *m_pGroup;
		IOPCBrowseServerAddressSpace *m_pIBrowse;
		IOPCItemMgt *m_pIItemMgt;

		CString	m_strFilterBranch;
		CString	m_strFilterLeaf;
		VARTYPE m_vtFilterType;
		DWORD m_dwFilterAccessRights;
		OPCNAMESPACETYPE m_cOpcNameSpace;

		CTreeCtrl *m_pBranchList;
		CListCtrl *m_pLeafList;
		CImageList m_cBranchImageList;
		CImageList m_cLeafImageList;

		int m_nSelIndex;
		int m_nListIndex;
		CObArray m_cItemList;
		CImageButton m_cNext;
		CImageButton m_cPrev;
		CImageButton m_cNew;
		CImageButton m_cDuplicate;
		CImageButton m_cDelete;
		CImageButton m_cValidate;
		CSmartToolTip m_cToolTip;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKItemAddDlg)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	public:
		CObArray& GetItemList () {return (m_cItemList);}
		int GetItemCount () {return (m_nListIndex);}

	protected:
		bool OnApplyChange ();
		void UpdateStatus ();

		void SelectItem (int nIndex, bool bDuplicate = false, bool bDotBitAddress = false);
		void AutoIncrementID (bool bDotBitAddress);

		HRESULT Validate (bool *pbDotBitAddress = NULL);

		// tag browsing 
		void InitializeBrowser ();
		void BrowseRootLevel ();
		void AddBranches (LPENUMSTRING pIEnumString, HTREEITEM hParent, DWORD dwData);
		void AddLeaves (LPENUMSTRING pIEnumString);
		void ExpandBranch (HTREEITEM hItem);
		void SelectBranch (HTREEITEM hItem);
		void AddDummyBranch (HTREEITEM hParent);
		void RemoveDummyBranch (HTREEITEM hParent);
		void DeleteChildBranches (HTREEITEM hParent);

		void BrowseToRoot ();

		// Generated message map functions
		//{{AFX_MSG(CKItemAddDlg)
		afx_msg void OnNew ();
		virtual BOOL OnInitDialog ();
		virtual void OnOK ();
		virtual void OnCancel ();
		afx_msg void OnBranchExpanding (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnBranchSelected (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnAccessFilterChange ();
		afx_msg void OnLeafFilterChange ();
		afx_msg void OnVartypeFilterChange ();
		afx_msg void OnNext ();
		afx_msg void OnPrevious ();
		afx_msg void OnValidateItem ();
		afx_msg void OnBranchFilterChange ();
		afx_msg void OnClickLeafList (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnChange ();
		afx_msg void OnDelete ();
		afx_msg void OnDuplicate ();
		afx_msg void OnAddLeaves ();
		afx_msg void OnShowWindow (BOOL bShow, UINT nStatus);
		afx_msg void OnBrowseFlat ();
		afx_msg void OnAutoValidate ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};

// **************************************************************************
class CKDuplicateItemCountDlg : public CDialog
	{
	public:
		CKDuplicateItemCountDlg ();
		int m_cnDuplicateItems;

	protected:
		virtual void DoDataExchange (CDataExchange *pDX);
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _ITEMADDDLG_H
