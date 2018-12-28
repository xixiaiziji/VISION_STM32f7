#ifndef __process_H
#define __process_H

/*include--------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "./lcd/bsp_lcd.h"
#include "./camera/bsp_ov2640.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#define GETR_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0xf800)>>8))		  			//返回8位 R
#define GETG_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0x07c0)>>3))	         //返回8位 G
#define GETB_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0x001f)<<3))      	//返回8位 B
#define THR 0x000F		//threshold
#define MAX_LINE 500	//maximum length of contour
#define PI 3.1415926
#define SAMPLENUMBER 512
#define e 2.71828

#define H_top 20
#define H_end 20
#define W_top 200
#define W_end 200

typedef struct
{
	uint16_t x;
	uint16_t y;
}rough_t;
typedef struct Contours  
{  
  int height;  
  int width;  
  int stateFlag;  
} contour;  
extern rough_t rough[MAX_LINE];
extern uint16_t Rough_length,flag_canny;
extern uint16_t image2[480][800] ;			
extern uint16_t imgn[480][800]  ;

//format conversion function 
uint16_t RGB565_to_Gray(uint16_t rgb_565);
uint8_t GET8Gray(uint16_t rgb_565);
uint32_t rgb565_2_rgb888(uint16_t RGB_565);
uint16_t rgb888_2_rgb565(uint32_t rgb888);

//functions of image denoising 
void KNNFilterOper(void);                           //KNN mean filter             
void maopao(uint16_t a[],uint16_t n);                
void MedianFilterOper(void);                        //median filter
void MeanFilterOper(void);                         //weighted mean filter

//functions of image enhancement
void Hist_plane(void);                             
                        
                             
//functions of image edge extraction  
void log_process(uint16_t thre);                      //log
void canny_process(uint16_t thre);                    //canny


//function of image thresholding  
uint16_t OET(void);                               //maximum entropy
uint16_t otsuThreshold(void);                     
void get_newimage(uint16_t thre);
                                                  

//functions of image morphological processsing 
void Erodation();                                  
void Dilation();                                   
void close();                                     
void open();

//functions of region labeling
uint16_t mylabel(void);
void labelset(uint16_t xs,uint16_t ys,uint16_t label);
void label_extract(uint16_t cnt,uint16_t TH);
	
//functions of feature extraction
uint16_t calc_size(uint16_t label,uint16_t *cx,uint16_t *cy);       
float calc_length(uint16_t label);                                  
float feature(uint16_t label,uint16_t center_x,uint16_t center_y);  
float trace(uint16_t xs,uint16_t ys);                               
void find_rough(uint16_t first_x,uint16_t first_y);                  
void size_extract(float size,float size_min,float size_max,uint16_t label);  
void ratio_extract(float ratio,float ratio_min,float ratio_max,uint16_t label);  

//Hough transformation
void HoughTrans(void);
void DoHough(void);
void GetPixelValueEx(uint16_t *value, uint16_t i, uint16_t j);

//distance measurement
void point(uint16_t a[4]);   //minimum enclosing rectangle
void point2(uint16_t a[4]);
float distance(float r,float f,uint16_t Object_W,uint16_t MAXCOL,uint16_t MINCOL);

//angle measurement
void maopao(uint16_t a[],uint16_t n);
float diagonal(uint16_t a[4],float m,float n);


//elementary operation
double my_abs(double a);
double fun(double x, uint16_t n);
uint16_t Max(uint16_t a[],uint16_t N);
uint16_t Min(uint16_t a[],uint16_t N);
#endif 
