#ifndef __KEY_H
#define __KEY_H


#include "stdint.h"

//////////////////////////////////////////////////////////////////////////////////
//本扫描键盘模块的特点：
//一、使用灵活：一体实现按键的普通、单击、双击、长按、保持以及组合等功能，无须事前为每个按键每种键值逐一进行宏定义，也无须逐一编写各事件的条件判断，
//                     只须为需要的按键事件编写相应的响应代码即可，同时留有特殊键组合等的扩展接口；
//                     可以选择每一按键事件的处理实时性，从而能够使强实时性的紧急按键优先得到处理，可自由选择中断处理及查询处理或二者混合的处理方式，
//                     灵活适配使应用项目能够兼备按键的强实时性要求以及超长（主循环执行一遍的时间长达1秒以上的）程序的适应性。
//二、注重通用：模块设计时注重通用性，按键事件（键值）依简单易懂的标准事件格式编写；除能满足几乎所有按键应用需求外，在按键数量上，
//                     从少到2-4个按键直到最大32个按键（包括端口直联、行列式矩阵、矩阵加直联混合）都可适用。
//三、稳定可靠：后台智能抖动消除、按键干扰杂波滤除措施有力，获取按键稳定可靠，不会产生重复按键，即使在CPU非常繁忙时也不会漏失按键。
//四、移植简便：所有可调整参数（数量不多）均以宏定义列出，除与硬件相关（按键个数及连接端口）的部分须根据具体系统修改外，其它均无须变化，很容易移植。
//                     程序可读性强，注释详尽丰富，其中包括函数调用关系及详细运用修改说明，如有未尽事宜，可提出探讨，本人尽量解答修改。
//五、高效节能：消抖无须延时等待，同时采取自适应变频扫键、键盘闲置检测、消抖读键双进程周期差异等多项智能措施尽量减少占用CPU的计算资源。


//正点原子@ALIENTEK
//技术论坛:http://www.openedv.com/forum.php?mod=viewthread&tid=277263，有问题可在本帖中提出讨论，谢谢。
//修改日期:2018/9/1
//版本：V2.2
//Made by warship
//////////////////////////////////////////////////////////////////////////////////

//根据需求可修改定义如下参数.
#define TICKS_INTERVAL_ms      5u       //ms,后台调用间隔时间
#define DEBOUNCE_TIME_ms       30u      //ms,消抖时间,即按键电平稳定持续时间
#define NORMAL_SCAN_GAP_ms     100u     //近期没有按键按下时的扫描间隔，可节省CPU消耗，之间不进行按键扫描
#define SHORT_TIME_ms          300u     //短按时间定义300ms,即松开按键后的判断时间，双击等动作要在这个时间内完成
#define LONG_TIME_ms           1200u    //长按时间定义1200ms
#define KEYBUFFSIZE            16u      //按键缓存FIFO深度，定义保存16个键值


#define DEBOUNCE_TIMES      (DEBOUNCE_TIME_ms/TICKS_INTERVAL_ms)       //消除抖动次数
#if DEBOUNCE_TIMES > 256u
#error "[DEBOUNCE_TIME_ms] is too large for uint8_t, check the keyPeriod function."
#endif
#define NORMAL_SCAN_FREQ    (NORMAL_SCAN_GAP_ms/TICKS_INTERVAL_ms)   //正常情况下扫键频率因子
#define SHORT_TICKS         (SHORT_TIME_ms/TICKS_INTERVAL_ms)
#define LONG_TICKS          (LONG_TIME_ms/TICKS_INTERVAL_ms)


//状态机键值事件宏定义如下：

#define  KEY_EVENT_FREE                  0u
#define  KEY_EVENT_FALLEDGE              1u  //每当按键按下时即触发一次该事件
#define  KEY_EVENT_RISEDGE               2u  //每当按键释放时即触发一次该事件
#define  KEY_EVENT_LONG_RRESS_START      3u  //初次满足长按时间要求时即触发一次该事件
#define  KEY_EVENT_SINGLE_CLICK          4u  //短按并松开时即触发一次该事件
#define  KEY_EVENT_DOUBLE_CLICK          5u  //连续两次短按并松开时即触发一次该事件
#define  KEY_EVENT_LONG_RRESS_HOLD       6u  //满足长按时间要求后，之后每次扫描检测到按键仍未释放均会触发一次该事件



//***************** 以下与具体系统的硬件相关 ********************************************

//按键硬件读端口位置
#define KEY0_IN           P32     //按键0输入端口
//#define KEY1_IN       PEin(3)     //按键1输入端口
//#define KEY2_IN       PEin(2)     //按键2输入端口
//#define WKUP_IN       PAin(0)     //按键3输入端口(WK_UP)

//硬件实体按键编号，键态字KeyS_Type依此顺序按位组合，每BIT位对应一个实体按键
#define KB_KEY0           0    //规定KEY0键用键态字的第0号bit位表示，该位为1表示KEY0键处于按下状态
//#define KB_KEY1       1     //规定KEY1键用键态字的第1号bit位表示，该位为1表示KEY1键处于按下状态
//#define KB_KEY2       2     //规定KEY2键用键态字的第2号bit位表示，该位为1表示KEY2键处于按下状态
//#define KB_WKUP       3     //规定KEY2键用键态字的第3号bit位表示，该位为1表示KEY3键处于按下状态

#define KeyNumMax         1     //硬件实体按键数量

//键态字类型定义（根据按键总数定义足够长度的数据类型，本例程只有4个键，用u8足矣,大于16键时请定义为u32）
typedef   uint8_t       key_var_t;
typedef   uint8_t       flag_var_t;
typedef   uint8_t       keyState_t;
typedef   uint8_t       keyEvent_t;

//************ 以下基本与硬件无关（除增删组合键码定义外一般无须修改） *********************

//定义一个特殊值用于复位状态机
#define KB_CLR      44



//这里可以定义一些特殊键码（如组合键等）,0代表没有事件
#define WKUP_PLUSKEY0_PRES  1             //示例：WKUP+KEY0组合按键（先按下WKUP再按下KEY0）



//功能键值输出：
#define KEYOUT_BASE_DEF(keyNum)       ((keyNum+1)<<3) //0-[此宏的最小值-1],为保留组合键值的空间,3为一个按键的事件数,数量为2^3

#define KEY_EVENT(keyNum,keyEvent)    (uint8_t)(KEYOUT_BASE_DEF(keyNum)+keyEvent)    //按键事件(即键值)宏定义
//有了上述宏定义后，无须再为各个按键单独写宏定义，使用KEY_EVENT(键编号,键值事件)就可以代表特定按键事件了。
//例如：用KEY_EVENT(KB_WKUP,DOUBLE_CLICK)就表示了WKUP键双击的键值（或称事件值）


//不使用组合按键等条件判断时，以下宏定义可删除
#define KEY0_ON             (1<<KB_KEY0)  //宏定义：按键未释放值
#define KEY1_ON             (1<<KB_KEY1)
#define KEY2_ON             (1<<KB_KEY2)
#define WKUP_ON             (1<<KB_WKUP)
#define KEY0_PRESSED        (keyTrig==KEY0_ON)  //宏定义：按键触发值
#define KEY1_PRESSED        (keyTrig==KEY1_ON)
#define KEY2_PRESSED        (keyTrig==KEY2_ON)
#define WKUP_PRESSED        (keyTrig==WKUP_ON)



/**************数据和函数接口声明*******************/


void key_init(void);//键硬件IO端口初始化，由主函数调用
void key_period(void);//本函数由SYSTICK调用，在后台扫描按键获取消除抖动后的稳定键值
keyEvent_t read_a_key_event(void);//读取按键值：由系统主循环调用。




//系统主循环 ->调用read_a_key_event()：系统主循环中通过调用read_a_key_event()，对FIFO队列中的按键进行查询处理。
//运用本代码模块的主要工作是三件事：
//一：硬件接口初始化：包括必要的GPIO初始化，实体硬按键的排序等。
//二：键值（或称事件）生成：对get_keys_events()函数进行改写，使之能输出所有有效的键值。所有按键的单击、双击、长按、保持等键值输出已经实现，如果有需要则在该函数中增加组合键等键值。
//三：键值（或称事件）处理：对键值的具体响应处理可在两个地方实现。如有强实时性的紧急按键需要优先处理的键值请在get_keys_event()函数编写代码，
//                         其它按键的响应请在系统主循环调用read_a_key_event()得到键值后编写代码。
//                         根据具体项目需求，也可全部响应代码都在主循环中编写，也可以全部响应代码都在get_keys_event()函数中编写。
//                         如果全部响应代码都在get_keys_event()函数中编写，则主循环中无须再处理按键（也无须调用read_a_key_event函数）。

#endif //__KEY_H
