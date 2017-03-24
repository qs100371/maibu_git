/*
 * =====================================================================================
 * 
 *     Modulename:
 *       Filename:  tang.c
 *
 *    Description:  唐诗表盘
 *    Corporation:  
 * 
 *         Author:  qs100371 
 *        Created:  2017年1月13日
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



#include "maibu_sdk.h"
#include "maibu_res.h"

#define MAX_LINE 851


static uint32_t CONTENT_KEY = 0x2002;
/*时间图层，图片图层位置*/
static GRect bgframe={{0,0},{128,128}};
static GRect timeframe={{0,0},{30,110}};

static GRect contentframe={{0,30},{98,110}};
//当前位置
static uint16_t offset=0,currentline=0,nextoffset=0;
static char content[300]={0};


/*图层句柄*/
static uint16_t g_layer_bg=-1,g_layer_time=-1,g_layer_content=-1;

/*窗口句柄*/
static uint32_t g_window=-1;


static void getpoem()
{
	//
	uint16_t i,j;
	memcpy(content,0,300);
    if(maibu_read_user_file(MY_FILE_KEY, offset, content, 300)<1) 
	{
		sprintf(content,"read data error!");
		app_persist_write_data_extend(CONTENT_KEY,content,strlen(content));
		return;
	};
	
	j=0;	
	for(i=0;i<strlen(content);i++)
	   {
		   if(content[i]=='\n'||content[i]==EOF) j+=1;
		   if(j==4)
		   {
			    break;
		   }
	   }
	content[i]='\0';
	
	currentline+=j;
	if (currentline<MAX_LINE) nextoffset=offset+i+1;
	if (i==strlen(content)&&j<4) //文件结尾
	{
		currentline=0;
		offset=0;
		nextoffset=0;
	}
	
	//app_persist_create(CONTENT_KEY, sizeof(content));		   
	app_persist_write_data_extend(CONTENT_KEY,content,strlen(content));
}

/*
 *--------------------------------------------------------------------------------------
 *     function:  app_watch_update
 *    parameter: 
 *       return:
 *  description:  更新图层
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static void app_watch_update()
{
	P_Window p_window = NULL;
	P_Layer p_layer = NULL;	
	GBitmap bitmap;	
	
	struct date_time dt;
    app_service_get_datetime(&dt);
	
	/*根据窗口ID获取窗口句柄*/
	p_window = app_window_stack_get_window_by_id(g_window);
	if (p_window == NULL) return;
	
	//刷新背景
	p_layer = app_window_get_layer_by_id(p_window, g_layer_bg);
	if (p_layer == NULL) return;
	
	res_get_user_bitmap(BG, &bitmap);
	app_layer_set_bitmap_bitmap(p_layer, &bitmap);	
	
	
	//显示时间
	char buff[20]={0};
	sprintf(buff,"%02d:%02d",dt.hour,dt.min);

	p_layer = app_window_get_layer_by_id(p_window, g_layer_time);
	if (p_layer == NULL) return;
	
	app_layer_set_text_text(p_layer,buff);
	
	if(strlen(content)==0) getpoem();
		
	
	//显示滚动文本
	p_layer = app_window_get_layer_by_id(p_window, g_layer_content);
	if (p_layer== NULL) return;
	
	LayerScroll ls={contentframe,CONTENT_KEY,strlen(content),U_GBK_SIMSUN_12,2};
	P_Layer p_layer_new=app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer_new, GColorBlack);
	app_window_replace_layer(p_window,p_layer,p_layer_new);
	
			
	/*窗口显示*/	
	app_window_update(p_window);
	
}


/*
 *--------------------------------------------------------------------------------------
 *     function:  app_watch_time_change 
 *    parameter: 
 *       return:
 *  description:  系统时间有变化时，更新时间图层
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static void app_watch_time_change(enum SysEventType type, void *context)
{
    /*时间更改*/
	if (type == SysEventTypeTimeChange)
	{
		app_watch_update();	
	}
}


/*定义后退按键事件*/
void scroll_select_back(void *context)
{
	P_Window p_window = (P_Window)context;
    //
	offset=nextoffset;
	getpoem();
	app_watch_update();
	
	/*窗口显示*/	
	
}

/*定义确定按键事件*/
void scroll_select_select(void *context)
{
	P_Window p_window = (P_Window)context;
    P_Layer p_layer = app_window_get_layer_by_id(p_window, g_layer_content);
	if (p_layer== NULL) return;
	app_window_set_current_selected_layer(p_window,p_layer);
	app_window_set_down_button(p_window);
	
	
	/*窗口显示*/	
	
}

/*
 *--------------------------------------------------------------------------------------
 *     function:  init_watch
 *    parameter: 
 *       return:
 *  description:  生成表盘窗口
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static P_Window init_watch(void)
{
	P_Window p_window = NULL;
	P_Layer p_layer=NULL;
    
	char buff[10]={0};
	
	/*创建一个窗口*/
	p_window = app_window_create();
	if (NULL == p_window) return NULL;
	
	
    //刷新背景
	
	GBitmap bitmap;
	res_get_user_bitmap(BG, &bitmap);
	LayerBitmap layerbmp = { bitmap, bgframe, GAlignLeft};	
	p_layer = app_layer_create_bitmap(&layerbmp);
	g_layer_bg=app_window_add_layer(p_window, p_layer);
	
    //显示时间
    struct date_time dt;
	app_service_get_datetime(&dt);
	
	sprintf(buff,"%02d:%02d",dt.hour,dt.min);
    LayerText timetext = {buff, timeframe, GAlignCenter, U_ASCII_ARIAL_24};//
	p_layer = app_layer_create_text(&timetext);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_time=app_window_add_layer(p_window, p_layer);		
	
	
	
	//显示滚动文本
	if (strlen(content)==0) sprintf(content,"唐诗三百首\n全本共有三百又二十首诗，由蘅塘退士选辑。此为第一卷80首。按<后退键>向下翻页。\nBy qs100371.");
	//
	app_persist_create(CONTENT_KEY, 300);
	app_persist_write_data_extend(CONTENT_KEY,content,strlen(content));
	LayerScroll ls={contentframe,CONTENT_KEY,strlen(content),U_GBK_SIMSUN_12,2};
	p_layer=app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_content=app_window_add_layer(p_window,p_layer); 
	
	
    
    	
	/*注册一个事件通知回调，当有时间改变时，立即更新时间*/
	maibu_service_sys_event_subscribe(app_watch_time_change);
	/*添加按键事件*/
	app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);
	app_window_click_subscribe(p_window, ButtonIdSelect, scroll_select_select);
    
	return p_window;
}



/*
 *--------------------------------------------------------------------------------------
 *     function:  main
 *    parameter: 
 *       return:
 *  description:  主程序
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
int main()
{
	/*模拟器模拟显示时打开，打包时屏蔽*/
	//simulator_init();
    
	
	/*创建显示表盘窗口*/
	P_Window p_window = init_watch();
	if (p_window != NULL)
	{
		/*放入窗口栈显示*/
		
		g_window = app_window_stack_push(p_window);
	}
		
	/*模拟器模拟显示时打开，打包时屏蔽*/
	//simulator_wait();

	return 0;
}
