#ifndef __SDIO_TEST_H
#define __SDIO_TEST_H

#include "stm32f7xx.h"

void SD_Test(void);
/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
uint8_t Status;
/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      50  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Buffer_Block_Tx[BLOCK_SIZE/4], Buffer_Block_Rx[BLOCK_SIZE/4];
uint32_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE/4], Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE/4];
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
//SD_Error Status = SD_OK;

/* Private function prototypes -----------------------------------------------*/
static void SD_EraseTest(void);
static void SD_SingleBlockTest(void);
void SD_MultiBlockTest(void);
static void Fill_Buffer(uint32_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
static TestStatus Buffercmp(uint32_t* pBuffer1, uint32_t* pBuffer2, uint32_t BufferLength);
static TestStatus eBuffercmp(uint32_t* pBuffer, uint32_t BufferLength);

/* Private functions ---------------------------------------------------------*/

#endif


/*****************************END OF FILE**************************/
