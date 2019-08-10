# OneEncrypt

*Yet Another OneDrive Encryptor*

利用 DLL 注入实现的无进程无感知 OneDrive 加密器

#### 工作方式

1. 通过 `IFEO` 劫持原生 `OneDrive` 客户端进程
2. 以调试器启动 `OneDrive` 绕过 `IFEO`，注入负责 `IO` 劫持的 `DLL`
3. 运行 `OneDrive`，启动器自行退出

其中被注入的 `DLL` 会劫持 `ReadFile` 等系统调用，在其进行真正的 IO 时额外做一层加解密。

由于只对 OneDrive 的 IO 做加解密处理，所以理论性能上要优于目前已有实现；并且用户可以几乎零感知；在忘记密码的情况下也不会丢失本机已有文件。



#### 劫持实现

系统调用劫持基于 `Detours` 库。

需要维护所有打开的 File Handler ，并记录其 curser 位置。



#### 加密算法

加密算法基于 `libsodium` 库，使用 `xchacha20poly1305` 。

在这里，选择加密算法的主要考量是速度和是否支持随机读取。每一个我们负责管理的 File Handler 都有一个其对应的 Encryptor，当拦截到 `seek` 等可能改变 cursor 位置的调用时，Encryptor 也应当做出相应改变。

参考 `BoxCryptor` 的实现，我们在每一个开启了加密的文件夹中都放置一个 `key.storage` 文件，该文件存储了文件名和其随机 nonce 的对应关系，该文件使用 MasterKey 加密 （其nonce直接存储于文件头部）。

随机 nonce 应在每次文件更新时都随机生成并重新加密[待定1]。



#### 待定

1. 是否支持增量更新？调研目前通用网盘对增量更新的支持性。



#### API

- ReadFile 读文件
- WriteFile 写文件
- CreateFileW 打开文件/文件夹
- CloseHandle 关闭文件/文件夹
- [Delay] FindFirstFileW 列目录
- [Delay] GetFileAttributesW 获取文件/文件夹属性
- [Delay] GetFileInformationByHandle 
- [Delay] GetFileInformationByHandleEx
- 



- QueryAttributeTagFile
- QueryBasicInformationFile
- QueryAllInformationFile
- QueryNameInformationFile
- QueryNormalizedNameInformationFile
- QueryObjectIdInformationFile
- QueryStreamInformationFile
- QuerySecurityFile
- QueryRemoteProtocolInformation
- QueryEAFile
- QueryInformationVolume
- FileSystemControl



#### 开发进度

- [Done] 启动器注入 `DLL` 流程
- [Done] 有状态函数劫持
- [Working] 实现文件系统基本 API HOOK
- [Working] 加密模块实现
- [Delay] 实现文件系统全部 API HOOK
- [Delay] `IFEO` 劫持启动
- [Delay] 支持 MasterKey 修改