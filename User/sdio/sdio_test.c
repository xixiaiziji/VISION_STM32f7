/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   SDIO sd�����������������ļ�ϵͳ��
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F429 ������  
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
#include "./sdio/sdio_test.h"
#include "./led/bsp_led.h"
#include "./sdio/bsp_sdio_sd.h"
#include "./usart/bsp_debug_usart.h"
#include "./sdram/bsp_sdram.h"  
#include "ff.h"	

#define GETR_FROM_RGB16(RGB565)  ((unsigned char)(( ((unsigned short int )RGB565) >>11)<<3))		  			//����8λ R
#define GETG_FROM_RGB16(RGB565)  ((unsigned char)(( ((unsigned short int )(RGB565 & 0x7ff)) >>5)<<2)) 	//����8λ G
#define GETB_FROM_RGB16(RGB565)  ((unsigned char)(( ((unsigned short int )(RGB565 & 0x1f))<<3)))       	//����8λ B

FATFS bmpfs[2]; 
FIL bmpfsrc, bmpfdst; 
FRESULT bmpres;

void SD_Test(void)
{

	LED_BLUE;
  /*------------------------------ SD Init ---------------------------------- */
	/* SD��ʹ��SDIO�жϼ�DMA�жϽ������ݣ��жϷ������λ��bsp_sdio_sd.c�ļ�β*/
  if(BSP_SD_Init() != MSD_OK)
  {    
		LED_RED;
    printf("SD����ʼ��ʧ�ܣ���ȷ��SD������ȷ���뿪���壬��һ��SD�����ԣ�\n");
  }
  else
  {
    printf("SD����ʼ���ɹ���\n");		 

		LED_BLUE;
    /*��������*/
    SD_EraseTest();
    
		LED_BLUE;
    /*single block ��д����*/
    SD_SingleBlockTest();
    
		//�ݲ�֧��ֱ�Ӷ���д������д���ö�������д���̴���
		LED_BLUE;
    /*muti block ��д����*/
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
    printf("SD���������Գɹ���\n");
  }
  else
  {
		LED_BLUE;
    printf("SD����������ʧ�ܣ�\n");
    printf("��ܰ��ʾ������SD����֧�ֲ������ԣ���SD����ͨ�������single��д���ԣ�����ʾSD���ܹ�����ʹ�á�\n");
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
    printf("Single block ���Գɹ���\n");

  }
  else
  {
		LED_RED;
    printf("Single block ����ʧ�ܣ���ȷ��SD����ȷ���뿪���壬��һ��SD�����ԣ�\n");
    
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
    printf("Multi block ���Գɹ���");

  }
  else
  {
		LED_RED;
    printf("Multi block ����ʧ�ܣ���ȷ��SD����ȷ���뿪���壬��һ��SD�����ԣ�");
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
 * ��������Screen_shot
 * ����  ����ȡLCDָ��λ��  ָ����ߵ����� ����Ϊ24λ���ɫbmp��ʽͼƬ
 * ����  : 	x								---ˮƽλ�� 
 *					y								---��ֱλ��  
 *					Width						---ˮƽ���   
 *					Height					---��ֱ�߶�  	
 *					filename				---�ļ���
 * ���  ��	0 		---�ɹ�
 *  				-1 		---ʧ��
 *	    		8			---�ļ��Ѵ���
 * ����  ��Screen_shot(0, 0, 800, 480, "/myScreen");-----ȫ����ͼ
 * 
 **************************************************************/ 
/*
 * bmp�ļ�ͷ��54���ֽڣ�����ǰ14���ֽ����ļ�ͷ��Ϣ����40���ֽ���λͼ��Ϣͷ��Ϣ
 * bmp�ļ�ͷ֮����Ǿ����������Ϣ
 * 0x42 0x4d :bm
 * 54        :ʵ��λͼ���ݵ�ƫ���ֽ���
 * 40        :λͼ��Ϣͷ�ṹ��ĳ���
 * 1         :ƽ����Ϊ1
 * 24        :24bit���ɫ
 */
int Screen_shot(unsigned short int x,\
                unsigned short int y,\
                unsigned short int Width,\
                unsigned short int Height,\
                unsigned char *filename)
{
	/* bmp  �ļ�ͷ 54���ֽ� */
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
	/*����˵��.bMP�ļ������ݴ��µ��ϣ������ҵġ�Ҳ����˵�����ļ������ȶ�������ͼ��������һ�е���ߵ�һ�����أ�Ȼ�������
	 �ڶ������ء����������ǵ����ڶ�����ߵ�һ�����أ���ߵڶ������ء����������ƣ����õ�����������һ�е�����һ�����ء�
	 �������ｫָ��ָ��ͼƬ���½ǵĵ�һ�����ص㣬Ȼ��������Ҷ����ݡ�*/
	uint16_t *p =(uint16_t *)(SDRAM_BANK_ADDR+(Height-1)*800*2);
	long file_size;     
	long width;
	long height;
	unsigned char r,g,b;	
	unsigned char tmp_name[30];
	unsigned int mybw;
	unsigned int read_data;
	char kk[4]={0,0,0,0};
	//printf("\r\nͼƬ���½ǵ�ַ0x%x\r\n",(uint32_t)p);
	/* ��*�� +������ֽ� + ͷ����Ϣ */
	file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + 54;		

	/* �ļ���С 4���ֽ� */
	header[2] = (unsigned char)(file_size &0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	width=Width;	
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) &0x000000ff;
	header[20] = (width >> 16) &0x000000ff;
	header[21] = (width >> 24) &0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	height = Height;
	header[22] = height &0x000000ff;
	header[23] = (height >> 8) &0x000000ff;
	header[24] = (height >> 16) &0x000000ff;
	header[25] = (height >> 24) &0x000000ff;
	
	/* ��filename ����һ���ĸ�ʽ������ tmp_name */
	sprintf((char*)tmp_name,"0:%s.bmp",filename);
	
	/* ע��һ�����������߼���Ϊ0 */
	f_mount(&bmpfs[0],"0:",1 );
    
	#if 0
	��/* �½�һ���ļ� */
	��bmpres = f_open( &bmpfsrc , (char*)tmp_name, FA_CREATE_NEW | FA_WRITE );	
	��/* �½��ļ�֮��Ҫ�ȹر��ٴ򿪲���д�� */
	��f_close(&bmpfsrc);
	��bmpres = f_open( &bmpfsrc , (char*)tmp_name,  FA_OPEN_EXISTING | FA_WRITE);
  #else
    bmpres = f_open( &bmpfsrc , (char*)tmp_name,  FA_OPEN_ALWAYS | FA_WRITE);
  #endif
	if ( bmpres == FR_OK )
	{    
		/* ��Ԥ�ȶ���õ�bmpͷ����Ϣд���ļ����� */
		bmpres = f_write(&bmpfsrc, header,sizeof(unsigned char)*54, &mybw);		
		
		/* �����ǽ�ָ�����ڵ����ݶ�����д���ļ�����ȥ */
		for(i=0; i<Height; i++)					
		{
			if( !(Width%4) )				/* �պ���4�ֽڶ��� */
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
				/* ����4�ֽڶ�������Ҫ���� */	
				bmpres = f_write(&bmpfsrc, kk,sizeof(unsigned char)*(Width%4), &mybw);
			}	
			/*����һ�к󣬽�ָ��ָ����һ�еĵ�һ�����ص�*/
			p-=(800+Width);
		}/* ������� */
		f_close(&bmpfsrc); 
		return 0;
	}
	else if ( bmpres == FR_EXIST )  //����ļ��Ѿ�����
	{
		return FR_EXIST;	 					//8
	}
	else/* ����ʧ�� */
	{
		return -1;
	}    
}
/*********************************************END OF FILE**********************/
