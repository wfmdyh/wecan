// **************************************************************************
// grouppropertysheet.h
//
// Description:
//	Defines a property sheet class and associated propety page classes for
//	OPC group properties.
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


#ifndef _GROUPPROPERTYSHEET_H
#define _GROUPPROPERTYSHEET_H

class CKServer;
class CKGroup;


// **************************************************************************
class CKGroupGeneralPage : public CPropertyPage
	{
	DECLARE_DYNCREATE (CKGroupGeneralPage)

	// Construction
	public:
		CKGroupGeneralPage ();
		~CKGroupGeneralPage ();

	// Dialog Data
		//{{AFX_DATA(CKGroupGeneralPage)
		enum { IDD = IDD_GROUP_GENERAL };
		BOOL	m_bActive;
		DWORD	m_dwLanguageID;
		CString	m_strName;
		float	m_fDeadband;
		long	m_lBias;
		int		m_nUpdateMethod;
		DWORD	m_dwUpdateRate;
		//}}AFX_DATA

		bool IsChanged () {return (m_bModified);}
		
		void SetChanged (bool bChanged) 
			{
			m_bModified = bChanged;
			SetModified (bChanged);
			}

	private:
		CNumericEdit m_cLangIDEdit;
		CRealNumEdit m_cDeadbandEdit;
		CNumericEdit m_cBiasEdit;
		CNumericEdit m_cUpdateRateEdit;

		bool m_bModified;

	// Overrides
		// ClassWizard generate virtual function overrides
		//{{AFX_VIRTUAL(CKGroupGeneralPage)
	public:
		virtual void OnOK ();
	protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		// Generated message map functions
		//{{AFX_MSG(CKGroupGeneralPage)
		virtual BOOL OnInitDialog ();
		afx_msg void OnChange ();
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP ()
	};

// **************************************************************************
class CKGroupInterfacesPage : public CPropertyPage
	{
	DECLARE_DYNCREATE (CKGroupInterfacesPage)

	// Construction
	public:
		CKGroupInterfacesPage ();
		~CKGroupInterfacesPage ();

	// Dialog Data
		//{{AFX_DATA(CKGroupInterfacesPage)
		enum { IDD = IDD_SUPPORTED_INTERFACES };
			// NOTE - ClassWizard will add data members here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_DATA

		CKGroup *m_pGroup;

	private:
		CImageList m_cImageList;

	// Overrides
		// ClassWizard generate virtual function overrides
		//{{AFX_VIRTUAL(CKGroupInterfacesPage)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		// Generated message map functions
		//{{AFX_MSG(CKGroupInterfacesPage)
		virtual BOOL OnInitDialog ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};

// **************************************************************************
class CKGroupPropertySheet : public CPropertySheet
	{
	DECLARE_DYNAMIC (CKGroupPropertySheet)

	// Construction
	public:
		CKGroupPropertySheet (CKServer *pServer);
		CKGroupPropertySheet (CKGroup *pGroup);

	// Attributes
	public:

	// Dialog Data
	private:
		CKGroupGeneralPage m_cGeneralPage;
		CKGroupInterfacesPage m_cInterfacePage;

		CKServer *m_pServer;
		CKGroup *m_pGroup;

	// Operations
	public:
		CKGroup* GetGroup () {return (m_pGroup);}

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKGroupPropertySheet)
	public:
		virtual int DoModal ();
		virtual BOOL OnInitDialog ();
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CKGroupPropertySheet ();

		// Generated message map functions
	protected:
		//{{AFX_MSG(CKGroupPropertySheet)
		afx_msg void OnApplyNow ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _GROUPPROPERTYSHEET_H
