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

//刷新模式，0 随便说，2 调戏 3 运动
static uint8_t g_mode=0;

static uint32_t TEXT_KEY = 0x2002;
/*时间图层，图片图层位置*/
static GRect bgframe={{0,0},{128,128}};
static GRect ghostframe={{30,59},{69,78}};
static GRect timeframe={{0,0},{30,128}};
static GRect textframe={{4,32},{42,124}};
static uint8_t pic_pos=0;
static int32_t seed=0;
//bool scrollable=false;
static int8_t MAX_TEXT=0,count_press_back=0;
uint32_t lasttime_press_back=0;
/*图片名数组*/
uint16_t bmp_array[18] = {PIC_01, PIC_02, PIC_03, PIC_04, PIC_05, PIC_06, PIC_07, PIC_08, PIC_09, PIC_10, PIC_11, PIC_12, PIC_13, PIC_14, PIC_15, PIC_16, PIC_17, PIC_18};

char presettext[][50]={
"其实我是一个演员.", 
"凭你的智慧,我唬得了你吗?",
"给我一个机会,我想做好人!",
"生活不是电影,生活比电影苦.",
"失去了的东西永远不会再回来.",
"人生没有彩排,每天都是现场直播.",
"不记得也好,忘却也是一种幸福...",
"常回家看看.",               //
"好好学习,认真做事.",       //工作或休息提醒
"今天有点冷.",              //气温提醒
"少小离家胖了回,乡音未改肉成堆."   //运动提醒
};
char emotion[19][50]={
	"工作中,勿扰!",//"正常1",
	"哎呀,妈呀!今夜阳光明媚~",//"正常2",
	"好无聊啊,你带我逛街去吧.",//"空虚1",
	"寂寞空虚冷.",//"空虚2",
	"今天真高兴!",//"高兴1",
	"幸福,就是和你爱的宠物过一辈子.",//"高兴2",
	"我想哭.",//"哭泣1",
	"呜呜呜...",//"哭泣2",
	"别摸我!",//"发怒1",
	"本宠不发威,你当我是病猫啊!",//"发怒2",
	"多希望有一天能把人当宠物来养啊!",//"演讲1",
	"本宠好色,请保持适当距离.",//"演讲2",
	"我左青龙,右白虎,背后纹上米老鼠.",//"向左走1",
	"超级无敌旋风腿,接招!",//"向左走2",
	"不要迷恋我,我只是宠物.",//"向右走1",
	"不要骑我,我是宠物,不是毛驴.",//"向右走2",
	"呼呼中...",//"睡眠1",
    "我要睡觉...",//"睡眠2"
	"轻点，菊花疼..."  //按太快了
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
	pic_pos++;
	if (pic_pos>=MAX_PIC) pic_pos=0;
	struct date_time dt;
    app_service_get_datetime(&dt);
	float temp;
	SportData spdata;
	SpeedInfo spinfo;
	if (0!= maibu_get_sport_data(&spdata, 0)) return;  //运动数据
	if(0!=maibu_get_temperature(&temp)) return;   //实时气温
	if (g_mode==0)
	{
		maibu_get_speed_info(&spinfo);
	    if (spinfo.flag==1) g_mode=2;  //实时速度
	}
	//g_mode=2;
	//每五分钟更新提醒语句
	if(dt.min%5==0&&g_mode==0)
	{
	   if((dt.mon==1)&&(dt.mday<=27))
		  sprintf(presettext[MAX_TEXT-4],"%s","要过年了,记得回家看看父母哦."); 
	   else
		   sprintf(presettext[MAX_TEXT-4],"%s","记得常回家看看父母哦.");
	   if((dt.hour>=9)&&(dt.hour<=17)&&(dt.wday>=1)&&(dt.wday<=5))
          sprintf(presettext[MAX_TEXT-3],"%s","工作时间,请勿打扰.");
       else
          sprintf(presettext[MAX_TEXT-3],"%s","休息时间，好好享受吧.");
	  
	   if (temp<20)
		   sprintf(presettext[MAX_TEXT-2],"%s","宝宝有点冷,带我做做运动吧.");
	   else if(temp<=35)
		   sprintf(presettext[MAX_TEXT-2],"%s","今天气温正好，好好享受吧.");
	   else
		   sprintf(presettext[MAX_TEXT-2],"%s","好热啊，宝宝要吃冰淇淋!");
	   	  
	   if (spdata.step<=5000)
		  sprintf(presettext[MAX_TEXT-1],"%s%d%s","今天才走了",spdata.step,"步,该加强锻炼了."); 
	   else if(spdata.step<=10000)
		   sprintf(presettext[MAX_TEXT-1],"%s%d%s","今天已经走了",spdata.step,"步,继续努力吧!"); 
	   else
		   sprintf(presettext[MAX_TEXT-1],"%s%d%s","今天走了",spdata.step,"步,主人真厉害!"); 
	  
	}
	
	/*根据窗口ID获取窗口句柄*/
	p_window = app_window_stack_get_window_by_id(g_window);
	if (p_window == NULL) return;
	
	//刷新背景
	p_layer = app_window_get_layer_by_id(p_window, g_layer_bg);
	if (p_layer == NULL) return;
	
	res_get_user_bitmap(BG, &bitmap);
	app_layer_set_bitmap_bitmap(p_layer, &bitmap);	
	
	
    
   
	
	//显示时间
	char buff[200]={0};
	sprintf(buff,"%02d:%02d",dt.hour,dt.min);
	
	
	p_layer = app_window_get_layer_by_id(p_window, g_layer_time);
	if (p_layer == NULL) return;
	
	app_layer_set_text_text(p_layer,buff);
	
	//显示动画图片
	
    
	p_layer = app_window_get_layer_by_id(p_window, g_layer_ghost);
	if (p_layer== NULL) return;
	
	//res_get_user_bitmap(bmp_array[pic_pos], &bitmap);
	//app_layer_set_bitmap_bitmap(p_layer, &bitmap);	
	
	//显示动画图片
	ghostframe.origin.x=myrand(50);
	res_get_user_bitmap(bmp_array[pic_pos], &bitmap);
	LayerBitmap layerbmp2 = { bitmap, ghostframe, GAlignLeft};
	P_Layer p_layer_new1=app_layer_create_bitmap(&layerbmp2);
	g_layer_ghost=app_window_replace_layer(p_window,p_layer,p_layer_new1);
	
	//显示滚动文本
	p_layer = app_window_get_layer_by_id(p_window, g_layer_text);
	if (p_layer== NULL) return;
		
	if (g_mode==0)
	{
	   int16_t textindex=myrand(MAX_TEXT);
	   sprintf(buff,"%s",presettext[textindex]);
	}
	else
		if(g_mode==1)
		{
			
			if (count_press_back>10)
			{
			   sprintf(buff,"%s",emotion[MAX_PIC]);
			}
		    else
               sprintf(buff,"%s",emotion[pic_pos]);
        }
	else
	{
		//运动模式，显示运动数据
		memset(buff,0,sizeof(buff));
		sprintf(buff,"%s%d\n","今日步数:",spdata.step);
		char tmpstr[20];
		sprintf(tmpstr,"%s%d\n","实时速度:",spinfo.real_time_speed);
		int i,j;
		j=strlen(buff);
		for(i=0;i<strlen(tmpstr);i++)
			buff[j+i]=tmpstr[i];
		sprintf(tmpstr,"%s%2.1f","体感温度:",temp);
		
		j=strlen(buff);
		for(i=0;i<strlen(tmpstr);i++)
			buff[j+i]=tmpstr[i];
	}


    
	
	/* if (strlen(presettext[textindex])>20)
		scrollable=true;
	else
		scrollable=false; */
	app_persist_create(TEXT_KEY, sizeof(buff));
	app_persist_write_data_extend(TEXT_KEY,buff,strlen(buff));
	//int16_t total_bytes = app_persist_get_data_size(TEXT_KEY);
	//sprintf(buff,"index=%d,tb=%d",textindex,total_bytes);
	//maibu_print_log(buff);
	LayerScroll ls={textframe,TEXT_KEY,sizeof(buff),U_GBK_SIMSUN_12,2};
	P_Layer p_layer_new2=app_layer_create_scroll(&ls);
	app_layer_set_bg_color(p_layer_new2, GColorBlack);
	g_layer_text=app_window_replace_layer(p_window,p_layer,p_layer_new2);
	//app_layer_set_scroll_text_key(p_layer,TEXT_KEY);
	//app_layer_set_bg_color(p_layer, GColorBlack);
			
	/*窗口显示*/	
	app_window_update(p_window);
	g_mode=0;
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
	g_mode=1;
	int32_t tt=app_get_time_number();
	
	if (tt-lasttime_press_back<2)
		count_press_back++;
	else 
	{
		count_press_back=0;
	    
	}
	lasttime_press_back=tt;
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
	//simulator_init();
    lasttime_press_back=app_get_time_number();
	seed=lasttime_press_back%65536;
	MAX_TEXT=sizeof(presettext)/sizeof(presettext[0]);
	
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
