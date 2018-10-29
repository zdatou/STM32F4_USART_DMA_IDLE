#include "key.h"
#include "delay.h"
St_Key Key_Proc;

//按键初始化函数
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOD_CLK_ENABLE();            //开启GPIOD时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();            //开启GPIOE时钟

    GPIO_Initure.Pin=GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10;//PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;       //输入
    GPIO_Initure.Pull=GPIO_PULLUP;         //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH; //高速
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
	GPIO_Initure.Pin = GPIO_PIN_14;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOD, &GPIO_Initure);
}

//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
u8 KEY_Scan(u8 mode)
{
    static u8 key_up=1;     //按键松开标志
    if(mode==1)key_up=1;    //支持连按
    if(key_up&&(KEY_UP==0||KEY_DOWN==0||KEY_RIGHT==0||KEY_LEFT==0||KEY_OK==0||KEY_CANNCE==0))
    {
        key_up=0;
        if(KEY_UP==0)       return KEY_UP_PRES;
        else if(KEY_DOWN == 0)  return KEY_DOWN_PRES;
        else if(KEY_RIGHT == 0)  return KEY_RIGHT_PRES;
        else if(KEY_LEFT == 0) return KEY_LEFT_PRES;   
		else if(KEY_OK == 0) return KEY_OK_PRES;
		else if(KEY_CANNCE == 0) return KEY_CANNCE_PRES;
    }else if(KEY_UP==1&&KEY_DOWN==1&&KEY_RIGHT==1&&KEY_LEFT==1&&KEY_OK==1&&KEY_CANNCE==1)key_up=1;
    return 0;   //无按键按下
}

void KeyRead(void)//20ms执行一次
{
	unsigned char ReadData;
	ReadData= KEY_Scan(1);
	Key_Proc.Key_Tri = ReadData&(ReadData^Key_Proc.Key_Value);//边沿检测（2次检测）
	Key_Proc.Key_Value = ReadData;
	
	if(Key_Proc.Key_Tri) //
	{
		Key_Proc.Key_Time = HAL_GetTick();
	}
	else if(ReadData != 0) //长按时开始记录长按时间 和 长按计数
	{ 
		Key_Proc.Key_Count++;
		if(Key_Proc.Key_Count >= 0xFFFE) Key_Proc.Key_Count = 0xFFFE;		
		Key_Proc.Key_DownTime += (HAL_GetTick() - Key_Proc.Key_Time);
		
		if(Key_Proc.Key_DownTime > 3000) //4s  长按超过3s时
		{
			Key_Proc.Key_DownTime = 3000;
			Key_Proc.Key_ProcTime = 100;  //两次长按的处理间隔 200ms
		}
		else if(Key_Proc.Key_DownTime > 2000)//2s
		{
			Key_Proc.Key_ProcTime = 400;
		}
		else
		{
			Key_Proc.Key_ProcTime = 1000;
		}
		
		Key_Proc.Key_Time = HAL_GetTick();
	}
	else
	{
		Key_Proc.Key_Count = 0;
		Key_Proc.Key_DownTime = 0; //没有按键按下时 清空长按时间
		Key_Proc.Key_ProcTime = 1000; //恢复1s处理一次的间隔
	}
}
