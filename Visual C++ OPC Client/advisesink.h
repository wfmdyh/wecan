// **************************************************************************
// advisesink.h
//
// Description:
//	Defines the CKAdviseSink class.
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


#ifndef _ADVISESINK_H
#define _ADVISESINK_H


// **************************************************************************
class CKAdviseSink : public IAdviseSink
	{
	public:
		CKAdviseSink ();

		// IUnknown Methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID *ppInterface);
		STDMETHODIMP_(ULONG) AddRef ();
		STDMETHODIMP_(ULONG) Release ();

		// IAdviseSink Methods
		STDMETHODIMP_(void) OnDataChange (FORMATETC *pFormatEtc, STGMEDIUM *pMedium); 
		STDMETHODIMP_(void) OnViewChange (unsigned long dwAspect, long lindex) {/*Not implemented*/};
		STDMETHODIMP_(void) OnRename (LPMONIKER pmk) {/*Not implemented*/};
		STDMETHODIMP_(void) OnSave () {/*Not implemented*/};
		STDMETHODIMP_(void) OnClose () {/*Not implemented*/};

	private: 

	protected:
		ULONG m_cRef; 
	};


#endif // _ADVISESINK_H
