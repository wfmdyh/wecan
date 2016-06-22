/////////////////////////////////////////////////////////////////////////////
  --  OPC Quick Client Distributed Source Code Version  --
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
INTRODUCTION:
/////////////////////////////////////////////////////////////////////////////

  The source code provide here is for a slightly stripped down version of our 
  OPC Quick Client application. (None of the code removed is central to 
  understanding how to use OPC.)  This application has been around for a few
  years and is used extensively by our developers and testers - it's proven
  code.  In it, you will see how to browse OPC servers, connect to them, add
  groups and items, and read and write data.  OPC versions 1.0 and 2.0 are 
  both supported.  You may borrow freely from this code to create your own
  OPC client applications.

  The workspace and project files were created using Microsoft Visual C++ 6.0.  
  Several project configurations are provided: Win32 Release, Win32 Debug,
  Unicode Release, and Unicode Debug.

  Before you dive into the code, it is recommended that you first familiarize
  yourself with the OPC Specification.  This information is contained in the
  "Data Access Custom Interface Standard" document publicly available for
  download from opcfoundation.org.  It is also recommended that you be fairly
  proficient with the C++ programming language, MFC, and COM.  
  
  Though a great deal of internal documentation is included in the code, you 
  may need to refer to the Microsoft Developer's Network (MSDN) documentation 
  for additional information on Windows specific functions, structures, and 
  MFC classes.

  There is a one-to-one relationship between certain object in a server 
  and here in this client.  For example, the client may maintain a CKGroup
  object which is associated with an OPC Group object in a server.  To 
  prevent confusion, objects on the server side are referred to as OPC <Objects>
  and the associated objects on the client side are given by their class name,
  which generally start with "CK" for Class Kepware, or simply "object".

  When you begin examining the code, we suggest that you look at the COM
  interface classes first:  advisesink.cpp, datasink20.cpp, and 
  shutdownsink.cpp.  From there, you should look at server.cpp, group.cpp,
  then item.cpp.  This covers all of the OPC stuff.  Having done that, all
  of the Windows GUI code will make more sense.  Look at document.cpp first,
  then the associated views: groupview.cpp, itemview.cpp, and eventview.cpp.
  You can look at incidental dialog and property sheet classes and the 
  miscellaneous utility classes like those in editfilters.cpp as you go.

  Markers used by the Visual C++ class wizard ("//{{AFX_VIRTUAL(CKDocument)",
  "//{{AFX_DATA_MAP(CKServerEnumerateGroupsDlg)", etc.) are scattered 
  throughout the code.  If you wish to make your "borrowed" code a bit more
  "pretty", you can remove them.  Just be aware that the class wizard will
  no longer work if you do.

  You will need to have the VC++ UNICODE libraries installed on your system
  in order to build the application.  These libraries are NOT installed by
  default, so make sure you have them using the steps below:
  
	1) Run the VC++ 6.0 install program and allow it to detect the components
	   installed on your system.
	2) Click the "Add/Remove" button.
	3) Select the "VC++ MFC and Template Libraries" item and click the "Change
	   Options" button.
	4) Select the "MS Foundation Class Libraries" item and click the "Change
	   Options" button.
	5) Make sure "Shared Libraries for UNICODE" is checked.  You might also
	   want to check "Static Libraries for UNICODE" while you're at it, 
	   though you won't need them to build the Quick Client project as is.


/////////////////////////////////////////////////////////////////////////////
LEGAL STUFF:
/////////////////////////////////////////////////////////////////////////////

  This programming example is provided "AS IS".  As such Kepware, Inc.
  makes no claims to the worthiness of the code and does not warranty
  the code to be error free.  It is provided freely and can be used in
  your own projects.  If you do find this code useful, place a little
  marketing plug for Kepware in your code.  While we would love to help
  every one who is trying to write a great OPC client application, the 
  uniqueness of every project and the limited number of hours in a day 
  simply prevents us from doing so.  If you really find yourself in a
  bind, please contact Kepware's technical support.  We will not be able
  to assist you with server related problems unless you are using KepServer
  or KepServerEx.