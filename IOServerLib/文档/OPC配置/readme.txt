						opc���ù���ʹ��˵��

1������ȷ���Ƿ�������µ�4���ļ���
1)��OPC Core Components 2.00 SDK 2.20.msi��
2)��wxp0823.reg����
3)��w2k0830.reg��
4)��Setup.exe����

ע�����
1)������в���������windows xp��©������ɱ���ʹ򲹶���
2)�����е�ʱ����ر�windows xp�ķ���ǽ��(����㲻����ȫ�ر�windows����ǽ,��μ�DCOM/OPC�ķ���ǽ����)
3)���ڿͻ������ӵ�ʱ��������õ������������ϣ����������õ��Ե�ip��

���ϸ�������µ�˳�����У�
����OPC�������ˣ�
1)�����Setup.exe��������ʾ���м��ɣ�
2)��������������(һ��Ҫ������������)��
3)�����е�ʱ����ر�windows xp�ķ���ǽ��(����㲻����ȫ�ر�windows����ǽ,��μ�DCOM/OPC�ķ���ǽ����)

����OPC�ͻ��ˣ�
1)������OPC �ͻ��ˣ�Ҳ��Ҫ����Setup.exe�������ã�
2)��������������(һ��Ҫ������������)��
3)���ͻ��˲�������windows����ǽ�������С�


�����ֵ���������:
1)����OPC Core Components 2.00 SDK 2.20.msi��
����opcenum.exe���ļ����ó�����opc����ᣨwww.opcfoundation.org���ṩ���ó������windows xp sp2�Լ�windows 2003����һ���ĸĶ���
2)����wxp0823.reg��(windows2000 ��Ϊw2k0830.reg)
�޸�windowsע����е�[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Ole]���µ�һЩ��ֵ����Ҫ��������DCOM�ķ��ʿ���Ȩ�ޡ�
���Ρ�Ĭ�Ϸ���Ȩ�ޡ�����Ĭ������Ȩ�ޡ�������ˡ�administrators,interactive,network,system,everyone,anonymous logon��������û���
�޸�windowsע���[HKEY_LOCAL_MACHINE\SOFTWARE\Classes\AppID\{13486D44-4821-11D2-A494-3CB306C10000}]���µ�һЩֵ������Ҫ���ǣ�
"RunAs"="Interactive User"
"LegacyAuthenticationLevel"=dword:00000001
4)����Setup.exe��
��Ҫ���޸ı������Ѿ����ڵ�OPC Server�ķ��ʿ���Ȩ�ޣ�ʹ�ø�OPC Server�ķ��ʿ��Ʋ���Ĭ�ϵ�Ȩ�ޣ�Ҳ���Ǽ̳���DCOM�������õ�Ȩ�ޣ���
ͬʱ��Ϊ�������ó���ĵ��ȡ�
�ؼ��
"RunAs"="Interactive User"
"AuthenticationLevel"=dword:00000001


