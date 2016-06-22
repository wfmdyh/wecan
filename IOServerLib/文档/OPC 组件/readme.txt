http://www.eehoo.net/opcdist.zip

These files are downloaded from www.opcfoundation.org
	opc_aeps.dll
	opccomn_ps.dll
	opchda_ps.dll
	opcproxy.dll
	aprxdist.exe
	opcenum.exe
They are interface proxy files for OPC.
You must install the proxies on both client and server machines.

*************************************************************
******* Do not overwrite existing newer versions.************
*************************************************************

Installaion:
	1. copy all these files to your windows system directory. For example:
		copy opcproxy.dll c:\winnt\system32
		copy opccomn_ps.dll c:\winnt\system32
		copy opc_aeps.dll c:\winnt\system32
		copy opchda_ps.dll c:\winnt\system32
		copy aprxdist.exe c:\winnt\system32
		copy opcenum.exe c:\winnt\system32
	2. install the proxy dlls
		REGSVR32 opcproxy.dll
		REGSVR32 opccomn_ps.dll
		REGSVR32 opc_aeps.dll
		REGSVR32 opchda_ps.dll
	3. run aprxdist.exe if actxprxy.dll doesn't exist in your windows system directory
		aprxdist
	4. install opcenum.exe
		opcenum /regserver

		