						opc配置工具使用说明

1首先请确定是否存在以下的4个文件：
1)“OPC Core Components 2.00 SDK 2.20.msi“
2)“wxp0823.reg”，
3)“w2k0830.reg”
4)“Setup.exe“。

注意事项：
1)、如果有病毒或者是windows xp有漏洞，请杀毒和打补丁；
2)、运行的时候请关闭windows xp的防火墙。(如果你不想完全关闭windows防火墙,请参见DCOM/OPC的防火墙配置)
3)、在客户端连接的时候，如果采用电脑名字连不上，则请您采用电脑的ip。

请严格参照以下的顺序运行：
对于OPC服务器端：
1)、点击Setup.exe，按照提示运行即可；
2)、重新启动电脑(一定要重新启动电脑)。
3)、运行的时候请关闭windows xp的防火墙。(如果你不想完全关闭windows防火墙,请参见DCOM/OPC的防火墙配置)

对于OPC客户端：
1)、对于OPC 客户端，也需要运行Setup.exe进行配置；
2)、重新启动电脑(一定要重新启动电脑)。
3)、客户端不用配置windows防火墙即可运行。


各部分的作用如下:
1)、“OPC Core Components 2.00 SDK 2.20.msi“
更新opcenum.exe等文件，该程序由opc基金会（www.opcfoundation.org）提供。该程序针对windows xp sp2以及windows 2003作了一定的改动。
2)、“wxp0823.reg”(windows2000 下为w2k0830.reg)
修改windows注册表中的[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Ole]键下的一些键值，主要用来配置DCOM的访问控制权限。
本次“默认访问权限”、“默认启动权限”中添加了“administrators,interactive,network,system,everyone,anonymous logon”这五个用户。
修改windows注册表[HKEY_LOCAL_MACHINE\SOFTWARE\Classes\AppID\{13486D44-4821-11D2-A494-3CB306C10000}]键下的一些值，最主要的是：
"RunAs"="Interactive User"
"LegacyAuthenticationLevel"=dword:00000001
4)、“Setup.exe“
主要是修改本机上已经存在的OPC Server的访问控制权限，使得该OPC Server的访问控制采用默认的权限（也就是继承自DCOM访问配置的权限）。
同时作为整个配置程序的调度。
关键项：
"RunAs"="Interactive User"
"AuthenticationLevel"=dword:00000001


