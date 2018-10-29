#include "timer.h"

	

TIM_HandleTypeDef TIM3_Handler;      //定时器句柄 
TIM_HandleTypeDef TIM6_Handler;
TIM_HandleTypeDef TIM7_Handler;

extern u32 lwip_localtime;	         //lwip本地时间计数器,单位:ms

u32 tick_time_us = 0;

void TIM_Init(u8 num, u16 arr, u16 psc)
{
	TIM_HandleTypeDef *htim;
	if(1 == num)
	{
	}
	else if(3 == num)
	{
		htim = &TIM3_Handler;
		htim->Instance = TIM3;
	}
	else if(6 == num)
	{
		htim = &TIM6_Handler;
		htim->Instance = TIM6;
	}
	else if(7 == num)
	{
		htim = &TIM7_Handler;
		htim->Instance = TIM7;
	}
	htim->Init.Prescaler = psc;
	htim->Init.Period    = arr;
	htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(htim);
}

void TIM_Base_Start(u8 num)
{
	switch(num)
	{
		case 3:
			TIM3->CNT = 0;
			HAL_TIM_Base_Start_IT(&TIM3_Handler);
			break;
		case 6:
			TIM6->CNT = 0;
			HAL_TIM_Base_Start_IT(&TIM6_Handler);
			break;
		case 7:
			TIM6->CNT = 0;
			HAL_TIM_Base_Start_IT(&TIM7_Handler);
			break;
		default:
			break;
	}
}

void TIM_Base_Stop(u8 num)
{
	switch(num)
	{
		case 3:
			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
			break;
		case 6:
			HAL_TIM_Base_Stop_IT(&TIM6_Handler);
			break;
		case 7:
			HAL_TIM_Base_Stop_IT(&TIM7_Handler);
			break;
		default:
			break;
	}
}

void Start_Tick(void)
{
	if(TIM7_Handler.Instance != TIM7) return;
	tick_time_us = 0;
	TIM_Base_Start(7);	
}

u32 Stop_Tick(void)
{
	if(TIM7_Handler.Instance != TIM7) return 0;
	TIM_Base_Stop(7);
	return tick_time_us + TIM7->CNT;
}

//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断   
	}
	else if(htim->Instance == TIM6)
	{
		__HAL_RCC_TIM6_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 2, 1);
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	}
	else if(htim->Instance == TIM7)
	{
		__HAL_RCC_TIM7_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM7_IRQn, 2, 1);
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
	}
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

void TIM6_DAC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TIM6_Handler);
}


void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TIM7_Handler);
}


//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		
	}
	else if(htim->Instance == TIM6)
	{
		HAL_IncTick();
		lwip_localtime += 1;
		//TaskTick();
	}
	else if(htim->Instance == TIM7)
	{
		tick_time_us += 1000;
	}
}


