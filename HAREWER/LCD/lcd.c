#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"

void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{
	u16 i,j;
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);
	for(i=ysta;i<yend;i++)
	{
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	}
}

void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);
	LCD_WR_DATA(color);
}

void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1;
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	if(delta_x>0)incx=1;
	else if (delta_x==0)incx=0;
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x;
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}

void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);
		LCD_DrawPoint(x0+b,y0-a,color);
		LCD_DrawPoint(x0-a,y0+b,color);
		LCD_DrawPoint(x0-a,y0-b,color);
		LCD_DrawPoint(x0+b,y0+a,color);
		LCD_DrawPoint(x0+a,y0-b,color);
		LCD_DrawPoint(x0+a,y0+b,color);
		LCD_DrawPoint(x0-b,y0+a,color);
		a++;
		if((a*a+b*b)>(r*r))
		{
			b--;
		}
	}
}

void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t;
	u16 i,TypefaceNum;
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=sizex/8*sizey;
	num=num-' ';
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);
	for(i=0;i<TypefaceNum;i++)
	{
		if(sizey==16)temp=ascii_1608[num][i];
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
			}
			else
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}

void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	while(*p!='\0')
	{
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}
}

u32 mypow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)result*=m;
	return result;
}

void LCD_ShowIntNum(u16 x,u16 y,int num,u8 len,u16 fc,u16 bc,u8 sizey)
{
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	u16 abs_num;
	if(num<0)
	{
		LCD_ShowChar(x,y,'-',fc,bc,sizey,0);
		x+=sizex;
		abs_num=-num;
		len--;
	}
	else
	{
		abs_num=num;
	}
	for(t=0;t<len;t++)
	{
		temp=(abs_num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1;
		}
		LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}

void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
		LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}
