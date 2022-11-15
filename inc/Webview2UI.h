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
		//������ҳ
		void SetHomepage(LPCTSTR url) { m_homepage = url; }
		//ȡ����ҳ
		LPCTSTR GetHomepage() { return m_homepage; }
		//���ÿɼ���
		void SetVisible(bool bVisible = true) override;
		//�����Ƿ�����javascript
		void SetIsScriptEnabled(bool enable);
		//�����Ƿ�����Ĭ��javascript�Ի���
		void SetAreDefaultScriptDialogsEnabled(bool enable);
		//�����Ƿ�����Webview�Ķ���Document��Nativeͨ��Messageͨ��
		void SetIsWebMessageEnabled(bool enable);
		//�����Ƿ�����Ĭ�ϵĴ���ҳ
		void SetIsBuiltInErrorPageEnabled(bool enable);
		//���ý�����Ŀ¼����Ϊ�����host
		void SetVirtualHostNameToFolderMapping(LPCTSTR hostName, LPCTSTR folderPath);
		//�����Ƿ������Ϸ�
		void SetAllowExternalDrop(bool allow);
		//�����Ƿ����������Ĳ˵�
		void SetContextMenusEnabled(bool enable);
		//�����Ƿ�������ټ�
		void SetAcceleratorKeysEnabled(bool enable);
		//�����Ƿ���ʾ״̬��
		void SetStatusBarEnabled(bool enable);
		//�����Ƿ���ԷŴ���С
		void SetZoomControlEnabled(bool enable);
		//���ô����ɹ����Ƿ��Զ�����
		void SetAutoNavigate(bool bAuto) { m_autoNavigate = bAuto; }
		//�����Ƿ�ֻ����https
		void SetMustHttps(bool must) { m_bMustHttps = must; }
		
		//��Native����ע�뵽webview�й�javascript����
		// T ����ʵ�� className ��Ա����, �����������صĴ�Ϊ javascript ���õĶ�����
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

		//���һ��javascript,��Document�����ɹ������
		void AddScriptToExecuteOnDocumentCreated(LPCTSTR script);
		/**
		 * @brief ִ��javascript����
		 * @param script javascript����
		 * @param handler �ص�����������Ϊ NULL
		*/
		void ExecuteScript(LPCTSTR script, ICoreWebView2ExecuteScriptCompletedHandler* handler);
		//��������ҳ
		void GotoHomePage();
		//������url
		void Navigate(LPCTSTR url);
		//ȡ��webveiw2���л���
		ICoreWebView2Environment* GetGetWebView2Env();
		//ȡ��controller
		ICoreWebView2Controller* GetWebView2Controller();
		//ȡ��webview2
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

