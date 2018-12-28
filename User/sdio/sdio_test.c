/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   SDIO sd卡测试驱动（不含文件系统）
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板  
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
#include "./sdio/sdio_test.h"
#include "./led/bsp_led.h"
#include "./sdio/bsp_sdio_sd.h"
#include "./usart/bsp_debug_usart.h"
#include "./sdram/bsp_sdram.h"  
#include "ff.h"	

#define GETR_FROM_RGB16(RGB565)  ((unsigned char)(( ((unsigned short int )RGB565) >>11)<<3))		  			//返回8位 R
#define GETG_FROM_RGB16(RGB565)  ((unsigned char)(( ((unsigned short int )(RGB565 & 0x7ff)) >>5)<<2)) 	//返回8位 G
#define GETB_FROM_RGB16(RGB565)  ((unsigned char)(( ((unsigned short int )(RGB565 & 0x1f))<<3)))       	//返回8位 B

FATFS bmpfs[2]; 
FIL bmpfsrc, bmpfdst; 
FRESULT bmpres;

void SD_Test(void)
{

	LED_BLUE;
  /*------------------------------ SD Init ---------------------------------- */
	/* SD卡使用SDIO中断及DMA中断接收数据，中断服务程序位于bsp_sdio_sd.c文件尾*/
  if(BSP_SD_Init() != MSD_OK)
  {    
		LED_RED;
    printf("SD卡初始化失败，请确保SD卡已正确接入开发板，或换一张SD卡测试！\n");
  }
  else
  {
    printf("SD卡初始化成功！\n");		 

		LED_BLUE;
    /*擦除测试*/
    SD_EraseTest();
    
		LED_BLUE;
    /*single block 读写测试*/
    SD_SingleBlockTest();
    
		//暂不支持直接多块读写，多块读写可用多个单块读写流程代替
		LED_BLUE;
    /*muti block 读写测试*/
    SD_MultiBlockTest();
  }
 
}


/**
  * @brief  Tests the SD card erase operation.
  * @param  None
  * @retval None
  */
void SD_EraseTest(void)
{  
  /*------------------- Block Erase ------------------------------------------*/
  if (Status == SD_OK)
  {
    /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
    Status = BSP_SD_Erase(0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));
  }

  if (Status == SD_OK)
  {
    Status = BSP_SD_ReadBlocks_DMA(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);

//    /* Check if the Transfer is finished */
//    Status = SD_WaitReadOperation();

//    /* Wait until end of DMA transfer */
//    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of erased blocks */
  if (Status == SD_OK)
  {
    EraseStatus = eBuffercmp(Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE/4);
  }
  
  if(EraseStatus == PASSED)
  {    
		LED_GREEN;
    printf("SD卡擦除测试成功！\n");
  }
  else
  {
		LED_BLUE;
    printf("SD卡擦除测试失败！\n");
    printf("温馨提示：部分SD卡不支持擦除测试，若SD卡能通过下面的single读写测试，即表示SD卡能够正常使用。\n");
  }
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
void SD_SingleBlockTest(void)
{  
  /*------------------- Block Read/Write --------------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_Block_Tx, BLOCK_SIZE/4, 0);

  if (Status == SD_OK)
  {
    /* Write block of 512 bytes on address 0 */
    Status = BSP_SD_WriteBlocks_DMA(Buffer_Block_Tx, 0x00, BLOCK_SIZE,1);
    /* Check if the Transfer is finished */
//    Status = SD_WaitWriteOperation();
//    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK)
  {
    /* Read block of 512 bytes from address 0 */
    Status = BSP_SD_ReadBlocks_DMA(Buffer_Block_Rx, 0x00, BLOCK_SIZE,1);
    /* Check if the Transfer is finished */
//    Status = SD_WaitReadOperation();
//    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
    TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BLOCK_SIZE/4);
  }
  
  if(TransferStatus1 == PASSED)
  {
    LED_GREEN;
    printf("Single block 测试成功！\n");

  }
  else
  {
		LED_RED;
    printf("Single block 测试失败，请确保SD卡正确接入开发板，或换一张SD卡测试！\n");
    
  }
}

/**
  * @brief  Tests the SD card Multiple Blocks operations.
  * @param  None
  * @retval None
  */
void SD_MultiBlockTest(void)
{  
  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE/4, 0x0);

  if (Status == SD_OK)
  {
    /* Write multiple block of many bytes on address 0 */
    Status = BSP_SD_WriteBlocks_DMA(Buffer_MultiBlock_Tx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    /* Check if the Transfer is finished */
//    Status = SD_WaitWriteOperation();
//    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK)
  {
    /* Read block of many bytes from address 0 */
    Status = BSP_SD_ReadBlocks_DMA(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    /* Check if the Transfer is finished */
//    Status = SD_WaitReadOperation();
//    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
    TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE/4);
  }
  
  if(TransferStatus2 == PASSED)
  {
		LED_GREEN;
    printf("Multi block 测试成功！");

  }
  else
  {
		LED_RED;
    printf("Multi block 测试失败，请确保SD卡正确接入开发板，或换一张SD卡测试！");
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer1 identical to pBuffer2
  *         FAILED: pBuffer1 differs from pBuffer2
  */
TestStatus Buffercmp(uint32_t* pBuffer1, uint32_t* pBuffer2, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}

/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferLength: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  * @retval None
  */
void Fill_Buffer(uint32_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
  uint32_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++)
  {
    pBuffer[index] = index + Offset;
  }
}

/**
  * @brief  Checks if a buffer has all its values are equal to zero.
  * @param  pBuffer: buffer to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer values are zero
  *         FAILED: At least one value from pBuffer buffer is different from zero.
  */
TestStatus eBuffercmp(uint32_t* pBuffer, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
    if ((*pBuffer != 0xFF) && (*pBuffer != 0x00))
    {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}
/**********************************************************
 * 函数名：Screen_shot
 * 描述  ：截取LCD指定位置  指定宽高的像素 保存为24位真彩色bmp格式图片
 * 输入  : 	x								---水平位置 
 *					y								---竖直位置  
 *					Width						---水平宽度   
 *					Height					---竖直高度  	
 *					filename				---文件名
 * 输出  ：	0 		---成功
 *  				-1 		---失败
 *	    		8			---文件已存在
 * 举例  ：Screen_shot(0, 0, 800, 480, "/myScreen");-----全屏截图
 * 
 **************************************************************/ 
/*
 * bmp文件头有54个字节，其中前14个字节是文件头信息，后40个字节是位图信息头信息
 * bmp文件头之后就是具体的像素信息
 * 0x42 0x4d :bm
 * 54        :实际位图数据的偏移字节数
 * 40        :位图信息头结构体的长度
 * 1         :平面数为1
 * 24        :24bit真彩色
 */
int Screen_shot(unsigned short int x,\
                unsigned short int y,\
                unsigned short int Width,\
                unsigned short int Height,\
                unsigned char *filename)
{
	/* bmp  文件头 54个字节 */
	unsigned char header[54] =
	{
		0x42, 0x4d, 0, 0, 0, 0, 
		0, 0, 0, 0, 54, 0, 
		0, 0, 40,0, 0, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 1, 0, 24, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 
		0, 0, 0
	};
	
	int i;
	int j;
	/*般来说，.bMP文件的数据从下到上，从左到右的。也就是说，从文件中最先读到的是图象最下面一行的左边第一个象素，然后是左边
	 第二个象素……接下来是倒数第二行左边第一个象素，左边第二个象素……依次类推，最后得到的是最上面一行的最右一个象素。
	 所以这里将指针指向图片左下角的第一个像素点，然后从左向右读数据。*/
	uint16_t *p =(uint16_t *)(SDRAM_BANK_ADDR+(Height-1)*800*2);
	long file_size;     
	long width;
	long height;
	unsigned char r,g,b;	
	unsigned char tmp_name[30];
	unsigned int mybw;
	unsigned int read_data;
	char kk[4]={0,0,0,0};
	//printf("\r\n图片左下角地址0x%x\r\n",(uint32_t)p);
	/* 宽*高 +补充的字节 + 头部信息 */
	file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + 54;		

	/* 文件大小 4个字节 */
	header[2] = (unsigned char)(file_size &0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;
	
	/* 位图宽 4个字节 */
	width=Width;	
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) &0x000000ff;
	header[20] = (width >> 16) &0x000000ff;
	header[21] = (width >> 24) &0x000000ff;
	
	/* 位图高 4个字节 */
	height = Height;
	header[22] = height &0x000000ff;
	header[23] = (height >> 8) &0x000000ff;
	header[24] = (height >> 16) &0x000000ff;
	header[25] = (height >> 24) &0x000000ff;
	
	/* 将filename 按照一定的格式拷贝到 tmp_name */
	sprintf((char*)tmp_name,"0:%s.bmp",filename);
	
	/* 注册一个工作区，逻辑号为0 */
	f_mount(&bmpfs[0],"0:",1 );
    
	#if 0
	　/* 新建一个文件 */
	　bmpres = f_open( &bmpfsrc , (char*)tmp_name, FA_CREATE_NEW | FA_WRITE );	
	　/* 新建文件之后要先关闭再打开才能写入 */
	　f_close(&bmpfsrc);
	　bmpres = f_open( &bmpfsrc , (char*)tmp_name,  FA_OPEN_EXISTING | FA_WRITE);
  #else
    bmpres = f_open( &bmpfsrc , (char*)tmp_name,  FA_OPEN_ALWAYS | FA_WRITE);
  #endif
	if ( bmpres == FR_OK )
	{    
		/* 将预先定义好的bmp头部信息写进文件里面 */
		bmpres = f_write(&bmpfsrc, header,sizeof(unsigned char)*54, &mybw);		
		
		/* 下面是将指定窗口的数据读出来写到文件里面去 */
		for(i=0; i<Height; i++)					
		{
			if( !(Width%4) )				/* 刚好是4字节对齐 */
			{
        #if 1
          for(j=0; j<Width; j++,p++)  
          {					
            //read_data = LCD_GetPoint(y+j, x+i);				
            read_data = *(p);
            r =  GETR_FROM_RGB16(read_data);
            g =  GETG_FROM_RGB16(read_data);
            b =  GETB_FROM_RGB16(read_data);

            bmpres = f_write(&bmpfsrc, &b,sizeof(unsigned char), &mybw);
            bmpres = f_write(&bmpfsrc, &g,sizeof(unsigned char), &mybw);
            bmpres = f_write(&bmpfsrc, &r,sizeof(unsigned char), &mybw);
          }
        #else
          LCD_GetLine_BGR24( y, x+i,Width,pColorDatapic);
          bmpres = f_write(&bmpfsrc, pColorDatapic,Width*3, &mybw);
          //bmpres = f_write(&bmpfsrc, pColorDatapic+Width,Width , &mybw);
          //bmpres = f_write(&bmpfsrc, pColorDatapic+2*Width,Width , &mybw);
        #endif
			}
			else
			{
				for(j=0;j<Width;j++,p++)
				{					
            read_data = *p;
					
					r =  GETR_FROM_RGB16(read_data);
					g =  GETG_FROM_RGB16(read_data);
					b =  GETB_FROM_RGB16(read_data);

					bmpres = f_write(&bmpfsrc, &b,sizeof(unsigned char), &mybw);
					bmpres = f_write(&bmpfsrc, &g,sizeof(unsigned char), &mybw);
					bmpres = f_write(&bmpfsrc, &r,sizeof(unsigned char), &mybw);
				}
				/* 不是4字节对齐则需要补齐 */	
				bmpres = f_write(&bmpfsrc, kk,sizeof(unsigned char)*(Width%4), &mybw);
			}	
			/*读完一行后，将指针指向上一行的第一个像素点*/
			p-=(800+Width);
		}/* 截屏完毕 */
		f_close(&bmpfsrc); 
		return 0;
	}
	else if ( bmpres == FR_EXIST )  //如果文件已经存在
	{
		return FR_EXIST;	 					//8
	}
	else/* 截屏失败 */
	{
		return -1;
	}    
}
/*********************************************END OF FILE**********************/
