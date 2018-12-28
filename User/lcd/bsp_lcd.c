/**
  ******************************************************************************
  * @file    bsp_lcd.c
  * @author  ZHANG
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   API of lcd 
  * @Platform STM32F767
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "./lcd/bsp_lcd.h"
#include "./fonts//font24.c"
#include "./fonts//font20.c"
#include "./fonts//font16.c"
#include "./fonts//font12.c"
#include "./fonts//font8.c"


#define POLY_X(Z)              ((int32_t)((Points + Z)->X))
#define POLY_Y(Z)              ((int32_t)((Points + Z)->Y)) 

#define ABS(X)  ((X) > 0 ? (X) : -(X))

static LTDC_HandleTypeDef  Ltdc_Handler;
static DMA2D_HandleTypeDef Dma2d_Handler;

/* Default LCD configuration with LCD Layer 1 */
static uint32_t            ActiveLayer = 0;
static LCD_DrawPropTypeDef DrawProp[MAX_LAYER_NUMBER];
/**
 * @brief  Initializes the LCD.
 * @param  None
 * @retval None
 */


/*根据液晶数据手册的参数配置*/
#define HBP  46		//HSYNC后的无效像素
#define VBP  23		//VSYNC后的无效行数

#define HSW   1		//HSYNC宽度
#define VSW   1		//VSYNC宽度

#define HFP  20		//HSYNC前的无效像素
#define VFP  22		//VSYNC前的无效行数

static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c);
static void FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);
static void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
static void LL_ConvertLineToARGB8888(void * pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode);
 /**
  * @brief  初始化控制LCD的IO
  * @param  无
  * @retval 无
  */
static void LCD_GPIO_Config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStruct;
  
  /* 使能LCD使用到的引脚时钟 */
                          //红色数据线
  LTDC_R0_GPIO_CLK_ENABLE();LTDC_R1_GPIO_CLK_ENABLE();LTDC_R2_GPIO_CLK_ENABLE();\
  LTDC_R3_GPIO_CLK_ENABLE();LTDC_R4_GPIO_CLK_ENABLE();LTDC_R5_GPIO_CLK_ENABLE();\
  LTDC_R6_GPIO_CLK_ENABLE();LTDC_R7_GPIO_CLK_ENABLE();LTDC_G0_GPIO_CLK_ENABLE();\
  LTDC_G1_GPIO_CLK_ENABLE();LTDC_G2_GPIO_CLK_ENABLE();LTDC_G3_GPIO_CLK_ENABLE();\
  LTDC_G3_GPIO_CLK_ENABLE();LTDC_G5_GPIO_CLK_ENABLE();LTDC_G6_GPIO_CLK_ENABLE();\
  LTDC_G7_GPIO_CLK_ENABLE();LTDC_B0_GPIO_CLK_ENABLE();LTDC_B1_GPIO_CLK_ENABLE();\
  LTDC_B2_GPIO_CLK_ENABLE();LTDC_B3_GPIO_CLK_ENABLE();LTDC_B4_GPIO_CLK_ENABLE();\
  LTDC_B5_GPIO_CLK_ENABLE();LTDC_B6_GPIO_CLK_ENABLE();LTDC_B7_GPIO_CLK_ENABLE();\
  LTDC_CLK_GPIO_CLK_ENABLE();LTDC_HSYNC_GPIO_CLK_ENABLE();LTDC_VSYNC_GPIO_CLK_ENABLE();\
  LTDC_DE_GPIO_CLK_ENABLE();LTDC_DISP_GPIO_CLK_ENABLE();LTDC_BL_GPIO_CLK_ENABLE();
/* GPIO配置 */

 /* 红色数据线 */
  GPIO_InitStruct.Pin =   LTDC_R0_GPIO_PIN;                             
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  
  GPIO_InitStruct.Pin =   LTDC_R0_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R0_AF;
  HAL_GPIO_Init(LTDC_R0_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =   LTDC_R1_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R1_AF;
  HAL_GPIO_Init(LTDC_R1_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_R2_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R2_AF;
  HAL_GPIO_Init(LTDC_R2_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_R3_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R3_AF;
  HAL_GPIO_Init(LTDC_R3_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_R4_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R4_AF;
  HAL_GPIO_Init(LTDC_R4_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_R5_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R5_AF;
  HAL_GPIO_Init(LTDC_R5_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_R6_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R6_AF;
  HAL_GPIO_Init(LTDC_R6_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_R7_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_R7_AF;
  HAL_GPIO_Init(LTDC_R7_GPIO_PORT, &GPIO_InitStruct);
  
  //绿色数据线
  GPIO_InitStruct.Pin =   LTDC_G0_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_G0_AF;
  HAL_GPIO_Init(LTDC_G0_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =   LTDC_G1_GPIO_PIN; 
  GPIO_InitStruct.Alternate = LTDC_G1_AF;
  HAL_GPIO_Init(LTDC_G1_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_G2_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_G2_AF;
  HAL_GPIO_Init(LTDC_G2_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_G3_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_G3_AF;
  HAL_GPIO_Init(LTDC_G3_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_G4_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_G4_AF;
  HAL_GPIO_Init(LTDC_G4_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_G5_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_G5_AF;
  HAL_GPIO_Init(LTDC_G5_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_G6_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_G6_AF;
  HAL_GPIO_Init(LTDC_G6_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_G7_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_G7_AF;
  HAL_GPIO_Init(LTDC_G7_GPIO_PORT, &GPIO_InitStruct);
  
  //蓝色数据线
  GPIO_InitStruct.Pin =   LTDC_B0_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B0_AF;
  HAL_GPIO_Init(LTDC_B0_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =   LTDC_B1_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B1_AF;
  HAL_GPIO_Init(LTDC_B1_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_B2_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B2_AF;
  HAL_GPIO_Init(LTDC_B2_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_B3_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B3_AF;
  HAL_GPIO_Init(LTDC_B3_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_B4_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B4_AF;
  HAL_GPIO_Init(LTDC_B4_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_B5_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B5_AF;
  HAL_GPIO_Init(LTDC_B5_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_B6_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B6_AF;
  HAL_GPIO_Init(LTDC_B6_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin =   LTDC_B7_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_B7_AF;
  HAL_GPIO_Init(LTDC_B7_GPIO_PORT, &GPIO_InitStruct);
  
  //控制信号线
  GPIO_InitStruct.Pin = LTDC_CLK_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_CLK_AF;
  HAL_GPIO_Init(LTDC_CLK_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = LTDC_HSYNC_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_HSYNC_AF;
  HAL_GPIO_Init(LTDC_HSYNC_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = LTDC_VSYNC_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_VSYNC_AF;
  HAL_GPIO_Init(LTDC_VSYNC_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = LTDC_DE_GPIO_PIN;
  GPIO_InitStruct.Alternate = LTDC_DE_AF;
  HAL_GPIO_Init(LTDC_DE_GPIO_PORT, &GPIO_InitStruct);
  
  //背光BL 及液晶使能信号DISP
  GPIO_InitStruct.Pin = LTDC_DISP_GPIO_PIN;                             
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  
  HAL_GPIO_Init(LTDC_DISP_GPIO_PORT, &GPIO_InitStruct);
  
  
  GPIO_InitStruct.Pin = LTDC_BL_GPIO_PIN; 
  HAL_GPIO_Init(LTDC_BL_GPIO_PORT, &GPIO_InitStruct);
  
}

void LCD_Init(void)
{
    RCC_PeriphCLKInitTypeDef  periph_clk_init_struct;  
    /* 使能LTDC时钟 */
    __HAL_RCC_LTDC_CLK_ENABLE();

    /* 使能DMA2D时钟 */
    __HAL_RCC_DMA2D_CLK_ENABLE();

    /* 配置LTDC参数 */
    Ltdc_Handler.Instance = LTDC;  

    /* 配置行同步信号宽度(HSW-1) */
    Ltdc_Handler.Init.HorizontalSync =HSW-1;
    /* 配置垂直同步信号宽度(VSW-1) */
    Ltdc_Handler.Init.VerticalSync = VSW-1;
    /* 配置(HSW+HBP-1) */
    Ltdc_Handler.Init.AccumulatedHBP = HSW+HBP-1;
    /* 配置(VSW+VBP-1) */
    Ltdc_Handler.Init.AccumulatedVBP = VSW+VBP-1;
    /* 配置(HSW+HBP+有效像素宽度-1) */
    Ltdc_Handler.Init.AccumulatedActiveW = HSW+HBP+LCD_PIXEL_WIDTH-1;
    /* 配置(VSW+VBP+有效像素高度-1) */
    Ltdc_Handler.Init.AccumulatedActiveH = VSW+VBP+LCD_PIXEL_HEIGHT-1;
    /* 配置总宽度(HSW+HBP+有效像素宽度+HFP-1) */
    Ltdc_Handler.Init.TotalWidth =HSW+ HBP+LCD_PIXEL_WIDTH + HFP-1; 
    /* 配置总高度(VSW+VBP+有效像素高度+VFP-1) */
    Ltdc_Handler.Init.TotalHeigh =VSW+ VBP+LCD_PIXEL_HEIGHT + VFP-1;

    /* 液晶屏时钟配置 */
    /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
    /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
    /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/5 = 38.4 Mhz */
    /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6Mhz */
    periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    periph_clk_init_struct.PLLSAI.PLLSAIN = 100;
    periph_clk_init_struct.PLLSAI.PLLSAIR = 5;
    periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);

    /* Initialize the LCD pixel width and pixel height */
    Ltdc_Handler.LayerCfg->ImageWidth  = LCD_PIXEL_WIDTH;
    Ltdc_Handler.LayerCfg->ImageHeight = LCD_PIXEL_HEIGHT;

    /* Configure R,G,B component values for LCD background color */
    Ltdc_Handler.Init.Backcolor.Red = 0;
    Ltdc_Handler.Init.Backcolor.Green = 0;
    Ltdc_Handler.Init.Backcolor.Blue = 0;
    /* Polarity configuration */
    /* Initialize the horizontal synchronization polarity as active low */
    Ltdc_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    /* Initialize the vertical synchronization polarity as active low */
    Ltdc_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    /* Initialize the data enable polarity as active low */
    Ltdc_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    /* Initialize the pixel clock polarity as input pixel clock */
    Ltdc_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

    HAL_LTDC_Init(&Ltdc_Handler);

    /* Configure the LCD Control pins */
    LCD_GPIO_Config();
    /* Configure the FMC Parallel interface : SDRAM is used as Frame Buffer for LCD */
    SDRAM_Init();
    /* Initialize the font */
    LCD_SetFont(&LCD_DEFAULT_FONT);
}

/**
  * @brief  Gets the LCD X size.
  * @retval Used LCD X size
  */
uint32_t LCD_GetXSize(void)
{
  return Ltdc_Handler.LayerCfg[ActiveLayer].ImageWidth;
}

/**
  * @brief  Gets the LCD Y size.
  * @retval Used LCD Y size
  */
uint32_t LCD_GetYSize(void)
{
  return Ltdc_Handler.LayerCfg[ActiveLayer].ImageHeight;
}

/**
  * @brief  Set the LCD X size.
  * @param  imageWidthPixels : image width in pixels unit
  * @retval None
  */
void LCD_SetXSize(uint32_t imageWidthPixels)
{
  Ltdc_Handler.LayerCfg[ActiveLayer].ImageWidth = imageWidthPixels;
}

/**
  * @brief  Set the LCD Y size.
  * @param  imageHeightPixels : image height in lines unit
  * @retval None
  */
void LCD_SetYSize(uint32_t imageHeightPixels)
{
  Ltdc_Handler.LayerCfg[ActiveLayer].ImageHeight = imageHeightPixels;
}

/**
  * @brief  Initializes the LCD layer in ARGB8888 format (32 bits per pixel).
  * @param  LayerIndex: Layer foreground or background
  * @param  FB_Address: Layer frame buffer
  * @retval None
  */
void LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address)
{     
  LTDC_LayerCfgTypeDef  layer_cfg;

  /* Layer Init */
  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = LCD_GetXSize();
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = LCD_GetYSize(); 
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  layer_cfg.FBStartAdress = FB_Address;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = LCD_GetXSize();
  layer_cfg.ImageHeight = LCD_GetYSize();
  
  HAL_LTDC_ConfigLayer(&Ltdc_Handler, &layer_cfg, LayerIndex); 

  DrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;
  DrawProp[LayerIndex].pFont     = &LCD_DEFAULT_FONT;
  DrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK; 
}

/**
  * @brief  Initializes the LCD layer in RGB565 format (16 bits per pixel).
  * @param  LayerIndex: Layer foreground or background
  * @param  FB_Address: Layer frame buffer
  * @retval None
  */
void LCD_LayerRgb565Init(uint16_t LayerIndex, uint32_t FB_Address)
{     
  LTDC_LayerCfgTypeDef  layer_cfg;

  /* Layer Init */
  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = LCD_GetXSize();
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = LCD_GetYSize(); 
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  layer_cfg.FBStartAdress = FB_Address;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = LCD_GetXSize();
  layer_cfg.ImageHeight = LCD_GetYSize();
  
  HAL_LTDC_ConfigLayer(&Ltdc_Handler, &layer_cfg, LayerIndex); 

  DrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;
  DrawProp[LayerIndex].pFont     = &LCD_DEFAULT_FONT;
  DrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK; 
}
/**

  * @brief  Initializes  the LCD layer 

  * @param  LayerIndex:  Layer foreground or background

  * @param  FB_Address:  Layer frame buffer

  * @param  PixelFormat: Layer PixelFormat

  * @retval None

  */

void LCD_LayerInit(uint16_t LayerIndex, uint32_t FB_Address,uint32_t PixelFormat)

{     

  LTDC_LayerCfgTypeDef  layer_cfg;



  /* Layer Init */

  layer_cfg.WindowX0 = 0;

  layer_cfg.WindowX1 = LCD_GetXSize();

  layer_cfg.WindowY0 = 0;

  layer_cfg.WindowY1 = LCD_GetYSize(); 

  layer_cfg.PixelFormat = PixelFormat;

  layer_cfg.FBStartAdress = FB_Address;

  layer_cfg.Alpha = 255;

  layer_cfg.Alpha0 = 0;

  layer_cfg.Backcolor.Blue = 0;

  layer_cfg.Backcolor.Green = 0;

  layer_cfg.Backcolor.Red = 0;

  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;

  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

  layer_cfg.ImageWidth = LCD_GetXSize();

  layer_cfg.ImageHeight = LCD_GetYSize();

  

  HAL_LTDC_ConfigLayer(&Ltdc_Handler, &layer_cfg, LayerIndex); 



  DrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;

  DrawProp[LayerIndex].pFont     = &LCD_DEFAULT_FONT;

  DrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK; 

}
/**
  * @brief  Selects the LCD Layer.
  * @param  LayerIndex: Layer foreground or background
  * @retval None
  */
void LCD_SelectLayer(uint32_t LayerIndex)
{
  ActiveLayer = LayerIndex;
} 

/**
  * @brief  Sets an LCD Layer visible
  * @param  LayerIndex: Visible Layer
  * @param  State: New state of the specified layer
  *          This parameter can be one of the following values:
  *            @arg  ENABLE
  *            @arg  DISABLE 
  * @retval None
  */
void LCD_SetLayerVisible(uint32_t LayerIndex, FunctionalState State)
{
  if(State == ENABLE)
  {
    __HAL_LTDC_LAYER_ENABLE(&Ltdc_Handler, LayerIndex);
  }
  else
  {
    __HAL_LTDC_LAYER_DISABLE(&Ltdc_Handler, LayerIndex);
  }
  __HAL_LTDC_RELOAD_CONFIG(&Ltdc_Handler);
} 

/**
  * @brief  Configures the transparency.
  * @param  LayerIndex: Layer foreground or background.
  * @param  Transparency: Transparency
  *           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF 
  * @retval None
  */
void LCD_SetTransparency(uint32_t LayerIndex, uint8_t Transparency)
{    
  HAL_LTDC_SetAlpha(&Ltdc_Handler, Transparency, LayerIndex);
}

/**
  * @brief  Sets an LCD layer frame buffer address.
  * @param  LayerIndex: Layer foreground or background
  * @param  Address: New LCD frame buffer value      
  * @retval None
  */
void LCD_SetLayerAddress(uint32_t LayerIndex, uint32_t Address)
{
  HAL_LTDC_SetAddress(&Ltdc_Handler, Address, LayerIndex);
}

/**
  * @brief  Sets display window.
  * @param  LayerIndex: Layer index
  * @param  Xpos: LCD X position
  * @param  Ypos: LCD Y position
  * @param  Width: LCD window width
  * @param  Height: LCD window height  
  * @retval None
  */
void LCD_SetLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Reconfigure the layer size */
  HAL_LTDC_SetWindowSize(&Ltdc_Handler, Width, Height, LayerIndex);
  
  /* Reconfigure the layer position */
  HAL_LTDC_SetWindowPosition(&Ltdc_Handler, Xpos, Ypos, LayerIndex); 
}

/**
  * @brief  Configures and sets the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @param  RGBValue: Color reference
  * @retval None
  */
void LCD_SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue)
{  
  /* Configure and Enable the color Keying for LCD Layer */
  HAL_LTDC_ConfigColorKeying(&Ltdc_Handler, RGBValue, LayerIndex);
  HAL_LTDC_EnableColorKeying(&Ltdc_Handler, LayerIndex);
}

/**
  * @brief  Disables the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @retval None
  */
void LCD_ResetColorKeying(uint32_t LayerIndex)
{   
  /* Disable the color Keying for LCD Layer */
  HAL_LTDC_DisableColorKeying(&Ltdc_Handler, LayerIndex);
}

/**
  * @brief  Sets the LCD text color.
  * @param  Color: Text color code ARGB(8-8-8-8)
  * @retval None
  */
void LCD_SetTextColor(uint32_t Color)
{
  DrawProp[ActiveLayer].TextColor = Color;
}

/**
  * @brief  Gets the LCD text color.
  * @retval Used text color.
  */
uint32_t LCD_GetTextColor(void)
{
  return DrawProp[ActiveLayer].TextColor;
}

/**
  * @brief  Sets the LCD background color.
  * @param  Color: Layer background color code ARGB(8-8-8-8)
  * @retval None
  */
void LCD_SetBackColor(uint32_t Color)
{
  DrawProp[ActiveLayer].BackColor = Color;
}

/**
  * @brief  Gets the LCD background color.
  * @retval Used background colour
  */
uint32_t LCD_GetBackColor(void)
{
  return DrawProp[ActiveLayer].BackColor;
}

/**
 * @brief  Sets the LCD Text and Background colors.
 * @param  TextColor: specifies the Text Color.
 * @param  BackColor: specifies the Background Color.
 * @retval None
 */
void LCD_SetColors(uint32_t TextColor, uint32_t BackColor)
{
     LCD_SetTextColor (TextColor);
     LCD_SetBackColor (BackColor);
}
/**
  * @brief  Sets the LCD text font.
  * @param  fonts: Layer font to be used
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  DrawProp[ActiveLayer].pFont = fonts;
}

/**
  * @brief  Gets the LCD text font.
  * @retval Used layer font
  */
sFONT *LCD_GetFont(void)
{
  return DrawProp[ActiveLayer].pFont;
}

/**
  * @brief  Reads an LCD pixel.
  * @param  Xpos: X position 
  * @param  Ypos: Y position 
  * @retval RGB pixel color
  */
uint32_t LCD_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint32_t ret = 0;
  
  if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888)
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint32_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos)));
  }
  else if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB888)
  {
    /* Read data value from SDRAM memory */
    ret = (*(__IO uint32_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos))) & 0x00FFFFFF);
  }
  else if((Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) || \
          (Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) || \
          (Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_AL88))  
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint16_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos)));    
  }
  else
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint8_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos)));    
  }
  
  return ret;
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: Color of the background
  * @retval None
  */
void LCD_Clear(uint32_t Color)
{ 
  /* Clear the LCD */ 
  LL_FillBuffer(ActiveLayer, (uint32_t *)(Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress), LCD_GetXSize(), LCD_GetYSize(), 0, Color);
}

///**
//  * @brief  Clears the selected line.
//  * @param  Line: Line to be cleared
//  * @retval None
//  */
//void LCD_ClearLine(uint32_t Line)
//{
//  uint32_t color_backup = DrawProp[ActiveLayer].TextColor;
//  DrawProp[ActiveLayer].TextColor = DrawProp[ActiveLayer].BackColor;
//  
//  /* Draw rectangle with background color */
//  LCD_FillRect(0, (Line * DrawProp[ActiveLayer].pFont->Height), LCD_GetXSize(), DrawProp[ActiveLayer].pFont->Height);
//  
//  DrawProp[ActiveLayer].TextColor = color_backup;
//  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);  
//}
/**
 * @brief  Clears the selected line.
 * @param  Line: the Line to be cleared.
 *   This parameter can be one of the following values:
 *     @arg LCD_LINE_x: where x can be: 0..13 if LCD_Currentfonts is Font16x24
 *                                      0..26 if LCD_Currentfonts is Font12x12 or Font8x12
 *                                      0..39 if LCD_Currentfonts is Font8x8
 * @retval None
 */
void LCD_ClearLine(uint32_t Line)
{
 uint16_t refcolumn = 0;
 /* Send the string character by character on lCD */
 while ((refcolumn < LCD_PIXEL_WIDTH) && (((refcolumn + DrawProp[ActiveLayer].pFont->Width)& 0xFFFF) >= DrawProp[ActiveLayer].pFont->Width))
 {
   /* Display one character on LCD */
   LCD_DisplayChar( refcolumn,LINE(Line), ' ');
   /* Decrement the column position by 16 */
   refcolumn += DrawProp[ActiveLayer].pFont->Width;
 }
}
/**
  * @brief  Displays one character.
  * @param  Xpos: Start column address
  * @param  Ypos: Line where to display the character shape.
  * @param  Ascii: Character ascii code
  *           This parameter must be a number between Min_Data = 0x20 and Max_Data = 0x7E 
  * @retval None
  */
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
    DrawChar(Xpos, Ypos, &DrawProp[ActiveLayer].pFont->table[(Ascii-' ') *\
    DrawProp[ActiveLayer].pFont->Height * ((DrawProp[ActiveLayer].pFont->Width + 7) / 8)]);
}

/**
  * @brief  Displays characters on the LCD.
  * @param  Xpos: X position (in pixel)
  * @param  Ypos: Y position (in pixel)   
  * @param  Text: Pointer to string to display on LCD
  * @param  Mode: Display mode
  *          This parameter can be one of the following values:
  *            @arg  CENTER_MODE
  *            @arg  RIGHT_MODE
  *            @arg  LEFT_MODE   
  * @retval None
  */
void LCD_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode)
{
  uint16_t ref_column = 1, i = 0;
  uint32_t size = 0, xsize = 0; 
  uint8_t  *ptr = Text;
  
  /* Get the text size */
  while (*ptr++) size ++ ;
  
  /* Characters number per line */
  xsize = (LCD_GetXSize()/DrawProp[ActiveLayer].pFont->Width);
  
  switch (Mode)
  {
  case CENTER_MODE:
    {
      ref_column = Xpos + ((xsize - size)* DrawProp[ActiveLayer].pFont->Width) / 2;
      break;
    }
  case LEFT_MODE:
    {
      ref_column = Xpos;
      break;
    }
  case RIGHT_MODE:
    {
      ref_column = - Xpos + ((xsize - size)*DrawProp[ActiveLayer].pFont->Width);
      break;
    }    
  default:
    {
      ref_column = Xpos;
      break;
    }
  }
  
  /* Check that the Start column is located in the screen */
  if ((ref_column < 1) || (ref_column >= 0x8000))
  {
    ref_column = 1;
  }

  /* Send the string character by character on LCD */
  while ((*Text != 0) & (((LCD_GetXSize() - (i*DrawProp[ActiveLayer].pFont->Width)) & 0xFFFF) >= DrawProp[ActiveLayer].pFont->Width))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(ref_column, Ypos, *Text);
    /* Decrement the column position by 16 */
    ref_column += DrawProp[ActiveLayer].pFont->Width;
    /* Point on the next character */
    Text++;
    i++;
  }  
}

/**
  * @brief  Displays a maximum of 60 characters on the LCD.
  * @param  Line: Line where to display the character shape
  * @param  ptr: Pointer to string to display on LCD
  * @retval None
  */
void LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr)
{  
  LCD_DisplayStringAt(0, LINE(Line), ptr, LEFT_MODE);
}
/**
 * @brief  在显示器上显示一个中文字符
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下字符的起始Y坐标
 * @param  usChar ：要显示的中文字符（国标码）
 * @retval 无
 */ 
void LCD_DispChar_CH ( uint16_t usX, uint16_t usY, uint16_t usChar)
{
    uint8_t ucPage, ucColumn;
    uint8_t ucBuffer [ 24*24/8 ];	

    uint32_t usTemp; 	


    uint32_t  xpos =0;
    uint32_t  Xaddress = 0;
    if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
    { /* RGB565 format */
        /*xpos表示当前行的显存偏移位置*/
        xpos = usX*LCD_PIXEL_WIDTH*2;
    }
    else
    { /* ARGB8888 format */
        /*xpos表示当前行的显存偏移位置*/
        xpos = usX*LCD_PIXEL_WIDTH*4;
    }


    /*Xaddress表示像素点*/
    Xaddress += usY;

    macGetGBKCode ( ucBuffer, usChar );	//取字模数据

    /*ucPage表示当前行数*/
    for ( ucPage = 0; ucPage < macHEIGHT_CH_CHAR; ucPage ++ )
    {
        /* 取出3个字节的数据，在lcd上即是一个汉字的一行 */
        usTemp = ucBuffer [ ucPage * 3 ];
        usTemp = ( usTemp << 8 );
        usTemp |= ucBuffer [ ucPage * 3 + 1 ];
        usTemp = ( usTemp << 8 );
        usTemp |= ucBuffer [ ucPage * 3 + 2];


        for ( ucColumn = 0; ucColumn < macWIDTH_CH_CHAR; ucColumn ++ ) 
        {			
            if ( usTemp & ( 0x01 << 23 ) )  //高位在前 				
            {         
              if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
              { /* RGB565 format */
                //字体色
                /* Write data value to all SDRAM memory */
                *(__IO uint16_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*Xaddress) + xpos) = DrawProp[ActiveLayer].TextColor;
              }
              else
              { /* ARGB8888 format */
                //字体色
                /* Write data value to all SDRAM memory */
                *(__IO uint32_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (4*Xaddress) + xpos) = DrawProp[ActiveLayer].TextColor;
              }

            }				
            else	
            {
                //背景色
              /* Write data value to all SDRAM memory */
             if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
              { /* RGB565 format */
                //字体色
                /* Write data value to all SDRAM memory */
                *(__IO uint16_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*Xaddress) + xpos) = DrawProp[ActiveLayer].BackColor;
              }
              else
              { /* ARGB8888 format */
                //字体色
                /* Write data value to all SDRAM memory */
                *(__IO uint32_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (4*Xaddress) + xpos) = DrawProp[ActiveLayer].BackColor;
              }
            }	
            /*指向当前行的下一个点*/	
            Xaddress++;			
            usTemp <<= 1;
            
        }
        /*显示完一行*/
        /*指向字符显示矩阵下一行的第一个像素点*/
        Xaddress += (LCD_PIXEL_WIDTH - macWIDTH_CH_CHAR);

    }
}
/**
  * @brief  显示一行字符，若超出液晶宽度，不自动换行。
						中英混显时，请把英文字体设置为Font16x24格式
  * @param  Line: 要显示的行编号LINE(0) - LINE(N)
  * @param  *ptr: 要显示的字符串指针
  * @retval None
  */
void LCD_DisplayStringLine_EN_CH(uint16_t Line, uint8_t *ptr)
{  
  uint16_t refcolumn = 0;
  /* 判断显示位置不能超出液晶的边界 */
  while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & (((refcolumn + DrawProp[ActiveLayer].pFont->Width) & 0xFFFF) >= DrawProp[ActiveLayer].pFont->Width)))
  {
    /* 使用LCD显示一个字符 */
		if ( * ptr <= 126 )	           	//英文字符
		{
					
			LCD_DisplayChar(refcolumn, LINE(Line), *ptr);
    /* 根据字体偏移显示的位置 */
			refcolumn += DrawProp[ActiveLayer].pFont->Width;
    /* 指向字符串中的下一个字符 */
			ptr++;
		}
		
		else	                            //汉字字符
		{	
			uint16_t usCh;
			
			/*一个汉字两字节*/
			usCh = * ( uint16_t * ) ptr;	
			/*交换编码顺序*/
			usCh = ( usCh << 8 ) + ( usCh >> 8 );		
			
			/*显示汉字*/
			LCD_DispChar_CH ( LINE(Line), refcolumn, usCh );
			/*显示位置偏移*/
			refcolumn += macWIDTH_CH_CHAR;
      /* 指向字符串中的下一个字符 */
			ptr += 2; 		
    }		

		

  }
}
/**
  * @brief  Draws an horizontal line.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Length: Line length
  * @retval None
  */
void LCD_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  uint32_t  Xaddress = 0;
  
  /* Get the line address */
  if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
  { /* RGB565 format */
    Xaddress = (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress) + 2*(LCD_GetXSize()*Ypos + Xpos);
  }
  else
  { /* ARGB8888 format */
    Xaddress = (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress) + 4*(LCD_GetXSize()*Ypos + Xpos);
  }
  
  /* Write line */
  LL_FillBuffer(ActiveLayer, (uint32_t *)Xaddress, Length, 1, 0, DrawProp[ActiveLayer].TextColor);
}

/**
  * @brief  Draws a vertical line.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Length: Line length
  * @retval None
  */
void LCD_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  uint32_t  Xaddress = 0;
  
  /* Get the line address */
  if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
  { /* RGB565 format */
    Xaddress = (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress) + 2*(LCD_GetXSize()*Ypos + Xpos);
  }
  else
  { /* ARGB8888 format */
    Xaddress = (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress) + 4*(LCD_GetXSize()*Ypos + Xpos);
  }
  
  /* Write line */
  LL_FillBuffer(ActiveLayer, (uint32_t *)Xaddress, 1, Length, (LCD_GetXSize() - 1), DrawProp[ActiveLayer].TextColor);
}

/**
  * @brief  Draws an uni-line (between two points).
  * @param  x1: Point 1 X position
  * @param  y1: Point 1 Y position
  * @param  x2: Point 2 X position
  * @param  y2: Point 2 Y position
  * @retval None
  */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, num_add = 0, num_pixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    num_add = deltay;
    num_pixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    num_add = deltax;
    num_pixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= num_pixels; curpixel++)
  {
    LCD_DrawPixel(x, y, DrawProp[ActiveLayer].TextColor);   /* Draw the current pixel */
    num += num_add;                            /* Increase the numerator by the top of the fraction */
    if (num >= den)                           /* Check if numerator >= denominator */
    {
      num -= den;                             /* Calculate the new numerator value */
      x += xinc1;                             /* Change the x as appropriate */
      y += yinc1;                             /* Change the y as appropriate */
    }
    x += xinc2;                               /* Change the x as appropriate */
    y += yinc2;                               /* Change the y as appropriate */
  }
}

/**
  * @brief  Draws a rectangle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Width: Rectangle width  
  * @param  Height: Rectangle height
  * @retval None
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Draw horizontal lines */
  LCD_DrawHLine(Xpos, Ypos, Width);
  LCD_DrawHLine(Xpos, (Ypos+ Height), Width);
  
  /* Draw vertical lines */
  LCD_DrawVLine(Xpos, Ypos, Height);
  LCD_DrawVLine((Xpos + Width), Ypos, Height);
}

/**
  * @brief  Draws a circle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Radius: Circle radius
  * @retval None
  */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t   decision;    /* Decision Variable */ 
  uint32_t  current_x;   /* Current X Value */
  uint32_t  current_y;   /* Current Y Value */
  
  decision = 3 - (Radius << 1);
  current_x = 0;
  current_y = Radius;
  
  while (current_x <= current_y)
  {
    LCD_DrawPixel((Xpos + current_x), (Ypos - current_y), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos - current_x), (Ypos - current_y), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos + current_y), (Ypos - current_x), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos - current_y), (Ypos - current_x), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos + current_x), (Ypos + current_y), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos - current_x), (Ypos + current_y), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos + current_y), (Ypos + current_x), DrawProp[ActiveLayer].TextColor);
    
    LCD_DrawPixel((Xpos - current_y), (Ypos + current_x), DrawProp[ActiveLayer].TextColor);
    
    if (decision < 0)
    { 
      decision += (current_x << 2) + 6;
    }
    else
    {
      decision += ((current_x - current_y) << 2) + 10;
      current_y--;
    }
    current_x++;
  } 
}

/**
  * @brief  Draws an poly-line (between many points).
  * @param  Points: Pointer to the points array
  * @param  PointCount: Number of points
  * @retval None
  */
void LCD_DrawPolygon(pPoint Points, uint16_t PointCount)
{
  int16_t x = 0, y = 0;
  
  if(PointCount < 2)
  {
    return;
  }
  
  LCD_DrawLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
  
  while(--PointCount)
  {
    x = Points->X;
    y = Points->Y;
    Points++;
    LCD_DrawLine(x, y, Points->X, Points->Y);
  }
}

/**
  * @brief  Draws an ellipse on LCD.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  XRadius: Ellipse X radius
  * @param  YRadius: Ellipse Y radius
  * @retval None
  */
void LCD_DrawEllipse(int Xpos, int Ypos, int XRadius, int YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float k = 0, rad1 = 0, rad2 = 0;
  
  rad1 = XRadius;
  rad2 = YRadius;
  
  k = (float)(rad2/rad1);  
  
  do { 
    LCD_DrawPixel((Xpos-(uint16_t)(x/k)), (Ypos+y), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos+(uint16_t)(x/k)), (Ypos+y), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos+(uint16_t)(x/k)), (Ypos-y), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos-(uint16_t)(x/k)), (Ypos-y), DrawProp[ActiveLayer].TextColor);      
    
    e2 = err;
    if (e2 <= x) {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;     
  }
  while (y <= 0);
}

/**
  * @brief  Draws a pixel on LCD.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  RGB_Code: Pixel color in ARGB mode (8-8-8-8)
  * @retval None
  */
void LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
  /* Write data value to all SDRAM memory */
  if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
  { /* RGB565 format */
    *(__IO uint16_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos))) = (uint16_t)RGB_Code;
  }
  else
  { /* ARGB8888 format */
    *(__IO uint32_t*) (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (4*(Ypos*LCD_GetXSize() + Xpos))) = RGB_Code;
  }
}

/**
  * @brief  Draws a bitmap picture loaded in the internal Flash in ARGB888 format (32 bits per pixel).
  * @param  Xpos: Bmp X position in the LCD
  * @param  Ypos: Bmp Y position in the LCD
  * @param  pbmp: Pointer to Bmp picture address in the internal Flash
  * @retval None
  */
void LCD_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
  uint32_t address;
  uint32_t input_color_mode = 0;
  
  /* Get bitmap data address offset */
  index = *(__IO uint16_t *) (pbmp + 10);
  index |= (*(__IO uint16_t *) (pbmp + 12)) << 16;
  
  /* Read bitmap width */
  width = *(uint16_t *) (pbmp + 18);
  width |= (*(uint16_t *) (pbmp + 20)) << 16;
  
  /* Read bitmap height */
  height = *(uint16_t *) (pbmp + 22);
  height |= (*(uint16_t *) (pbmp + 24)) << 16; 
  
  /* Read bit/pixel */
  bit_pixel = *(uint16_t *) (pbmp + 28);   
  
  /* Set the address */
  address = Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress + (((LCD_GetXSize()*Ypos) + Xpos)*(4));
  
  /* Get the layer pixel format */    
  if ((bit_pixel/8) == 4)
  {
    input_color_mode = CM_ARGB8888;
  }
  else if ((bit_pixel/8) == 2)
  {
    input_color_mode = CM_RGB565;   
  }
  else 
  {
    input_color_mode = CM_RGB888;
  }
  
  /* Bypass the bitmap header */
  pbmp += (index + (width * (height - 1) * (bit_pixel/8)));  
  
  /* Convert picture to ARGB8888 pixel format */
  for(index=0; index < height; index++)
  {
    /* Pixel format conversion */
    LL_ConvertLineToARGB8888((uint32_t *)pbmp, (uint32_t *)address, width, input_color_mode);
    
    /* Increment the source and destination buffers */
    address+=  (LCD_GetXSize()*4);
    pbmp -= width*(bit_pixel/8);
  } 
}

/**
  * @brief  Draws a full rectangle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Width: Rectangle width  
  * @param  Height: Rectangle height
  * @retval None
  */
void LCD_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  uint32_t  x_address = 0;
  
  /* Set the text color */
  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);
  
  /* Get the rectangle start address */
  if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
  { /* RGB565 format */
    x_address = (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress) + 2*(LCD_GetXSize()*Ypos + Xpos);
  }
  else
  { /* ARGB8888 format */
    x_address = (Ltdc_Handler.LayerCfg[ActiveLayer].FBStartAdress) + 4*(LCD_GetXSize()*Ypos + Xpos);
  }
  /* Fill the rectangle */
  LL_FillBuffer(ActiveLayer, (uint32_t *)x_address, Width, Height, (LCD_GetXSize() - Width), DrawProp[ActiveLayer].TextColor);
}

/**
  * @brief  Draws a full circle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Radius: Circle radius
  * @retval None
  */
void LCD_FillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  decision;     /* Decision Variable */ 
  uint32_t  current_x;   /* Current X Value */
  uint32_t  current_y;   /* Current Y Value */
  
  decision = 3 - (Radius << 1);
  
  current_x = 0;
  current_y = Radius;
  
  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);
  
  while (current_x <= current_y)
  {
    if(current_y > 0) 
    {
      LCD_DrawHLine(Xpos - current_y, Ypos + current_x, 2*current_y);
      LCD_DrawHLine(Xpos - current_y, Ypos - current_x, 2*current_y);
    }
    
    if(current_x > 0) 
    {
      LCD_DrawHLine(Xpos - current_x, Ypos - current_y, 2*current_x);
      LCD_DrawHLine(Xpos - current_x, Ypos + current_y, 2*current_x);
    }
    if (decision < 0)
    { 
      decision += (current_x << 2) + 6;
    }
    else
    {
      decision += ((current_x - current_y) << 2) + 10;
      current_y--;
    }
    current_x++;
  }
  
  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);
  LCD_DrawCircle(Xpos, Ypos, Radius);
}

/**
  * @brief  Draws a full poly-line (between many points).
  * @param  Points: Pointer to the points array
  * @param  PointCount: Number of points
  * @retval None
  */
void LCD_FillPolygon(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0, X2 = 0, Y2 = 0, X_center = 0, Y_center = 0, X_first = 0, Y_first = 0, pixelX = 0, pixelY = 0, counter = 0;
  uint16_t  image_left = 0, image_right = 0, image_top = 0, image_bottom = 0;
  
  image_left = image_right = Points->X;
  image_top= image_bottom = Points->Y;
  
  for(counter = 1; counter < PointCount; counter++)
  {
    pixelX = POLY_X(counter);
    if(pixelX < image_left)
    {
      image_left = pixelX;
    }
    if(pixelX > image_right)
    {
      image_right = pixelX;
    }
    
    pixelY = POLY_Y(counter);
    if(pixelY < image_top)
    { 
      image_top = pixelY;
    }
    if(pixelY > image_bottom)
    {
      image_bottom = pixelY;
    }
  }  
  
  if(PointCount < 2)
  {
    return;
  }
  
  X_center = (image_left + image_right)/2;
  Y_center = (image_bottom + image_top)/2;
  
  X_first = Points->X;
  Y_first = Points->Y;
  
  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    X2 = Points->X;
    Y2 = Points->Y;    
    
    FillTriangle(X, X2, X_center, Y, Y2, Y_center);
    FillTriangle(X, X_center, X2, Y, Y_center, Y2);
    FillTriangle(X_center, X2, X, Y_center, Y2, Y);   
  }
  
  FillTriangle(X_first, X2, X_center, Y_first, Y2, Y_center);
  FillTriangle(X_first, X_center, X2, Y_first, Y_center, Y2);
  FillTriangle(X_center, X2, X_first, Y_center, Y2, Y_first);   
}

/**
  * @brief  Draws a full ellipse.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  XRadius: Ellipse X radius
  * @param  YRadius: Ellipse Y radius  
  * @retval None
  */
void LCD_FillEllipse(int Xpos, int Ypos, int XRadius, int YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float k = 0, rad1 = 0, rad2 = 0;
  
  rad1 = XRadius;
  rad2 = YRadius;
  
  k = (float)(rad2/rad1);
  
  do 
  {       
    LCD_DrawHLine((Xpos-(uint16_t)(x/k)), (Ypos+y), (2*(uint16_t)(x/k) + 1));
    LCD_DrawHLine((Xpos-(uint16_t)(x/k)), (Ypos-y), (2*(uint16_t)(x/k) + 1));
    
    e2 = err;
    if (e2 <= x) 
    {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;
  }
  while (y <= 0);
}

/**
  * @brief  Enables the display.
  * @retval None
  */
void LCD_DisplayOn(void)
{
  /* Display On */
  __HAL_LTDC_ENABLE(&Ltdc_Handler);
  HAL_GPIO_WritePin(LTDC_DISP_GPIO_PORT, LTDC_DISP_GPIO_PIN, GPIO_PIN_SET);        /* Assert LCD_DISP pin */
  HAL_GPIO_WritePin(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN, GPIO_PIN_SET);  /* Assert LCD_BL_CTRL pin */
}

/**
  * @brief  Disables the display.
  * @retval None
  */
void LCD_DisplayOff(void)
{
  /* Display Off */
  __HAL_LTDC_DISABLE(&Ltdc_Handler);
  HAL_GPIO_WritePin(LTDC_DISP_GPIO_PORT, LTDC_DISP_GPIO_PIN, GPIO_PIN_RESET);      /* De-assert LCD_DISP pin */
  HAL_GPIO_WritePin(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN, GPIO_PIN_RESET);/* De-assert LCD_BL_CTRL pin */
}

/**
  * @brief  Clock Config.
  * @param  hltdc: LTDC handle
  * @param  Params
  * @note   This API is called by LCD_Init()
  *         Being __weak it can be overwritten by the application
  * @retval None
  */
void LCD_ClockConfig(LTDC_HandleTypeDef *hltdc, void *Params)
{
  static RCC_PeriphCLKInitTypeDef  periph_clk_init_struct;

  /* RK043FN48H LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/5 = 38.4 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6Mhz */
  periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  periph_clk_init_struct.PLLSAI.PLLSAIN = 192;
  periph_clk_init_struct.PLLSAI.PLLSAIR = 5;
  periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);
}


/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: Line where to display the character shape
  * @param  Ypos: Start column address
  * @param  c: Pointer to the character data
  * @retval None
  */
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c)
{
  uint32_t i = 0, j = 0;
  uint16_t height, width;
  uint8_t  offset;
  uint8_t  *pchar;
  uint32_t line;
  
  height = DrawProp[ActiveLayer].pFont->Height;
  width  = DrawProp[ActiveLayer].pFont->Width;
  
  offset =  8 *((width + 7)/8) -  width ;
  
  for(i = 0; i < height; i++)
  {
    pchar = ((uint8_t *)c + (width + 7)/8 * i);
    
    switch(((width + 7)/8))
    {
      
    case 1:
      line =  pchar[0];      
      break;
      
    case 2:
      line =  (pchar[0]<< 8) | pchar[1];      
      break;
      
    case 3:
    default:
      line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];      
      break;
    } 
    
    for (j = 0; j < width; j++)
    {
      if(line & (1 << (width- j + offset- 1))) 
      {
        LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].TextColor);
      }
      else
      {
        LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].BackColor);
      } 
    }
    Ypos++;
  }
}

/**
  * @brief  Fills a triangle (between 3 points).
  * @param  x1: Point 1 X position
  * @param  y1: Point 1 Y position
  * @param  x2: Point 2 X position
  * @param  y2: Point 2 Y position
  * @param  x3: Point 3 X position
  * @param  y3: Point 3 Y position
  * @retval None
  */
static void FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3)
{ 
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, num_add = 0, num_pixels = 0,
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    num_add = deltay;
    num_pixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    num_add = deltax;
    num_pixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= num_pixels; curpixel++)
  {
    LCD_DrawLine(x, y, x3, y3);
    
    num += num_add;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  } 
}

/**
  * @brief  Fills a buffer.
  * @param  LayerIndex: Layer index
  * @param  pDst: Pointer to destination buffer
  * @param  xSize: Buffer width
  * @param  ySize: Buffer height
  * @param  OffLine: Offset
  * @param  ColorIndex: Color index
  * @retval None
  */
static void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) 
{
  /* Register to memory mode with ARGB8888 as color Mode */ 
  Dma2d_Handler.Init.Mode         = DMA2D_R2M;
  if(Ltdc_Handler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565)
  { /* RGB565 format */ 
    Dma2d_Handler.Init.ColorMode    = DMA2D_RGB565;
  }
  else
  { /* ARGB8888 format */
    Dma2d_Handler.Init.ColorMode    = DMA2D_ARGB8888;
  }
  Dma2d_Handler.Init.OutputOffset = OffLine;      
  
  Dma2d_Handler.Instance = DMA2D;
  
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&Dma2d_Handler) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&Dma2d_Handler, LayerIndex) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&Dma2d_Handler, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&Dma2d_Handler, 10);
      }
    }
  } 
}

/**
  * @brief  Converts a line to an ARGB8888 pixel format.
  * @param  pSrc: Pointer to source buffer
  * @param  pDst: Output color
  * @param  xSize: Buffer width
  * @param  ColorMode: Input color mode   
  * @retval None
  */
static void LL_ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
{    
  /* Configure the DMA2D Mode, Color Mode and output offset */
  Dma2d_Handler.Init.Mode         = DMA2D_M2M_PFC;
  Dma2d_Handler.Init.ColorMode    = DMA2D_ARGB8888;
  Dma2d_Handler.Init.OutputOffset = 0;     
  
  /* Foreground Configuration */
  Dma2d_Handler.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  Dma2d_Handler.LayerCfg[1].InputAlpha = 0xFF;
  Dma2d_Handler.LayerCfg[1].InputColorMode = ColorMode;
  Dma2d_Handler.LayerCfg[1].InputOffset = 0;
  
  Dma2d_Handler.Instance = DMA2D; 
  
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&Dma2d_Handler) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&Dma2d_Handler, 1) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&Dma2d_Handler, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&Dma2d_Handler, 10);
      }
    }
  } 
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
