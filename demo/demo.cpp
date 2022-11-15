// demo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "demo.h"
#include <DuiLib/UIlib.h>
#include "FrameWnd.h"

using namespace DuiLib;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("..\\..\\skin"));

    ::CoInitialize(NULL);

    CPaintManagerUI::LoadPlugin(_T("duiwebview2.dll"));
    CFrameWnd* pFrame = new CFrameWnd();
    pFrame->Create(NULL, _T("Webview2 Demo"), UI_WNDSTYLE_FRAME, 0);
    pFrame->ShowWindow();
    pFrame->CenterWindow();

    CPaintManagerUI::MessageLoop();

    ::CoUninitialize();
    return 0;
}