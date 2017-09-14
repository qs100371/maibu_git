/*
 * =====================================================================================
 * 
 *     Modulename:
 *       Filename:  watchface_ghost.c
 *
 *    Description:  符鬼表盘
 *    Corporation:  
 * 
 *         Author:  qs100371 
 *        Created:  2016年12月26日
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

#define MAX_PIC 18


static uint32_t TEXT_KEY = 0x2002;
/*时间图层，图片图层位置*/
static GRect bgframe={{0,0},{128,128}};
static GRect ghostframe={{30,59},{69,78}};
static GRect timeframe={{0,0},{30,128}};
static GRect textframe={{4,31},{42,124}};
static uint8_t pic_pos=0;
static int32_t seed=0;
bool scrollable=false;
static int8_t MAX_TEXT=0;

/*图片名数组*/
uint16_t bmp_array[18] = {PIC_01, PIC_02, PIC_03, PIC_04, PIC_05, PIC_06, PIC_07, PIC_08, PIC_09, PIC_10, PIC_11, PIC_12, PIC_13, PIC_14, PIC_15, PIC_16, PIC_17, PIC_18};

static char presettext[][120]={"扫地只不过是我的表面工作，我真正地身份是一位研究僧！",
"其实我是一个演员。",
"我对你的敬仰有如滔滔江水，连绵不绝，有如黄河泛滥一发不可收拾。",
"喜欢一个人需要理由吗？需要吗？不需要吗？需要吗？",
"我就是美貌与智慧并重,英雄与侠义的化身——唐伯虎。",
"我一秒钟几十万上下，我会和你去踢球？",
"凭你的智慧，我唬得了你吗？",
"快点回火星吧，地球是很危险地。",
"我们虽然穷，但我们不说脏话！",
"如果，我多一张船票，你会不会跟我一起走？",
"生命中充满了巧合，两条平行线也会有相交的一天。",
"往往都是事情改变人，人却改变不了事情。",
"我最讨厌你们这些打劫的了，一点技术含量都没有！",
"上帝会把我们身边最好的东西拿走，以提醒我们得到的太多！",
"给我一个机会，我想做好人！",
"生活不是电影，生活比电影苦。",
"世界上最遥远的距离不是生和死，而是站在你面前却不能说：‘我爱你’ 。",
"失去了的东西永远不会再回来 。",
"我不相信有天堂，因为我被困在这个地狱的时间太长了 。",
"你不是说爱是不需要语言的吗？",
"人生不能像作菜，把所有的料都准备好了才下锅 。",
"作女人其实挺简单，只要男人好，我做什么都行。",
"不求最好，但求最贵!",
"能力越大，责任越重。",
"这不是傻，这是爱情。",
"人生没有彩排，每天都是现场直播。",
"星星在哪里都很亮的，就看你有没有抬头去看它们。",
"如果你不出去走走，你就会以为这就是世界。",
"拉着你的手，就像左手摸右手。",
"我拼命读书为了将来，谁知道没有将来！",
"不记得也好，忘却也是一种幸福……",
"当我决定和你度过下半辈子时，我希望我的下半生赶快开始。",
"我定你个肺！",
"有些人一辈子都在骗人，而有些人用一辈子去骗一个人。",
"伏特加是我们拥有的一种奢侈品，鱼子酱也是我们拥有的一种奢侈品，但时间不是。",
"爱情这东西，时间很关键。认识得太早或太晚，都不行。",
"不是子弹，而是美女杀死了野兽。",
"我知道要想不被人拒绝，最好的办法就是先拒绝别人。",
"我这一辈子不知道还会喜欢多少个女人,不到最后我也不知道会喜欢哪一个。",
"如果我知道怎么舍弃你，那该有多好。",
"我甚至连他的一张照片都没有。他只活在我的记忆里。"
};

/*时间图层ID，通过该图层ID获取图层句柄*/
static int8_t g_layer_time =-1,g_layer_ghost=-1,g_layer_bg=-1,g_layer_text=-1;

/*窗口ID, 通过该窗口ID获取窗口句柄*/
static int32_t g_window = -1;

static int32_t myrand(int32_t x)
{
      seed=(seed*129+1)%65536;
     
      return seed*x/65536;
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
	/*根据窗口ID获取窗口句柄*/
	p_window = app_window_stack_get_window_by_id(g_window);
	if (p_window == NULL)
	{
		return;
	}
	//刷新背景
	p_layer = app_window_get_layer_by_id(p_window, g_layer_bg);
	if (p_layer == NULL)
	{
		return;
	}
	res_get_user_bitmap(BG, &bitmap);
	app_layer_set_bitmap_bitmap(p_layer, &bitmap);	
	
	
    struct date_time dt;
    app_service_get_datetime(&dt);
   
	
	//显示时间
	char buff[20]={0};
	sprintf(buff,"%02d:%02d",dt.hour,dt.min);
	
	
	/*获取时间图层句柄*/
	p_layer = app_window_get_layer_by_id(p_window, g_layer_time);
	if (p_layer == NULL)
	{
		return;
	}
	app_layer_set_text_text(p_layer,buff);
	
	//显示动画图片
	pic_pos++;
	if (pic_pos>=MAX_PIC) pic_pos=0;
    
	p_layer = app_window_get_layer_by_id(p_window, g_layer_ghost);
	if (p_layer== NULL)
		{
		    return;
		}
	res_get_user_bitmap(bmp_array[pic_pos], &bitmap);
	app_layer_set_bitmap_bitmap(p_layer, &bitmap);	
	
	//显示滚动文本
	p_layer = app_window_get_layer_by_id(p_window, g_layer_text);
	if (p_layer== NULL)
		{
		    return;
		}
	
	int16_t textindex=myrand(MAX_TEXT);
	/* if (strlen(presettext[textindex])>20)
		scrollable=true;
	else
		scrollable=false; */
	app_persist_create(TEXT_KEY, sizeof(presettext[textindex]));
	app_persist_write_data_extend(TEXT_KEY,presettext[textindex],strlen(presettext[textindex]));
	//int16_t total_bytes = app_persist_get_data_size(TEXT_KEY);
	//sprintf(buff,"index=%d,tb=%d",textindex,total_bytes);
	//maibu_print_log(buff);
	LayerScroll ls={textframe,TEXT_KEY,sizeof(presettext[textindex]),U_GBK_SIMSUN_12,2};
	P_Layer p_layer_new=app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer_new, GColorBlack);
	g_layer_text=app_window_replace_layer(p_window,p_layer,p_layer_new);
	//app_layer_set_scroll_text_key(p_layer,TEXT_KEY);
	//app_layer_set_bg_color(p_layer, GColorBlack);
			
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
   /*  if (scrollable)
	{
		scrollable=false;
		app_window_set_down_button(p_window);
		return;
	} */
	app_watch_update();
	
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
    char buff[200]={0};
	
	/*创建一个窗口*/
	p_window = app_window_create();
	if (NULL == p_window)
	{
		return NULL;
	}
	
    //刷新背景
	
	GBitmap bitmap;
	res_get_user_bitmap(BG, &bitmap);
	LayerBitmap layerbmp1 = { bitmap, bgframe, GAlignLeft};	
	p_layer = app_layer_create_bitmap(&layerbmp1);
	g_layer_bg=app_window_add_layer(p_window, p_layer);
	
    
	//显示动画图片
	res_get_user_bitmap(bmp_array[pic_pos], &bitmap);
	LayerBitmap layerbmp2 = { bitmap, ghostframe, GAlignLeft};
	p_layer=app_layer_create_bitmap(&layerbmp2);
	g_layer_ghost=app_window_add_layer(p_window,p_layer);
	
	
	//显示滚动文本
	sprintf(buff,"欢迎你,我的主人!");
    //maibu_print_log(buff);
	app_persist_create(TEXT_KEY, sizeof(buff));
	app_persist_write_data_extend(TEXT_KEY,buff,sizeof(buff));
	int16_t total_bytes = app_persist_get_data_size(TEXT_KEY);
	LayerScroll ls={textframe,TEXT_KEY,total_bytes,U_GBK_SIMSUN_12,2};
	p_layer=app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_text=app_window_add_layer(p_window,p_layer); 
    //sprintf(buff,"g_layer_text=%d",g_layer_text);
	// maibu_print_log(buff);
    
	
	//显示时间
    struct date_time dt;
	app_service_get_datetime(&dt);
	
	sprintf(buff,"%02d:%02d",dt.hour,dt.min);
    LayerText timetext = {buff, timeframe, GAlignCenter, U_ASCII_ARIAL_30};//
	p_layer = app_layer_create_text(&timetext);
	app_layer_set_bg_color(p_layer, GColorBlack);
	g_layer_time=app_window_add_layer(p_window, p_layer);
    
    	
	/*注册一个事件通知回调，当有时间改变时，立即更新时间*/
	maibu_service_sys_event_subscribe(app_watch_time_change);
	/*添加按键事件*/
	app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);
	
    
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
    MAX_TEXT=sizeof(presettext)/sizeof(presettext[0]);
	seed=app_get_time_number()%65536;
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
