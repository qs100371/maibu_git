
#include <stdlib.h>
#include <stdio.h>



#include "maibu_sdk.h"
#include "maibu_res.h"

/*时间图层，图片图层位置*/


static GRect text1frame={{0,5},{18,128}};
static GRect text2frame={{0,25},{12,128}};
static GRect text3frame={{0,40},{30,128}};
static GRect text31frame={{0,70},{30,128}};
static GRect text4frame={{0,100},{20,128}};



/*图层句柄*/
static uint16_t g_layer_text1=-1,g_layer_text2=-1,g_layer_text3=-1,g_layer_text31=-1,g_layer_text4=-1;

/*窗口句柄*/
static uint32_t g_window=-1;


/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;







/*
 *--------------------------------------------------------------------------------------
 *     function:  request_web
 *    parameter: 
 *       return:
 *  description:  请求网络数据
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
void request_web()
{
	
	char param[30] = "";
	sprintf(param, "%s,%s,%s", "expect","opencode","opentime");
	
	g_comm_id_web = maibu_comm_request_web("http://f.apiplus.cn/ssq-1.json", param, 0);	
}


/*
 *--------------------------------------------------------------------------------------
 *     function:  
 *    parameter: 
 *       return:
 *  description:  接受WEB数据回调
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
void web_recv_callback(const uint8_t *buff, uint16_t size)
{
	
	char tmp[30],tmp1[30];
	//"expect":"2017027","opencode":"02,04,11,14,27,30+05","opentime":"2017-03-09 21:20:40","opentimestamp":1489065640
	
	
   
	
	
	
	/*根据窗口ID获取窗口句柄*/
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_window);	
	if (NULL == p_window) return;
	
	P_Layer p_layer_temp = app_window_get_layer_by_id(p_window, g_layer_text2);
	

	if (NULL != p_layer_temp)
	{
		maibu_get_json_str(buff, "expect", tmp,30);
		sprintf(tmp1,"第%s期",tmp);
		
		app_layer_set_text_text(p_layer_temp, tmp1);	
	}
    p_layer_temp = app_window_get_layer_by_id(p_window, g_layer_text3);
	

	if (NULL != p_layer_temp)
	{
		memset(tmp,0,sizeof(tmp)); 
		maibu_get_json_str(buff, "opencode", tmp,30);
		memset(tmp1,0,sizeof(tmp1));
		memcpy(tmp1,tmp,11);
		app_layer_set_text_text(p_layer_temp, tmp1);	
	}
	 p_layer_temp = app_window_get_layer_by_id(p_window, g_layer_text31);
	

	if (NULL != p_layer_temp)
	{
		memset(tmp1,0,sizeof(tmp1));
		memcpy(tmp1,tmp+12,9);
		app_layer_set_text_text(p_layer_temp, tmp1);	
	}
	
	p_layer_temp = app_window_get_layer_by_id(p_window, g_layer_text4);
	

	if (NULL != p_layer_temp)
	{
		memset(tmp,0,sizeof(tmp)); 
		maibu_get_json_str(buff, "opentime", tmp,30);
		memcpy(tmp+16,"开奖",6);
		app_layer_set_text_text(p_layer_temp, tmp);	
	}
	
	/*更新窗口*/	
	app_window_update(p_window);
}





void comm_result_callback(enum ECommResult result, uint32_t comm_id, void *context)
{
	

	/*如果上一次请求WEB通讯失败，并且通讯ID相同，则重新发送*/
	if ((result == ECommResultFail) && (comm_id == g_comm_id_web))
	{
		request_web();
	}

} 
static void select_back(void *context)
{
	
	P_Window p_window = (P_Window)context;
	app_window_stack_pop(p_window);
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
	
	
    
	
	
    //显示时间
     struct date_time dt;
	app_service_get_datetime(&dt);
	
	sprintf(buff,"双色球");
    LayerText text1 = {buff, text1frame, GAlignLeft,U_ASCII_ARIAL_14};//
	p_layer = app_layer_create_text(&text1);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text1=app_window_add_layer(p_window, p_layer);	 	
	
	
	sprintf(buff,"  ");
    LayerText text2 = {buff, text2frame, GAlignLeft,U_ASCII_ARIAL_12};//
	p_layer = app_layer_create_text(&text2);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text2=app_window_add_layer(p_window, p_layer);
		
	
    
	sprintf(buff,"  ");
	LayerText text3 = {buff, text3frame, GAlignLeft,U_ASCII_ARIAL_24};//
	p_layer = app_layer_create_text(&text3);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text3=app_window_add_layer(p_window, p_layer);
	
	sprintf(buff,"  ");
	LayerText text31 = {buff, text31frame, GAlignLeft,U_ASCII_ARIAL_24};//
	p_layer = app_layer_create_text(&text31);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text31=app_window_add_layer(p_window, p_layer);
	
	sprintf(buff,"联网查询中...");
	LayerText text4 = {buff, text4frame, GAlignLeft,U_ASCII_ARIAL_12};//
	p_layer = app_layer_create_text(&text4);
	//app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text4=app_window_add_layer(p_window, p_layer);
    
    
	
	
	
	
	/*注册接受WEB数据回调函数*/
	maibu_comm_register_web_callback(web_recv_callback);
	
	/*注册通讯结果回调*/
	maibu_comm_register_result_callback(comm_result_callback);
    app_window_click_subscribe(p_window, ButtonIdBack, select_back);
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
	
	simulator_init();
    
	
	/*创建显示表盘窗口*/
	P_Window p_window = init_watch();
	if (p_window != NULL)
	{
		/*放入窗口栈显示*/
		
		g_window = app_window_stack_push(p_window);
	}
	request_web();	
	
	simulator_wait();

	return 0;
}
