// **************************************************************************
// serverpropertysheet.h
//
// Description:
//	Defines a property sheet class and associated property page classes for
//	OPC server properties.
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


#ifndef _SERVERPROPERTYSHEET_H
#define _SERVERPROPERTYSHEET_H

class CKServer;


// **************************************************************************
class CKServerGeneralPage : public CPropertyPage
	{
	DECLARE_DYNCREATE (CKServerGeneralPage)

	// Construction
	public:
		// constructor
		CKServerGeneralPage ();

	// Dialog Data
		//{{AFX_DATA(CKServerGeneralPage)
		enum { IDD = IDD_SERVER_GENERAL };
		CString	m_strRemoteMachine;
		CString	m_strProgID;
		//}}AFX_DATA

	private:
		void DisplayComponentCatList (HTREEITEM hParent, CATID clsid);
		void DisplayGeneralOPCServers (HTREEITEM hParent);

		CTreeCtrl *m_pServerList;
		CImageList m_cImageList;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKServerGeneralPage)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	public:

	protected:

		// Generated message map functions
		//{{AFX_MSG(CKServerGeneralPage)
		virtual BOOL OnInitDialog ();
		afx_msg void OnSelChanged (NMHDR *pNMHDR, LRESULT *pResult);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()

	private:
	};

// **************************************************************************
class CKServerStatusPage : public CPropertyPage
	{
	DECLARE_DYNCREATE (CKServerStatusPage)

	// Construction
	public:
		CKServerStatusPage ();
		~CKServerStatusPage ();

	// Dialog Data
		//{{AFX_DATA(CKServerStatusPage)
		enum { IDD = IDD_SERVER_STATUS };
			// NOTE - ClassWizard will add data members here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_DATA

		CKServer *m_pServer;

	// Overrides
		// ClassWizard generate virtual function overrides
		//{{AFX_VIRTUAL(CKServerStatusPage)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		// Generated message map functions
		//{{AFX_MSG(CKServerStatusPage)
		virtual BOOL OnInitDialog ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};

// **************************************************************************
class CKServerInterfacesPage : public CPropertyPage
	{
	DECLARE_DYNCREATE (CKServerInterfacesPage)

	// Construction
	public:
		CKServerInterfacesPage ();
		~CKServerInterfacesPage ();

	// Dialog Data
		//{{AFX_DATA(CKServerInterfacesPage)
		enum { IDD = IDD_SUPPORTED_INTERFACES };
			// NOTE - ClassWizard will add data members here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_DATA

		CKServer *m_pServer;

	private:
		CImageList m_cImageList;

	// Overrides
		// ClassWizard generate virtual function overrides
		//{{AFX_VIRTUAL(CKServerInterfacesPage)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		// Generated message map functions
		//{{AFX_MSG(CKServerInterfacesPage)
		virtual BOOL OnInitDialog ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};

// **************************************************************************
class CKServerPropertySheet : public CPropertySheet
	{
	DECLARE_DYNAMIC (CKServerPropertySheet)

	// Construction
	public:
		CKServerPropertySheet (CKServer *pServer = NULL);

	// Attributes
	public:

	private:
		CKServerGeneralPage m_cGeneralPage;
		CKServerStatusPage m_cStatusPage;
		CKServerInterfacesPage m_cInterfacePage;

		CKServer *m_pServer;

	// Operations
	public:
		CKServer* GetServer () {return (m_pServer);}

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKServerPropertySheet)
		public:
		virtual int DoModal ();
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CKServerPropertySheet ();
		// Generated message map functions
	protected:
		//{{AFX_MSG(CKServerPropertySheet)
			// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _SERVERPROPERTYSHEET_H
