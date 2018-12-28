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

/*简单任务管理*/
uint32_t Task_Delay[3];
uint8_t dispBuf[100];
OV2640_IDTypeDef OV2640_Camera_ID;
float d;                         //距离参数
uint16_t p[4];                    //最小外接矩形4个点
uint16_t dig;                     //角度参数
uint16_t thre;                      //阈值参数
uint16_t ratio,center_x,center_y,cnt;

#define LABEL_EX_TH  1000  //伪目标抹除阈值   小于它则被抹掉
#define FRAME_RATE_DISPLAY 0
uint8_t fps=0;
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    u16 i,j;
	/* Configure the system clock to 200 MHz */
	  SystemClock_Config();  
	  DEBUG_USART_Config();/*串口初始化*/  
    LED_GPIO_Config();	 /* LED 端口初始化 */
	  LCD_Init();  /*初始化液晶屏*/
   
	/* 初始化液晶为 ARGB8888 或者 RGB565 模式模式 */ 
    LCD_LayerInit(0, LCD_FRAME_BUFFER_LAYER0,RGB565);
    LCD_LayerInit(1, LCD_FRAME_BUFFER_LAYER1,ARGB8888);
    LCD_DisplayOn();  /* 使能液晶背光*/ 
    
	  LCD_SelectLayer(0); /* 选择液晶背景层*/
    LCD_Clear(LCD_COLOR_BLACK);   /*把背景层刷黑色*/

    LCD_SelectLayer(1);  /* 选择液晶前景层*/
    LCD_Clear(TRANSPARENCY); /*把前景层刷黑色*/

    /*默认设置不透明	，该函数参数为不透明度，范围 0-0xff ，0为全透明，0xff为不透明*/
    LCD_SetTransparency(0, 0xFF);
    LCD_SetTransparency(1, 0xFF);
    
    LED_BLUE;
    
    LCD_SetFont(&LCD_DEFAULT_FONT);
    LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);
	  LCD_ClearLine(19);
    LCD_DisplayStringLine_EN_CH(19,(uint8_t* )" 模式:UXGA 800x480");
    CAMERA_DEBUG("STM32F746 DCMI 驱动OV2640例程");

/*------------------------DCMI-DMA驱动初始化及配置------------------------------*/ 
    
	 	OV2640_HW_Init();    /* 初始化摄像头GPIO及IIC */ 
    OV2640_ReadID(&OV2640_Camera_ID);/* 读取摄像头芯片ID，确定摄像头正常连接 */

    OV2640_UXGAConfig();
    OV2640_DCMI_Config();
 
 while(1)
	{		
		DCMI_Start();  //开启捕捉数据
		HAL_Delay(600);//丢弃上电后不稳定的图像数据
		OV2640_DMA_Config();
    
		HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_SNAPSHOT, LCD_FRAME_BUFFER_LAYER0,lcd_width*lcd_height/2); //使能DCMI采集数据
	  HAL_Delay(60);////不可于60否则数据不完整
		
		//开辟SDRAM空间存储灰度图片
	//搬运显存数据到数组（数组首地址已定义在SDROM中）
		for(i=0;i<img_height;i++)
	 {
		OV2640_DMA_M2M_Config(LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,(uint32_t)image2[i],img_width*2);
	  HAL_Delay(1);
	 }

/*-------------------------图像预处理----------------------------*/		  
	//灰度预处理
	for(i=0; i<img_height; i++) //获得图像
	 {
		 for(j=0; j<img_width; j++)
		 {
		 image2[i][j]=RGB565_to_Gray(image2[i][j]);
		 }
 	 }	   
	 
	  MeanFilterOper();         //加权均值滤波————效果较好
	 //MedianFilterOper();    //中值滤波----过度平滑导致一些边缘被抹除
	 //KNNFilterOper();      //knn滤波
	 //Hist_plane();          //直方图均衡化
	
	 //lcd显示
	// for(i=0;i<img_height;i++)
	 //{
	//	OV2640_DMA_M2M_Config((uint32_t)image2[i],LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,img_width*2);
	//  HAL_Delay(1);
	// }
	
	
/*---------------------图像分割及边缘检测----------------------------*/	 
    
	  thre=otsuThreshold();    //方法1 自动阈值选取
	   //thre=OET();          //方法2
  	get_newimage(thre);      //图像分割（内含形态学滤波）
	  
	  //log_process(thre);//边缘检测
   
		
	 //lcd显示
	  for(i=0;i<img_height;i++)
		 {
			OV2640_DMA_M2M_Config((uint32_t)image2[i],LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,img_width*2);
			HAL_Delay(1);
		 }

/*-----------区域标记、目标提取（用于阈值化效果不好时会增加处理时间）-------------------------------*/		 
	  
	   LCD_SetTextColor(LCD_COLOR_BLACK);  //中心局部计算区
	   LCD_DrawRect(W_top,H_top,400,440);	 //起始点(W_top,H_top)w宽度,h高度
	   
		 //根据噪声情况选择
	   cnt=mylabel();                       //区域标记
	   label_extract(cnt,LABEL_EX_TH);       //目标提取
	 
		//lcd显示
		 for(i=0;i<img_height;i++)
		 {
			OV2640_DMA_M2M_Config((uint32_t)image2[i],LCD_FRAME_BUFFER_LAYER0+i*(img_width)*2,img_width*2);
			HAL_Delay(1);
		 }
		
/*-------------------距离计算-距离显示-串口输出----------------------------*/			      
		 
		  point2(p);                           //求绝缘子两端最大点
	   
	    LCD_SetTextColor(LCD_COLOR_RED);  
	    LCD_DrawRect(p[3],p[1],p[2]-p[3],p[0]-p[1]);	  //最小矩形
	   
  	  d=distance(0.015,0.4,15,p[2],p[3]);
	   //测量距离值(参数为像元尺寸、焦距、物距、最大最小横坐标)
     
			LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);
			LCD_ClearLine(18);
			sprintf((char*)dispBuf, " 垂直距离:%.2fcm",(float)d);
			LCD_DisplayStringLine_EN_CH(18,dispBuf);
     
		 /*调用printf函数，因为重定向了fputc，printf的内容会输出到串口*/
		  printf("垂直距离为:%.2fcm",(float)d);
			
/*-------------------角度计算-角度显示-串口输出----------------------------*/			      
		      
		/*	if (d<=40)
			{
			 dig=diagonal(p,1.4,0.155);
			 LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);
			 LCD_ClearLine(LINE(15));
			 sprintf((char*)dispBuf, "抓取角度:%.2fcm",(float)dig);
			 LCD_DisplayStringLine_EN_CH(LINE(17),dispBuf);
			// printf("%d",(int)dig);
			 Usart_SendHalfWord(DEBUG_USART1,(uint16_t)d);
			}
    */	
	  //LCD_SelectLayer(1);  /* 选择液晶前景层*/
    //LCD_Clear(TRANSPARENCY); /*把前景层刷黑色*/
	
 }	
  
while(1)
    {
        //显示帧率，默认不显示		
#if FRAME_RATE_DISPLAY		
        if(Task_Delay[0]==0)
        {
                    
            LCD_SetColors(LCD_COLOR_YELLOW,TRANSPARENCY);

            LCD_ClearLine(17);
            sprintf((char*)dispBuf, " 帧率:%d FPS", fps/5);

            /*输出帧率*/
            LCD_DisplayStringLine_EN_CH(17,dispBuf);
            //重置
            fps =0;

            Task_Delay[0]=5000; //此值每1ms会减1，减到0才可以重新进来这里
            

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

//void Delay(__IO uint32_t nCount)	 //简单的延时函数
//{
//	for(; nCount != 0; nCount--);
//}
/*********************************************END OF FILE**********************/

