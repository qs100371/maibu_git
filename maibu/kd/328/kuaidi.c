
#include <stdlib.h>
#include <stdio.h>



#include "maibu_sdk.h"
#include "maibu_res.h"


/*图层句柄*/
static uint16_t g_layer_msg=-1;

/*窗口句柄*/
static uint32_t g_window=-1;

#define SCROLL_KEY	10

#define KUAIDI_KEY 1

#define KUAIDI_NUM 17

static char msg[400]= {0};

/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;

//快递列表
static char kuaidi_list[KUAIDI_NUM][2][30]=
{
	{"申通","shentong"},
	{"顺丰","shunfeng"},
	{"圆通","yuantong"},
	{"韵达","yunda"},
	{"中通","zhongtong"},
	{"EMS","ems"},
	{"京东","jingdong"},
	{"包裹平邮挂号信","bgpyghx"},
	{"汇通","huitong"},
	{"如风达","rufengda"},
	{"国通","guotong"},
	{"全峰","quanfeng"},
	{"天天","tiantian"},
	{"优速","youshu"},
	{"宅急送","zjs"},
	{"河北建华","hebeijianhua"},
	{"郑州建华","zzjh"}
};

static char status[10][20]= {"查询异常", "暂无记录", "在途", "派送" , "签收" , "拒收" , "疑难","无效","超时","签收失败"};


static char ad_str[]="数据由kuaidiAPI提供";

static char com_str[20]= {0};
static char nu_str[20]= {0};


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
	char url[200] = {0};

	sprintf(url, "http://www.kuaidiapi.cn/rest/?uid=86310&key=5190ba71d0b947e59fe9eab1accce1f7&order=%s&id=%s&last=true",nu_str ,com_str);

	char param[] = "status,time,content";


	g_comm_id_web = maibu_comm_request_web(url, param, 0);
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

	char tmpstr[200];
	//{"message":"ok","nu":"50442951903914","ischeck":"0","condition":"00",
	//"com":"huitongkuaidi","status":"1","state":"0","comcontact":"95320",
	//"comurl":"http://www.800bestex.com/","data":[{"time":"2017-03-24 21:58:53",
	//"context":"合肥市|发件|合肥市【合肥转运中心】，正发往【天津转运中心】","location":""}]}

	int32_t i,j;
	/* maibu_get_json_int(buff, "status",&i );
	if(i==0)
		strcat(msg,"物流单暂无结果\n");
	else if(i==2)
		strcat(msg,"接口出现异常\n");
	else
	{ */
		//查询成功
		maibu_get_json_int(buff, "status",&j );
		strcat(msg,status[j]);
		strcat(msg,"\n");
		maibu_get_json_str(buff, "time", tmpstr,sizeof(tmpstr));
		strcat(msg,tmpstr);
		strcat(msg,"\n");
		maibu_get_json_str(buff, "content", tmpstr,sizeof(tmpstr));
		strcat(msg,tmpstr);
		strcat(msg,"\n");
	//}
	strcat(msg,ad_str);





	app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

	/*显示msg*/
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_window);
	LayerScroll ls = {{{0,16},{112,128}}, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_12,2};
	P_Layer p_layer = app_layer_create_scroll(&ls);
	P_Layer p_layer_old = app_window_get_layer_by_id(p_window, g_layer_msg);
	g_layer_msg = app_window_replace_layer(p_window,p_layer_old,p_layer);


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

static void select_up(void *context)
{

	P_Window p_window = (P_Window)context;
	app_window_set_up_button(p_window);
}
static void select_down(void *context)
{

	P_Window p_window = (P_Window)context;
	app_window_set_down_button(p_window);
}

static void request_kuaidi()
{
	char tmpstr[100]= {0},com_str0[20]= {0};
	if(app_persist_read_data(KUAIDI_KEY,0,tmpstr,app_persist_get_data_size(KUAIDI_KEY))==0)
		return;
	memset(com_str,0,sizeof(com_str));
	memset(nu_str,0,sizeof(nu_str));
	int i,j;
	//快递公司名称
	for(i=0; tmpstr[i]==' '&&i<strlen(tmpstr); i++);
	for(j=i; tmpstr[j]!=' '&&j<strlen(tmpstr); j++);
	memcpy(com_str0,tmpstr+i,j-i);
	//快递单号
	for(i=j; tmpstr[i]==' '&&i<strlen(tmpstr); i++);
	for(j=i; tmpstr[j]!=' '&&j<strlen(tmpstr); j++);
	memcpy(nu_str,tmpstr+i,j-i);

	//maibu_print_log(nu_str);
	sprintf(msg,"%s\n%s\n",com_str0,nu_str);

	app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

	/*显示msg*/
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_window);
	LayerScroll ls = {{{0,16},{112,128}}, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_12,2};
	P_Layer p_layer = app_layer_create_scroll(&ls);
	P_Layer p_layer_old = app_window_get_layer_by_id(p_window, g_layer_msg);
	g_layer_msg = app_window_replace_layer(p_window,p_layer_old,p_layer);


	/*更新窗口*/
	app_window_update(p_window);

	for(i=0; i<KUAIDI_NUM; i++)
	{
		if(strstr(kuaidi_list[i][0],com_str0))
			strcpy(com_str,kuaidi_list[i][1]);
	}
	request_web();

}

static void select_select(void *context)
{

	P_Window p_window = (P_Window)context;
	//查询快递
	request_kuaidi();

}

void watchapp_comm_callback(enum ESyncWatchApp type, uint8_t *context, uint16_t context_len)
{
	if(type == ESyncWatchAppUpdateParam)
	{

		if(context_len > 0)
		{
			int i;
			for(i=0; context[i]!='\n'&& i<context_len; i++);
			app_persist_write_data_extend(KUAIDI_KEY,context,i);
			request_kuaidi();
		}
		
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


	//char buff[30]= {0};

	/*创建一个窗口*/
	p_window = app_window_create();
	if (NULL == p_window) return NULL;
	app_plug_status_bar_create(p_window,NULL,NULL,NULL);
	app_plug_status_bar_add_time(p_window);
	app_plug_status_bar_add_battery(p_window);
	app_plug_status_bar_add_ble(p_window);
    
	
    
	strcpy(msg,"快递去哪儿了?\n支持:");
	int i;
	for(i=0; i<KUAIDI_NUM; i++)
	{
		strcat(msg,kuaidi_list[i][0]);
		strcat(msg,",");
	}
	strcat(msg,"\n");
	strcat(msg,ad_str);

	app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

	LayerScroll ls = {{{0,16},{112,128}}, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_12,2};
	P_Layer layer = app_layer_create_scroll(&ls);
	g_layer_msg=app_window_add_layer(p_window, layer);

    
	/*注册接受WEB数据回调函数*/
	maibu_comm_register_web_callback(web_recv_callback);

	/*注册通讯结果回调*/
	maibu_comm_register_result_callback(comm_result_callback);


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
	app_persist_create(KUAIDI_KEY, 100);

	/* char tmp[]="圆通 884573866664145695";
	app_persist_write_data_extend(KUAIDI_KEY,tmp,sizeof(tmp)); 
 */
	/*创建显示表盘窗口*/
	P_Window p_window = init_watch();
	if (p_window != NULL)
	{
		/*放入窗口栈显示*/

		g_window = app_window_stack_push(p_window);
	}
    //注册手机设置回调函数
	maibu_comm_register_watchapp_callback(watchapp_comm_callback);
	
	if(app_persist_get_data_size(KUAIDI_KEY)>10)
	{
		request_kuaidi();
	}


	//simulator_wait();

	return 0;
}
