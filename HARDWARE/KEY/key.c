#include "key.h"
#include "delay.h"
St_Key Key_Proc;

//������ʼ������
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOD_CLK_ENABLE();            //����GPIODʱ��
	__HAL_RCC_GPIOE_CLK_ENABLE();            //����GPIOEʱ��

    GPIO_Initure.Pin=GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10;//PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;       //����
    GPIO_Initure.Pull=GPIO_PULLUP;         //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH; //����
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
	GPIO_Initure.Pin = GPIO_PIN_14;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOD, &GPIO_Initure);
}

//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
u8 KEY_Scan(u8 mode)
{
    static u8 key_up=1;     //�����ɿ���־
    if(mode==1)key_up=1;    //֧������
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
    return 0;   //�ް�������
}

void KeyRead(void)//20msִ��һ��
{
	unsigned char ReadData;
	ReadData= KEY_Scan(1);
	Key_Proc.Key_Tri = ReadData&(ReadData^Key_Proc.Key_Value);//���ؼ�⣨2�μ�⣩
	Key_Proc.Key_Value = ReadData;
	
	if(Key_Proc.Key_Tri) //
	{
		Key_Proc.Key_Time = HAL_GetTick();
	}
	else if(ReadData != 0) //����ʱ��ʼ��¼����ʱ�� �� ��������
	{ 
		Key_Proc.Key_Count++;
		if(Key_Proc.Key_Count >= 0xFFFE) Key_Proc.Key_Count = 0xFFFE;		
		Key_Proc.Key_DownTime += (HAL_GetTick() - Key_Proc.Key_Time);
		
		if(Key_Proc.Key_DownTime > 3000) //4s  ��������3sʱ
		{
			Key_Proc.Key_DownTime = 3000;
			Key_Proc.Key_ProcTime = 100;  //���γ����Ĵ����� 200ms
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
		Key_Proc.Key_DownTime = 0; //û�а�������ʱ ��ճ���ʱ��
		Key_Proc.Key_ProcTime = 1000; //�ָ�1s����һ�εļ��
	}
}
