#ifndef __proess_H
#define __proess_H
#include "stm32f7xx.h"
#include "./lcd/bsp_lcd.h"
#include "./camera/bsp_ov2640.h"

#define GETR_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0xf800)>>8))		  			//����8λ R
#define GETG_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0x07c0)>>3))	         //����8λ G
#define GETB_FROM_RGB16(RGB_565)  ((uint8_t)(((uint16_t)RGB_565&0x001f)<<3))      	//����8λ B
#define THR 0x000F		//��ֵ����ֵ
#define MAX_LINE 500	//��������󳤶�
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
extern uint16_t image2[480][800] ;				//����ͼ�����ݶ�����SDRAM��
extern uint16_t imgn[480][800]  ;

//��ʽת��
uint16_t RGB565_to_Gray(uint16_t rgb_565);
uint8_t GET8Gray(uint16_t rgb_565);
uint32_t rgb565_2_rgb888(uint16_t RGB_565);
uint16_t rgb888_2_rgb565(uint32_t rgb888);

//ͼ��ƽ��
void KNNFilterOper(void);                           //K���ھ�ֵ�˲�(kѡ5)             
void maopao(uint16_t a[],uint16_t n);                
void MedianFilterOper(void);                        //��ֵ�˲�
void MeanFilterOper(void);                        //��Ȩ��ֵ�˲�

//ͼ����ǿ
void Hist_plane(void);                //ֱ��ͼ���⻯
                        
                             
//��Ե��ȡ
void log_process(uint16_t thre);                      //log���
void canny_process(uint16_t thre);                    //canny���


//��ֵ��
uint16_t OET(void);                               //�����
uint16_t otsuThreshold(void);                     //�����䷽��
void get_newimage(uint16_t thre);
                                                  //����RGB��ɫͼ�ָ�

//��̬ѧ����
void Erodation();                                  //��ʴ
void Dilation();                                 //����
void close();                                     
void open();

//������
uint16_t mylabel(void);
void labelset(uint16_t xs,uint16_t ys,uint16_t label);
void label_extract(uint16_t cnt,uint16_t TH);
	
//������ȡ
uint16_t calc_size(uint16_t label,uint16_t *cx,uint16_t *cy);  //�������������
float calc_length(uint16_t label); //�����ܳ�
float feature(uint16_t label,uint16_t center_x,uint16_t center_y);  //����Բ�ζ�
float trace(uint16_t xs,uint16_t ys);  //����Ǯ׷��
void find_rough(uint16_t first_x,uint16_t first_y);  //�߽���ٷ�
void size_extract(float size,float size_min,float size_max,uint16_t label);   //���������ȡ����
void ratio_extract(float ratio,float ratio_min,float ratio_max,uint16_t label);  //����Բ�ζȳ�ȡ����

//����任
void HoughTrans(void);
void DoHough(void);
void GetPixelValueEx(uint16_t *value, uint16_t i, uint16_t j);

//�������
void point(uint16_t a[4]);   //��С��Ӿ���
void point2(uint16_t a[4]);
float distance(float r,float f,uint16_t Object_W,uint16_t MAXCOL,uint16_t MINCOL);

//�ǶȲ���
void maopao(uint16_t a[],uint16_t n);
float diagonal(uint16_t a[4],float m,float n);


//������ѧ����
double my_abs(double a);
double fun(double x, uint16_t n);
uint16_t Max(uint16_t a[],uint16_t N);
uint16_t Min(uint16_t a[],uint16_t N);
#endif 
