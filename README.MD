# OneEncrypt

*Yet Another OneDrive Encryptor*

利用 DLL 注入实现的无进程无感知 OneDrive 加密器

### 工作方式

1. 通过 `IFEO` 劫持原生 `OneDrive` 客户端进程
2. 以调试器启动 `OneDrive` 绕过 `IFEO`，注入负责 `IO` 劫持的 `DLL`
3. 运行 `OneDrive`，启动器自行退出

其中被注入的 `DLL` 会劫持 `ReadFile` 等系统调用，在其进行真正的 IO 时额外做一层加解密。

由于只对 OneDrive 的 IO 做加解密处理，所以理论性能上要优于目前已有实现；并且用户可以几乎零感知；在忘记密码的情况下也不会丢失本机已有文件。



### 劫持实现

系统调用劫持基于 `Detours` 库。

需要维护所有打开的 File Handler ，并记录其 curser 位置。



### 加密算法

加密算法基于 `libsodium` 库，使用 `xchacha20poly1305` 。

在这里，选择加密算法的主要考量是速度和是否支持随机读取。每一个我们负责管理的 File Handler 都有一个其对应的 Encryptor，当拦截到 `seek` 等可能改变 cursor 位置的调用时，Encryptor 也应当做出相应改变。

~~参考 `BoxCryptor` 的实现，我们在每一个开启了加密的文件夹中都放置一个 `key.storage` 文件，该文件存储了文件名和其随机 nonce 的对应关系，该文件使用 MasterKey 加密 （其nonce直接存储于文件头部）。~~

~~随机 nonce 应在每次文件更新时都随机生成并重新加密[待定1]。~~

key 与文件分开放置可能会导致客户端尝试从服务端同步数据时，由于文件的同步顺序不确定导致可能不能解密。BoxCryptor 没有该问题，因为其是用户需要使用该文件时才解密，而这时 Onedrive 已经同步完文件；若未同步 key.storage 文件，则不对用户显示该目录的所有文件即可。

但是将文件的 `nonce` 放置在文件内有一定的问题。首先是实现会更复杂，文件长度的不一致导致需要 Hook 更多 API ，并且加密解密的过程也需要额外做一些处理。

更严重的问题是安全问题。

1. 对于已知明文的文件(如很多固定的二进制文件)，远程可以拿到一对 `nonce` 和 `enc(key, nonce)` 的对应。这样远端至少可以任意修改已知文件。
2. 对于未知的文件，由于无条件接收服务端传送的文件，所以该 `nonce` 可以被远端覆盖掉。此时若接收该远程文件，则本地会解密到错误的密文。

在这种场景下，`alpha` 版本接受该安全风险。



### API

- ReadFile 读文件
- WriteFile 写文件
- SetFilePointer 改变文件游标
- CreateFileW 打开文件/文件夹
- CloseHandle 关闭文件/文件夹
- FindFirstFileW 列目录
- GetFileAttributesW 获取文件/文件夹属性
- GetFileInformationByHandle 
- GetFileInformationByHandleEx



### RoadMap

#### Alpha 版本

- 注入启动器
- 文件头部放置 `nonce` 
- 不支持修改 `key`
- 支持可配置的 `key` 和加密路径
- API 支持：
  - [Done] ReadFile
  - [Done] WriteFile
  - [Done] CreateFileW
  - [Done] CloseHandle
  - [Done] SetFilePointer
  - [Done] GetFileSize

#### Beta 版本

- 更完善的 API 支持
- 对 MBS 支持
- 支持异步的读写操作
- 对软链接的支持
- 支持修改 `key`
- 区分不同 `Onedrive` 进程
- `IFEO` 方式启动



### 开发指南

请自备 `Windows 10` 环境，安装 [Git](https://git-scm.com/download/win), [Visual Studio 2017/2019](https://visualstudio.microsoft.com/) (务必请勾选 `Platform ToolSet v141` )。

使用 Visual Studio 打开项目，**若提示升级请点击不要升级**。因为有些项目配置并没有保存在配置中，请手动配置以下选项：

- 将每个项目中的 `General -> Windows SDK Version` 修改为你系统中的版本（下拉会自动提示已有版本）。
- 右键将 `Boot` 项目设置为启动项目。
- 将 `Boot` 项目的 `C/C++ -> Working Directory` 修改为 `$(SolutionDir)x64\Release`。

因为注入代码的原因，调试 `FileHook` 暂不可用，请通过打 log 的方式 debug吧 >_< 。 

