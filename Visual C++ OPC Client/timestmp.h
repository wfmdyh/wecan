// **************************************************************************
// timestamp.h
//
// Description:
//	Defines a class to contain time stamp data.
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


#ifndef __TIMESTMP_H
#define __TIMESTMP_H

// Marcro to determin if year y is a leap year:
#define ISLEAP(y)  (((y % 4) == 0) && ((y % 100) || (y % 400) == 0))

// Number of milliseconds per day:
#define MSPERDAY  86400000L // (3600 * 24 * 1000)

// Number of seconds per day:
#define SECPERDAY  86400L 	//(3600 * 24)

// WIN.INI date format codes
#define VDATE_MMDDYY  0
#define VDATE_DDMMYY  1
#define VDATE_YYMMDD  2

// International date format information
typedef struct _tagINTLDATEFORMAT
	{
	int   nFmtLen;
	int   nFmt;
	TCHAR szSeparator[2];
	TCHAR szShortDate[16];
	TCHAR szFmtString[16];
	BOOL  bDayLeadingZero;
	BOOL  bMonthLeadingZero;
	BOOL  bFullYear;
	} INTLDATEFORMAT;

// International time format information
typedef struct _tagINTLTIMEFORMAT
	{
	int   nFmtLen;
	TCHAR szSeparator[2];
	TCHAR szAMString[8];
	TCHAR szPMString[8];
	BOOL  b24Hr;
	BOOL  bLeadingZero;
	} INTLTIMEFORMAT;

// **************************************************************************
class CTimeStamp
	{
	public:
		CTimeStamp (SYSTEMTIME &st);
		CTimeStamp ()
			{
			m_lMS = 0;
			m_nDay = 0;
			}

		// Called once by owning application to initialize notification
		// mechanism.
		static void GlobalInit ();

		void Assign (SYSTEMTIME &st, bool bInitialize = true);
		void Assign ();

		bool SetTime (LPCTSTR lpTime, bool bInitialize = true);
		bool SetDate (LPCTSTR lpDate, bool bInitialize = true);

		// cnChars is the number if characters the buffer will hold minus the terminating NULL
		void FormatTime (LPTSTR szBuff, int cnChars, bool bUseSec);
		void FormatDate (LPTSTR szBuff, int cnChars);

		void IncSec (long lIntervalSec);
		void IncMS (long lIntervalMS);

		bool operator < (CTimeStamp &cts) const;
		bool operator > (CTimeStamp &cts) const;
		bool operator >= (CTimeStamp &cts) const;
		bool operator <= (CTimeStamp &cts) const;
		bool operator == (CTimeStamp &cts) const;

		int Compare (CTimeStamp &cts) const;

		long DiffMS (CTimeStamp &cts) const;
		long DiffSec (CTimeStamp &cts) const;

		void IncDay (int cnDays);

		CTimeStamp &operator -= (long lIntervalMS);

		SYSTEMTIME &GetTimeStamp () {return (m_st);}
		long GetMS () {return (m_lMS);}

		/*
		static int GetMaxDateLen ()
			{
			if (!stDateFmt.nFmtLen)
				SetDateTimeFormat ();

			return (stDateFmt.nFmtLen);
			}

		static int GetMaxTimeLen ()
			{
			if (!stTimeFmt.nFmtLen)
				SetDateTimeFormat ();

			return (stTimeFmt.nFmtLen);
			}
			*/

	private:
		// Static formatting information obtained from win.ini
		static INTLDATEFORMAT stDateFmt;
		static INTLTIMEFORMAT stTimeFmt;
		
		// Window and accompanying proc to handle notifications of
		// change to date/time formats
		static HWND sm_hwndNotify;
		static long WINAPI NotifyWndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static void SetDateTimeFormat ();

		// To synchonize access to static formatting information
		static CCriticalSection sm_cs;	

		void ConvertDateToDays ();
		void ConvertTimeToMS ();

		void ConvertDateFromDays ();
		void ConvertTimeFromMS ();

		SYSTEMTIME m_st;

		long m_lMS;		// Time value in MS since midnight
		int m_nDay;		// Date as day of the year
	};


#endif	// __TIMESTMP_H
