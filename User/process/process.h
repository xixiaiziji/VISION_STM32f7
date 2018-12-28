#ifndef __proess_H
#define __proess_H
#include "stm32f7xx.h"
#include "./lcd/bsp_lcd.h"
#include "./camera/bsp_ov2640.h"

#define GETR_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0xf800)>>8))		  			//返回8位 R
#define GETG_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0x07c0)>>3))	         //返回8位 G
#define GETB_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0x001f)<<3))      	//返回8位 B
#define THR 0x000F		//二值化阈值
#define MAX_LINE 500	//轮廓线最大长度
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
extern uint16_t image2[480][800] ;				//轮廓图像数据定义在SDRAM中
extern uint16_t imgn[480][800]  ;

//格式转换
uint16_t RGB565_to_Gray(uint16_t rgb_565);
uint8_t GET8Gray(uint16_t rgb_565);
uint32_t rgb565_2_rgb888(uint16_t RGB_565);
uint16_t rgb888_2_rgb565(uint32_t rgb888);

//图像平滑
void KNNFilterOper(void);                           //K近邻均值滤波(k选5)             
void maopao(uint16_t a[],uint16_t n);                
void MedianFilterOper(void);                        //中值滤波
void MeanFilterOper(void);                        //加权均值滤波

//图像增强
void Hist_plane(void);                //直方图均衡化
                        
                             
//边缘提取
void log_process(uint16_t thre);                      //log检测
void canny_process(uint16_t thre);                    //canny检测


//阈值化
uint16_t OET(void);                               //最大熵
uint16_t otsuThreshold(void);                     //最大类间方差
void get_newimage(uint16_t thre);
                                                  //基于RGB彩色图分割

//形态学处理
void Erodation();                                  //腐蚀
void Dilation();                                 //膨胀
void close();                                     
void open();

//区域标记
uint16_t mylabel(void);
void labelset(uint16_t xs,uint16_t ys,uint16_t label);
void label_extract(uint16_t cnt,uint16_t TH);
	
//特征提取
uint16_t calc_size(uint16_t label,uint16_t *cx,uint16_t *cy);  //区域面积与重心
float calc_length(uint16_t label); //区域周长
float feature(uint16_t label,uint16_t center_x,uint16_t center_y);  //区域圆形度
float trace(uint16_t xs,uint16_t ys);  //轮廓钱追踪
void find_rough(uint16_t first_x,uint16_t first_y);  //边界跟踪法
void size_extract(float size,float size_min,float size_max,uint16_t label);   //根据面积抽取对象
void ratio_extract(float ratio,float ratio_min,float ratio_max,uint16_t label);  //根据圆形度抽取对象

//哈夫变换
void HoughTrans(void);
void DoHough(void);
void GetPixelValueEx(uint16_t *value, uint16_t i, uint16_t j);

//距离测量
void point(uint16_t a[4]);   //最小外接矩形
void point2(uint16_t a[4]);
float distance(float r,float f,uint16_t Object_W,uint16_t MAXCOL,uint16_t MINCOL);

//角度测量
void maopao(uint16_t a[],uint16_t n);
float diagonal(uint16_t a[4],float m,float n);


//基本数学运算
double my_abs(double a);
double fun(double x, uint16_t n);
uint16_t Max(uint16_t a[],uint16_t N);
uint16_t Min(uint16_t a[],uint16_t N);
#endif 
