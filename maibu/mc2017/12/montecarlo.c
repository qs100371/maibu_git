/*
 * =====================================================================================
 * 
 *     Modulename:
 *       Filename:  掷色子
 *
 *    Description:  
 *    Corporation:
 * 
 *         Author:  
 *        Created:  
 *
 * =====================================================================================
 *
 * =====================================================================================
 * 
 *   MODIFICATION HISTORY :
 *    
 *		     DATE :
 *		     DESC :
 * =====================================================================================
 */	
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "maibu_sdk.h"
#include "maibu_res.h"

static uint32_t TEXT_KEY = 0x2002;

static GRect frame1={{5,0},{20,118}};
static GRect frame2={{5,25},{40,118}};
static GRect frame3={{5,61},{45,118}};
static GRect frame4={{5,106},{20,118}};
static int8_t g_layer1=-1,g_layer2=-1,g_layer3=-1,g_layer4=-1;
static int8_t N;
static int32_t seed;

static int32_t myrand(int32_t x)
{
      seed=(seed*129+1)%65536;
      
      return seed*x/65536;
}

/*定义向上按键事件*/
void scroll_select_up(void *context)
{
	
	P_Window wtmp = (P_Window)context;
	//app_window_set_down_button(wtmp);	
	if (N<8)
	{
		N++;
		//刷新layer1
		P_Layer p_layer = NULL;	
		char buff[20]={0};
		//刷新layer1
		p_layer = app_window_get_layer_by_id(wtmp, g_layer1);
	    if (p_layer == NULL)
	   {
		return;
	   }
	   sprintf(buff,"%d选1",N);
	   app_layer_set_text_text(p_layer,buff);
	   
	   //清空layer3,4
			
	   p_layer = app_window_get_layer_by_id(wtmp, g_layer3);
	    if (p_layer == NULL)
	   {
		return;
	   }
		
	   sprintf(buff," ");
	   app_layer_set_text_text(p_layer,buff);
	
	   p_layer = app_window_get_layer_by_id(wtmp, g_layer4);
	    if (p_layer == NULL)
	   {
		return;
	   }
	   sprintf(buff,"  ");
	   app_layer_set_text_text(p_layer,buff);
	   
	   app_window_update(wtmp);
	}
    return;	
	
	
}


/*定义向下按键事件*/
void scroll_select_down(void *context)
{
	P_Window wtmp = (P_Window)context;
	//app_window_set_up_button(wtmp);
    if (N>2)
	{   
        P_Layer p_layer = NULL;	
		char buff[50]={0};
		N--;
		//刷新layer1
		p_layer = app_window_get_layer_by_id(wtmp, g_layer1);
	    if (p_layer == NULL)
	   {
		return;
	   }
	   sprintf(buff,"%d选1",N);
	   app_layer_set_text_text(p_layer,buff);
	   
	   
	   //清空layer3,4
			
	   p_layer = app_window_get_layer_by_id(wtmp, g_layer3);
	    if (p_layer == NULL)
	   {
		return;
	   }
		
	   sprintf(buff," ");
	   app_layer_set_text_text(p_layer,buff);
	
	   p_layer = app_window_get_layer_by_id(wtmp, g_layer4);
	    if (p_layer == NULL)
	   {
		return;
	   }
	   sprintf(buff,"  ");
	   app_layer_set_text_text(p_layer,buff);
	   app_window_update(wtmp);
	   
	   
	}	
}

/*定义确定按键事件*/
void scroll_select_select(void *context)
{
	P_Window wtmp = (P_Window)context;
	//app_window_set_select(wtmp);	
	P_Layer p_layer = NULL;	
	char buff[50]={0};
	p_layer = app_window_get_layer_by_id(wtmp, g_layer3);
	    if (p_layer == NULL)
	   {
		return;
	   }
	//layer3动画效果
	int8_t i,j;
	int32_t t1;
	i=0;
	do
	{	
		for(j=0;j<N;j++)
		{
			if(i>20)break;
			t1=app_get_time_number();
			do
			{
			}while(app_get_time_number()-t1>1000);
			//sprintf(buff,"%d",app_get_time_number()-t1);
			//maibu_print_log(buff);
			sprintf(buff,"%d",j);
	        app_layer_set_text_text(p_layer,buff);
			app_window_update(wtmp);
			i++;
		}
	
	}while(i<20);
	
	
	
	
	//结果显示
	
	
		
	int32_t lukynum=myrand(N)+1;
	sprintf(buff,"%d",lukynum);
	app_layer_set_text_text(p_layer,buff);
	//app_window_update(wtmp);
	//刷新layer4，显示结果
	p_layer = app_window_get_layer_by_id(wtmp, g_layer4);
	    if (p_layer == NULL)
	   {
		return;
	   }
	sprintf(buff,"神已为你选定第%d项!",lukynum);
	app_layer_set_text_text(p_layer,buff);
	app_window_update(wtmp);
	
}

/*定义后退按键事件*/
void scroll_select_back(void *context)
{
	P_Window p_window = (P_Window)context;
	app_window_stack_pop(p_window);
}


int main()
{
 //simulator_init();
 
    seed=app_get_time_number()%65536;
	
	
	/*创建窗口*/
	P_Window p_window = app_window_create();
	if (NULL == p_window)
	{
		return 0;	
	}
    char buff[100]={0};
	N=3;
	P_Layer p_layer=NULL;
	//建立N选1设定图层layer1
	
	sprintf(buff,"%d 选1:",N);
    LayerText lt1 = {buff, frame1, GAlignLeft, U_ASCII_ARIAL_16};//
	p_layer = app_layer_create_text(&lt1);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer1=app_window_add_layer(p_window, p_layer);
	
	//建立描述滚动图层layer2
	sprintf(buff,"1.吃饭 2.睡觉 3.打豆豆 ......。按确定开始");
	app_persist_create(TEXT_KEY, sizeof(buff));
	app_persist_write_data_extend(TEXT_KEY,buff,sizeof(buff));
	int16_t total_bytes = app_persist_get_data_size(TEXT_KEY);
	LayerScroll ls={frame2,TEXT_KEY,strlen(buff),U_GBK_SIMSUN_12,1};
	p_layer=app_layer_create_scroll(&ls);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer2=app_window_add_layer(p_window,p_layer); 
	
	
	//建立动画文字显示图层layer3
	sprintf(buff," ");
    LayerText lt3 = {buff, frame3, GAlignCenter, U_ASCII_ARIAL_42};//
	p_layer = app_layer_create_text(&lt3);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer3=app_window_add_layer(p_window, p_layer);
	
	
	
	//建立结果描述图层layer4
	sprintf(buff," ");
    LayerText lt4 = {buff, frame4, GAlignLeft, U_GBK_SIMSUN_12};//
	p_layer = app_layer_create_text(&lt4);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer4=app_window_add_layer(p_window, p_layer);


	

	/*添加按键事件，实现上下翻页功能*/
	app_window_click_subscribe(p_window, ButtonIdDown, scroll_select_down);
	app_window_click_subscribe(p_window, ButtonIdUp, scroll_select_up);
	app_window_click_subscribe(p_window, ButtonIdSelect, scroll_select_select);
	app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);

	/*把窗口放入窗口栈中显示*/
	app_window_stack_push(p_window);
	
	
//simulator_wait();

	return 0;
}
