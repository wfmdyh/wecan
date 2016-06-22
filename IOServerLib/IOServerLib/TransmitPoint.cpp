#include "stdafx.h"
#include "TransmitPoint.h"


TransmitPoint::TransmitPoint()
{
	VariantInit(&m_varValue);

	m_colChannel = -1;
	m_colDevice = -1;
	m_colPoint = -1;
}


TransmitPoint::~TransmitPoint()
{

}

void TransmitPoint::AddPointParam(string strName, string strValue)
{
	m_ParamPoint[strName] = strValue;
}

string TransmitPoint::GetValue()
{
	char szValue[50] = { 0 };
	string strValue = "";
	if (m_varValue.vt == VT_R4)
	{
		float val = m_varValue.fltVal;
		_snprintf_s(szValue, 50, "%f", val);
		strValue._Copy_s(szValue, 50, 50);
	}

	return strValue;
}

void TransmitPoint::SetValue(VARIANT var)
{
	VariantCopy(&m_varValue, &var);
}

//将一个VARIANT映射成字符串的值
BOOL TransmitPoint::MapVariantToStringVal(VARIANT vtVal, wstring &strValue)
{
	// Assume success until a problem arises:
	BOOL bSuccess = TRUE;
	wchar_t wszVal[20] = { 0 };
	// Cast string to proper value:
	switch (vtVal.vt)
	{
	case VT_BOOL:
		swprintf_s(wszVal, L"%hd", vtVal.boolVal);
		break;
	case VT_UI1:
		swprintf_s(wszVal, L"%uc", vtVal.bVal);
		break;
	case VT_I1:
		swprintf_s(wszVal, L"%c", vtVal.cVal);
		break;
	case VT_UI2:
		swprintf_s(wszVal, L"%hu", vtVal.uiVal);
		break;
	case VT_I2:
		swprintf_s(wszVal, L"%d", vtVal.iVal);
		break;
	case VT_UI4:
		swprintf_s(wszVal, L"%lu", vtVal.ulVal);
		break;
	case VT_I4:
		swprintf_s(wszVal, L"%ld", vtVal.lVal);
		break;
	case VT_R4:
		swprintf_s(wszVal, L"%hf", vtVal.fltVal);
		break;
	case VT_R8:
		swprintf_s(wszVal, L"%f", vtVal.dblVal);
		break;
	case VT_BSTR:
		swprintf_s(wszVal, L"%s", vtVal.bstrVal);
		break;
		// We don't expect array types.  If so, return false:
	case VT_UI1 | VT_ARRAY:
	case VT_I1 | VT_ARRAY:
	case VT_UI2 | VT_ARRAY:
	case VT_I2 | VT_ARRAY:
	case VT_UI4 | VT_ARRAY:
	case VT_I4 | VT_ARRAY:
	case VT_R4 | VT_ARRAY:
	case VT_R8 | VT_ARRAY:
		bSuccess = FALSE;
		break;

	default:
		// Unexpected variant type.  Return false:
		bSuccess = FALSE;
		break;
	}
	strValue = wszVal;
	// Return success:
	return (bSuccess);
}

void TransmitPoint::SetColIndex(int nChannel, int nDevice, int nPoint)
{
	m_colChannel = nChannel;
	m_colDevice = nDevice;
	m_colPoint = nPoint;
}

int TransmitPoint::GetColChannel()
{
	return m_colChannel;
}

int TransmitPoint::GetColDevice()
{
	return m_colDevice;
}

int TransmitPoint::GetColPoint()
{
	return m_colPoint;
}

map<string, string> TransmitPoint::GetParams()
{
	return m_ParamPoint;
}

VARIANT TransmitPoint::GetVarValue()
{
	return m_varValue;
}