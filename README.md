# QQ_simple 项目

这是一个基于Qt的 QQ 简易框架。

## ✨ 功能特点
- 功能 1：登录窗口设计

- <img width="434" height="315" alt="image" src="https://github.com/user-attachments/assets/7924ed70-5146-4b96-9978-94628fa81167" />

- 功能 2：通讯录设计

- <img width="310" height="685" alt="image" src="https://github.com/user-attachments/assets/79a64623-2a86-4f17-aa6d-3aacc68debc8" />

- 功能 3：通讯录设计

- <img width="1041" height="730" alt="image" src="https://github.com/user-attachments/assets/cbfdeff9-d2c5-48f9-8735-c7224263153a" />

- 保留了接口，可以对按键进行设计

## 🛠️ 安装说明

你需要先安装 Visual Studio 2022。

1. 克隆项目
git clone https://github.com/jiang-yi335776/QQ_simple.git

2. 安装依赖 
在Visual Studio扩展中下载QT VS tools 插件

## 🚀 如何使用

1.在Visual Studio打开本地解决方案，QtQQ.sln文件

2.将Resources中的图片添加到CCMainWindow.qrc文件中

## 🚀 可以从本项目中学习到
#### 1.核心的GUI模块

##### 基础窗口类:

QWidget: 所有自定义窗口的基类。
QDialog: 用于登录窗口或弹出式对话框。
QMainWindow: 作为主程序框架。

##### 最常用到的布局管理：

QVBoxLayout (垂直布局), QHBoxLayout (水平布局), QGridLayout (网格布局)

##### 项目中常用的控件：

QPushButton: 用于“发送”、“登录”等按钮。

QLineEdit: 用于输入单行文本（如账号、IP地址）。

QTextEdit / QTextBrowser: 用于显示聊天记录和编辑发送内容。这里通常涉及到富文本（Rich Text）的处理。

QLabel: 显示静态文本或图片（如头像）。

QComboBox: 下拉框，用于选择字体大小或在线状态。

QToolButton: 带图标的按钮，常用于工具栏（如加粗、斜体、保存记录）。

QToolBox**: **这是模仿 QQ 好友列表（抽屉效果）最经典用到的控件，用于实现好友分组的折叠和展开。

QTableWidget: 用于显示在线用户列表的具体信息。

#### 2.网络通信模块

这是实现即时通讯的核心。 QQ_Simple 项目采用 UDP进行广播和短消息发送，TCP 进行文件传输。(没有完全实现)

QUdpSocket：

用于实现局域网内的消息广播*。

用于发送简短的聊天消息

用于上线/下线通知（广播 Presence 信息）。

QTcpServer & QTcpSocket:

通常用于文件传输功能。TCP 提供可靠连接，适合传输大数据流。

服务端监听端口，客户端连接后发送文件数据流。

QHostInfo & QNetworkInterface:

用于获取本机的主机名、IP 地址，以便在局域网中标识自己。

#### 3.文件与系统操作

**QFile**:

用于读取和写入文件（在文件传输功能中）。

可能用于保存和读取聊天记录。

**QFileDialog**:

打开标准文件选择对话框，让用户选择要发送的文件或保存路径。

**QSystemTrayIcon**:

系统托盘图标。即使关闭主窗口，程序也可以最小化到右下角托盘，并在收到消息时闪烁或弹出气泡提示。

**QSound** :

用于播放上线、下线或收到消息时的提示音

#### 4.事件处理

**closeEvent (重写)**:

重写关闭事件，实现“点击关闭按钮时是退出程序还是最小化到托盘”的逻辑。

**Signals & Slots (信号与槽)**:

这是 Qt 的核心机制，贯穿全项目。例如：点击发送按钮（Signal） -> 触发发送数据的函数（Slot）；Socket 收到数据（Signal） -> 触发读取解析的函数（Slot）。

