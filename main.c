#include "led.h"
#include "delay.h"

#include "sys.h"
#include "beep.h"

#include "key.h"
 
u8 Flag300ms=0;//300ms���ı�־

 int main(void)
 {
 	vu8 key=0;	
	delay_init();	    	 //��ʱ������ʼ��	  
 	LED_Init();			     //LED�˿ڳ�ʼ��
	KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
	BEEP_Init();     	//��ʼ���������˿�
	LED0=0;					//�ȵ������
	while(1)
	{
 		key=Read_A_Key();	//�õ���ֵ
	  				   
		switch(key)
		{				 
      //����һ�㰴�����ԣ����¼��������ã���
			case KEY_EVENT(KB_KEY2,PRESS_DOWN):	//KEY2���¼���Ч������LED0��ת	
				LED0=!LED0;
				break;
			case KEY_EVENT(KB_KEY0,PRESS_DOWN):	//KEY0���¼���Ч������LED1��ת	 
				LED1=!LED1; //����������
				break;
			
			//������������������������ԣ����磨������������
			case KEY_EVENT(KB_KEY1,SINGLE_CLICK):	//KEY1�����������ٰ������ɿ���	 
				LED0=!LED0;
				break;
			case KEY_EVENT(KB_WKUP,DOUBLE_CLICK):	//WKUP��˫���������������Σ�
				BEEP=!BEEP; //���Ʒ��������л�ֹͣ
				break;
			case WKUP_PLUSKEY0_PRES:	//WKUP+KEY0��ϰ������Ȱ���WKUP�ٰ���KEY0��
				LED0=!LED0; //��������ͬʱ��ת
				LED1=!LED1;
				break;
			case KEY_EVENT(KB_WKUP,LONG_RRESS_START):	//������WKUP
				LED1=!LED1; //����LED1��ת
				break;
			case KEY_EVENT(KB_KEY1,LONG_PRESS_HOLD):	//������KEY1��ÿ300ms����һ�Σ�ʵ�����Ƶ��ӱ�ĳ���"+"ʱ�������ٵ���ʱ��Ĺ��ܣ�
				if(Flag300ms){Flag300ms=0;LED0=!LED0;} //����LED0ÿ300ms��תһ��
				break;
		}
	 delay_ms(50);//�����ʱֵ����ģ��CPU������������ʱ�䣬�������������ֵ�����ܰ��������������������Ч�� 
	}	 
}

