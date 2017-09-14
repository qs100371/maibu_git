
#include <stdlib.h>
#include <stdio.h>



#include "maibu_sdk.h"
#include "maibu_res.h"


/*图层句柄*/
static uint16_t g_layer_msg = -1;

/*窗口句柄*/
static uint32_t g_window = -1;

#define SCROLL_KEY	10

#define KUAIDI_KEY 1

#define KUAIDI_NUM 17

static unsigned char msg[400] = {0};
static GRect msgframe = {{0, 20}, {156, 176}};
/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;

//快递列表
static char kuaidi_list[KUAIDI_NUM][2][30] =
{
    {"申通", "shentong"},
    {"顺丰", "shunfeng"},
    {"圆通", "yuantong"},
    {"韵达", "yunda"},
    {"中通", "zhongtong"},
    {"EMS", "ems"},
    {"京东", "jd"},
    {"邮政", "youzhengguonei"},
    {"汇通", "huitongkuaidi"},
    {"如风达", "rufengda"},
    {"国通", "guotongkuaidi"},
    {"全峰", "quanfengkuaidi"},
    {"天天", "tiantian"},
    {"优速", "youshuwuliu"},
    {"宅急送", "zhaijisong"},
    {"河北建华", "hebeijianhua"},
    {"郑州建华", "zhengzhoujianhua"}
};

static char status[10][20] = {"暂无结果", " ", " ", "在途" , "揽件" , "拒收" , "签收", "退签", "派件", "退回"};


static char ad_str[] = "数据由kuaidi.com提供";

static char com_str[20] = {0};
static char nu_str[20] = {0};

static P_Window init_watch(void);


void window_reloading(void)
{
    /*根据窗口ID获取窗口句柄*/
    P_Window p_old_window = (P_Window) app_window_stack_get_window_by_id(g_window);
    if (NULL != p_old_window)
    {
        P_Window p_window = init_watch();
        if (NULL != p_window)
        {
            g_window = app_window_stack_replace_window(p_old_window, p_window);
        }
    }

}
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
    //http://api.kuaidi.com/openapi.html?id=b9b531008d97bd644ecf5611eadfe593&com=%s&nu=%s&show=0&muti=1&order=desc
    sprintf(url, "http://api.sqqbd.com/h/kd?com=%s&nu=%s&show=0&muti=1&order=desc", com_str, nu_str);

    char param[] = "time,context,status";


    g_comm_id_web = maibu_comm_request_web(url, param, 0);
}

/* static int hex2int(char c)
{

    return (c >= 'a' ? (10 + c - 'a') : (c - '0'));
}


static unsigned char *unicode2utf8(unsigned char *str)
{
    //  \u3010\u5df2\u7b7e\u6536\uff0c\u7b7e\u6536\u4eba\u662f\u5feb\u9012\u9a7f\u7ad9\u3011
    unsigned char utf8[4], *out;
    int i, j = strlen(str);
	unsigned long ch;
    out = (unsigned char *)malloc(200);
    memset(out,0,sizeof(out));

    for (i = 0; 6*i < j; i++)
    {
        //ch = hex2int(*(str + 6 * i + 2)) << 12 + hex2int(*(str + 6 * i + 3)) << 8 + hex2int(*(str + 6 * i + 4)) << 4 + hex2int(*(str + 6 * i + 5));
        ch = hex2int(*(str + 6 * i + 2)) *16*16*16 + hex2int(*(str + 6 * i + 3)) *16*16 + hex2int(*(str + 6 * i + 4)) *16 + hex2int(*(str + 6 * i + 5));
        memset(utf8,0,4);
		utf8[0] = 0xE0 | (ch /(64 *64));
        utf8[1] = 0x80 | ((ch /64) & 0x3F);
        utf8[2] = 0x80 | (ch & 0x3F);

        strcat(out, utf8);

    }

    return out;
} */


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

    unsigned char tmpstr[200];


    int32_t j;

    //查询成功
    maibu_get_json_int(buff, "status", &j);
    strcat(msg, status[j]);
    strcat(msg, "\n");
    maibu_get_json_str(buff, "time", tmpstr, sizeof(tmpstr));
    strcat(msg, tmpstr);
    strcat(msg, "\n");
    maibu_get_json_str(buff, "context", tmpstr, sizeof(tmpstr));

    strcat(msg, tmpstr);
    //strcat(msg, unicode2utf8(tmpstr));
    strcat(msg, "\n");


    strcat(msg, ad_str);





    app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

    /*显示msg*/
    P_Window p_window = (P_Window) app_window_stack_get_window_by_id(g_window);
    LayerScroll ls = {msgframe, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_16, 2, GColorWhite};
    P_Layer p_layer = app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer, GColorBlack);
    P_Layer p_layer_old = app_window_get_layer_by_id(p_window, g_layer_msg);
    g_layer_msg = app_window_replace_layer(p_window, p_layer_old, p_layer);


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

    P_Window p_window = (P_Window) context;
    app_window_stack_pop(p_window);
}

static void select_up(void *context)
{

    P_Window p_window = (P_Window) context;
    P_Layer p_layer = app_window_get_layer_by_id(p_window, g_layer_msg);
    app_window_set_current_selected_layer(p_window, p_layer);
    app_window_set_up_button(p_window);
}
static void select_down(void *context)
{

    P_Window p_window = (P_Window) context;
    P_Layer p_layer = app_window_get_layer_by_id(p_window, g_layer_msg);
    app_window_set_current_selected_layer(p_window, p_layer);
    app_window_set_down_button(p_window);
}

static void request_kuaidi()
{
    char tmpstr[100] = {0}, com_str0[20] = {0};
    if (app_persist_read_data(KUAIDI_KEY, 0, tmpstr, app_persist_get_data_size(KUAIDI_KEY)) == 0)
        return;
    memset(com_str, 0, sizeof(com_str));
    memset(nu_str, 0, sizeof(nu_str));
    int i, j;
    //快递公司名称
    for (i = 0; tmpstr[i] == ' ' && i < strlen(tmpstr); i++);
    for (j = i; tmpstr[j] != ' ' && j < strlen(tmpstr); j++);
    memcpy(com_str0, tmpstr + i, j - i);
    //快递单号
    for (i = j; tmpstr[i] == ' ' && i < strlen(tmpstr); i++);
    for (j = i; tmpstr[j] != ' ' && j < strlen(tmpstr); j++);
    memcpy(nu_str, tmpstr + i, j - i);

    //maibu_print_log(nu_str);
    sprintf(msg, "%s\n%s\n", com_str0, nu_str);

    app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

    /*显示msg*/
    P_Window p_window = (P_Window) app_window_stack_get_window_by_id(g_window);
    LayerScroll ls = {msgframe, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_16, 2, GColorWhite};
    P_Layer p_layer = app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer, GColorBlack);
    P_Layer p_layer_old = app_window_get_layer_by_id(p_window, g_layer_msg);
    g_layer_msg = app_window_replace_layer(p_window, p_layer_old, p_layer);


    /*更新窗口*/
    app_window_update(p_window);

    for (i = 0; i < KUAIDI_NUM; i++)
    {
        if (strstr(kuaidi_list[i][0], com_str0))
            strcpy(com_str, kuaidi_list[i][1]);
    }
    request_web();

}

static void select_select(void *context)
{

    P_Window p_window = (P_Window) context;
    //查询快递
    request_kuaidi();

}

void watchapp_comm_callback(enum ESyncWatchApp type, uint8_t *context, uint16_t context_len)
{
    if (type == ESyncWatchAppUpdateParam)
    {

        if (context_len > 0)
        {
            int i;
            for (i = 0; context[i] != '\n' && i < context_len; i++);
            if (i > 10)			app_persist_write_data_extend(KUAIDI_KEY, context, i);
            request_kuaidi();
        }
        else
        {
            //显示默认界面
            window_reloading();
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
    P_Layer p_layer = NULL;


    //char buff[30]= {0};

    /*创建一个窗口*/
    p_window = app_window_create();
    if (NULL == p_window) return NULL;
    app_plug_status_bar_create(p_window, NULL, NULL, NULL);
    app_plug_status_bar_add_time(p_window);
    app_plug_status_bar_add_battery(p_window);
    app_plug_status_bar_add_ble(p_window);



    strcpy(msg, "快递去哪儿了?\n支持:");
    int i;
    for (i = 0; i < KUAIDI_NUM; i++)
    {
        strcat(msg, kuaidi_list[i][0]);
        strcat(msg, ",");
    }
    strcat(msg, "\n");
    strcat(msg, ad_str);

    app_persist_write_data_extend(SCROLL_KEY, msg, strlen(msg));

    LayerScroll ls = {msgframe, SCROLL_KEY,  strlen(msg), U_GBK_SIMSUN_16, 2, GColorWhite};
    P_Layer layer = app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer, GColorBlack);
    g_layer_msg = app_window_add_layer(p_window, layer);


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

    /* char tmp[] = "汇通 50442951903914";
    app_persist_write_data_extend(KUAIDI_KEY, tmp, sizeof(tmp));  */

    /*创建显示表盘窗口*/
    P_Window p_window = init_watch();
    if (p_window != NULL)
    {
        /*放入窗口栈显示*/

        g_window = app_window_stack_push(p_window);
    }
    //注册手机设置回调函数
    maibu_comm_register_watchapp_callback(watchapp_comm_callback);

    if (app_persist_get_data_size(KUAIDI_KEY) > 10)
    {
        request_kuaidi();
    }


    //simulator_wait();

    return 0;
}



