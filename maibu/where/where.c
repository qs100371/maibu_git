
#include <stdlib.h>
#include <stdio.h>



#include "maibu_sdk.h"
#include "maibu_res.h"


/*图层句柄*/
static uint16_t g_layer_msg=-1;

/*窗口句柄*/
static uint32_t g_window=-1;

#define SCROLL_KEY	10

static char msg[200]= {0};

/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;

/*GPS及海拔数据结构*/
static SGpsAltitude g_gps_altitude;

/*Phone通讯ID*/
static uint32_t g_comm_id_gps = 0;

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
	char url[200] = "";
	sprintf(url, "http://api.go2map.com/engine/api/regeocoder/json?type=1&contenttype=utf8&points=%s,%s",g_gps_altitude.lon ,g_gps_altitude.lat);

	char param[50] = "province,city,district,road,address,name";


	g_comm_id_web = maibu_comm_request_web(url, param, 0);
}

/*
 *--------------------------------------------------------------------------------------
 *     function:  weather_phone_recv_callback
 *    parameter:
 *       return:
 *  description:  接受手机数据回调
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
void phone_recv_callback(enum ERequestPhone type, void *context)
{
	//uint8_t* ptr = context;
	//uint8_t i = 0;
	/*如果不是GPS海拔数据， 退出*/
	if (type != ERequestPhoneGPSAltitude)
	{
		return;
	}

	/*提取经度纬度*/
	memcpy(&g_gps_altitude, (SGpsAltitude *)context, sizeof(SGpsAltitude));
    float al,ac;
	maibu_get_altitude (&al, &ac);
	char buff[40]={0};
	/*memset(msg,0,sizeof(msg));
	strcat(msg,"经度:");
	memcpy(buff,g_gps_altitude.lon,8);
	strcat(msg,buff);
	strcat(msg,"\n纬度:  ");
	memset(buff,0,sizeof(buff));
	memcpy(buff,g_gps_altitude.lat,7);
	strcat(msg,buff);
	strcat(msg,"\n"); */
	sprintf(msg,"经度:%s\n纬度:%s",g_gps_altitude.lon,g_gps_altitude.lat);
	sprintf(buff,"\n海拔:%f\n",al);
	strcat(msg,buff);
    
	app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

	/*显示msg*/
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_window);
	LayerScroll ls = {{{0,16},{112,128}}, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_14,2};
	P_Layer p_layer = app_layer_create_scroll(&ls);
	P_Layer p_layer_old = app_window_get_layer_by_id(p_window, g_layer_msg);
	g_layer_msg = app_window_replace_layer(p_window,p_layer_old,p_layer);

	/*更新窗口*/
	app_window_update(p_window);
	/*请求Web*/
	request_web();
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

	char tmpstr[50];
	//{"response":{"data":[{"address":"文坛路2号","name":"倪庄新村公交车站","province":"河北","pois":[],
	//"road":"西环南路","district":"海港区","y":4825837.175699507,"x":1.3306538999493986E7,
	//"city":"秦皇岛"}]},"status":"ok"}



	maibu_get_json_str(buff, "province", tmpstr,sizeof(tmpstr));
	strcat(msg,tmpstr);
	maibu_get_json_str(buff, "city", tmpstr,sizeof(tmpstr));
	strcat(msg,tmpstr);
	strcat(msg,"\n");
	maibu_get_json_str(buff, "district", tmpstr,sizeof(tmpstr));
	strcat(msg,tmpstr);
	//strcat(msg,"\n");
	maibu_get_json_str(buff, "road", tmpstr,sizeof(tmpstr));
	strcat(msg,tmpstr);
	strcat(msg,"\n");
	maibu_get_json_str(buff, "address", tmpstr,sizeof(tmpstr));
	strcat(msg,tmpstr);
	strcat(msg,"\n");
	maibu_get_json_str(buff, "name", tmpstr,sizeof(tmpstr));
	strcat(msg,tmpstr);
	//strcat(msg,"\n");

	app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

	//显示msg
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_window);
	LayerScroll ls = {{{0,16},{112,128}}, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_14,2};
	P_Layer p_layer = app_layer_create_scroll(&ls);
	P_Layer p_layer_old = app_window_get_layer_by_id(p_window, g_layer_msg);
	g_layer_msg = app_window_replace_layer(p_window,p_layer_old,p_layer);


	//更新窗口
	app_window_update(p_window);
}





void comm_result_callback(enum ECommResult result, uint32_t comm_id, void *context)
{
	if ((result == ECommResultFail) && (comm_id == g_comm_id_gps))
	{
		g_comm_id_gps = maibu_comm_request_phone(ERequestPhoneGPSAltitude,  NULL,0);
	}


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

static void select_up(void *context)
{

	P_Window p_window = (P_Window)context;
	P_Layer p_layer = app_window_get_layer_by_id(p_window, g_layer_msg);
	app_window_set_current_selected_layer(p_window,p_layer);
	app_window_set_up_button(p_window);
}
static void select_down(void *context)
{

	P_Window p_window = (P_Window)context;
	P_Layer p_layer = app_window_get_layer_by_id(p_window, g_layer_msg);
	app_window_set_current_selected_layer(p_window,p_layer);
	app_window_set_down_button(p_window);
}

static void select_select(void *context)
{

	P_Window p_window = (P_Window)context;
	//刷新gps
	g_comm_id_gps = maibu_comm_request_phone(ERequestPhoneGPSAltitude,  NULL,0);
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


	

	/*创建一个窗口*/
	p_window = app_window_create();
	if (NULL == p_window) return NULL;
	app_plug_status_bar_create(p_window,NULL,NULL,NULL);
	app_plug_status_bar_add_time(p_window);
	app_plug_status_bar_add_battery(p_window);
	app_plug_status_bar_add_ble(p_window);

	strcpy(msg,"我去哪儿了?\n\n\n\n按选择键刷新");
	app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

	LayerScroll ls = {{{0,16},{112,128}}, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_14,2};
	P_Layer layer = app_layer_create_scroll(&ls);
	//app_layer_set_bg_color(layer, GColorBlack);
	g_layer_msg=app_window_add_layer(p_window, layer);


	/*注册接受WEB数据回调函数*/
	maibu_comm_register_web_callback(web_recv_callback);
	/*注册接受手机数据回调函数*/
	maibu_comm_register_phone_callback((CBCommPhone)phone_recv_callback);
	/*注册通讯结果回调*/
	maibu_comm_register_result_callback(comm_result_callback);
	/*请求GPS数据*/
	g_comm_id_gps = maibu_comm_request_phone(ERequestPhoneGPSAltitude,  NULL,0);

	app_window_click_subscribe(p_window, ButtonIdBack, select_back);
	app_window_click_subscribe(p_window, ButtonIdUp, select_up);
	app_window_click_subscribe(p_window, ButtonIdDown, select_down);
	app_window_click_subscribe(p_window, ButtonIdSelect, select_select);
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

	//simulator_init();
	//创建储存空间
	app_persist_create(SCROLL_KEY, 300);

	/*创建显示表盘窗口*/
	P_Window p_window = init_watch();
	if (p_window != NULL)
	{
		/*放入窗口栈显示*/

		g_window = app_window_stack_push(p_window);
	}



	//simulator_wait();

	return 0;
}
