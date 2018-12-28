/**
  ******************************************************************************
  * @file    main.c
  * @author  ZHANG
  * @version V1.0
  * @date    2018-07-18
  * @brief   main.c of insulator image processing
  * @Platform STM32F767
  ******************************************************************************
  */
  
#include "stm32f7xx.h"
#include "./led/bsp_led.h"
#include "./usart/bsp_debug_usart.h"
#include "./lcd/bsp_lcd.h"
#include <stdlib.h>
#include "main.h"
#include "./camera/bsp_ov2640.h"
#include "./process/process.h"
#include "./systick/bsp_SysTick.h"

/*���������*/
uint32_t Task_Delay[3];
uint8_t dispBuf[100];
OV2640_IDTypeDef OV2640_Camera_ID;
float d;                         //�������
uint16_t p[4];                    //��С��Ӿ���4����
uint16_t dig;                     //�ǶȲ���
uint16_t thre;                      //��ֵ����
uint16_t ratio,center_x,center_y,cnt;

#define LABEL_EX_TH  1000  //αĿ��Ĩ����ֵ   С������Ĩ��
#define FRAME_RATE_DISPLAY 0
uint8_t fps=0;
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
    u16 i,j;
	/* Configure the system clock to 200 MHz */
	  SystemClock_Config();  
	  DEBUG_USART_Config();/*���ڳ�ʼ��*/  
    LED_GPIO_Config();	 /* LED �˿ڳ�ʼ�� */
	  LCD_Init();  /*��ʼ��Һ����*/
   
	/* ��ʼ��Һ��Ϊ ARGB8888 ���� RGB565 ģʽģʽ */ 
    LCD_LayerInit(0, LCD_FRAME_BUFFER_LAYER0,RGB565);
    LCD_LayerInit(1, LCD_FRAME_BUFFER_LAYER1,ARGB8888);
    LCD_DisplayOn();  /* ʹ��Һ������*/ 
    
	  LCD_SelectLayer(0); /* ѡ��Һ��������*/
    LCD_Clear(LCD_COLOR_BLACK);   /*�ѱ�����ˢ��ɫ*/

    LCD_SelectLayer(1);  /* ѡ��Һ��ǰ����*/
    LCD_Clear(TRANSPARENCY); /*��ǰ����ˢ��ɫ*/

    /*Ĭ�����ò�͸��	���ú�������Ϊ��͸���ȣ���Χ 0-0xff ��0Ϊȫ͸����0xffΪ��͸��*/
    LCD_SetTransparency(0, 0xFF);
    LCD_SetTransparency(1, 0xFF);
    
    LED_BLUE;
    
    LCD_SetFont(&LCD_DEFAULT_FONT);
    LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);
	  LCD_ClearLine(19);
    LCD_DisplayStringLine_EN_CH(19,(uint8_t* )" ģʽ:UXGA 800x480");
    CAMERA_DEBUG("STM32F746 DCMI ����OV2640����");

/*------------------------DCMI-DMA������ʼ��������------------------------------*/ 
    
	 	OV2640_HW_Init();    /* ��ʼ������ͷGPIO��IIC */ 
    OV2640_ReadID(&OV2640_Camera_ID);/* ��ȡ����ͷоƬID��ȷ������ͷ�������� */

    OV2640_UXGAConfig();
    OV2640_DCMI_Config();
 
 while(1)
	{		
		DCMI_Start();  //������׽����
		HAL_Delay(600);//�����ϵ���ȶ���ͼ������
		OV2640_DMA_Config();
    
		HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_SNAPSHOT, LCD_FRAME_BUFFER_LAYER0,lcd_width*lcd_height/2); //ʹ��DCMI�ɼ�����
	  HAL_Delay(60);////������60�������ݲ�����
		
		//����SDRAM�ռ�洢�Ҷ�ͼƬ
	//�����Դ����ݵ����飨�����׵�ַ�Ѷ�����SDROM�У�
		for(i=0;i<img_height;i++)
	 {
		OV2640_DMA_M2M_Config(LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,(uint32_t)image2[i],img_width*2);
	  HAL_Delay(1);
	 }

/*-------------------------ͼ��Ԥ����----------------------------*/		  
	//�Ҷ�Ԥ����
	for(i=0; i<img_height; i++) //���ͼ��
	 {
		 for(j=0; j<img_width; j++)
		 {
		 image2[i][j]=RGB565_to_Gray(image2[i][j]);
		 }
 	 }	   
	 
	  MeanFilterOper();         //��Ȩ��ֵ�˲���������Ч���Ϻ�
	 //MedianFilterOper();    //��ֵ�˲�----����ƽ������һЩ��Ե��Ĩ��
	 //KNNFilterOper();      //knn�˲�
	 //Hist_plane();          //ֱ��ͼ���⻯
	
	 //lcd��ʾ
	// for(i=0;i<img_height;i++)
	 //{
	//	OV2640_DMA_M2M_Config((uint32_t)image2[i],LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,img_width*2);
	//  HAL_Delay(1);
	// }
	
	
/*---------------------ͼ��ָ��Ե���----------------------------*/	 
    
	  thre=otsuThreshold();    //����1 �Զ���ֵѡȡ
	   //thre=OET();          //����2
  	get_newimage(thre);      //ͼ��ָ�ں���̬ѧ�˲���
	  
	  //log_process(thre);//��Ե���
   
		
	 //lcd��ʾ
	  for(i=0;i<img_height;i++)
		 {
			OV2640_DMA_M2M_Config((uint32_t)image2[i],LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,img_width*2);
			HAL_Delay(1);
		 }

/*-----------�����ǡ�Ŀ����ȡ��������ֵ��Ч������ʱ�����Ӵ���ʱ�䣩-------------------------------*/		 
	  
	   LCD_SetTextColor(LCD_COLOR_BLACK);  //���ľֲ�������
	   LCD_DrawRect(W_top,H_top,400,440);	 //��ʼ��(W_top,H_top)w���,h�߶�
	   
		 //�����������ѡ��
	   cnt=mylabel();                       //������
	   label_extract(cnt,LABEL_EX_TH);       //Ŀ����ȡ
	 
		//lcd��ʾ
		 for(i=0;i<img_height;i++)
		 {
			OV2640_DMA_M2M_Config((uint32_t)image2[i],LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,img_width*2);
			HAL_Delay(1);
		 }
		
/*-------------------�������-������ʾ-�������----------------------------*/			      
		 
		  point2(p);                           //���Ե����������
	   
	    LCD_SetTextColor(LCD_COLOR_RED);  
	    LCD_DrawRect(p[3],p[1],p[2]-p[3],p[0]-p[1]);	  //��С����
	   
  	  d=distance(0.015,0.4,15,p[2],p[3]);
	   //��������ֵ(����Ϊ��Ԫ�ߴ硢���ࡢ��ࡢ�����С������)
     
			LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);
			LCD_ClearLine(18);
			sprintf((char*)dispBuf, " ��ֱ����:%.2fcm",(float)d);
			LCD_DisplayStringLine_EN_CH(18,dispBuf);
     
		 /*����printf��������Ϊ�ض�����fputc��printf�����ݻ����������*/
		  printf("��ֱ����Ϊ:%.2fcm",(float)d);
			
/*-------------------�Ƕȼ���-�Ƕ���ʾ-�������----------------------------*/			      
		      
		/*	if (d<=40)
			{
			 dig=diagonal(p,1.4,0.155);
			 LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);
			 LCD_ClearLine(LINE(15));
			 sprintf((char*)dispBuf, "ץȡ�Ƕ�:%.2fcm",(float)dig);
			 LCD_DisplayStringLine_EN_CH(LINE(17),dispBuf);
			// printf("%d",(int)dig);
			 Usart_SendHalfWord(DEBUG_USART1,(uint16_t)d);
			}
    */	
	  //LCD_SelectLayer(1);  /* ѡ��Һ��ǰ����*/
    //LCD_Clear(TRANSPARENCY); /*��ǰ����ˢ��ɫ*/
	
 }	
  
while(1)
    {
        //��ʾ֡�ʣ�Ĭ�ϲ���ʾ		
#if FRAME_RATE_DISPLAY		
        if(Task_Delay[0]==0)
        {
                    
            LCD_SetColors(LCD_COLOR_YELLOW,TRANSPARENCY);

            LCD_ClearLine(17);
            sprintf((char*)dispBuf, " ֡��:%d FPS", fps/5);

            /*���֡��*/
            LCD_DisplayStringLine_EN_CH(17,dispBuf);
            //����
            fps =0;

            Task_Delay[0]=5000; //��ֵÿ1ms���1������0�ſ������½�������
            

        }

#endif
    }


}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Activate the OverDrive to reach the 200 MHz Frequency */  
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2; 
  
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }  
}

//void Delay(__IO uint32_t nCount)	 //�򵥵���ʱ����
//{
//	for(; nCount != 0; nCount--);
//}
/*********************************************END OF FILE**********************/

