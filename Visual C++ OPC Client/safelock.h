// **************************************************************************
// safelock.h
//
// Description:
//	An auto releasing synchronization object using MFC.
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


#ifndef _SAFELOCK_H
#define _SAFELOCK_H


// **************************************************************************
class CSafeLock
	{
	public:
		CSafeLock (CSyncObject *pso)
			{
			ASSERT (pso);
			pso->Lock ();
			m_pso = pso;
			}

		~CSafeLock ()
			{
			m_pso->Unlock ();
			}
	
	private:
		CSyncObject *m_pso;
	};


#endif	// _SAFELOCK_H
