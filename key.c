#include "key.h"

#include <STC89C5xRC.H>

static key_var_t get_hal_key_code(void);
//按键状态机，本函数一般由get_keys_events()内部调用
static keyEvent_t get_single_key_event(uint8_t keyNum, key_var_t keyTrig, key_var_t keyCont, uint16_t keyTime);
static keyEvent_t get_keys_event(key_var_t keyStableValue, uint16_t keyTime);

//按键初始化函数
void key_init(void) {//IO初始化
  KEY0_IN = 1;
}

//硬件按键编码（为应用三行读键程序而准备）
//以战舰版的四键为例（最大暂支持16键，KeyS_Type定义为uint32_t则可支持32键）
key_var_t get_hal_key_code(void) {
  key_var_t ktmp = 0;
  if (!KEY0_IN)   ktmp |= 1 << KB_KEY0;
  return ktmp;
}

//********************************************************************************



/******************** 用户应用程序按键判断接口函数(请根据具体需求修改) *********************************/
//最终由key_period()在SYSTICK中调用，主循环调用时请使用read_a_key_event()
//返回事件值，用户程序直接处理事先定义的键值即可。

//可适应的按键类型如下：
//普通：按下即有效，不用等按键释放
//单击：按下再松开后有效
//双击：快速单击两次（两次单击之间时间不超过SHORT_TICKS）
//长按：按下超过规定的时间LONG_TICKS，超过后可连续输出，通过软件可设置间隔一定时间输出一次键值
//组合：双键组合（其实多键组合也可同理实现）
/**********************************************************************************/



//按键预处理程序:  允许对有强实时性需要的紧要按键无须排队优先执行，其效果有点儿类似回调函数
//本函数根据实际项目需求可以有三种具体实现模式选择：
//模式一：如果按键处理占用时间短（无延时语句、长循环等），按键要求强实时处理，则可以把所有的按键处理都放在这里
//        这样主循环就无须处理按键了（相当于使用中断服务的方式处理全部按键）
//模式二：对按键处理实时性要求不高，能够忍受一定程序的按键响应时延，可以把所有按键处理放在主循环中查询响应，
//模式三（前两种的折中方案）：强实时性需要紧急处理的键，直接在这里写执行代码，其它允许延时的键留待主循环查询处理，形式如下例所示。
keyEvent_t get_keys_event(key_var_t keyStableValue, uint16_t keyTime) {

  uint8_t i;
  keyEvent_t  keyEvent = 0;
  key_var_t keyTrig = 0;      //存有本次读键时的按键触发状态
  static key_var_t keyCont = 0;      //存有本次读键时的实时键态
  keyTrig = keyStableValue & (keyStableValue ^ keyCont); //调用三行读键方法,其实核心只有此行，使得keyTrig在某按键被按下后有且只有一次读取到对应位为1;
  keyCont = keyStableValue;
  //以下是按键判断，用户可根据需要随意添加或删改（注释掉的部分也可根据需要作为参考语句使用）
  //注意：排在前面的判断条件具有高的优先级，一旦条件满足即刻返回，不再执行后续语句。
  //按键的判断条件设定技巧：
  // 		keyTrig中体现了对应按键的触发状态，在某按键被按下后有且只有一次读取到对应位为1;
  // 		keyCont则体现了当前按键的状态，处于按下的对应位为1，处于松开的对应位为0;
  // 		而KeyTime里面，记录了当前键态持续的时间
  do {
    //以下是组合键键码判断
//    if ((keyCont == (WKUP_ON + KEY0_ON)) && KEY0_PRESSED) { //WKUP+KEY0组合按键（先按下WKUP再按下KEY0）
//      //get_single_key_event(KB_CLR, keyTrig, keyCont, keyTime); //复位状态机，防止本按键对其干扰(本按键与状态机有冲突时请调用此句)
//      keyEvent = WKUP_PLUSKEY0_PRES;
//      break; //如果是组合键就不去检测单个按键了
//    }

    //以下是单键事件判断，使用状态机得到单击、双击、长按、保持等键码
    for (i = 0; i < KeyNumMax; i++) {
      keyEvent = get_single_key_event(i, keyTrig, keyCont, keyTime);
      if (keyEvent)
        break;
    }
  } while (0);

  //return keyEvent; //模式二时，本函数简化到只须这一句，以下可全部删除。

  //下面是按键即时处理，处理后的按键事件不需要输出
//  switch (keyEvent) {
//  case KEY_EVENT(KB_KEY1, DOUBLE_CLICK)://KEY1双击，执行两灯同时翻转（仅作为示例）
//    LED0 = !LED0; LED1 = !LED1; //控制两灯翻转
//    keyEvent = 0;
//    break;
//  default:
//  }
  return keyEvent;
}






//**********************  以下为通用函数，一般不用修改  ****************************

keyEvent_t keyQueue[KEYBUFFSIZE];
uint8_t pKeyQueueTail = 0;
//读取按键值：由主循环调用。从按键缓存中读取按键值，无键则返回0
keyEvent_t read_a_key_event(void) {
  static uint8_t pKeyQueueHead = 0;//读键指针
  if (pKeyQueueHead == KEYBUFFSIZE)
    pKeyQueueHead = 0;//按键缓冲区循环使用
  if (pKeyQueueHead == pKeyQueueTail)
    return 0;//键已经取尽，返回0
  return keyQueue[pKeyQueueHead++];
}




//按键扫描函数：一般由Systick中断服务程序以一定时间节拍调用此函数
//采用了键盘自适应变频扫描措施，在键盘正常稳定期间（非消抖期间）扫描频率降低以减少CPU资源占用
//该函数将影响全局变量：消除抖动后的稳定键态值keyStableTemp及累计时长keyTime

//按键已经全部释放
#define ALL_KEY_RELEASED        (keyStableTemp==0)  //简化的条件

void key_period(void) {
  static key_var_t keyScanValLast = 0;
  static key_var_t keyStableTemp = 0; //存有稳定(消除抖动后)的键态(读键前)
  static uint16_t keyTime = 0;        //存有本次读键时当前键态持续的时长
  static flag_var_t debouncing_flag = 0;
  static uint8_t debounce_cnt = 0;

  key_var_t keyScanVal;
  keyEvent_t keyEvent;

  keyTime++;//在稳定键态（包括无键）状态下，全局变量KeyTime是持续增加的

  if (keyTime >= (0xffff - 0xffff % NORMAL_SCAN_FREQ)) {//防止keyTime溢出,keytime为16位无符号数，最大值为0xffff
    keyTime = (LONG_TICKS * 2 - LONG_TICKS * 2 % NORMAL_SCAN_FREQ);
  }
  if ((!debouncing_flag) && ((keyTime > LONG_TICKS) && ALL_KEY_RELEASED && (keyTime % NORMAL_SCAN_FREQ) || (keyTime % (NORMAL_SCAN_FREQ / 2))))
    return;

  keyScanVal = get_hal_key_code();//扫描键盘，得到实时键值（合并），可存16个键值（按下相应位为1松开为0）;

  if (keyScanVal == keyStableTemp) {//如果键值等于旧存值,则复位消抖计数器（注意：只要消抖中途读到一次键值等于旧存值，消抖计数器均从0开始重新计数）
    debounce_cnt = 0;
    keyScanValLast = keyScanVal;
  }
  else {//如果当前值不等于旧存值，即键值有变化
    debouncing_flag = 1;//标示为消抖期
    if (keyScanVal ^ keyScanValLast) {//如果按键不稳定（即新键值有变化），重新计时
      debounce_cnt = 0;
      keyScanValLast = keyScanVal;
    }
    else {//临时值稳定
      if (++debounce_cnt >= DEBOUNCE_TIMES) {
        debouncing_flag = 0;//消抖期结束
        debounce_cnt = 0;//并复位消抖计数器
        keyTime = 1; //新键值累计时长复位为1个时间单位
        keyStableTemp = keyScanVal;//键值更新为当前值
      }
    }
  }


  if (debouncing_flag || keyTime >= LONG_TICKS && ALL_KEY_RELEASED) {//键盘长时间闲置，直接返回（绝大部分时间基本都是这种状态，此举将大大节省CPU资源）
    return;
  }

  keyEvent = get_keys_event(keyStableTemp, keyTime);//从键预处理程序中读键值
  if (keyEvent) {//如果有新的键值
    keyQueue[pKeyQueueTail++] = keyEvent;//存入按键缓冲区(pKeyQueueTail永远指向下一空位置)
    if (pKeyQueueTail == KEYBUFFSIZE)
      pKeyQueueTail = 0;//按键缓冲区循环使用
  }

}

//***************************************************************
//  无须单击、双击、长按、连续保持等功能键的可以删除以下函数
//***************************************************************
/**　多功能按键状态机
    入口参数：实体按键编号（参数为KEYCLR用于复位状态机）
    返回：键值（按键事件值）＝(KeyNum+2)*10+键事件值; 其它返回0.
*/
#define THE_KEY_IS_UNSET        (!(keyCont & keyID))
#define THE_KEY_IS_SET          (keyCont & keyID)
#define THE_KEY_PRESSED         ((keyTrig & keyID) && THE_KEY_IS_SET)


#define  KEY_STATE_FREE              0u
#define  KEY_STATE_DOWN              1u
#define  KEY_STATE_DOWN_THEN_FREE    2u
#define  KEY_STATE_LONG_PRESS        3u




uint8_t get_single_key_event(uint8_t keyNum, key_var_t keyTrig, key_var_t keyCont, uint16_t keyTime) {
  //按键记忆状态(每字节低四位存state，高4位存repeatTimes)
  static uint8_t KeyState[KeyNumMax];

  key_var_t keyID;
  keyEvent_t event = 0;
  keyState_t state;
  uint8_t i, repeatTimes;
  if (keyNum == KB_CLR) {//参数为KB_CLR时，则消除所有按键记忆状态
    for (i = 0; i < KeyNumMax; i++)
      KeyState[i] = 0;
    return 0;
  }
  keyID = (key_var_t)1 << keyNum;
  state = KeyState[keyNum] & 0x0f; //取相应的记忆状态值
  repeatTimes = KeyState[keyNum] >> 4;

  if (keyTrig && (keyTrig != keyID))
    state = KEY_STATE_FREE; //出现其它键，则状态清0

  switch (state) {
  case KEY_STATE_FREE://状态0：键完全松开
    if (THE_KEY_PRESSED) {  //初次按键触发并有效
      event = KEY_EVENT_FALLEDGE;
      repeatTimes = 1;
      state = KEY_STATE_DOWN;//初次按键有效，变成状态1
    }
    else //无效电平，即按键为松开状态
      event = KEY_EVENT_FREE;
    break;

  case KEY_STATE_DOWN://状态1：初次按键触发并有效
    if (THE_KEY_IS_UNSET) { //检测到按键松开
      event = KEY_EVENT_RISEDGE;
      state = KEY_STATE_DOWN_THEN_FREE;//按键按下后松开，变成状态2
    }
    else if (keyTime >= LONG_TICKS) {//按键未松开，且持续时间已经超过LONG_TICKS
      event = KEY_EVENT_LONG_RRESS_START;
      state = KEY_STATE_LONG_PRESS;//即长按触发启动，变成状态5
    }
    break;

  case KEY_STATE_DOWN_THEN_FREE://状态2：按键按下后已松开
    if (THE_KEY_PRESSED) { //再次检测到按下
      event = KEY_EVENT_FALLEDGE;
      repeatTimes++;//重按次数累计
      state = KEY_STATE_DOWN;
    }
    else {//持续松开
      if (keyTime >= SHORT_TICKS) {//如果松开时间超过SHORT_TICKS，即一次按键结束
        state = KEY_STATE_FREE;//因按键松开时间超过SHORT_TICKS，则复位成状态0
        if (1 == repeatTimes)
          event = KEY_EVENT_SINGLE_CLICK;//次数为1的情况下触发单击事件
        else if (repeatTimes >= 2)
          event = KEY_EVENT_DOUBLE_CLICK;//重按次数为2的情况下触发双击事件
      }
    } //隐含：如果松开时间还没有超过SHORT_TICKS，仍然维持状态2，有待后续判断
    break;

  case KEY_STATE_LONG_PRESS://状态5：长按触发已经启动
    if (THE_KEY_IS_SET) {  //如果按键仍持续按下
      if (repeatTimes <= 1)
        event = KEY_EVENT_LONG_RRESS_HOLD;//长按并保持按键事件成立
    }
    else { //如果按键松开
      event = KEY_EVENT_RISEDGE;
      state = KEY_STATE_FREE; //则恢复到状态0
    }
    break;
  }
  KeyState[keyNum] = state; //保存相应的记忆状态值
  KeyState[keyNum] += repeatTimes << 4;
  if (event) //输出所有事件
    //  if (event >= KEY_EVENT_SINGLE_CLICK) //设定只输出特殊功能键（修改此处可输出按下/松开等一般事件）
    return KEYOUT_BASE_DEF(keyNum) + event;
  else
    return 0;
}

