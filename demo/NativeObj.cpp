#include "NativeObj.h"
#include <atlstr.h>

typedef struct _memberInfo {
	int id;
	const wchar_t* funcName;
}MemberInfo;


enum DISPIDLIST {
	idAdd = 1,
};

static MemberInfo mapMember[] =
{
	{ idAdd, L"Add" },
};


enum PROPERTYDISPIDLIST
{
	idInfo = 100,
};

static MemberInfo mapProperty[] =
{
	{ idInfo, L"info" },
};


STDMETHODIMP NativeObj::get_Info(BSTR* pVal)
{
	if (!pVal) return E_INVALIDARG;
	*pVal = m_info.Copy();
	return S_OK;
}

STDMETHODIMP NativeObj::put_Info(BSTR val)
{
	m_info = val;
	return S_OK;
}

STDMETHODIMP NativeObj::Add(long param1, long param2, long* pRet)
{
	if (!pRet) return E_INVALIDARG;
	*pRet = param1 + param2;
	return S_OK;
}

// IDispatch implementation
STDMETHODIMP NativeObj::GetTypeInfoCount(UINT* pctinfo)
{
	*pctinfo = 1;
	return S_OK;
}

STDMETHODIMP NativeObj::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP NativeObj::GetIDsOfNames(
    REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
	HRESULT hr = S_OK;
	for (int i = 0; i < (int)cNames; i++)
	{
		CString cszName = rgszNames[i];
		int memberCount = sizeof(mapMember) / sizeof(MemberInfo);
		BOOL bFind = FALSE;
		for (int j = 0; j < memberCount; j++)
		{
			if (cszName == mapMember[j].funcName)
			{
				bFind = TRUE;
				rgDispId[i] = mapMember[j].id;
				break;
			}
		}

		if (!bFind)
		{
			int propertyCount = sizeof(mapProperty) / sizeof(MemberInfo);
			for (int j = 0; j < propertyCount; ++j)
			{
				if (cszName == mapProperty[j].funcName)
				{
					bFind = TRUE;
					rgDispId[i] = mapProperty[j].id;
					break;
				}
			}
		}

		if (bFind)
			continue;

		// One or more are unknown so set the return code accordingly
		hr = ResultFromScode(DISP_E_UNKNOWNNAME);
		rgDispId[i] = DISPID_UNKNOWN;
	}

	return hr;
}

STDMETHODIMP NativeObj::Invoke(
    DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
    VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (!pDispParams)
		return E_INVALIDARG;

	if (wFlags & DISPATCH_METHOD)
	{
		int memberCount = sizeof(mapMember) / sizeof(MemberInfo);
		switch (dispIdMember)
		{
		case idAdd:
		{
			if (pDispParams->cArgs < 2) {
				return E_INVALIDARG;
			}

			if (pDispParams->rgvarg[0].vt != VT_I4 || 
				pDispParams->rgvarg[1].vt != VT_I4) {
				return E_INVALIDARG;
			}

			VariantClear(pVarResult);
			pVarResult->vt = VT_I4;

			return Add(pDispParams->rgvarg[1].intVal, pDispParams->rgvarg[0].intVal, &pVarResult->lVal);
		}
		break;
		default:
			return DISP_E_MEMBERNOTFOUND;
			break;
		}
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		switch (dispIdMember)
		{
		case idInfo:
			if (pVarResult) {
				long mode = 0;
				VariantClear(pVarResult);
				pVarResult->vt = VT_BSTR;
				get_Info(&pVarResult->bstrVal);
				return S_OK;
			}
			break;
		default:
			return DISP_E_MEMBERNOTFOUND;
		}
	}
	else if (wFlags & DISPATCH_PROPERTYPUT)
	{
		switch (dispIdMember)
		{
		case idInfo:
			if (pDispParams->cArgs < 1) {
				return E_INVALIDARG;
			}

			if (pDispParams->rgvarg[0].vt != VT_BSTR) {
				return E_INVALIDARG;
			}

			put_Info(pDispParams->rgvarg[0].bstrVal);
			return S_OK;
		default:
			return DISP_E_MEMBERNOTFOUND;
		}
	}
	return DISP_E_MEMBERNOTFOUND;
}
