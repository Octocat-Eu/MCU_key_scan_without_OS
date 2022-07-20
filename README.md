# MCU_key_scan_without_OS
一个MCU裸机的键盘扫描程序（事件通知型）
本程序修改自【 http://www.openedv.com/forum.php?mod=viewthread&tid=277263 】，大量精简了函数调用，将能合并的函数合并，使理解起来更容易一些，加入了3变频扫描（消抖期5ms、按键期50ms、闲置期100ms，可自己定制）
>## 一、使用灵活：
>一体实现按键的普通、单击、双击、长按、保持以及组合等功能，无须事前为每个按键每种键值逐一进行宏定义，也无须逐一编写各事件的条件判断，
只须为需要的按键事件编写相应的响应代码即可，同时留有特殊键组合等的扩展接口； 可以选择每一按键事件的处理实时性，从而能够使强实时性的紧急按键优先得到处理，可自由选择中断处理及查询处理或二者混合的处理方式， 灵活适配使应用项目能够兼备按键的强实时性要求以及超长（主循环执行一遍的时间长达 1 秒以上的）程序的适应性。

>## 二、注重通用：
>模块设计时注重通用性，按键事件（键值）依简单易懂的标准事件格式编写；除能满足几乎所有按键应用需求外，在按键数量上， 从少到 2-4 个按键直到最大 32 个按键（包括端口直联、行列式矩阵、矩阵加直联混合）都可适用。

>## 三、稳定可靠：
>后台智能抖动消除、按键干扰杂波滤除措施有力，获取按键稳定可靠，不会产生重复按键，即使在 CPU 非常繁忙时也不会漏失按键。

>## 四、移植简便：
>所有可调整参数（数量不多）均以宏定义列出，除与硬件相关（按键个数及连接端口）的部分须根据具体系统修改外，其它均无须变化，很容易移植。 程序可读性强，注释详尽丰富，其中包括函数调用关系及详细运用修改说明，如有未尽事宜，可提出探讨，本人尽量解答修改。

>## 五、高效节能：
>消抖无须延时等待，同时采取自适应变频扫键（消抖期5ms、按键期50ms、闲置期100ms，可自己定制）、键盘闲置检测、消抖读键双进程周期差异等多项智能措施尽量减少占用 CPU 的计算资源。
