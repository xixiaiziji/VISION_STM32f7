/**
  ******************************************************************************
  * @file    process.c
  * @author  ZHANG
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   ��Ե��ͼ����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F767 ������  
  ******************************************************************************
  */

#include "./process/process.h"
#include "math.h"
rough_t rough[MAX_LINE];
static uint16_t label;                      //��ͨ����
uint16_t Rough_length=0,flag_canny=0;
uint16_t image2[480][800] __attribute__((at(IMAGE_FRAME_BUFFER)));				//����ͼ�����ݶ�����SDRAM��
uint16_t imgn[480][800]  __attribute__((at(IMGN_FRAME_BUFFER)));
uint16_t m[480][800]  __attribute__((at((uint32_t)0xD0500000)));
uint16_t o[480][800]  __attribute__((at((uint32_t)0xD0600000)));
uint16_t counters[933][180] __attribute__((at((uint32_t)0xD0700000))); 
uint16_t Hough[4][9] = {
{-1, 0, 1,
-1, 0, 1,
-1, 0, 1},
{-1, -1, 0,
-1, 0, 1,
0, 1, 1},
{-1, -1, -1,
0, 0, 0,
1, 1, 1},
{0, -1, -1,
1, 0, -1,
1, 1, 0}
}; 
/*-------------------------------
			  y
    5   6   7
			  |
	  4 -- -- 0	x
			  |   
    3   2   1
    �����Ź���
*------------------------------*/
const int offset[8][2]={{1,0},{1,1},{0,1},{-1,1},
						 {-1,0},{-1,-1},{0,-1},{1,-1}};



/**********************************
function:get_newimage
description:��ֵ��ͼ���㷨
calls:no
called by:no
input:��ֵ
output:void
**********************************/
void get_newimage(uint16_t thre)
{
	uint16_t x=0,y=0,TH=0;
	uint16_t Camera_Data;
	uint32_t i,m;
for(x=H_top; x<img_height-H_end; x++) //���ͼ��
	 {
		 for(y=W_top; y<img_width-W_end; y++)
		 {
	//for(x=0;x<img_height;x++)		//���ͼ��
	//{
	//	for(y=0;y<img_width;y++)
	//	{
		   Camera_Data=image2[x][y]; /* ���������һ�����ص�Camera_Data���� */			      
				if((Camera_Data&0x001f)>=thre)//��ֵ��
					image2[x][y]=0x0000;		
				else
					image2[x][y]=0xffff;		
		}
	}
	for(x=H_top; x<img_height-H_end; x++) 
	 {
		 for(y=W_top; y<img_width-W_end; y++)
		 {
	//for(x=2;x<img_height-2;x++)				//�˲�
	//{
	//	for(y=2;y<img_width-2;y++)           //�����ܱ�8������
		//{
			i= image2[x-1][y-1]+image2[x][y-1]+image2[x+1][y-1]
				+image2[x-1][y]+  image2[x][y]  +image2[x+1][y]
				+image2[x-1][y+1]+image2[x][y+1]+image2[x+1][y+1];
			if(i>=0x0003fffc)              //����4���׵��򽫴˵��ú�
				image2[x][y]=0x0000;
			else
				image2[x][y]=0xffff;				
		}
	}
	Dilation();
	Dilation();
	
	for(x=H_top; x<img_height-H_end; x++) 
	 {
		 for(y=W_top; y<img_width-W_end; y++)
		 {
	//for(x=2;x<img_height-2;x++)				
	//{
		//for(y=2;y<img_width-2;y++)
	//	{
			if(image2[x][y]==0xffff)//����˵�Ϊ�׵�			
			 {				
				m= image2[x-1][y-1]+image2[x][y-1]+image2[x+1][y-1]
					+image2[x-1][y]+                +image2[x+1][y]
					+image2[x-1][y+1]+image2[x][y+1]+image2[x+1][y+1];
				if(m<=0x0003fffc)        //���һ���׵���Χ����4���ڵ�����Ϊ�ڵ�
					image2[x][y]=0x0000;
				else
					image2[x][y]=0xffff;
				if(m==0x00000000)
				  image2[x][y]=0x0000;
				else
				  image2[x][y]=0xffff;
			   }
			   else           
		  	{
			  image2[x][y]=0x0000;
			  }
		 }
	}
	Erodation();
	Erodation(); 
	
	
}

/**********************************
function:mylabel()
description:��ͨ������
calls:��
called by:��
input:
output:���������������Ǿ���imgn[][]
**********************************/
void labelset(uint16_t xs,uint16_t ys,uint16_t label)
{
    uint16_t i,j,cnt,im,ip,jp,jm;
	 
	for(;;)
	{ 
		cnt=0;
  	for(i=H_top; i<img_height-H_end; i++) 
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
		//for(i=2; i<img_height-2; i++)
	  // {
		 // for(j=2; j<img_width-2; j++)
		//   {
		     if(imgn[i][j]==label)
				 {
				  im=i-1;
					ip=i+1;
					jm=j-1;
				  jp=j+1;
					if(im<0) im=0;
					if(ip>img_height-H_end) ip=img_height-H_end-1;
          if(jm<0) im=0;
          if(jp>img_width-W_end) ip=img_width-W_end-1;
				  if(imgn[im][jm]==0x0000)
					{
					  imgn[im][jm]=label;cnt++;
					}
					if(imgn[i][jm]==0x0000)
					{
					  imgn[i][jm]=label;cnt++;
					}
					if(imgn[ip][jm]==0x0000)
					{
					  imgn[ip][jm]=label;cnt++;
					}
					if(imgn[im][j]==0x0000)
					{
					  imgn[im][j]=label;cnt++;
					}
					if(imgn[ip][j]==0x0000)
					{
					  imgn[ip][j]=label;cnt++;
					}
					if(imgn[im][jp]==0x0000)
					{
					  imgn[im][jp]=label;cnt++;
					}
					if(imgn[i][jp]==0x0000)
					{
					  imgn[i][jp]=label;cnt++;
					}
					if(imgn[ip][jp]==0x0000)
					{
					  imgn[ip][jp]=label;cnt++;
					}
				 }
				
		   }
	    }
	 if(cnt==0x0000) break;
	}
}		
uint16_t mylabel()
{
	 uint16_t i,j,k=0;uint16_t label=0x0000;
	for(i=H_top; i<img_height-H_end; i++) 
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
	// for(i=2; i<img_height-2; i++)
	// {
		// for(j=2; j<img_width-2; j++)
		// {
		  imgn[i][j]=image2[i][j];
		 }
	 }
  for(i=H_top; i<img_height-H_end; i++) 
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
	 // for(i=2; i<img_height-2; i++)
	// {
		// for(j=2; j<img_width-2; j++)
		// {
		   if(imgn[i][j]==0x0000)
			 {
				label++;
				imgn[i][j]=label;
				labelset(i,j,label);
       }
		 }
	 }
	 return label;
}
/**********************************
function:label_extract()
description:��ͨ�����ȡ
calls:��
called by:��
input:�������,��ֵ
output:����С������С��TH�ľ���image2
**********************************/
void label_extract(uint16_t cnt,uint16_t TH)
{
  uint16_t i,j,k=0,size=0;
for(k=0; k<cnt; k++)
	{
	size=0;
	for(i=H_top; i<img_height-H_end; i++) 
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
		   if(imgn[i][j]==k)
			 {
				size++;
       }
		 }
	 }
	 if(size<TH)
	 {
	
	for(i=H_top; i<img_height-H_end; i++) 
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
		   if(imgn[i][j]==k)
			 {
				image2[i][j]=0xffff;
       }
		 }
	 }
	 
	 }
 }
}
/**********************************
function:trace()
description:׷��������
calls:��
called by:��
input:��ʼλ��
output:void
**********************************/
float trace(uint16_t xs,uint16_t ys)
{
	uint16_t x=xs,y=ys,no,vec=5;
	float l=0;
	no=imgn[x+1][y];
	for(;;)
	 {
	   if(x==xs && y==ys && l!=0) return l;
		 imgn[x][y]=0x0000;
		 switch(vec)
		 {
			 case 3:
				 if(imgn[x+1][y]!=no && imgn[x+1][y-1]==no)
				 {x=x+1;y=y;l++;vec=0;continue;}
			 case 4:
				 if(imgn[x+1][y-1]!=no && imgn[x][y-1]==no)
				 {x=x+1;y=y-1;l+=sqrt(2);vec=1;continue;}//l+=root2
			 case 5:
				 if(imgn[x][y-1]!=no && imgn[x-1][y-1]==no)
				 {x=x;y=y-1;l++;vec=2;continue;}
			 case 6:
				 if(imgn[x-1][y-1]!=no && imgn[x-1][y]==no)
				 {x=x-1;y=y-1;l+=sqrt(2);;vec=3;continue;}//l+=root2
			 case 7:
				 if(imgn[x-1][y]!=no && imgn[x-1][y+1]==no)
				 {x=x-1;y=y;l++;vec=4;continue;}
			 case 0:
				 if(imgn[x-1][y+1]!=no && imgn[x][y+1]==no)
				 {x=x-1;y=y+1;l+=sqrt(2);;vec=5;continue;}//l+=root2
			 case 1:
				 if(imgn[x][y+1]!=no && imgn[x+1][y+1]==no)
				 {x=x;y=y+1;l++;vec=6;continue;}
			 case 2:
				 if(imgn[x+1][y+1]!=no && imgn[x+1][y]==no)
				 {x=x+1;y=y+1;l+=sqrt(2);;vec=7;continue;}//l+=root2
				 vec=3;
		 }
	 }
}
/************************************************
function:find_rough
�߽���ٷ�������
����׼�򣺴�ͼ������Ͻ������ɨ�裬������Ե��ʱ˳����٣�ֱ�����ٺ�����ص���ʼ���λ��Ϊֹ��
һ���߸��ٽ����󣬽���ɨ����һ�����ٵ�
�жϸõ��Ƿ�ΪĿ��㣬����Ѹ�Ŀ��㶨Ϊ����ʼ�㣬��ʱ��ת90����Ϊ�µĸ��ٷ����������·���ĵ�
������Ŀ�����˳ʱ��ת45�ȼ����ж�
*************************************************/
void find_rough(uint16_t first_x,uint16_t first_y)
{
	uint16_t x,y;		//��ʱ����
	uint16_t i=0;
	uint16_t m=0;
	uint16_t find_next=0;
	uint16_t find_time=0;
	uint16_t tempx=0,tempy=0;				
	uint16_t z=0;
	
	x=first_x;
	y=first_y;
	
	for(i=0;i<MAX_LINE;i++)
	{
	//	image2[x][y]=0xF800;			//���Ѿ���¼�ĵ����
		find_next=0;
		find_time=0;
		while(find_next==0)		//���û���ҵ���һ����ͼ���
		{
			if(m>=8) m=m-8;		  //��������ķ�����
			//����߽��˾������µ�ļ�⣬����˳ʱ����ת45��
			if((x+offset[m][0]==480)||(x+offset[m][0]<0)||(y+offset[m][1]==800)||(y+offset[m][1]<0))
			{
				m++;
				find_time++;
			}
			else
			{
				//�����һ����ص������
				if((x+offset[m][0]==rough[0].x)&&(y+offset[m][1]==rough[0].y))
				{
					return;
				}
				//�����⵽�˺ڵ㣬��ʹ��ⷽ����ʱ����ת90��
				if(imgn[x+offset[m][0]][y+offset[m][1]]==0x0000)
				{
					rough[Rough_length].x=x;
					rough[Rough_length].y=y;
					//image2[x][y]=0xF800;
					x=x+offset[m][0];					//����ƫ��
					y=y+offset[m][1];
					Rough_length++;				//��������һ
					if(m<2)	     // ��ʱ����ת90��
						m=m+6;
					else 
						m=m-2;
					find_next=1;						//����ѭ��
				}
				else				//û�кڵ�,����˳ʱ����ת45��
				{
					m++;
					find_time++;
				}
			}
			if(find_time==8)		//���8�ζ�û���ҵ���һ���ڵ�
			{
				x=rough[Rough_length-1].x;			//����ƫ��,������һ����
				y=rough[Rough_length-1].y;
				Rough_length--;								//���ȼ���һ
				i=i-1;
				return;
			}
		}
	
		if(Rough_length>=50)
				{
					for(z=0;(z<Rough_length&&z<MAX_LINE);z++)
					{
						tempx=rough[z].x;
				  	tempy=rough[z].y;
						image2[tempx][tempy]=0xF800;//��ɫ	
					}
				}
	}
}
/**********************************
function:calc_size()
description:�������������λ��
calls:��
called by:��
input:��Ǻ�
output:����������ص�����
**********************************/
uint16_t calc_size(uint16_t label,uint16_t *cx,uint16_t *cy)
{
  uint16_t i,j,tx=0,ty=0,size=0;
	 for(i=2; i<img_height-2; i++)
	 {
		 for(j=2; j<img_width-2; j++)
		 {
		    if(imgn[i][j]==label)
					tx+=i;ty+=j;size++;
			}
	 }
		if(size==0x0000) return 0;
		tx=(uint16_t)(tx/size);
		ty=(uint16_t)(ty/size);
	  return size;
}
/**********************************
function:calc_length()
description:�����ܳ�
calls:��
called by:��
input:��Ǻ�
output:����������ص�����
**********************************/
float calc_length(uint16_t label)
{
	uint16_t i,j;
	float leng=1;
	 for(i=2; i<img_height-2; i++)
	 {
		 for(j=2; j<img_width-2; j++)
		 {
		    if(imgn[i][j]==label)
				{
					leng=trace(i-1,j);
				  return leng;
				}
			}
	 }
   return 0;
}
/**********************************
function:feature()
description:Բ�ζ�
calls:��
called by:��
input:cnt(�������),size(���),length(�ܳ�),ratio(Բ�ζ�),����
output:����������ص�����
**********************************/
float feature(uint16_t label,uint16_t center_x,uint16_t center_y)
{
	uint16_t i,j,cx,cy;
	float L,size,length,ratio;
	size=calc_size(label,&cx,&cy); //���
	center_x=cx;
	center_y=cy;
	
	L=calc_length(label); //�ܳ�
	length=L;
		
	ratio=4*PI*size/(L*L);   //Բ�ζȣ�Խ�ӽ�ԲֵԽ��
	imgn[cx][cy]=0x0000;                  //�������
	return ratio;

}
/**********************************
function:size_extract()
description:���������Χ��ȡ����
calls:��
called by:��
input:�������������������Сֵ
output:void
**********************************/
void size_extract(float size,float size_min,float size_max,uint16_t label)
{
	uint16_t x,y;
	if(size>=size_min && size<=size_max)
	{
	 for(x=2; x<img_height-2; x++)
	 {
		 for(y=2; y<img_width-2; y++)
		 {
				if(imgn[x][y]!=label)
					imgn[x][y]=0xffff;
			}
	 }
  }
}
/**********************************
function:ratio_extract()
description:����Բ�ζȷ�Χ��ȡ����
calls:��
called by:��
input:�������������������Сֵ
output:void
**********************************/
void ratio_extract(float ratio,float ratio_min,float ratio_max,uint16_t label)
{
	uint16_t x,y;
	if(ratio>=ratio_min && ratio<=ratio_max)
	{
	 for(x=2; x<img_height-2; x++)
	 {
		 for(y=2; y<img_width-2; y++)
		 {
				if(imgn[x][y]!=label)
					imgn[x][y]=0xffff;
			}
	 }
   }
}
/**********************************
function:distance()
description:���(Ԥ�Ȳ⵽��Ӿ��θ�����Ӿ��εĿ�ʵ�ʿ�Ȳ���)
calls:��
called by:��
input:��ֵ
output:void
**********************************/
float distance(float r,float f,uint16_t Object_W,uint16_t MAXCOL,uint16_t MINCOL)
{
	//����Ϊ��Ԫ�ߴ硢���ࡢ��ࡢ�����С������
	float Image_w,k,D;                         
	float a=29.4;      //�̶�����
	float b=18.7;    //�̶�����
	//r=0.015;          //mm  (7Ӣ�� 800��480 123mm��68mm ���:0.15mm)
  //f=4;                             //mm�������
  //Image_w=r*sqrt((MAXROW-MINROW)^2+(XCOL-NCOL)^2);   
  Image_w=r*(MAXCOL-MINCOL);                 //��ֱ��
	k=f*(Object_W/Image_w);                    //�����
  D=a*k-b;                     //mmʵ�ʾ��룺w/W=d/D;1/d+1/D=1�Ƴ�
  return D;
}
/**********************************
function:point()
description:��С��Ӿ��ε�4�������ѡȡ
calls:��
called by:��
input:��������
output:void
**********************************/
void point(uint16_t a[4])
{
	uint16_t i,j,b[4]={240,240,400,400};
	for(i=H_top+10; i<img_height-H_end-10; i++) 
	 {
		 for(j=W_top+10; j<img_width-W_end-10; j++)
		 {
			if(image2[i][j]==0x0000)
			{
				if(b[0]<i) b[0]=i;
			  if(b[1]>i) b[1]=i;
				if(b[2]<j) b[2]=j;
			  if(b[3]>j) b[3]=j;
			}
			  
		}
	}
   a[0]=b[0];   //���������
   a[1]=b[1];   //��С������
	 a[2]=b[2];    //��������
   a[3]=b[3];  	  //��С������
}
void point2(uint16_t a[4])
{
	uint16_t i,j,b[4]={0,0,0,0};
	for(i=H_top+10; i<img_height-H_end-10; i++) 
	 {
		 for(j=W_top+10; j<img_width-W_end-10; j++)
		 {
			if(image2[i][j]==0x0000)//ʹ��ͼ���ϵĵ�һ������Ϊ�Աȵ�
			{
				b[0]=i;
			  b[1]=i;
				b[2]=j;
			  b[3]=j;
				goto here;
			}
			  
		}
	}
	here:
	for(i=H_top+10; i<img_height-H_end-10; i++) 
	 {
		 for(j=W_top+10; j<img_width-W_end-10; j++)
		 {
			if(image2[i][j]==0x0000)
			{
				if(b[0]<i) b[0]=i;
			  if(b[1]>i) b[1]=i;
				if(b[2]<j) b[2]=j;
			  if(b[3]>j) b[3]=j;
			}
			  
		}
	}
   a[0]=b[0];   //���������
   a[1]=b[1];   //��С������
	 a[2]=b[2];    //��������
   a[3]=b[3];  	  //��С������
}
/**********************************
function:diagonal()
description:ץȡ�Ƕȼ�����
calls:��
called by:��
input:��������
output:�н�
**********************************/
float diagonal(uint16_t a[4],float m,float n)
{
  uint16_t i=0,j,b[400];
	float k,r,diag;
	for(j=W_top;j<img_width-W_end;j++)
		{
			if(image2[a[1]][j]==0x0000)
			{
			   b[i]=j;
				 i++;
			}
		}
	  maopao(b,i);             //��С��������	 
	  a[0]=b[i-1];   //�϶�����������
		a[1]=b[0];     //�϶�����С������
		k=(a[0]-a[1])/(a[2]-a[3]);
		r=m*k+n;    //����
		diag=(r/PI)*180;    //�Ƕ�    
		return diag;
}
/**********************************
function:Houghtrans()
description:Hough�任���Բ
            (1)����һ��2ά�ۼ����飬��Ϊa[Theta][r]
            (2)��ʼ��������aΪ0,Ȼ���ÿ��ͼ��ռ��и����ı�Ե�㣬��Thetaȡ��Theta�������п���ֵ
               ������R=x*cos(theta)+y*sin(theta)���R
            (3)�ٸ���Theta��R��ֵ(��������),��a�����ۼ�a[theta][r]++,�ۼӽ����󣬸���theta��R��ֵ
               ��֪���ٵ㹲�ߣ���a[theta][r]��ֵ��ͬ�theta��rҲ������ֱ�߷��̲���
calls:��
called by:��
input:��ֵ
output:void
**********************************/
void HoughTrans()  
    {  
        //x*cos(theta)+y*sin(theta)=r;   
        uint16_t x,y,indexTemp,rValue_M,theta_M,iCount; 
		  	uint16_t thetaMax=180,CounterMax=0;
			  uint16_t RMax =933; 
			  //uint16_t counters[933][180];     //�ۼ�����
        uint16_t theta;  
        uint16_t rValue;   
      
			for(x=0;x<RMax;x++)				
	      {
	        for(y=0;y<thetaMax;y++)
	        {   
						counters[x][y]=0x0000;
					}
				}
			for(x=2;x<img_height-2;x++)				
	      {
	        for(y=2;y<img_width-2;y++)
	        {
              if(image2[x][y]==0xffff) 
						{		
							for(theta=0;theta<thetaMax;theta++)  
							{
							 rValue=(uint16_t)(x*cos(theta*PI/thetaMax)+y*sin(theta*PI/thetaMax));  
                if (rValue>=0)  
                  counters[theta][rValue]++;											
							}    
            }  
          }  
        }  
	//ת��hough��(p,t)
				
        //�õ���Ѳ���  
        for (rValue=0;rValue<RMax;rValue++)  
        {  
            for (theta=0;theta<thetaMax;theta++)  
            {  
							if(CounterMax<counters[rValue][theta])
							  CounterMax=counters[rValue][theta];
							  rValue_M=rValue;
							  theta_M=theta;
            }  
        }
				for(x=2;x<img_height-2;x++)				
					{
						for(y=2;y<img_width-2;y++)
							{
								 if(image2[x][y]==0xffff) 
					  	   {		
								   rValue=(uint16_t)(x*cos(theta_M*PI/thetaMax)+y*sin(theta_M*PI/thetaMax));  
                     if(rValue==rValue_M)
											 image2[x][y]=0xF800;//��ɫ
										 else
											 image2[x][y]=0x0000;//��ɫ
									 }
								 else
									 image2[x][y]=0x0000;
							 }
						 }
     } 
		
////����ƫ�Ƶ�ַ
void GetPixelValueEx(uint16_t *value, uint16_t i, uint16_t j)
{
value[0]=imgn[i-1][j-1]&0x001f;
value[1]=imgn[i][j-1]&0x001f;
value[2]=imgn[i+1][j-1]&0x001f;
value[3]=imgn[i-1][j]&0x001f;
value[4]=imgn[i][j]&0x001f;
value[5]=imgn[i+1][j]&0x001f;
value[6]=imgn[i-1][j+1]&0x001f;
value[7]=imgn[i][j+1]&0x001f;
value[8]=imgn[i+1][j+1]&0x001f;
}		
void DoHough()
{
  uint16_t temp=0,i,j,k,m;
	uint16_t value[9];///3*3ģ��
  uint16_t t,t1;	
	uint16_t m_iValve=50;
for(i=2;i<img_height-2;i++)
  {
		
    for(j=2;j<img_width-2;i++)
      {
        GetPixelValueEx(value,i,j);
				t=0;t1=0;
        for(k=0;k<3;k++)
         {
           for(m=0;m<8;m++)///��ģ����
             {
               t1+= Hough[k][m]*value[m];
             }
          if(t1>t)
             t=t1;
          }
        if(t>0x005d)
        image2[i][j]=0xF800;
        //else
        //image2[i][j]=0x0000;
       }
   }
}

/**********************************
function:canny_process()
description:canny���ӱ�Ե���
calls:��
called by:��
input:��ֵ
output:void
**********************************/
void canny_process(uint16_t thre)	
{
	
	uint16_t a,b,t,c1,c2,c3,c4,c5,c6,c7,c8,c9;
  uint16_t x,y,i,fxr,th1,th2;
  uint16_t dy,dx;
	uint16_t tem[9];
for(x=0;x<img_height;x++)
	 {
	   for(y=0;y<img_width;y++)
	  {
		  m[x][y]=0x0000;
			o[x][y]=0x0000;
		 }
 	 }	
//��˹�˲�����ƽ��
for (a=2;a<=img_height-2;a++)   
{
  for (b=2;b<=img_width-2;b++)   
  {
        c1=image2[a][b-1]&0x001f;
        c2=image2[a-1][b]&0x001f;
        c3=image2[a][b]&0x001f;
        c4=image2[a+1][b]&0x001f;
        c5=image2[a][b+1]&0x001f;
        c6=image2[a-1][b-1]&0x001f;
        c7=image2[a-1][b+1]&0x001f;
        c8=image2[a+1][b-1]&0x001f;
        c9=image2[a+1][b+1]&0x001f;
        t=(2*c1+2*c2+2*c4+2*c5+4*c3+1*c6+1*c7+1*c8+1*c9)/16;
		    fxr=t<<11|t<<6|t;
		//��˹����
	//	  |1 2 1|            c6  c2  c7     
	//	  |2 4 2|*1/16       c1  c3  c5     
	//	  |1 2 1|            c8  c4  c9    
       if (fxr<0x0000)
           image2[a][b]=0x0000;
        else if (fxr>0xffff)
           image2[a][b]=0xffff;
				else
           image2[a][b]=fxr;
 }
}	
	
 //��ÿ�����أ��������������΢�ֽ����Եõ��ݶȴ�С������
 for(x=1;x<img_height-1;x++)				
	{
		for(y=1;y<img_width-1;y++)
		{
		 dy= ((   image2[x][y+1]&0x001f   - image2[x][y]&0x001f 
			    + image2[x+1][y+1]&0x001f - image2[x+1][y]&0x001f)*0.5); //y�����ݶ�              
     dx= ((    image2[x+1][y]&0x001f  - image2[x][y]&0x001f
			    + image2[x+1][y+1]&0x001f - image2[x][y+1]&0x001f)*0.5); //x�����ݶ�
     m[x][y]=my_abs(dy)+my_abs(dx);//�ݶȴ�С
		 //o[x][y]=(int16_t)arctan(dy/dx);//�ݶȷ���(����)
		 m[x][y]=(m[x][y]<<11)|(m[x][y]<<6)|(m[x][y]);
		
		

			//���ַ�������
			 // 3 2 1
			 // 0 x 0
			 // 1 2 3
		  if ((o[x][y]<PI/16)&&(o[x][y]>-PI/16))//0��
			    o[x][y]=0;
      else if ((o[x][y]<PI*3/16)&&(o[x][y]>PI/16))//45��
			    o[x][y]=1;	
		  else if ((o[x][y]<PI*5/16)&&(o[x][y]>PI*3/16))//-45��
			    o[x][y]=2;	
      else	
 			    o[x][y]=3;                   //90��
			
    }
  }
	
 for(x=1;x<img_height-1;x++)
	 {
	   for(y=1;y<img_width-1;y++)
	  {
		  imgn[x][y]=0x0000;
		 }
 	 }
//�Ǽ���ֵ����
//�����ĵ������䷽�������ݶȷ�ֵ���������������	 
for(x=2;x<img_height-1;x++)				
	{
		for(y=2;y<img_width-1;y++)
		{
		  if(o[x][y]==0)//ˮƽ
			{
				if((m[x][y]>m[x][y-1]) && (m[x][y]>m[x][y+1]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else 
					   imgn[x][y]=0x0000;
			}
			 if(o[x][y]==1)//��������
			{
				if((m[x][y]>m[x+1][y-1]) && (m[x][y]>m[x-1][y+1]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else 
					   imgn[x][y]=0x0000;
			}
			 if(o[x][y]==2)//��ֱ
			{
				if((m[x][y]>m[x-1][y]) && (m[x][y]>m[x+1][y]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else  
					   imgn[x][y]=0x0000;
			}
			 if(o[x][y]==3)//��������
			{
				if((m[x][y]>m[x-1][y-1]) && (m[x][y]>m[x+1][y+1]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else 
					   imgn[x][y]=0x0000;
			}
		
		}
	}
	 

	
	 for(x=0;x<img_height;x++)
	 {
	   for(y=0;y<img_width;y++)
	  {
		  m[x][y]=imgn[x][y];
			imgn[x][y]=0x0000;
		 }
 	 }    
//���ݶ�ȡ������ֵ
	th1=thre;
	th2=th1*2;
	for(x=2;x<img_height-1;x++)				
	{
		for(y=2;y<img_width-1;y++)
		{
      if((m[x][y])<th1)//����ֵ����
           imgn[a][b]=0x0000;//��
      else if((m[x][y])>th2)//����ֵ����
           imgn[a][b]=m[x][y]<<11|m[x][y]<<6|m[x][y];
          else //���ڸߵ���ֵ֮��Ŀ���8�����Ƿ��и��ڸ���ֵ������Ϊ��Ե
            tem[0]=m[x-1][y-1]; tem[1]=m[x-1][y]; tem[2]=m[x-1][y+1];
		       	tem[3]=m[x][y-1];   tem[4]=m[x][y]; tem[5]=m[x][y+1];
			      tem[6]=m[x+1][y-1], tem[7]=m[x+1][y];tem[8]=m[x+1][y+1];	
            maopao((uint16_t*)tem,9);
            if (tem[8]>th2)
             imgn[a][b]=tem[8]<<11|tem[8]<<6|tem[8];
             else
             imgn[a][b]=0x0000;			
     }
  } 
}
/**********************************
function:log_process()
description:LOG���ӱ�Ե���
calls:��
called by:��
input:��ֵ
output:void
**********************************/
void log_process(uint16_t thre)
{
uint16_t a,b,t,c1,c2,c3,c4,c5,c6,c7,c8,c9,fxr;
uint16_t x,y,th1,th2,tem[9];
uint32_t i,ii,jj,r;
  
//��˹�˲�����ƽ��
for (a=2;a<=img_height-2;a++)   
{
  for (b=2;b<=img_width-2;b++)   
  {
    		c1=image2[a][b-1]&0x001f;
        c2=image2[a-1][b]&0x001f;
        c3=image2[a][b]&0x001f;
        c4=image2[a+1][b]&0x001f;
        c5=image2[a][b+1]&0x001f;
        c6=image2[a-1][b-1]&0x001f;
        c7=image2[a-1][b+1]&0x001f;
        c8=image2[a+1][b-1]&0x001f;
        c9=image2[a+1][b+1]&0x001f;
        t=(2*c1+2*c2+2*c4+2*c5+4*c3+c6+c7+c8+c9)/16;
		    fxr=t<<11|t<<6|t;
		//��˹����
	//	  |1 2 1|            c6  c2  c7
	//	  |2 4 2|*1/16       c1  c3  c5
	//	  |1 2 1|            c8  c4  c9
        if (fxr<0x0000)
          image2[a][b]=0x0000;
        else if (fxr>0xffff)
          image2[a][b]=0xffff;
				else
           image2[a][b]=fxr;
 }
}
//������˹��
for (a=2;a<=img_height-2;a++)
{
   for (b=2;b<=img_width-2;b++)
    {
        c1=image2[a][b-1]&0x001f;
        c2=image2[a-1][b]&0x001f;
        c3=image2[a][b]&0x001f;
        c4=image2[a+1][b]&0x001f;
        c5=image2[a][b+1]&0x001f;
        c6=image2[a-1][b-1]&0x001f;
        c7=image2[a-1][b+1]&0x001f;
        c8=image2[a+1][b-1]&0x001f;
        c9=image2[a+1][b+1]&0x001f;
        //t=-1*c1-1*c2-1*c4-1*c5+4*c3+0*c6+0*c7+0*c8+0*c9;//��ģ��2����Ч������
			  //t=-2*c1-2*c2-2*c4-2*c5+4*c3+c6+c7+c8+c9;//ģ��1 ��Ե����Ч���� 
			  t=c1+c2+c4+c5-4*c3+0*c6+0*c7+0*c8+0*c9;//ģ��2 Ч����1��
			  //t=c1+c2+c4+c5-8*c3+1*c6+1*c7+1*c8+1*c9;//ģ��3 Ч��һ��
		    fxr=t<<11|t<<6|t;
		    image2[a][b]=fxr;
			  imgn[a][b]=fxr;
		//������˹����   1 -2  1    0  1  0    1  1  1
		//              -2  4 -2    1 -4  1    1 -8  1
		//               1 -1  1    0  1  0    1  1  1
		//������˹�񻯣��Թ�����Ͷ˵��Ϊ����
		
		//sobel����  -1 0 1   1 2 1
		//         y -2 0 2 x 0 0 0
		//           -1 0 1  -1-2-1
			
			if((fxr&0x001f)>=thre)//��ֵostu��ѡȡ
           image2[a][b]=0x0000;//�� 
        else
           image2[a][b]=0xffff;//��
					
      }
   }
//�߽紦��
//˫��ֵ
	//th2=THR;
	//th1=th2*0.6;
	//for(x=2;x<img_height-2;x++)				
	//{
	//	for(y=2;y<img_width-2;y++)
		//{
   //   if((imgn[x][y]&0x001f)>=th2)//����ֵ����
	//		{
		//		image2[x][y]=0x0000;//��
		//	}
   //    if((imgn[x][y]&0x001f)<=th1)//����ֵ����
		//	{
		//			image2[x][y]=0xffff;
	//		}
   //    if(th2<(image2[x][y]&0x001f)<th1)
		//	 { //���ڸߵ���ֵ֮��Ŀ���8�����Ƿ��и��ڸ���ֵ������Ϊ��Ե
   //      tem[0]=imgn[x-1][y-1]&0x001f; tem[1]=imgn[x-1][y]&0x001f; tem[2]=imgn[x-1][y+1]&0x001f;
		//     tem[3]=imgn[x][y-1]&0x001f;   tem[4]=imgn[x][y]&0x001f;   tem[5]=imgn[x][y+1]&0x001f;
		//	   tem[6]=imgn[x+1][y-1]&0x001f, tem[7]=imgn[x+1][y]&0x001f; tem[8]=imgn[x+1][y+1]&0x001f;	
    //     maopao(tem,9);
    //     if((tem[8]&0x001f)>=th2)
	//				 image2[x][y]=0x0000;
   //        else
   //         image2[x][y]=0xffff;		
	//			}
							
  //   }
 // } 
}
/**********************************
function: otsuThreshold()
description: otsu�Զ���ֵѡȡ�㷨
calls: ��
called by: ��
input: void
return: 16λ����ֵ
**********************************/		
uint16_t otsuThreshold()
{
    const uint16_t GrayScale=0x001f;//2^5=32���Ҷȼ�
    uint16_t pixelCount[GrayScale];//�Ҷ�ֱ��ͼ
    uint16_t i,j,k, pixelSum=img_width*img_height,threshold = 0;
	  double N0,N1,w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp=-1, deltaMax = -1;

	  for (i=0;i<GrayScale;i++)
    {
        pixelCount[i] = 0;
    }

    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ��������Ҷ�ֱ��ͼ������Ϊ�Ҷ�ֵ������Ϊ����
		for (i=0;i<img_height;i++)
    {
        for (j=0;j<img_width; j++)
        {
            pixelCount[image2[i][j]&0x001f]++;  
        }
    }

    //������ֵ
    for (i = 0;i<GrayScale;i++) 
    { 
        N0 += pixelCount[i];  //ǰ�����ظ���        
        N1 = pixelSum-N0;     //�������ظ���
        if(0==N1)break;   //ǰ������������
        w0 = N0/pixelSum; //ǰ�����ر���
        w1 = 1-w0;       //�������ر���
        for (j= 0;j<=i;j++) //iΪ��ֵ
        { 
            u0tmp += j*pixelCount[j]; //�Ҷ���ֵ
        } 
        u0 = u0tmp/w0; //ƽ���Ҷ�
        for(k= i+1;k<GrayScale;k++) 
        { 
            u1tmp += k*pixelCount[k]; 
        }
        u1 = u1tmp / w1; //ƽ���Ҷ�
        deltaTmp = w0*w1*(u0 - u1)*(u0 - u1);//��䷽��
				if (deltaTmp > deltaMax)
        {
           deltaMax = deltaTmp;
           threshold = i;   //����󷽲��Ӧ�ĻҶ�ֵ 
        }
    }

    return threshold;
}
/**********************************
function: OET()
description: ������Զ���ֵѡȡ�㷨
calls: ��
called by: ��
input: void
return: 16λ����ֵ
**********************************/		
uint16_t OET()
{
	  const uint16_t GrayScale_2=0x001f;//2^5=32���Ҷȼ�
    uint16_t pixelCount[GrayScale_2];//�Ҷ�ֱ��ͼ
	  uint16_t pixelgailv[GrayScale_2];//�Ҷȸ���
	  uint16_t H,HMax=0;//��
    uint16_t i, j,k, pixelSum=img_width*img_height,threshold = 0;
	  double p1=0,p2=0,H1=0,H2=0;

	  for (i=0;i<GrayScale_2;i++)
    {
        pixelCount[i] = 0;
    }

    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ��������Ҷ�ֱ��ͼ������Ϊ�Ҷ�ֵ������Ϊ����
    for (i=0;i<img_height;i++)
    {
        for (j=0;j<img_width; j++)
        {
            pixelCount[image2[i][j]&0x001f]++;  
        }
    }
    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ��ʡ����Ҷ�ֱ��ͼ������Ϊ�Ҷ�ֵ������Ϊ����
		for (i=0;i<GrayScale_2;i++)
    {
        pixelgailv[i]=pixelCount[i]/i;
    }
		for(i=0;i<GrayScale_2;i++) 
		{
			if (pixelCount[i]!=0)
			{				
				for(j=0;j<=i;j++)
				{
				H1=pixelgailv[j]*log(pixelgailv[j])+H1; 
				}
				 for(k=i+1;k<GrayScale_2;k++) 
        { 
				H2=pixelgailv[i]*log(pixelgailv[i])+H2; 
		    }
		  }
	  H=-(H1+H2); 
		if (H > HMax)
        {
           HMax = H;
           threshold = i;   //����󷽲��Ӧ�ĻҶ�ֵ 
        }
    } 
 return threshold;
}
/**********************************
function: MeanFilterOper()
description: ��Ȩ����˹�˲�(�Ҷ�ͼ)---ģ������
calls: ��
called by: ��
input: void
return: void
**********************************/	
void MeanFilterOper()
{
	uint16_t x,y,i[9],t=0;
	for(x=2;x<img_height-2;x++)				
	{
		for(y=2;y<img_width-2;y++)           //�����ܱ�8������
		{
			i[0]=image2[x-1][y-1]&0x001f;
			i[1]=image2[x][y-1]&0x001f;
			i[2]=image2[x+1][y-1]&0x001f;
			i[3]=image2[x-1][y]&0x001f;
			i[4]=image2[x][y]&0x001f;
			i[5]=image2[x+1][y]&0x001f;
			i[6]=image2[x-1][y+1]&0x001f;
			i[7]=image2[x][y+1]&0x001f;
			i[8]=image2[x+1][y+1]&0x001f;  
			//t=(1*i[0]+3*i[1]+1*i[2]+3*i[3]+6*i[4]+3*i[5]+1*i[6]+3*i[7]+1*i[8])/22;//Ч����
			t=(i[0]+2*i[1]+i[2]+2*i[3]+4*i[4]+2*i[5]+i[6]+2*i[7]+i[8])/16;//��˹ģ��ƽ��Ч����
			//��Ȩƽ��ģ��
	//	  |1 2 1|             |1 3 1|  
	//	  |2 4 2|*1/16        |3 6 3|*1/22
	//	  |1 2 1|             |1 3 1|  
			t=t<<11|t<<6|t;
			if(t<0x0000)
           image2[x][y]=0x0000;
      else if (t>0xffff)
           image2[x][y]=0xffff;
			else
           image2[x][y]=t;
		}

		}
	}
/**********************************
function: KNNFilterOper()
description: K���ھ�ֵ�˲�(�Ҷ�ͼ)kѡ5
calls: ��
called by: ��
input: void
return: void
**********************************/	
void KNNFilterOper()
{
	uint16_t x,y,z,j,i[8],k[8],t=0,temp=0;
 for(x=2;x<img_height-2;x++)				
	{
		for(y=2;y<img_width-2;y++)           //�����ܱ�8������
		{
			t=image2[x][y]&0x001f;
			i[0]=image2[x-1][y-1]&0x001f;
			i[1]=image2[x][y-1]&0x001f;
			i[2]=image2[x+1][y-1]&0x001f;
			i[3]=image2[x-1][y]&0x001f;
			i[4]=image2[x+1][y]&0x001f;
			i[5]=image2[x-1][y+1]&0x001f;
			i[6]=image2[x][y+1]&0x001f;
			i[7]=image2[x+1][y+1]&0x001f; 
      
      k[0]=my_abs(t-i[0]);
			k[1]=my_abs(t-i[1]);
			k[2]=my_abs(t-i[2]);
			k[3]=my_abs(t-i[3]);
		  k[4]=my_abs(t-i[4]);
		  k[5]=my_abs(t-i[5]);
		  k[6]=my_abs(t-i[6]);
		  k[7]=my_abs(t-i[7]);
			
			for(z=0;z<7;z++)  
    {    
        for(j=0;j<7-z;j++)  
            if(k[j]>k[j+1])  
            {  
                temp=k[j];  
                k[j]=k[j+1];  
                k[j+1]=temp;   
							
						  	temp=i[j];  
                i[j]=i[j+1];  
                i[j+1]=temp;   
            }  
    }  
			t=(4*t+2*i[0]+2*i[1]+2*i[2]+2*i[3]+i[4]+i[5]+i[6]+i[7])/16;
			//ѡk������������صĻҶȲ���С�����أ�����k�����صľ�ֵ����������ֵ
			t=t<<11|t<<6|t;
			if(t<0x0000)
           image2[x][y]=0x0000;
      else if (t>0xffff)
           image2[x][y]=0xffff;
			else
           image2[x][y]=t;
		}

		}
}
/**********************************
function: MedianFilterOper()
description: �Ҷ�ͼ�����ֵ�˲� 3*3---ģ������
calls: maopao()
called by: ��
input: void
return: void
**********************************/		
void MedianFilterOper()
{
	uint16_t x,y,i[9];
 for(x=2;x<img_height-2;x++)				//��ֵ�˲�
	{
		for(y=2;y<img_width-2;y++)     		//�����ܱ�8������
		{
			i[0]=image2[x-1][y-1]&0x001f;
			i[1]=image2[x][y-1]&0x001f;
			i[2]=image2[x+1][y-1]&0x001f;
			i[3]=image2[x-1][y]&0x001f;
			i[4]=image2[x][y]&0x001f;
			i[5]=image2[x+1][y]&0x001f;
			i[6]=image2[x-1][y+1]&0x001f;
			i[7]=image2[x][y+1]&0x001f;
			i[8]=image2[x+1][y+1]&0x001f;
			maopao(i,9);  
			image2[x][y]=i[5]<<11|i[5]<<6|i[5];
		}
	}
}
/**********************************
function: Hist_plane()
description: ֱ��ͼ���⻯
calls: sort(),weight()
called by: ��
input: void
return: void
**********************************/	
void Hist_plane()
{
    uint16_t i,j;
    uint16_t hist[0x001f];          //�Ҷ�ֱ��ͼ
    float histPDF[0x001f];    //��һ��ֱ��ͼ
	  float histCDF[0x001f];    //�ۻ�ֱ��ͼ
	  uint16_t histEQU[0x001f];    //ֱ��ͼ���⻯
		uint16_t size=img_width*img_height;
  
    //�Ҷ�ֱ��ͼ  
   	for(i=H_top; i<img_height-H_end; i++) //���ͼ��
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
	//for (i=0; i<img_height; i++)  
   // {  
      //  for (j=0; j<img_width; j++)  
       // {  
					hist[image2[i][j]&0x001f]++; 
        }  
			}
   
		//��һ��ֱ��ͼ 
    for (i=0; i<0x001f; i++)
    {  
 			histPDF[i]=(float)hist[i]/size;
    }  
    
		//�ۻ�ֱ��ͼ
  	for (i=0; i<0x001f; i++)
    {  
       if (0==i)
				 histCDF[i] = histPDF[i];  
       else 
				 histCDF[i] = histCDF[i-1] + histPDF[i];  
    }  
    
		//ֱ��ͼ���⻯
    for (i=0; i<0x001f; i++)
    {  
       histEQU[i] = (uint16_t)(31.0 * histCDF[i] + 0.5);  
    }  
  	for(i=H_top; i<img_height-H_end; i++) //���ͼ��
	 {
		 for(j=W_top; j<img_width-W_end; j++)
		 {
		// for(i=0; i<img_height; i++) 
   // {  
        // for(j=0;j<img_width; j++)
       // {  
          imgn[i][j]= histEQU[image2[i][j]&0x001f];
          image2[i][j]=(imgn[i][j]<<11)|(imgn[i][j]<<6)|imgn[i][j];
				}  
    } 
	  
}
/**********************************
function: Dilation()
description: ��������
calls: ��
called by: ��
input: void
return: void
**********************************/	
void Dilation()
{
 uint16_t i,j,m,n,flag; 
   for(j=H_top; j<img_height-H_end; j++) 
	 {
		 for(i=W_top; i<img_width-W_end; i++)
		 {
	// for(j=0;j<img_height;j++)
	// {
	//   for(i=0;i<img_width;i++)
	  //{
		 imgn[j][i]=image2[j][i];
		 }
 	 }
  for(i=H_top+1; i<img_height-H_end-1; i++) //���ͼ��
	 {
		 for(j=W_top; j<img_width-W_end-1; j++)
		 {
	 //for(i=1; i<img_height-1 ;i++)
   // {
       // for(j=1; j<img_width-1 ;j++)
       // {
            flag = 1;
            for(m=i-1; m<i+2 ;m++)
            {
                for(n=j-1; n<j+2 ;n++)
                {
                    //����������һ��Ϊ1���򽫸õ�����Ϊ1
                    if(imgn[i][j]== 0xffff
                        ||imgn[m][n] == 0xffff)
                    {
                        flag = 0;
                        break;
                    }
                }
                if(flag == 0)
                {
                    break;
                }
            }
            if(flag == 0)
            {
                image2[i][j] = 0xffff;
            }
            else
            {
                image2[i][j] = 0x0000;
            }
        }
    }
}

/**********************************
function: Erodation(void)
description: ��ʴ����
calls: ��
called by: ��
input: void
return: void
**********************************/	
void Erodation()
{
    uint16_t i,j,m,n,flag; 
 for(j=H_top; j<img_height-H_end; j++) 
	 {
		 for(i=W_top; i<img_width-W_end; i++)
		 {
	// for(j=0;j<img_height;j++)
	// {
	 //  for(i=0;i<img_width;i++)
	 // {
		 imgn[j][i]=image2[j][i];
		 }
 	 }
  for(i=H_top+1; i<img_height-H_end-1; i++) //���ͼ��
	 {
		 for(j=W_top+1; j<img_width-W_end-1; j++)
		 {
	//for(i=1; i<img_height-1 ;i++)
  //  {
    //    for(j=1; j<img_width-1 ;j++)
     //   {
            flag = 1;
            for(m=i-1; m<i+2 ;m++)
            {
                for(n=j-1; n<j+2 ;n++)
                {
                    //����������һ��Ϊ0���򽫸õ�����Ϊ0
                    if(imgn[i][j]== 0x0000
                        ||imgn[m][n] == 0x0000)
                    {
                        flag = 0;
                        break;
                    }
                }
                if(flag == 0)
                {
                    break;
                }
            }
            if(flag == 0)
            {
                image2[i][j] = 0x0000;
            }
            else
            {
                image2[i][j] = 0xffff;
            }
        }
    }
}
/**********************************
function: close()
description: �����㣨�������ٸ�ʴ��ȥ����ϸ�ڲ���
calls: ��
called by: ��
input: void
return: void
**********************************/	
void close()
{
  	Dilation();
		Erodation();
	
}
/**********************************
function: open()
description: �����㣨�ȸ�ʴ�����ͣ�ȥ����С��ϸ��
calls: ��
called by: ��
input: void
return: void
**********************************/	
void open()
{
    Dilation();
		Erodation();
}
/**********************************
function: MedianFilterOper()
description: ð������(��С����)
calls: maopao()
called by: ��
input: void
return: void
**********************************/		
void maopao(uint16_t a[],uint16_t n)  
{   
    uint16_t flag=0,i,j,t; 
    for(i=0;i<n-1;i++)  
    {  
        flag=0;  
        for(j=0;j<n-i-1;j++)  
            if(a[j]>a[j+1])  
            {  
                t=a[j];  
                a[j]=a[j+1];  
                a[j+1]=t;  
                flag=1;  
            }  
        if(flag==0)  
            break;  
    }  
} 

/**********************************
function: my_abs()
description: �����ֵ
calls: ��
called by: ��
input: double a
return: double |a|
**********************************/		
double my_abs(double a)
{
	if(a<0)
		return -a;
	else
		return a;
}
/**********************************
function:fun()
description:��x��n�η�����
calls:find_rough
called by:no
**********************************/
double fun(double x, uint16_t n)      
{
   uint16_t i;
   double sum=1;
   if(n>=1)
   for(i=1;i<=n;i++)
       sum*=x;
    else
   sum=1;
   return sum;
}
/**********************************
function:max()min()
description:�����Сֵ����
calls:a[]
called by:no
**********************************/
uint16_t Max(uint16_t a[],uint16_t N)
{
		uint16_t t=a[0],i;
		for(i=1;i<N-1;i++)
			{
	    if(t<a[i]) t=a[i];
			}
		return t;
}
uint16_t Min(uint16_t a[],uint16_t N)
{
		uint16_t t=a[0],i;
		for(i=1;i<N-1;i++)
	   {
			 if(t>a[i]) t=a[i];
	   }
		return t;
} 
/**********************************
function: RGB565_to_Gray()
description: RGB565��ʽת�Ҷ�ͼ��(RGB565��ʽ��ʾ)
calls: ��
called by: ��
input: uint16_t RGB565
return: uint16_t GRAY
**********************************/		
uint16_t RGB565_to_Gray(uint16_t rgb_565)
{
		uint16_t t;	
		uint16_t R,G,B;	
		R=rgb_565&0xf800>>11;
		G=rgb_565&0x07C0>>6;
		B=rgb_565&0x001F;
		t=(R*30+G*60+B*10)/100;//תΪ�Ҷ�ֵ(RGB565��ʽ��ʾ)t=(R*1.1+G*5.9+3*B)/10;R = G = B = 0.3R + 0.6G + 0.1B; 
		return (t<<11)|(t<<6)|t;	
} 

/**********************************
function: GET8Gray()
description: 16λRGB565��ʽת8λ�Ҷ�
calls: ��
called by: ��
input: uint16_t RGB565
return: uint8_t GRAY
**********************************/		
uint8_t GET8Gray(uint16_t rgb_565)
{  
		uint8_t R,G,B;
		uint16_t Gray;	
		R=GETR_FROM_RGB16(rgb_565);
		G=GETG_FROM_RGB16(rgb_565);
		B=GETB_FROM_RGB16(rgb_565);
		Gray = (uint8_t)((R*299 + G*587 + B*114)/1000);
		return Gray;
}
/**********************************
function: GET8Gray()
description: 16λRGB565��ʽͼ��ת��Ϊ24λrgb888
calls: ��
called by: ��
input: uint16_t RGB565
return: uint32_t RGB888
other: 16bit RGB656 R4 R3 R2 R1 R0 G5 G4 G3 G2 G1 G0 B4 B3 B2 B1 B0
       24ibt RGB888 R4 R3 R2 R1 R0 R2 R1 R0 G5 G4 G3 G2 G1 G0 G1 G0 B4 B3 B2 B1 B0 B2 B1 B0
**********************************/

uint32_t rgb565_2_rgb888(uint16_t RGB_565)  
{   
	  uint8_t R2R1R0,G1G0,B2B1B0,rgb888_R,rgb888_G,rgb888_B;
		uint32_t RGB_888;
		//RGB888��λ����
	  R2R1R0=(uint8_t)((RGB_565&0x3800)>>11);
    G1G0=(uint8_t)((RGB_565&0x0060)>>5);
    B2B1B0=(uint8_t)(RGB_565&0007);
		//RGB888����������
	  rgb888_R=GETR_FROM_RGB16(RGB_565)|R2R1R0;     
		rgb888_G=GETG_FROM_RGB16(RGB_565)|G1G0;  
		rgb888_B=GETB_FROM_RGB16(RGB_565)|B2B1B0;  
				
		RGB_888= rgb888_R<<16|rgb888_G<<8|rgb888_B;  
		 
		return RGB_888;
}  
/**********************************
function: rgb888_2_rgb565()
description: 24λrgb888ת��Ϊ16λRGB565��ʽ
calls: ��
called by: ��
input: uint32_t RGB888
return: uint16_t RGB565
**********************************/		

uint16_t rgb888_2_rgb565(uint32_t rgb888)  
{   
		uint16_t rgb565_R,rgb565_G,rgb565_B;
		uint16_t RGB_565;
			
		rgb565_R =(uint16_t)((rgb888&0x00FF0000)>>19);     
		rgb565_G =(uint16_t)((rgb888&0x0000FF00)>>10);  
		rgb565_B =(uint16_t)((rgb888&0x000000FF)>>3);  
				
		RGB_565=(rgb565_R<<11)|(rgb565_G<<5)|rgb565_B;  
		 
		return RGB_565;
}   
