/**
	Duilib Webview2
	Author: Bruce Yan
	email: yanyan.mail@gmail.com
*/
#pragma once
#include "DuiLib/UIlib.h"
#include <wil/com.h>
#include <wrl.h>
#include <webview2.h>
#include <map>

#ifdef DUIWEBVIEW2_EXPORTS
#define DUIWEB2API	__declspec(dllexport)
#else
#define DUIWEB2API	__declspec(dllimport)
#endif

namespace DuiLib {
	class DUIWEB2API CWebview2UI : public CControlUI
	{
	public:
		CWebview2UI();
		~CWebview2UI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void DoInit() override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
	public:
		//设置主页
		void SetHomepage(LPCTSTR url) { m_homepage = url; }
		//取得主页
		LPCTSTR GetHomepage() { return m_homepage; }
		//设置可见性
		void SetVisible(bool bVisible = true) override;
		//设置是否启用javascript
		void SetIsScriptEnabled(bool enable);
		//设置是否允许默认javascript对话框
		void SetAreDefaultScriptDialogsEnabled(bool enable);
		//设置是否允许Webview的顶层Document与Native通过Message通信
		void SetIsWebMessageEnabled(bool enable);
		//设置是否允许默认的错误页
		void SetIsBuiltInErrorPageEnabled(bool enable);
		//设置将本地目录设置为虚拟的host
		void SetVirtualHostNameToFolderMapping(LPCTSTR hostName, LPCTSTR folderPath);
		//设置是否允许拖放
		void SetAllowExternalDrop(bool allow);
		//设置是否允许上下文菜单
		void SetContextMenusEnabled(bool enable);
		//设置是否允许加速键
		void SetAcceleratorKeysEnabled(bool enable);
		//设置是否显示状态栏
		void SetStatusBarEnabled(bool enable);
		//设置是否可以放大缩小
		void SetZoomControlEnabled(bool enable);
		//设置创建成功后是否自动导航
		void SetAutoNavigate(bool bAuto) { m_autoNavigate = bAuto; }
		//设置是否只允许https
		void SetMustHttps(bool must) { m_bMustHttps = must; }
		
		//将Native对象注入到webview中供javascript调用
		// T 必须实现 className 成员函数, 这条函数返回的串为 javascript 调用的对象名
		template<typename T>
		void InjectObjectToScript()
		{
			if (!m_pManager || !m_webview) return;
			wil::com_ptr<T> hostObj = Microsoft::WRL::Make<T>(m_pManager->GetPaintWindow());
			VARIANT remoteObjectAsVariant = {};
			hostObj.query_to<IDispatch>(&remoteObjectAsVariant.pdispVal);
			remoteObjectAsVariant.vt = VT_DISPATCH;
			m_webview->AddHostObjectToScript(T::className(), &remoteObjectAsVariant);
			remoteObjectAsVariant.pdispVal->Release();
		}

		//添加一段javascript,当Document创建成功后调用
		void AddScriptToExecuteOnDocumentCreated(LPCTSTR script);
		/**
		 * @brief 执行javascript代码
		 * @param script javascript代码
		 * @param handler 回调函数，可以为 NULL
		*/
		void ExecuteScript(LPCTSTR script, ICoreWebView2ExecuteScriptCompletedHandler* handler);
		//导航到主页
		void GotoHomePage();
		//导航到url
		void Navigate(LPCTSTR url);
		//取得webveiw2运行环境
		ICoreWebView2Environment* GetGetWebView2Env();
		//取得controller
		ICoreWebView2Controller* GetWebView2Controller();
		//取得webview2
		ICoreWebView2* GetWebView2();
	protected:
		void ConfirmCacheFolder();
		void put_AllowExternalDrop(BOOL allow);
		void put_ContextMenusEnabled(BOOL enable);
		void put_AcceleratorKeysEnabled(BOOL enable);
		void put_StatusBarEnabled(BOOL enable);
		void put_ZoomControlEnabled(BOOL enable);
		HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller);
	protected:
		bool m_autoNavigate = true;
		bool m_scriptEnable = true;
		bool m_defaultScriptDialogsEnabled = true;
		bool m_isWebMessageEnabled = true;
		bool m_isBuiltInErrorPageEnabled = false;
		bool m_allowExternalDrop = false;
		bool m_contextMenusEnabled = false;
		bool m_acceleratorKeysEnabled = false;
		bool m_statusBarEnabled = false;
		bool m_setZoomControlEnabled = false;
		bool m_bMustHttps = true;
		CDuiString m_homepage;
		CDuiString m_cachePath;
#pragma warning(push)
#pragma warning(disable:4251)
		std::map<CDuiString, CDuiString> m_hfMap;
		wil::com_ptr<ICoreWebView2Environment> m_webViewEnv;
		wil::com_ptr<ICoreWebView2Controller> m_webController;
		wil::com_ptr<ICoreWebView2> m_webview;
		wil::com_ptr<ICoreWebView2Settings> m_settings;
#pragma warning(pop)
	};
}

