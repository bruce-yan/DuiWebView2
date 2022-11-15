#include "FrameWnd.h"
#include "NativeObj.h"
#include "CheckFailure.h"
#include <wrl.h>
#include <nlohmann/json.hpp>
#include <atlstr.h>
#include <codecvt>

using json = nlohmann::json;

std::wstring_convert<std::codecvt_utf8<wchar_t>> g_conv;

void CFrameWnd::Init() {
	m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
	m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));
	m_pWebMain = static_cast<CWebview2UI*>(m_pm.FindControl(_T("mainview")));
	m_pWebTest = static_cast<CWebview2UI*>(m_pm.FindControl(_T("testview")));

	CDuiRect rc;
	GetWindowRect(m_hWnd, &rc);
	::SetWindowPos(m_hWnd,
		NULL,
		rc.left,
		rc.top,
		MulDiv(rc.GetWidth(), GetScale(), 100),
		MulDiv(rc.GetHeight(), GetScale(), 100),
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void CFrameWnd::OnPrepare() 
{

}

void CFrameWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("windowinit")) {
		OnPrepare();
	}
	else if( msg.sType == _T("click") ) {
		if( msg.pSender == m_pCloseBtn ) {
			PostQuitMessage(0);
			return; 
		}
		else if( msg.pSender == m_pMinBtn ) { 
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); return; 
		}
		else if( msg.pSender == m_pMaxBtn ) { 
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; 
		}
		else if( msg.pSender == m_pRestoreBtn ) { 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
		}
	}
	else if (msg.sType == _T("selectchanged")) {
		CDuiString name = msg.pSender->GetName();
		if (name == _T("mainswitch")) {
			auto pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("content")));
			if (pControl && pControl->GetCurSel() != 0) pControl->SelectItem(0);
		}
		else if (name == _T("testswitch")) {
			auto pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("content")));
			if (pControl && pControl->GetCurSel() != 1) pControl->SelectItem(1);
		}
	}
	//以下为Webview2事件
	//WebView2控件创建成功
	else if (msg.sType == _T("OnWebView2Created")) {
		auto wv2 = static_cast<DuiLib::CWebview2UI*>(msg.pSender);

		wil::com_ptr<ICoreWebView2> webView = wv2->GetWebView2();
		//1. 为本地网页目录添加资源映射
		wil::com_ptr<ICoreWebView2_3> webView2_3;
		webView2_3 = webView.try_query<ICoreWebView2_3>();
		if (webView2_3)
		{
			webView2_3->SetVirtualHostNameToFolderMapping(
				L"native.test", L"..\\..\\nativeweb",
				COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
		}

		//2. 插入 Native 对象, 供JavaScript调用
		wv2->InjectObjectToScript<NativeObj>();

		//3. 处理新窗口请求
		webView->add_NewWindowRequested(
			Callback<ICoreWebView2NewWindowRequestedEventHandler>(
				[this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) {
					//所有请求在原窗口中打开
					args->put_NewWindow(sender);
					return S_OK;
				})
			.Get(), nullptr);
	}
	//开始导航
	else if (msg.sType == _T("NavigationStarting")) {
		ICoreWebView2* webview = reinterpret_cast<ICoreWebView2*>(msg.wParam);
		ICoreWebView2NavigationStartingEventArgs* args = reinterpret_cast<ICoreWebView2NavigationStartingEventArgs*>(msg.lParam);
		//...
	}
	//导航完成
	else if (msg.sType == _T("NavigationCompleted")) {
		ICoreWebView2* webview = reinterpret_cast<ICoreWebView2*>(msg.wParam);
		ICoreWebView2NavigationCompletedEventArgs* args = reinterpret_cast<ICoreWebView2NavigationCompletedEventArgs*>(msg.lParam);
		BOOL success = FALSE;
		CHECK_FAILURE(args->get_IsSuccess(&success));
		if (!success)
		{
			COREWEBVIEW2_WEB_ERROR_STATUS webErrorStatus;
			CHECK_FAILURE(args->get_WebErrorStatus(&webErrorStatus));
			//这里可以根据错误进行不同处理，比如显示一定自定义的错误页
			//m_pWebview2->Navigate(L"https://native.myassets/myerror.html");
		}
	}
	//收到web前端的调用消息
	else if (msg.sType == _T("WebMessageReceived")) {
		ICoreWebView2* webview = reinterpret_cast<ICoreWebView2*>(msg.wParam);
		ICoreWebView2WebMessageReceivedEventArgs* args = reinterpret_cast<ICoreWebView2WebMessageReceivedEventArgs*>(msg.lParam);
		wil::unique_cotaskmem_string message;
		CHECK_FAILURE(args->TryGetWebMessageAsString(&message));
		auto msg = g_conv.to_bytes(message.get());
		try {
			auto cmdObj = json::parse(msg);
			auto cmd = cmdObj["cmd"].get<std::string>();
			if (cmd.compare("hello") == 0){
				webview->PostWebMessageAsJson(LR"({"cmd":"hello-response"})");
			}
			else if (cmd.compare("runscript") == 0) {
				m_pWebTest->ExecuteScript(LR"(scriptFunction(5,11);)", NULL);
			}
		}
		catch (std::exception& e) {
			ATLTRACE("%s\n", e.what());
		}
	}
	//iFrame 导航完成
	else if (msg.sType == _T("FrameNavigationCompleted")) {
		ICoreWebView2* webview = reinterpret_cast<ICoreWebView2*>(msg.wParam);
		ICoreWebView2NavigationCompletedEventArgs* args = reinterpret_cast<ICoreWebView2NavigationCompletedEventArgs*>(msg.lParam);
		BOOL success;
		CHECK_FAILURE(args->get_IsSuccess(&success));
		if (!success)
		{
			COREWEBVIEW2_WEB_ERROR_STATUS webErrorStatus;
			CHECK_FAILURE(args->get_WebErrorStatus(&webErrorStatus));
			wil::unique_cotaskmem_string uri;
			webview->get_Source(&uri);
			//...
		}
	}
}

LRESULT CFrameWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_pm.Init(m_hWnd);
	CDialogBuilder builder;
	CControlUI* pRoot = builder.Create(_T("main.xml"), NULL, NULL, &m_pm);

	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);
	Init();
	return 0;
}

LRESULT CFrameWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CFrameWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostQuitMessage(0L);

	bHandled = FALSE;
	return 0;
}

LRESULT CFrameWnd::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( ::IsIconic(*this) ) bHandled = FALSE;
    return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CFrameWnd::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFrameWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFrameWnd::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	if (!::IsZoomed(*this)) {
		RECT rcSizeBox = m_pm.GetSizeBox();
		if (pt.y < rcClient.top + rcSizeBox.top) {
			if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
			return HTTOP;
		}
		else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
			if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
			return HTBOTTOM;
		}
		if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
		if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
	}

	RECT rcCaption = m_pm.GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
		CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
		if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0 &&
			_tcscmp(pControl->GetClass(), DUI_CTR_OPTION) != 0 &&
			_tcscmp(pControl->GetClass(), DUI_CTR_TEXT) != 0)
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CFrameWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SIZE szRoundCorner = m_pm.GetRoundCorner();
    if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
        CDuiRect rcWnd;
        ::GetWindowRect(*this, &rcWnd);
        rcWnd.Offset(-rcWnd.left, -rcWnd.top);
        rcWnd.right++; rcWnd.bottom++;
        HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
        ::SetWindowRgn(*this, hRgn, TRUE);
        ::DeleteObject(hRgn);
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CFrameWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int primaryMonitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int primaryMonitorHeight = ::GetSystemMetrics(SM_CYSCREEN);
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CDuiRect rcWork = oMonitor.rcWork;
	rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);
	if (rcWork.right > primaryMonitorWidth) rcWork.right = primaryMonitorWidth;
	if (rcWork.bottom > primaryMonitorHeight) rcWork.right = primaryMonitorHeight;
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
	lpMMI->ptMaxPosition.x = rcWork.left;
	lpMMI->ptMaxPosition.y = rcWork.top;
	lpMMI->ptMaxSize.x = rcWork.right;
	lpMMI->ptMaxSize.y = rcWork.bottom;
	bHandled = FALSE;
	return 0;
}

LRESULT CFrameWnd::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if( wParam == SC_CLOSE ) {
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(GetHWND());
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed ) {
		if( !bZoomed ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(true);
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(false);
		}
	}
	return lRes;
}

LRESULT CFrameWnd::OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;
	CDPI::OnDPIChanged(m_hWnd, wParam, lParam);
	return 0;
}

LRESULT CFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ) {
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	case WM_DPICHANGED:	   lRes = OnDPIChanged(uMsg, wParam, lParam, bHandled); break;
	default:
	bHandled = FALSE;
	}
	if( bHandled ) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
