/**
  ******************************************************************************
  * @file    process.c
  * @author  ZHANG
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   绝缘子图像处理
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F767 开发板  
  ******************************************************************************
  */

#include "./process/process.h"
#include "math.h"
rough_t rough[MAX_LINE];
static uint16_t label;                      //连通域标记
uint16_t Rough_length=0,flag_canny=0;
uint16_t image2[480][800] __attribute__((at(IMAGE_FRAME_BUFFER)));				//轮廓图像数据定义在SDRAM中
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
    链表存放规律
*------------------------------*/
const int offset[8][2]={{1,0},{1,1},{0,1},{-1,1},
						 {-1,0},{-1,-1},{0,-1},{1,-1}};



/**********************************
function:get_newimage
description:二值化图像算法
calls:no
called by:no
input:阈值
output:void
**********************************/
void get_newimage(uint16_t thre)
{
	uint16_t x=0,y=0,TH=0;
	uint16_t Camera_Data;
	uint32_t i,m;
for(x=H_top; x<img_height-H_end; x++) //获得图像
	 {
		 for(y=W_top; y<img_width-W_end; y++)
		 {
	//for(x=0;x<img_height;x++)		//获得图像
	//{
	//	for(y=0;y<img_width;y++)
	//	{
		   Camera_Data=image2[x][y]; /* 从数组读出一个像素到Camera_Data变量 */			      
				if((Camera_Data&0x001f)>=thre)//二值化
					image2[x][y]=0x0000;		
				else
					image2[x][y]=0xffff;		
		}
	}
	for(x=H_top; x<img_height-H_end; x++) 
	 {
		 for(y=W_top; y<img_width-W_end; y++)
		 {
	//for(x=2;x<img_height-2;x++)				//滤波
	//{
	//	for(y=2;y<img_width-2;y++)           //遍历周边8个像素
		//{
			i= image2[x-1][y-1]+image2[x][y-1]+image2[x+1][y-1]
				+image2[x-1][y]+  image2[x][y]  +image2[x+1][y]
				+image2[x-1][y+1]+image2[x][y+1]+image2[x+1][y+1];
			if(i>=0x0003fffc)              //大于4个白点则将此点置黑
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
			if(image2[x][y]==0xffff)//如果此点为白点			
			 {				
				m= image2[x-1][y-1]+image2[x][y-1]+image2[x+1][y-1]
					+image2[x-1][y]+                +image2[x+1][y]
					+image2[x-1][y+1]+image2[x][y+1]+image2[x+1][y+1];
				if(m<=0x0003fffc)        //如果一个白点周围大于4个黑点则置为黑点
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
description:连通区域标记
calls:无
called by:无
input:
output:返回区域个数及标记矩阵imgn[][]
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
description:连通区域抽取
calls:无
called by:无
input:区域个数,阈值
output:返回小于像素小于TH的矩阵image2
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
description:追踪轮廓线
calls:无
called by:无
input:开始位置
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
边界跟踪法！！！
跟踪准则：从图像的左上角逐个点扫描，遇到边缘点时顺序跟踪，直至跟踪后续点回到起始点的位置为止。
一条线跟踪结束后，接着扫描下一个跟踪点
判断该点是否为目标点，是则把该目标点定为新起始点，逆时针转90度作为新的跟踪方向继续监测新方向的点
若不是目标点则顺时针转45度继续判断
*************************************************/
void find_rough(uint16_t first_x,uint16_t first_y)
{
	uint16_t x,y;		//临时坐标
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
	//	image2[x][y]=0xF800;			//将已经记录的点清除
		find_next=0;
		find_time=0;
		while(find_next==0)		//如果没有找到下一个点就继续
		{
			if(m>=8) m=m-8;		  //返回最初的方向了
			//到达边界了就跳过下点的检测，方向顺时针旋转45度
			if((x+offset[m][0]==480)||(x+offset[m][0]<0)||(y+offset[m][1]==800)||(y+offset[m][1]<0))
			{
				m++;
				find_time++;
			}
			else
			{
				//如果下一个点回到了起点
				if((x+offset[m][0]==rough[0].x)&&(y+offset[m][1]==rough[0].y))
				{
					return;
				}
				//如果检测到了黑点，就使检测方向逆时针旋转90度
				if(imgn[x+offset[m][0]][y+offset[m][1]]==0x0000)
				{
					rough[Rough_length].x=x;
					rough[Rough_length].y=y;
					//image2[x][y]=0xF800;
					x=x+offset[m][0];					//坐标偏移
					y=y+offset[m][1];
					Rough_length++;				//长度增加一
					if(m<2)	     // 逆时针旋转90度
						m=m+6;
					else 
						m=m-2;
					find_next=1;						//跳出循环
				}
				else				//没有黑点,方向顺时针旋转45度
				{
					m++;
					find_time++;
				}
			}
			if(find_time==8)		//如果8次都没有找到下一个黑点
			{
				x=rough[Rough_length-1].x;			//坐标偏移,返回上一个点
				y=rough[Rough_length-1].y;
				Rough_length--;								//长度减少一
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
						image2[tempx][tempy]=0xF800;//红色	
					}
				}
	}
}
/**********************************
function:calc_size()
description:区域面积及重心位置
calls:无
called by:无
input:标记号
output:面积（即像素点数）
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
description:区域周长
calls:无
called by:无
input:标记号
output:面积（即像素点数）
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
description:圆形度
calls:无
called by:无
input:cnt(对象个数),size(面积),length(周长),ratio(圆形度),重心
output:面积（即像素点数）
**********************************/
float feature(uint16_t label,uint16_t center_x,uint16_t center_y)
{
	uint16_t i,j,cx,cy;
	float L,size,length,ratio;
	size=calc_size(label,&cx,&cy); //面积
	center_x=cx;
	center_y=cy;
	
	L=calc_length(label); //周长
	length=L;
		
	ratio=4*PI*size/(L*L);   //圆形度，越接近圆值越大
	imgn[cx][cy]=0x0000;                  //标记重心
	return ratio;

}
/**********************************
function:size_extract()
description:根据面积范围抽取对象
calls:无
called by:无
input:面积，对象个数，最大最小值
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
description:根据圆形度范围抽取对象
calls:无
called by:无
input:面积，对象个数，最大最小值
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
description:测距(预先测到外接矩形根据外接矩形的宽及实际宽度测量)
calls:无
called by:无
input:阈值
output:void
**********************************/
float distance(float r,float f,uint16_t Object_W,uint16_t MAXCOL,uint16_t MINCOL)
{
	//参数为像元尺寸、焦距、物距、最大最小横坐标
	float Image_w,k,D;                         
	float a=29.4;      //固定参数
	float b=18.7;    //固定参数
	//r=0.015;          //mm  (7英寸 800×480 123mm×68mm 点距:0.15mm)
  //f=4;                             //mm相机焦距
  //Image_w=r*sqrt((MAXROW-MINROW)^2+(XCOL-NCOL)^2);   
  Image_w=r*(MAXCOL-MINCOL);                 //像直径
	k=f*(Object_W/Image_w);                    //物像比
  D=a*k-b;                     //mm实际距离：w/W=d/D;1/d+1/D=1推出
  return D;
}
/**********************************
function:point()
description:最小外接矩形的4个顶点的选取
calls:无
called by:无
input:顶点数组
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
   a[0]=b[0];   //最大纵坐标
   a[1]=b[1];   //最小纵坐标
	 a[2]=b[2];    //最大横坐标
   a[3]=b[3];  	  //最小横坐标
}
void point2(uint16_t a[4])
{
	uint16_t i,j,b[4]={0,0,0,0};
	for(i=H_top+10; i<img_height-H_end-10; i++) 
	 {
		 for(j=W_top+10; j<img_width-W_end-10; j++)
		 {
			if(image2[i][j]==0x0000)//使用图像上的第一个点作为对比点
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
   a[0]=b[0];   //最大纵坐标
   a[1]=b[1];   //最小纵坐标
	 a[2]=b[2];    //最大横坐标
   a[3]=b[3];  	  //最小横坐标
}
/**********************************
function:diagonal()
description:抓取角度及弧度
calls:无
called by:无
input:顶点数组
output:夹角
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
	  maopao(b,i);             //由小到大排序	 
	  a[0]=b[i-1];   //上顶点最大横坐标
		a[1]=b[0];     //上顶点最小横坐标
		k=(a[0]-a[1])/(a[2]-a[3]);
		r=m*k+n;    //弧度
		diag=(r/PI)*180;    //角度    
		return diag;
}
/**********************************
function:Houghtrans()
description:Hough变换检测圆
            (1)建立一个2维累加数组，设为a[Theta][r]
            (2)开始设置数组a为0,然后对每个图像空间中给定的边缘点，让Theta取遍Theta轴上所有可能值
               并根据R=x*cos(theta)+y*sin(theta)算出R
            (3)再根据Theta和R的值(整数化后),对a进行累加a[theta][r]++,累加结束后，根据theta和R的值
               可知多少点共线，即a[theta][r]的值，同蕋heta和r也给出了直线方程参数
calls:无
called by:无
input:阈值
output:void
**********************************/
void HoughTrans()  
    {  
        //x*cos(theta)+y*sin(theta)=r;   
        uint16_t x,y,indexTemp,rValue_M,theta_M,iCount; 
		  	uint16_t thetaMax=180,CounterMax=0;
			  uint16_t RMax =933; 
			  //uint16_t counters[933][180];     //累加数组
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
	//转换hough域(p,t)
				
        //得到最佳参数  
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
											 image2[x][y]=0xF800;//红色
										 else
											 image2[x][y]=0x0000;//黑色
									 }
								 else
									 image2[x][y]=0x0000;
							 }
						 }
     } 
		
////计算偏移地址
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
	uint16_t value[9];///3*3模板
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
           for(m=0;m<8;m++)///对模板卷积
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
description:canny算子边缘检测
calls:无
called by:无
input:阈值
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
//高斯滤波线性平滑
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
		//高斯算子
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
	
 //对每个像素，计算横向和纵向的微分近似以得到梯度大小及方向
 for(x=1;x<img_height-1;x++)				
	{
		for(y=1;y<img_width-1;y++)
		{
		 dy= ((   image2[x][y+1]&0x001f   - image2[x][y]&0x001f 
			    + image2[x+1][y+1]&0x001f - image2[x+1][y]&0x001f)*0.5); //y方向梯度              
     dx= ((    image2[x+1][y]&0x001f  - image2[x][y]&0x001f
			    + image2[x+1][y+1]&0x001f - image2[x][y+1]&0x001f)*0.5); //x方向梯度
     m[x][y]=my_abs(dy)+my_abs(dx);//梯度大小
		 //o[x][y]=(int16_t)arctan(dy/dx);//梯度方向(弧度)
		 m[x][y]=(m[x][y]<<11)|(m[x][y]<<6)|(m[x][y]);
		
		

			//划分方向区域
			 // 3 2 1
			 // 0 x 0
			 // 1 2 3
		  if ((o[x][y]<PI/16)&&(o[x][y]>-PI/16))//0度
			    o[x][y]=0;
      else if ((o[x][y]<PI*3/16)&&(o[x][y]>PI/16))//45度
			    o[x][y]=1;	
		  else if ((o[x][y]<PI*5/16)&&(o[x][y]>PI*3/16))//-45度
			    o[x][y]=2;	
      else	
 			    o[x][y]=3;                   //90度
			
    }
  }
	
 for(x=1;x<img_height-1;x++)
	 {
	   for(y=1;y<img_width-1;y++)
	  {
		  imgn[x][y]=0x0000;
		 }
 	 }
//非极大值抑制
//若中心点在沿其方向邻域梯度幅值最大则保留否则抑制	 
for(x=2;x<img_height-1;x++)				
	{
		for(y=2;y<img_width-1;y++)
		{
		  if(o[x][y]==0)//水平
			{
				if((m[x][y]>m[x][y-1]) && (m[x][y]>m[x][y+1]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else 
					   imgn[x][y]=0x0000;
			}
			 if(o[x][y]==1)//右上左下
			{
				if((m[x][y]>m[x+1][y-1]) && (m[x][y]>m[x-1][y+1]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else 
					   imgn[x][y]=0x0000;
			}
			 if(o[x][y]==2)//垂直
			{
				if((m[x][y]>m[x-1][y]) && (m[x][y]>m[x+1][y]))
					   imgn[x][y]=m[x][y]<<11|m[x][y]<<6|m[x][y];
			  else  
					   imgn[x][y]=0x0000;
			}
			 if(o[x][y]==3)//左上右下
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
//对梯度取两次阈值
	th1=thre;
	th2=th1*2;
	for(x=2;x<img_height-1;x++)				
	{
		for(y=2;y<img_width-1;y++)
		{
      if((m[x][y])<th1)//低阈值处理
           imgn[a][b]=0x0000;//黑
      else if((m[x][y])>th2)//高阈值处理
           imgn[a][b]=m[x][y]<<11|m[x][y]<<6|m[x][y];
          else //介于高低阈值之间的看其8邻域是否有高于高阈值的有则为边缘
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
description:LOG算子边缘检测
calls:无
called by:无
input:阈值
output:void
**********************************/
void log_process(uint16_t thre)
{
uint16_t a,b,t,c1,c2,c3,c4,c5,c6,c7,c8,c9,fxr;
uint16_t x,y,th1,th2,tem[9];
uint32_t i,ii,jj,r;
  
//高斯滤波线性平滑
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
		//高斯算子
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
//拉普拉斯锐化
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
        //t=-1*c1-1*c2-1*c4-1*c5+4*c3+0*c6+0*c7+0*c8+0*c9;//负模板2——效果可以
			  //t=-2*c1-2*c2-2*c4-2*c5+4*c3+c6+c7+c8+c9;//模板1 边缘轮廓效果好 
			  t=c1+c2+c4+c5-4*c3+0*c6+0*c7+0*c8+0*c9;//模板2 效果比1好
			  //t=c1+c2+c4+c5-8*c3+1*c6+1*c7+1*c8+1*c9;//模板3 效果一般
		    fxr=t<<11|t<<6|t;
		    image2[a][b]=fxr;
			  imgn[a][b]=fxr;
		//拉普拉斯算子   1 -2  1    0  1  0    1  1  1
		//              -2  4 -2    1 -4  1    1 -8  1
		//               1 -1  1    0  1  0    1  1  1
		//拉普拉斯锐化，对孤立点和端点更为敏感
		
		//sobel算子  -1 0 1   1 2 1
		//         y -2 0 2 x 0 0 0
		//           -1 0 1  -1-2-1
			
			if((fxr&0x001f)>=thre)//阈值ostu法选取
           image2[a][b]=0x0000;//黑 
        else
           image2[a][b]=0xffff;//白
					
      }
   }
//边界处理
//双阈值
	//th2=THR;
	//th1=th2*0.6;
	//for(x=2;x<img_height-2;x++)				
	//{
	//	for(y=2;y<img_width-2;y++)
		//{
   //   if((imgn[x][y]&0x001f)>=th2)//高阈值处理
	//		{
		//		image2[x][y]=0x0000;//黑
		//	}
   //    if((imgn[x][y]&0x001f)<=th1)//低阈值处理
		//	{
		//			image2[x][y]=0xffff;
	//		}
   //    if(th2<(image2[x][y]&0x001f)<th1)
		//	 { //介于高低阈值之间的看其8邻域是否有高于高阈值的有则为边缘
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
description: otsu自动阈值选取算法
calls: 无
called by: 无
input: void
return: 16位的阈值
**********************************/		
uint16_t otsuThreshold()
{
    const uint16_t GrayScale=0x001f;//2^5=32级灰度级
    uint16_t pixelCount[GrayScale];//灰度直方图
    uint16_t i,j,k, pixelSum=img_width*img_height,threshold = 0;
	  double N0,N1,w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp=-1, deltaMax = -1;

	  for (i=0;i<GrayScale;i++)
    {
        pixelCount[i] = 0;
    }

    //统计灰度级中每个像素在整幅图像中的个数——灰度直方图横坐标为灰度值纵坐标为个数
		for (i=0;i<img_height;i++)
    {
        for (j=0;j<img_width; j++)
        {
            pixelCount[image2[i][j]&0x001f]++;  
        }
    }

    //计算阈值
    for (i = 0;i<GrayScale;i++) 
    { 
        N0 += pixelCount[i];  //前景像素个数        
        N1 = pixelSum-N0;     //背景像素个数
        if(0==N1)break;   //前景无像素跳出
        w0 = N0/pixelSum; //前景像素比例
        w1 = 1-w0;       //背景像素比例
        for (j= 0;j<=i;j++) //i为阈值
        { 
            u0tmp += j*pixelCount[j]; //灰度总值
        } 
        u0 = u0tmp/w0; //平均灰度
        for(k= i+1;k<GrayScale;k++) 
        { 
            u1tmp += k*pixelCount[k]; 
        }
        u1 = u1tmp / w1; //平均灰度
        deltaTmp = w0*w1*(u0 - u1)*(u0 - u1);//类间方差
				if (deltaTmp > deltaMax)
        {
           deltaMax = deltaTmp;
           threshold = i;   //求最大方差对应的灰度值 
        }
    }

    return threshold;
}
/**********************************
function: OET()
description: 最大熵自动阈值选取算法
calls: 无
called by: 无
input: void
return: 16位的阈值
**********************************/		
uint16_t OET()
{
	  const uint16_t GrayScale_2=0x001f;//2^5=32级灰度级
    uint16_t pixelCount[GrayScale_2];//灰度直方图
	  uint16_t pixelgailv[GrayScale_2];//灰度概率
	  uint16_t H,HMax=0;//熵
    uint16_t i, j,k, pixelSum=img_width*img_height,threshold = 0;
	  double p1=0,p2=0,H1=0,H2=0;

	  for (i=0;i<GrayScale_2;i++)
    {
        pixelCount[i] = 0;
    }

    //统计灰度级中每个像素在整幅图像中的个数——灰度直方图横坐标为灰度值纵坐标为个数
    for (i=0;i<img_height;i++)
    {
        for (j=0;j<img_width; j++)
        {
            pixelCount[image2[i][j]&0x001f]++;  
        }
    }
    //统计灰度级中每个像素在整幅图像中的概率——灰度直方图横坐标为灰度值纵坐标为个数
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
           threshold = i;   //求最大方差对应的灰度值 
        }
    } 
 return threshold;
}
/**********************************
function: MeanFilterOper()
description: 加权均高斯滤玻(灰度图)---模糊处理
calls: 无
called by: 无
input: void
return: void
**********************************/	
void MeanFilterOper()
{
	uint16_t x,y,i[9],t=0;
	for(x=2;x<img_height-2;x++)				
	{
		for(y=2;y<img_width-2;y++)           //遍历周边8个像素
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
			//t=(1*i[0]+3*i[1]+1*i[2]+3*i[3]+6*i[4]+3*i[5]+1*i[6]+3*i[7]+1*i[8])/22;//效果好
			t=(i[0]+2*i[1]+i[2]+2*i[3]+4*i[4]+2*i[5]+i[6]+2*i[7]+i[8])/16;//高斯模板平滑效果好
			//加权平滑模板
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
description: K近邻均值滤波(灰度图)k选5
calls: 无
called by: 无
input: void
return: void
**********************************/	
void KNNFilterOper()
{
	uint16_t x,y,z,j,i[8],k[8],t=0,temp=0;
 for(x=2;x<img_height-2;x++)				
	{
		for(y=2;y<img_width-2;y++)           //遍历周边8个像素
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
			//选k个与待处理像素的灰度差最小的像素，将该k个像素的均值代替待处理均值
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
description: 灰度图像的中值滤波 3*3---模糊处理
calls: maopao()
called by: 无
input: void
return: void
**********************************/		
void MedianFilterOper()
{
	uint16_t x,y,i[9];
 for(x=2;x<img_height-2;x++)				//中值滤波
	{
		for(y=2;y<img_width-2;y++)     		//遍历周边8个像素
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
description: 直方图均衡化
calls: sort(),weight()
called by: 无
input: void
return: void
**********************************/	
void Hist_plane()
{
    uint16_t i,j;
    uint16_t hist[0x001f];          //灰度直方图
    float histPDF[0x001f];    //归一化直方图
	  float histCDF[0x001f];    //累积直方图
	  uint16_t histEQU[0x001f];    //直方图均衡化
		uint16_t size=img_width*img_height;
  
    //灰度直方图  
   	for(i=H_top; i<img_height-H_end; i++) //获得图像
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
   
		//归一化直方图 
    for (i=0; i<0x001f; i++)
    {  
 			histPDF[i]=(float)hist[i]/size;
    }  
    
		//累积直方图
  	for (i=0; i<0x001f; i++)
    {  
       if (0==i)
				 histCDF[i] = histPDF[i];  
       else 
				 histCDF[i] = histCDF[i-1] + histPDF[i];  
    }  
    
		//直方图均衡化
    for (i=0; i<0x001f; i++)
    {  
       histEQU[i] = (uint16_t)(31.0 * histCDF[i] + 0.5);  
    }  
  	for(i=H_top; i<img_height-H_end; i++) //获得图像
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
description: 膨胀运算
calls: 无
called by: 无
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
  for(i=H_top+1; i<img_height-H_end-1; i++) //获得图像
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
                    //自身及邻域有一个为1，则将该点设置为1
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
description: 腐蚀运算
calls: 无
called by: 无
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
  for(i=H_top+1; i<img_height-H_end-1; i++) //获得图像
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
                    //自身及邻域有一个为0，则将该点设置为0
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
description: 闭运算（先膨胀再腐蚀）去除暗细节部分
calls: 无
called by: 无
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
description: 开运算（先腐蚀再膨胀）去除较小亮细节
calls: 无
called by: 无
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
description: 冒泡排序(由小到大)
calls: maopao()
called by: 无
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
description: 求绝对值
calls: 无
called by: 无
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
description:求x的n次方函数
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
description:最大最小值函数
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
description: RGB565格式转灰度图像(RGB565格式显示)
calls: 无
called by: 无
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
		t=(R*30+G*60+B*10)/100;//转为灰度值(RGB565格式显示)t=(R*1.1+G*5.9+3*B)/10;R = G = B = 0.3R + 0.6G + 0.1B; 
		return (t<<11)|(t<<6)|t;	
} 

/**********************************
function: GET8Gray()
description: 16位RGB565格式转8位灰度
calls: 无
called by: 无
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
description: 16位RGB565格式图像转换为24位rgb888
calls: 无
called by: 无
input: uint16_t RGB565
return: uint32_t RGB888
other: 16bit RGB656 R4 R3 R2 R1 R0 G5 G4 G3 G2 G1 G0 B4 B3 B2 B1 B0
       24ibt RGB888 R4 R3 R2 R1 R0 R2 R1 R0 G5 G4 G3 G2 G1 G0 G1 G0 B4 B3 B2 B1 B0 B2 B1 B0
**********************************/

uint32_t rgb565_2_rgb888(uint16_t RGB_565)  
{   
	  uint8_t R2R1R0,G1G0,B2B1B0,rgb888_R,rgb888_G,rgb888_B;
		uint32_t RGB_888;
		//RGB888补位运算
	  R2R1R0=(uint8_t)((RGB_565&0x3800)>>11);
    G1G0=(uint8_t)((RGB_565&0x0060)>>5);
    B2B1B0=(uint8_t)(RGB_565&0007);
		//RGB888各分量计算
	  rgb888_R=GETR_FROM_RGB16(RGB_565)|R2R1R0;     
		rgb888_G=GETG_FROM_RGB16(RGB_565)|G1G0;  
		rgb888_B=GETB_FROM_RGB16(RGB_565)|B2B1B0;  
				
		RGB_888= rgb888_R<<16|rgb888_G<<8|rgb888_B;  
		 
		return RGB_888;
}  
/**********************************
function: rgb888_2_rgb565()
description: 24位rgb888转换为16位RGB565格式
calls: 无
called by: 无
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
