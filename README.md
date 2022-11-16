# duiwebview2

### 介绍
Webview2 的 Duilib插件\
[Webview2](https://learn.microsoft.com/zh-cn/microsoft-edge/webview2/) 是微软推出的 基于 Chromium 的Web控件。\
与 CEF 相比的优势：在Win10（包含）之后随系统发布，不需要再随应用一起分发，这样可以有效减少应用安装包的大小，并可随微软官方补丁更新。

### 项目说明
```
duiwebview2     插件项目
demo            使用事例
bin\nativeweb   本地测试网页
bin\skin        duilib界面资源
webview2.xml    文件中描述了 Webview2 控件支持的属性和事件
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
