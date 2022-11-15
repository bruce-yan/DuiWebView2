#include "pch.h"
#include "../inc/Webview2UI.h"
#include <wrl.h>
#include "CheckFailure.h"
#include <shlobj_core.h>
#include <atlutil.h>

using namespace Microsoft::WRL;

namespace DuiLib {
	CWebview2UI::CWebview2UI()
	{
	}

	CWebview2UI::~CWebview2UI()
	{
	}

	LPCTSTR CWebview2UI::GetClass() const
	{
		return L"Webview2UI";
	}

	LPVOID CWebview2UI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, L"Webview2") == 0) return static_cast<CWebview2UI*>(this);
		return CControlUI::GetInterface(pstrName);

	}

	void CWebview2UI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("homepage")) == 0) SetHomepage(pstrValue);
		else if (_tcsicmp(pstrName, _T("autonavi")) == 0) SetAutoNavigate(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("scriptEnable")) == 0) SetIsScriptEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("defaultScriptDialogs")) == 0) SetAreDefaultScriptDialogsEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("webMessageEnabled")) == 0) SetIsWebMessageEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("builtInErrorPageEnabled")) == 0) SetIsBuiltInErrorPageEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("allowExternalDrop")) == 0) SetAllowExternalDrop(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("contextMenusEnabled")) == 0) SetContextMenusEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("acceleratorKeysEnabled")) == 0) SetAcceleratorKeysEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("statusBarEnabled")) == 0) SetStatusBarEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("zoomControlEnabled")) == 0) SetZoomControlEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("mustHttps")) == 0) SetMustHttps(_tcsicmp(pstrValue, _T("true")) == 0);
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CWebview2UI::DoInit()
	{
		if (!m_pManager) return;
		ConfirmCacheFolder();

		HWND hWnd = m_pManager->GetPaintWindow();
		if (!IsWindow(hWnd)) {
			return;
		}

		CreateCoreWebView2EnvironmentWithOptions(nullptr, m_cachePath, nullptr,
			Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[this, hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
					CHECK_FAILURE(result);
					m_webViewEnv = env;

					// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
					env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
						this, &CWebview2UI::OnCreateCoreWebView2ControllerCompleted).Get());
					return S_OK;
				}).Get());
	}

	void CWebview2UI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		__super::SetPos(rc, bNeedInvalidate);
		if (m_webController)
			m_webController->put_Bounds(rc);
	}

	void CWebview2UI::ConfirmCacheFolder()
	{
		TCHAR appdata[MAX_PATH] = { 0 };
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata);
		CPath path(appdata);
		GetModuleFileName(NULL, appdata, MAX_PATH);
		CPath process(appdata);
		process.StripPath();
		process.RemoveExtension();
		path.Append(process);
		path.Append(L"webcache");
		if (!path.FileExists()) {
			SHCreateDirectory(nullptr, path);
		}
		m_cachePath = path;
	}

	void CWebview2UI::put_AllowExternalDrop(BOOL allow)
	{
		if (!m_webController) {
			return;
		}
		wil::com_ptr<ICoreWebView2Controller4> controller4 =
			m_webController.try_query<ICoreWebView2Controller4>();
		if (controller4)
		{
			CHECK_FAILURE(controller4->put_AllowExternalDrop(allow));
		}
	}

	void CWebview2UI::put_ContextMenusEnabled(BOOL enable)
	{
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_AreDefaultContextMenusEnabled(enable));
	}

	void CWebview2UI::put_AcceleratorKeysEnabled(BOOL enable)
	{
		if (!m_settings) return;
		wil::com_ptr<ICoreWebView2Settings3> settings3 = m_settings.try_query<ICoreWebView2Settings3>();
		CHECK_FAILURE(settings3->put_AreBrowserAcceleratorKeysEnabled(enable));
	}

	void CWebview2UI::put_StatusBarEnabled(BOOL enable)
	{
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_IsStatusBarEnabled(enable));
	}

	void CWebview2UI::put_ZoomControlEnabled(BOOL enable)
	{
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_IsZoomControlEnabled(enable));
	}

	HRESULT CWebview2UI::OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller)
	{
		RETURN_IF_FAILED(result);
		if (controller != nullptr) {
			m_webController = controller;
			m_webController->put_IsVisible(IsVisible());
			m_webController->get_CoreWebView2(&m_webview);
			m_webview->get_Settings(&m_settings);

			auto color = GetBkColor();
			COREWEBVIEW2_COLOR backgroundColor = { 255, GetRValue(color), GetGValue(color), GetBValue(color) };
			wil::com_ptr<ICoreWebView2Controller2> controller2 = m_webController.query<ICoreWebView2Controller2>();
			controller2->put_DefaultBackgroundColor(backgroundColor);
		}

		m_settings->put_IsScriptEnabled(m_scriptEnable ? TRUE : FALSE);
		m_settings->put_AreDefaultScriptDialogsEnabled(m_defaultScriptDialogsEnabled ? TRUE : FALSE);
		m_settings->put_IsWebMessageEnabled(m_isWebMessageEnabled ? TRUE : FALSE);
		m_settings->put_IsBuiltInErrorPageEnabled(m_isBuiltInErrorPageEnabled ? TRUE : FALSE);

		/*
		wil::com_ptr<ICoreWebView2_3> webView2_3;
		webView2_3 = m_webview.try_query<ICoreWebView2_3>();
		if (webView2_3)
		{
			//! [AddVirtualHostNameToFolderMapping]
			// Setup host resource mapping for local files.
			webView2_3->SetVirtualHostNameToFolderMapping(
				L"appassets.test", L"test",
				COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
			webView2_3->SetVirtualHostNameToFolderMapping(
				L"appassets.assets", L"assets",
				COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
			//! [AddVirtualHostNameToFolderMapping]
		}
		*/

		// Resize WebView to fit the bounds of the parent window
		m_webController->put_Bounds(m_rcItem);

		put_AllowExternalDrop(m_allowExternalDrop ? TRUE : FALSE);
		put_ContextMenusEnabled(m_contextMenusEnabled ? TRUE : FALSE);
		put_AcceleratorKeysEnabled(m_acceleratorKeysEnabled ? TRUE : FALSE);
		put_StatusBarEnabled(m_statusBarEnabled ? TRUE : FALSE);
		put_ZoomControlEnabled(m_setZoomControlEnabled ? TRUE : FALSE);

		EventRegistrationToken token;
		m_webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
			[this](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
				if (m_bMustHttps) {
					wil::unique_cotaskmem_string uri;
					args->get_Uri(&uri);
					std::wstring source(uri.get());
					if (source.substr(0, 5) != L"https") {
						args->put_Cancel(true);
					}
				}
				if (m_pManager) {
					m_pManager->SendNotify(this, L"NavigationStarting", (WPARAM)webview, (LPARAM)args);
				}
				return S_OK;
			}).Get(), &token);

		m_webview->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
			[this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
				if (m_pManager) {
					m_pManager->SendNotify(this, L"NavigationCompleted", (WPARAM)sender, (LPARAM)args);
				}
				return S_OK;
			})
			.Get(), &token);

		m_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
			[this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
				if (m_pManager) {
					m_pManager->SendNotify(this, L"WebMessageReceived", (WPARAM)webview, (LPARAM)args);
				}
				return S_OK;
			}).Get(), &token);

		m_webview->add_FrameNavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
			[this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
				if (m_pManager) {
					m_pManager->SendNotify(this, L"FrameNavigationCompleted", (WPARAM)sender, (LPARAM)args);
				}
				return S_OK;
			})
			.Get(), nullptr);

		if (m_homepage.IsEmpty()) {
			m_homepage = L"about:blank";
		}

		if (m_pManager)
			m_pManager->SendNotify(this, L"OnWebView2Created");

		if (m_autoNavigate) {
			GotoHomePage();
		}
		return S_OK;
	}

	void CWebview2UI::SetVisible(bool bVisible)
	{
		if (IsVisible() == bVisible) return;
		__super::SetVisible(bVisible);
		if (m_webController) {
			m_webController->put_IsVisible(bVisible ? TRUE : FALSE);
		}
	}

	void CWebview2UI::SetIsScriptEnabled(bool enable)
	{
		if (m_scriptEnable == enable) return;
		m_scriptEnable = enable;
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_IsScriptEnabled(enable ? TRUE : FALSE));
	}

	void CWebview2UI::SetAreDefaultScriptDialogsEnabled(bool enable)
	{
		if (m_defaultScriptDialogsEnabled == enable) return;
		m_defaultScriptDialogsEnabled = enable;
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_AreDefaultScriptDialogsEnabled(enable ? TRUE : FALSE));
	}

	void CWebview2UI::SetIsWebMessageEnabled(bool enable)
	{
		if (m_isWebMessageEnabled == enable) return;
		m_isWebMessageEnabled = enable;
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_IsWebMessageEnabled(enable ? TRUE : FALSE));
	}

	void CWebview2UI::SetIsBuiltInErrorPageEnabled(bool enable)
	{
		if (m_isBuiltInErrorPageEnabled == enable) return;
		m_isBuiltInErrorPageEnabled = enable;
		if (!m_settings) return;
		CHECK_FAILURE(m_settings->put_IsBuiltInErrorPageEnabled(enable ? TRUE : FALSE));
	}

	void CWebview2UI::SetVirtualHostNameToFolderMapping(LPCTSTR hostName, LPCTSTR folderPath)
	{
		m_hfMap[hostName] = folderPath;
		if (!m_webview) return;
		wil::com_ptr<ICoreWebView2_3> webView3 = m_webview.try_query<ICoreWebView2_3>();
		if (webView3)
		{
			webView3->SetVirtualHostNameToFolderMapping(hostName, folderPath, COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
		}
	}

	void CWebview2UI::SetAllowExternalDrop(bool allow)
	{
		if (m_allowExternalDrop == allow) return;
		m_allowExternalDrop = allow;
		put_AllowExternalDrop(allow ? TRUE : FALSE);
	}

	void CWebview2UI::SetContextMenusEnabled(bool enable)
	{
		if (m_contextMenusEnabled == enable) return;
		m_contextMenusEnabled = enable;
		put_ContextMenusEnabled(enable ? TRUE : FALSE);

	}

	void CWebview2UI::SetAcceleratorKeysEnabled(bool enable)
	{
		if (m_acceleratorKeysEnabled == enable) return;
		m_acceleratorKeysEnabled = enable;
		put_AcceleratorKeysEnabled(enable ? TRUE : FALSE);

	}

	void CWebview2UI::SetStatusBarEnabled(bool enable)
	{
		if (m_statusBarEnabled == enable) return;
		m_statusBarEnabled = enable;
		put_StatusBarEnabled(enable ? TRUE : FALSE);
	}

	void CWebview2UI::SetZoomControlEnabled(bool enable)
	{
		if (m_setZoomControlEnabled == enable) return;
		m_setZoomControlEnabled = enable;
		put_ZoomControlEnabled(enable ? TRUE : FALSE);
	}

	void CWebview2UI::AddScriptToExecuteOnDocumentCreated(LPCTSTR script)
	{
		if (!m_webview) return;
		m_webview->AddScriptToExecuteOnDocumentCreated(script, NULL);
	}

	void CWebview2UI::ExecuteScript(LPCTSTR script, ICoreWebView2ExecuteScriptCompletedHandler* handler)
	{
		if (!m_webview) return;
		m_webview->ExecuteScript(script, handler);
	}

	void CWebview2UI::GotoHomePage()
	{
		if (!m_webview) return;
		m_webview->Navigate(m_homepage);
	}

	void CWebview2UI::Navigate(LPCTSTR url)
	{
		if (!m_webview) return;
		m_webview->Navigate(url);
	}

	ICoreWebView2Environment* CWebview2UI::GetGetWebView2Env()
	{
		return m_webViewEnv.get();
	}

	ICoreWebView2Controller* CWebview2UI::GetWebView2Controller()
	{
		return m_webController.get();
	}

	ICoreWebView2* CWebview2UI::GetWebView2()
	{
		return m_webview.get();
	}
}
