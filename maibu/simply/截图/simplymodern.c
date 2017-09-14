/*
 * =====================================================================================
 * 
 *     Modulename:
 *       Filename:  
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



#include "maibu_sdk.h"
#include "maibu_res.h"

/*时间图层，图片图层位置*/
//static GRect bgframe={{0,0},{128,128}};
static GRect text0frame={{0,0},{12,128}};
static GRect timeframe={{0,12},{42,128}};
static GRect text1frame={{0,54},{30,128}};

static GRect text2frame={{0,104},{24,30}};
static GRect text3frame={{30,98},{16,50}};
static GRect text4frame={{30,114},{14,40}};

static GRect text5frame={{82,104},{24,30}};
static GRect text6frame={{112,98},{16,16}};
static GRect text7frame={{112,114},{14,16}};

/*图层句柄*/
static uint16_t g_layer_bg=-1,g_layer_time=-1,g_layer_text0=-1,g_layer_text1=-1,g_layer_text2=-1,
                g_layer_text3=-1,g_layer_text4=-1,g_layer_text5=-1,g_layer_text6=-1,g_layer_text7=-1;

/*窗口句柄*/
static uint32_t g_window=-1;


static SportData g_sport_data;
const static char wday_str[7][8]={"周日","周一","周二","周三","周四","周五","周六"}; 


/*
 *--------------------------------------------------------------------------------------
 *     function:  watch_update
 *    parameter: 
 *       return:
 *  description:  更新图层
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static void watch_update()
{
	P_Window p_window = NULL;
	P_Layer p_layer = NULL;	
	GBitmap bitmap;	
	
	struct date_time dt;
    app_service_get_datetime(&dt);
	SLunarData sld={0};
	
	/*根据窗口ID获取窗口句柄*/
	p_window = app_window_stack_get_window_by_id(g_window);
	if (p_window == NULL) return;
	
	//刷新背景
	/* p_layer = app_window_get_layer_by_id(p_window, g_layer_bg);
	if (p_layer == NULL) return;
	
	res_get_user_bitmap(BG, &bitmap);
	app_layer_set_bitmap_bitmap(p_layer, &bitmap);	 */
	
	
	//显示时间
	char buff[20]={0};
	sprintf(buff,"%02d:%02d",dt.hour,dt.min);

	p_layer = app_window_get_layer_by_id(p_window, g_layer_time);
	if (p_layer == NULL) return;	
	app_layer_set_text_text(p_layer,buff);
	
	
	SportData data;
	maibu_get_sport_data(&data, 0);
	
	uint16_t deltastep=data.step-g_sport_data.step;
	if (deltastep<80)
	{   //显示正常界面
        
		if (dt.min %2)
	    sprintf(buff,"%d-%d%s",dt.mon,dt.mday,wday_str[dt.wday]);
        else
	   {
		maibu_get_lunar_calendar(NULL,&sld);
		sprintf(buff,"%s%s",sld.mon,sld.day);
	   }
	    p_layer = app_window_get_layer_by_id(p_window, g_layer_text1);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
	
		sprintf(buff,"%d",data.floor);
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text2);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"%d",data.step);
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text3);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"Steps");
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text4);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		float temp;
	    maibu_get_temperature(&temp);
		if(temp<36&&temp>10) temp=temp-10;
	
	    sprintf(buff,"%d",(int)temp);
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text5);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"。");
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text6);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"C");
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text7);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
	}
	else
	{
		//显示运动界面
		sprintf(buff,"%d/%dm",data.step,data.distance/100);
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text1);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		//SpeedInfo si;
		//maibu_get_speed_info(&si);
		//sprintf(buff,"%d",3.6*si.real_time_speed/100);
		sprintf(buff,"%d",(data.distance-g_sport_data.distance)*60/10000);
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text2);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"%d",deltastep);
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text3);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"Speed");
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text4);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
			
	    //sprintf(buff,"%d",100000/(si.real_time_speed*60));
		sprintf(buff,"%d",100000/(data.distance-g_sport_data.distance));
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text5);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"'");
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text6);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
		
		sprintf(buff,"p");
		p_layer = app_window_get_layer_by_id(p_window, g_layer_text7);
	    if (p_layer == NULL) return;
	    app_layer_set_text_text(p_layer,buff);
	}
	//uint16_t k=data.step/1000;
	
	
	g_sport_data=data;		
	/*窗口显示*/	
	app_window_update(p_window);
	
}


/*
 *--------------------------------------------------------------------------------------
 *     function:  watch_time_change 
 *    parameter: 
 *       return:
 *  description:  系统时间有变化时，更新时间图层
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static void watch_time_change(enum SysEventType type, void *context)
{
    /*时间更改*/
	if (type == SysEventTypeTimeChange)
	{
		//watch_update();	
	}
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
    
	char buff[30]={0};
	
	/*创建一个窗口*/
	p_window = app_window_create();
	if (NULL == p_window) return NULL;
	
	
    //刷新背景
	
	/* GBitmap bitmap;
	res_get_user_bitmap(BG, &bitmap);
	LayerBitmap layerbmp = { bitmap, bgframe, GAlignLeft};	
	p_layer = app_layer_create_bitmap(&layerbmp);
	g_layer_bg=app_window_add_layer(p_window, p_layer); */
	
    //显示时间
    struct date_time dt;
	app_service_get_datetime(&dt);
	
	sprintf(buff,"17:04");
    LayerText timetext = {buff, timeframe, GAlignCenter,U_ASCII_ARIAL_42};//
	p_layer = app_layer_create_text(&timetext);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_time=app_window_add_layer(p_window, p_layer);		
	
	
	sprintf(buff,"89999/60299m");
    LayerText text1 = {buff, text1frame, GAlignCenter,U_ASCII_ARIAL_16};//
	p_layer = app_layer_create_text(&text1);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text1=app_window_add_layer(p_window, p_layer);
		
	SportData data;
	maibu_get_sport_data(&data, 0);
	g_sport_data = data;
    
	sprintf(buff,"65");
	LayerText text2 = {buff, text2frame, GAlignRight,U_ASCII_ARIAL_24};//
	p_layer = app_layer_create_text(&text2);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text2=app_window_add_layer(p_window, p_layer);
	
	sprintf(buff,"125");
	LayerText text3 = {buff, text3frame, GAlignLeft,U_ASCII_ARIAL_16};//
	p_layer = app_layer_create_text(&text3);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text3=app_window_add_layer(p_window, p_layer);
    
    sprintf(buff,"Speed");
	LayerText text4 = {buff, text4frame, GAlignLeft,U_ASCII_ARIAL_12};//
	p_layer = app_layer_create_text(&text4);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text4=app_window_add_layer(p_window, p_layer);
	
	float temp;
	maibu_get_temperature(&temp);
	if(temp<36&&temp>10) temp=temp-10;
	
	sprintf(buff,"10");
	LayerText text5 = {buff, text5frame, GAlignRight,U_ASCII_ARIAL_24};//
	p_layer = app_layer_create_text(&text5);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text5=app_window_add_layer(p_window, p_layer);
	
	sprintf(buff,"'");
	LayerText text6 = {buff, text6frame, GAlignLeft,U_ASCII_ARIAL_12};//
	p_layer = app_layer_create_text(&text6);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text6=app_window_add_layer(p_window, p_layer);
	
	sprintf(buff,"p");
	LayerText text7 = {buff, text7frame, GAlignLeft,U_ASCII_ARIAL_12};//
	p_layer = app_layer_create_text(&text7);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text7=app_window_add_layer(p_window, p_layer);
	
	//uint16_t k=data.step/1000;
	
	
	/* LayerText text0 = {buff, text0frame, GAlignLeft,U_ASCII_ARIAL_12};//
	p_layer = app_layer_create_text(&text0);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text0=app_window_add_layer(p_window, p_layer); */
	
	/*注册一个事件通知回调，当有时间改变时，立即更新时间*/
	maibu_service_sys_event_subscribe(watch_time_change);
	
    
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
	simulator_init();
    
	
	/*创建显示表盘窗口*/
	P_Window p_window = init_watch();
	if (p_window != NULL)
	{
		/*放入窗口栈显示*/
		
		g_window = app_window_stack_push(p_window);
	}
		
	/*模拟器模拟显示时打开，打包时屏蔽*/
	simulator_wait();

	return 0;
}
