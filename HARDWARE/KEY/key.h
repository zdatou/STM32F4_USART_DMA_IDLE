#ifndef _KEY_H
#define _KEY_H
#include "sys.h"

typedef struct _ST_KEY_
{
	u16 Key_Flag;//������־
	u16 Key_Count;//��������
	u8  Key_Value;//����ֵ
	u8  Key_Tri;//��������
	u8  Key_Cnt;//�������� 
	u32 Key_Time; //����ʱ��
	u32 Key_DownTime; //����ʱ��
	u32 Key_ProcTime; //���δ�����
} St_Key;


#define KEY_UP        HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15)
#define KEY_DOWN      HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14)
#define KEY_RIGHT     HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11)
#define KEY_LEFT      HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12)
#define KEY_OK        HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13)
#define KEY_CANNCE    HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10)


#define KEY_UP_PRES 	1
#define KEY_DOWN_PRES	2
#define KEY_RIGHT_PRES	3
#define KEY_LEFT_PRES   4
#define KEY_OK_PRES		5
#define KEY_CANNCE_PRES 6

extern St_Key Key_Proc;

void KEY_Init(void);
u8 KEY_Scan(u8 mode);
void KeyRead(void);
#endif
