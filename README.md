# lua-unreal-content-browser

这是一个在 UnrealEngine 中用于显示 Lua 脚本的编辑器扩展插件

安装步骤：  
1. 下载插件
```powershell
git clone git@github.com:krikommp/lua-unreal-content-browser.git
```
2. 需要在项目目录中的 Config/DefaultEditor.ini 中添加
```ini
[/Script/ContentBrowserData.ContentBrowserDataSubsystem]
+EnabledDataSources="LuaData"
```

使用步骤：  
1. 重要：由于在这个文件浏览器设计中，将 lua 资源当作 c++ 文件，所以需要打开 Show C++ Classes 否则将看不到 lua 资源 
2. 打开项目后在任意 Content 目录中右键 -> New Lua Script 就可以创建一个新的 lua 脚本
3. 双击创建出的脚本文件可以使用当前代码编辑器打开

