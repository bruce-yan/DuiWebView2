#pragma once
#include <wrl.h>
#include "NativeObjTest_h.h"
#include <atlbase.h>

class NativeObj : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    INativeObj, IDispatch>
{
public:
    static LPCWSTR className() { return L"NativeObj"; }
    NativeObj(HWND hHostWnd) { m_hostWnd = hHostWnd; }
    ~NativeObj() {}
    STDMETHODIMP get_Info(BSTR* pVal) override;
    STDMETHODIMP put_Info(BSTR val) override;

    STDMETHODIMP Add(long param1, long param2, long *pRet) override;

    // IDispatch implementation
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) override;

    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;

    STDMETHODIMP GetIDsOfNames(
        REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override;

    STDMETHODIMP Invoke(
        DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
        VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override;
private:
    CComBSTR m_info;
    HWND m_hostWnd;
};

