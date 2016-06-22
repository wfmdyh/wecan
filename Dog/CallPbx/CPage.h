// CPage.h : CCPage ������
#pragma once
#include "resource.h"       // ������
#include <atlctl.h>
#include "CallPbx_i.h"
#include "_ICPageEvents_CP.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif

using namespace ATL;

// CCPage
class ATL_NO_VTABLE CCPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ICPage, &IID_ICPage, &LIBID_CallPbxLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IOleControlImpl<CCPage>,
	public IOleObjectImpl<CCPage>,
	public IOleInPlaceActiveObjectImpl<CCPage>,
	public IViewObjectExImpl<CCPage>,
	public IOleInPlaceObjectWindowlessImpl<CCPage>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CCPage>,
	public CProxy_ICPageEvents<CCPage>,
	public IQuickActivateImpl<CCPage>,
#ifndef _WIN32_WCE
	public IDataObjectImpl<CCPage>,
#endif
	public IProvideClassInfo2Impl<&CLSID_CPage, &__uuidof(_ICPageEvents), &LIBID_CallPbxLib>,
	public IObjectSafetyImpl<CCPage, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
	public CComCoClass<CCPage, &CLSID_CPage>,
	public CComControl<CCPage>
{
public:

	int m_nTimer;
	CCPage()
	{
		m_bWindowOnly = true;//�����ʼ��Ϊtrue������m_hWnd��ΪNULL
	}

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_INSIDEOUT |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST
)

DECLARE_REGISTRY_RESOURCEID(IDR_CPAGE)


BEGIN_COM_MAP(CCPage)
	COM_INTERFACE_ENTRY(ICPage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IQuickActivate)
#ifndef _WIN32_WCE
	COM_INTERFACE_ENTRY(IDataObject)
#endif
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(CCPage)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// ʾ����
	// PROP_ENTRY_TYPE("������", dispid, clsid, vtType)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CCPage)
	CONNECTION_POINT_ENTRY(__uuidof(_ICPageEvents))
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CCPage)
	CHAIN_MSG_MAP(CComControl<CCPage>)
	DEFAULT_REFLECTION_HANDLER()
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
END_MSG_MAP()
// �������ԭ��:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* const arr[] =
		{
			&IID_ICPage,
		};

		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// ICPage
public:
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		// ��������������Ϊ di.prcBounds ָ���ľ���
		HRGN hRgnOld = NULL;
		if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
			hRgnOld = NULL;
		bool bSelectOldRgn = false;

		HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

		if (hRgnNew != NULL)
		{
			bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
		}

		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("CPage");
#ifndef _WIN32_WCE
		TextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			pszText,
			lstrlen(pszText));
#else
		ExtTextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			ETO_OPAQUE,
			NULL,
			pszText,
			ATL::lstrlen(pszText),
			NULL);
#endif

		if (bSelectOldRgn)
			SelectClipRgn(di.hdcDraw, hRgnOld);

		return S_OK;
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
	STDMETHOD(Connect)(BSTR m_IP, BSTR m_Port);
	STDMETHOD(DisConnect)(void);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	STDMETHOD(GetDogCode)(BSTR* pResult);
	STDMETHOD(GetCardCode)(LONG ComNumber, LONG Baud, BSTR* pResult);
	STDMETHOD(SetCardCode)(LONG ComNumber, LONG Baud, BSTR Code, BSTR* pResult);
};

OBJECT_ENTRY_AUTO(__uuidof(CPage), CCPage)
