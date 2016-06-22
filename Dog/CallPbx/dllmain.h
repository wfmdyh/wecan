// dllmain.h : 模块类的声明。

class CCallPbxModule : public ATL::CAtlDllModuleT< CCallPbxModule >
{
public :
	DECLARE_LIBID(LIBID_CallPbxLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CALLPBX, "{A90D4B61-835A-4C73-8BED-1F6D08AD4796}")
};

extern class CCallPbxModule _AtlModule;
