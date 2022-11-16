# duiwebview2

### 介绍
WebView2 Duilib 插件\
本项目实现对 Webview2 封装, 以便能在duilib中调用WebView2的功能\
[Webview2](https://learn.microsoft.com/zh-cn/microsoft-edge/webview2/) 是微软推出的 基于 Chromium 的Web控件。\
与 CEF 相比的优势：在Win10（包含）之后随系统发布，不需要再随应用一起分发，这样可以有效减少应用安装包的大小，并可随微软官方补丁更新。

### 项目说明
```
duiwebview2     插件项目
demo            使用事例
bin\nativeweb   本地测试网页
bin\skin        duilib界面资源
webview2.xml    文件中描述了 Webview2 控件支持的属性和事件
inc             控件头文件
```
### 环境配置
1.  VS2022
2.  使用 NuGet 安装 [Microsoft.Web.WebView2](https://learn.microsoft.com/zh-cn/microsoft-edge/webview2/)。duiwebview2, demo 都需要安装
3.  安装 [vcpkg](https://github.com/microsoft/vcpkg "vcpkg github")

### 初始化插件
```
CPaintManagerUI::LoadPlugin(_T("duiwebview2.dll"));
```
### 创建控件
在界面描述文件加添加以下节点即可：
```
<Webview2 name="mainview" homepage="https://baidu.com" bkcolor="0xFFFFFFFF"/>
```
### 网页向Host发送消息
<font color="#dd0000">需将 webMessageEnabled 设为 "true"</font>
```
function OnTest1(){
    let cmd = {'cmd':'hello'}
    window.chrome.webview.postMessage(JSON.stringify(cmd));		                
}
```
### 网页接收Host的消息
<font color="#dd0000">需将 webMessageEnabled 设为 "true"</font>
```
window.chrome.webview.addEventListener('message', arg => {
    let eventInfo = JSON.stringify(arg.data);
    console.log(eventInfo);
});
```

### Host接收网页消息
在 Notify(TNotifyUI& msg) 的 WebMessageReceived 事件中处理
```
if (msg.sType == _T("WebMessageReceived")) {
    ICoreWebView2* webview = reinterpret_cast<ICoreWebView2*>(msg.wParam);
    ICoreWebView2WebMessageReceivedEventArgs* args = reinterpret_cast<ICoreWebView2WebMessageReceivedEventArgs*>(msg.lParam);
    wil::unique_cotaskmem_string message;
    CHECK_FAILURE(args->TryGetWebMessageAsString(&message));
    //将unicode转为utf8
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

```

### Host向网页发送消息
```
//webview 为 ICoreWebView2 接口指针
webview->PostWebMessageAsJson(LR"({"cmd":"hello-response"})");
```
### 设置本地目录的虚拟映射表
如果想访问本地目录的网页，可以设置虚拟映射表
```
auto wv2 = static_cast<DuiLib::CWebview2UI*>(msg.pSender);
wil::com_ptr<ICoreWebView2> webView = wv2->GetWebView2();
//为本地网页目录添加资源映射
wil::com_ptr<ICoreWebView2_3> webView2_3;
webView2_3 = webView.try_query<ICoreWebView2_3>();
if (webView2_3)
{
    //这样就可以用 https://native.test/test.html 访问本地网页了
    webView2_3->SetVirtualHostNameToFolderMapping(
        L"native.test", L"..\\..\\nativeweb",
        COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
}
```

### 注入本地对象供javascript调用
NativeObj为c++实现的双接口本地对象,具体可查看Demo的实现\
我使用了一种简单的方法，并不标准
```
auto wv2 = static_cast<DuiLib::CWebview2UI*>(msg.pSender);
wv2->InjectObjectToScript<NativeObj>();
```

### 主要属性
属性|类型|默认值|说明
-----|:-----:|:-----:|----------
homepage|STRING|""|主页
autonavi|BOOL|"true"|创建成功后自动导航
scriptEnable|BOOL|"true"|是否允许执行script
defaultScriptDialogs|BOOL|"true"|是否允许默认script对话
webMessageEnabled|BOOL|"true"|是否允许web message
builtInErrorPageEnabled|BOOL|"false"|是否允许默认错误页
allowExternalDrop|BOOL|"false"|是否允许拖拽
contextMenusEnabled|BOOL|"false"|是否允许右键菜单
acceleratorKeysEnabled|BOOL|"false"|是否允许快捷键
statusBarEnabled|BOOL|"false"|是否允许状态栏
zoomControlEnabled|BOOL|"false"|是否允许放大缩小
mustHttps|BOOL|"true"|是否必须为https

### 主要事件
<font color="#dd0000">事件的参数需要强制类型转换成最终类型</font>
事件|参数|说明
:---:|:---:|---
OnWebView2Created||WebView2创建成功
NavigationStarting|ICoreWebView2* msg.wParam <br/>ICoreWebView2NavigationStartingEventArgs* msg.lParam|开始导航
NavigationCompleted|ICoreWebView2* msg.wParam<br/>ICoreWebView2NavigationCompletedEventArgs* msg.lParam|导航完成
WebMessageReceived|ICoreWebView2* msg.wParam<br/>ICoreWebView2WebMessageReceivedEventArgs* msg.lParam|收到web页发送的消息
FrameNavigationCompleted|ICoreWebView2* msg.wParam<br/>ICoreWebView2NavigationCompletedEventArgs* msg.lParam|iFrame导航完成

### 关于应用发布
[微软推荐分发模式](https://learn.microsoft.com/zh-cn/microsoft-edge/webview2/concepts/distribution)\
我是这样做的：
1. 检查注册表确定是否已经安装webview2.
2. 如果没安装，调用Evergreen Bootstrapper引导程序安装webview2.
3. 理论上win10之后（含win10）的系统默认都安装了webview2

### Demo 效果
![界面1](https://gitee.com/bruce_code/duiwebview2/raw/master/docimg/demo1.png)\

![界面2](https://gitee.com/bruce_code/duiwebview2/raw/master/docimg/demo2.png)