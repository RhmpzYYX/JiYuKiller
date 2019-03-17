# JiYuKiller
极域电子教室 杀手 防控制软件
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](JiYuKiller)
[![Version](https://img.shields.io/badge/version-1.0-green.svg)](JiYuKiller)
[![Liscence](https://img.shields.io/badge/liscence-MIT-green.svg)](JiYuKiller)

说明
---

本软件不是简简单单的强制杀死极域电子教室，而是会智能检测域电子教室是否进行全屏广播，一旦发现全屏广播，将立即调整为窗口模式，你不仅可以继续进行你的操作，而且还可以看老师的操作，自由+学习两不误，这不是很爽的事情吗？


极域并没有使用驱动保护进程（可能是他们的HOOK技术不过关，WinXP有保护，Win7没有），如果在旧版本极域有驱动保护进程，可选择本软件进行卸载驱动，
**或直接使用本软件进行内核模式强杀**。


该软件对极域电子教室进行反HOOK，极域并不能控制键盘或鼠标，因此在被控制时你依然可以自由操作电脑，而且教师只能看你的电脑，并不能对你的电脑进行操控（**已测试**），发现“黑屏肃静”也会立即反控制。


如果您在使用软件过程中发现Bug，欢迎向我反馈！


QQ：1501076885


软件主要界面和操作方法
---
在默认情况下，您不需要更改如何参数，直接右键以管理员身份运行本软件即可，本软件会自动进行操作。

![Image text](https://raw.githubusercontent.com/717021/JiYuKiller/master/help.jpg)

1. 极域进程操作面板，分为：杀死清除、挂起、取消挂起、重新运行程序四个操作。如果已加载驱动并启用“严格模式”，程序将会在内核进行操作（威力更强）。
2. 程序运行状态输出区域，其中可能有一些非常有用的出错信息。
3. AOP 强制置顶定时器，开启以后本软件将会强制置顶，显示在极域的上方。
4. MK 未完成，请不要使用
5. CK 非法检查器。它会不断检查极域是否开启全屏广播，或黑屏肃静，一旦检测到，立即反控制。注意，关闭该检查器将导致极域脱离控制。
6. 软件是否显示在其他软件上方（与AOP不同的是极域会不断置顶，未开启AOP时极域依然会非法显示在顶部）。
7. 勾选该项则默认杀死目标，而不是去调整极域的广播窗口。
8. “严格模式”安全锁。勾选此项程序才会调用驱动去操作（驱动可能不稳定，请慎用）。
9. 加载驱动按钮，点击加载或卸载驱动。
10. 调试命令输入框。你可以修改源码添加自定义调试命令。其他命令请参考源码。回车执行命令。
11. 执行命令。
12. CK  强制执行一次非法检查器。
13. CK AUTO F 未完成，请勿使用。
14. 勾选程序将会自动注入病毒到极域。
15. 强制卸载极域的驱动。
16. 注入病毒到极域。
17. 病毒自毁备用按钮。按下后病毒将会和极域同归于尽。
18. 注入到极域的病毒通信窗口。请勿关闭，否则病毒将会和主程序失去联系。

0.被调戏的极域窗口。











