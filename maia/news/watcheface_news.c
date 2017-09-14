
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "maibu_sdk.h"
#include "maibu_res.h"

/*
 * 窗口
 */
static int32_t g_window_id             = -1;
static int32_t g_layer_time = -1;

static uint32_t NEWS_1_DATA_KEY = 0x2003;
static uint32_t NEWS_2_DATA_KEY = 0x2004;
static uint32_t NEWS_3_DATA_KEY = 0x2005;
static uint32_t NEWS_4_DATA_KEY = 0x2006;

static char g_str_news_1[60] = {0};
static char g_str_news_2[60] = {0};
static char g_str_news_3[60] = {0};
static char g_str_news_4[60] = {0};

//news 1文本图层坐标及高宽
#define NEWS_1_X 		7
#define NEWS_1_Y 		16
#define NEWS_1_H 		37
#define NEWS_1_W		169

//news 2文本图层坐标及高宽
#define NEWS_2_X 		7
#define NEWS_2_Y 		57
#define NEWS_2_H 		37
#define NEWS_2_W		169

//news 3文本图层坐标及高宽
#define NEWS_3_X 		7
#define NEWS_3_Y 		97
#define NEWS_3_H 		37
#define NEWS_3_W		169

//news 4文本图层坐标及高宽
#define NEWS_4_X 		7
#define NEWS_4_Y 		138
#define NEWS_4_H 		37
#define NEWS_4_W		169

#define NEWS_URL		"http://v.juhe.cn/toutiao/index?key=72f177dd702b579e76c69791d3d8e3cf&type="
static char g_str_news_url[186] = {0};

/*
 * 网络请求
 */
static uint32_t g_news_request_start_time = 0;
static uint32_t g_comm_id_web_news   = 0;

/*
 * 其他
 */
static bool g_new_install = true;

/*
 * 定时器
 */
static char		g_common_timer            = 0;//定时器id

/*
 * 获取系统时间，单位为s
 */
uint32_t get_system_time_seconds(void)
{
  struct date_time tt1;
  app_service_get_datetime(&tt1);
  return app_get_time(&tt1);
}

P_Window init_window(void);
void timer_callback(date_time_t tick_time, uint32_t millis, void *context);

void window_reloading(void)
{
  /*根据窗口ID获取窗口句柄*/
  P_Window p_old_window = (P_Window)app_window_stack_get_window_by_id(g_window_id);
  if (NULL != p_old_window)
    {
      P_Window p_window = init_window();
      if (NULL != p_window)
        {
          g_window_id = app_window_stack_replace_window(p_old_window, p_window);
        }
    }

}

void news_4_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[60] = {0};
  maibu_get_json_str(buff, "title", buffer, sizeof(buffer));
  sprintf(g_str_news_4, "%s", buffer);
  app_persist_write_data_extend(NEWS_4_DATA_KEY, g_str_news_4, strlen(g_str_news_4));
  window_reloading();
}

void news_3_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[60] = {0};
  maibu_get_json_str(buff, "title", buffer, sizeof(buffer));
  sprintf(g_str_news_3, "%s", buffer);
  app_persist_write_data_extend(NEWS_3_DATA_KEY, g_str_news_3, strlen(g_str_news_3));
  window_reloading();

  //请求news 4
  sprintf(g_str_news_url, "%s%s", NEWS_URL, "keji");
  maibu_comm_register_web_callback(news_4_callback);
  g_comm_id_web_news = maibu_comm_request_web(g_str_news_url, "title", 0);
}

void news_2_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[60] = {0};
  maibu_get_json_str(buff, "title", buffer, sizeof(buffer));
  sprintf(g_str_news_2, "%s", buffer);
  app_persist_write_data_extend(NEWS_2_DATA_KEY, g_str_news_2, strlen(g_str_news_2));
  window_reloading();

  //请求news 3
  sprintf(g_str_news_url, "%s%s", NEWS_URL, "guonei");
  maibu_comm_register_web_callback(news_3_callback);
  g_comm_id_web_news = maibu_comm_request_web(g_str_news_url, "title", 0);
}

void news_1_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[60] = {0};
  maibu_get_json_str(buff, "title", buffer, sizeof(buffer));
  sprintf(g_str_news_1, "%s", buffer);
  app_persist_write_data_extend(NEWS_1_DATA_KEY, g_str_news_1, strlen(g_str_news_1));
  window_reloading();

  //请求news 2
  sprintf(g_str_news_url, "%s%s", NEWS_URL, "guoji");
  maibu_comm_register_web_callback(news_2_callback);
  g_comm_id_web_news = maibu_comm_request_web(g_str_news_url, "title", 0);
}

void request_news()
{
  //新闻请求起始时间记录
  g_news_request_start_time = get_system_time_seconds();

  /* 请求 news 1 */
  sprintf(g_str_news_url, "%s%s", NEWS_URL, "top");
  maibu_comm_register_web_callback(news_1_callback);
  g_comm_id_web_news = maibu_comm_request_web(g_str_news_url, "title", 0);
}

/*
 * 创建并显示图片图层
 */
int32_t display_layer_bmp(P_Window p_window, GRect *temp_p_frame, enum GAlign how_to_align, enum GColor black_or_white, int bmp_key)
{

  GBitmap bmp_point;
  P_Layer temp_P_Layer = NULL;

  res_get_user_bitmap(bmp_key, &bmp_point);
  LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, how_to_align};
  temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);
  if (temp_P_Layer != NULL)
    {
      app_layer_set_bg_color(temp_P_Layer, black_or_white);
      return app_window_add_layer(p_window, temp_P_Layer);
    }

  return 0;
}


/*
 * 创建并显示文本图层
 */
/* int32_t display_layer_text(P_Window p_window,GRect  *temp_p_frame,enum GAlign how_to_align,enum GColor color,char * str,uint8_t font_type,enum GColor fcolor)
{
	LayerText temp_LayerText = {0};
	temp_LayerText.text = str;
	temp_LayerText.frame = *temp_p_frame;
	temp_LayerText.alignment = how_to_align;
	temp_LayerText.font_type = font_type;
	temp_LayerText.foregroundColor = fcolor;
	P_Layer p_layer = app_layer_create_text(&temp_LayerText);

	if(p_layer != NULL)
	{
		app_layer_set_bg_color(p_layer, color);
		return app_window_add_layer(p_window, p_layer);
	}
	return 0;
} */
void select_back(void *context)
{
  request_news();
}

void watch_time_change(enum SysEventType type, void *context)
{
  /*时间更改*/
  if (type == SysEventTypeTimeChange)
    {
      struct date_time dt;
      app_service_get_datetime(&dt);
      char str[20] = "";
      sprintf(str, "%02d:%02d", dt.hour, dt.min);
      P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_window_id);
      if (p_window == NULL) return;
      P_Layer p_layer = app_window_get_layer_by_id(p_window, g_layer_time);
      if (p_layer == NULL) return;
      app_layer_set_text_text(p_layer, str);
      app_window_update(p_window);
    }
}


P_Window init_window(void)
{
  P_Window p_window = app_window_create();
  if (NULL == p_window)
    {
      return NULL;
    }

  //变量声明
  LayerScroll layer_scroll = {0};
  P_Layer	p_layer = NULL;
  GRect temp_frame = {{0, 0}, {176, 176}};

  //添加背景图层
  display_layer_bmp(p_window, &temp_frame, GAlignCenter, GColorBlack, RES_BITMAP_WATCHFACE_NEWS_BG);

  struct date_time dt;
  app_service_get_datetime(&dt);
  char str[20] = "";

  GRect timeframe = {{120, 0}, {16, 50}};
  sprintf(str, "%02d:%02d", dt.hour, dt.min);
  LayerText lt = {str, timeframe, GAlignRight, GColorBlack, U_ASCII_ARIALBD_16};
  p_layer = app_layer_create_text(&lt);
  app_layer_set_bg_color(p_layer, GColorWhite);
  g_layer_time = app_window_add_layer(p_window, p_layer);

  //添加news1图层
  /*初始化滚动结构体, 依次为滚动图层显示框架, 内容文件KEY，需要显示的内容大小、字体字号*/
  temp_frame.origin.x = NEWS_1_X;
  temp_frame.origin.y = NEWS_1_Y;
  temp_frame.size.h = NEWS_1_H;
  temp_frame.size.w = NEWS_1_W;
  layer_scroll.frame = temp_frame;
  layer_scroll.key = NEWS_1_DATA_KEY;
  layer_scroll.total_bytes = 60;
  layer_scroll.font_type = U_ASCII_ARIAL_16;
  layer_scroll.gap = 2;
  layer_scroll.color = GColorRed;
  p_layer = app_layer_create_scroll(&layer_scroll);
  app_layer_set_bg_color(p_layer, GColorWhite);
  app_window_add_layer(p_window, p_layer);

  //添加news2图层
  /*初始化滚动结构体, 依次为滚动图层显示框架, 内容文件KEY，需要显示的内容大小、字体字号*/
  temp_frame.origin.x = NEWS_2_X;
  temp_frame.origin.y = NEWS_2_Y;
  temp_frame.size.h = NEWS_2_H;
  temp_frame.size.w = NEWS_2_W;
  layer_scroll.frame = temp_frame;
  layer_scroll.key = NEWS_2_DATA_KEY;
  layer_scroll.total_bytes = 60;
  layer_scroll.font_type = U_ASCII_ARIAL_16;
  layer_scroll.gap = 2;
  layer_scroll.color = GColorBlue;
  p_layer = app_layer_create_scroll(&layer_scroll);
  app_layer_set_bg_color(p_layer, GColorWhite);
  app_window_add_layer(p_window, p_layer);

  //添加news3图层
  /*初始化滚动结构体, 依次为滚动图层显示框架, 内容文件KEY，需要显示的内容大小、字体字号*/
  temp_frame.origin.x = NEWS_3_X;
  temp_frame.origin.y = NEWS_3_Y;
  temp_frame.size.h = NEWS_3_H;
  temp_frame.size.w = NEWS_3_W;
  layer_scroll.frame = temp_frame;
  layer_scroll.key = NEWS_3_DATA_KEY;
  layer_scroll.total_bytes = 60;
  layer_scroll.font_type = U_ASCII_ARIAL_16;
  layer_scroll.gap = 2;
  layer_scroll.color = GColorPurple;
  p_layer = app_layer_create_scroll(&layer_scroll);
  app_layer_set_bg_color(p_layer, GColorWhite);
  app_window_add_layer(p_window, p_layer);

  //添加news4图层
  /*初始化滚动结构体, 依次为滚动图层显示框架, 内容文件KEY，需要显示的内容大小、字体字号*/
  temp_frame.origin.x = NEWS_4_X;
  temp_frame.origin.y = NEWS_4_Y;
  temp_frame.size.h = NEWS_4_H;
  temp_frame.size.w = NEWS_4_W;
  layer_scroll.frame = temp_frame;
  layer_scroll.key = NEWS_4_DATA_KEY;
  layer_scroll.total_bytes = 60;
  layer_scroll.font_type = U_ASCII_ARIAL_16;
  layer_scroll.gap = 2;
  layer_scroll.color = GColorBlack;
  p_layer = app_layer_create_scroll(&layer_scroll);
  app_layer_set_bg_color(p_layer, GColorWhite);
  app_window_add_layer(p_window, p_layer);

  /*注册一个事件通知回调，当有时间改变是，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);
  app_window_click_subscribe(p_window, ButtonIdBack, select_back);
  return p_window;
}

void news_comm_result_callback(enum ECommResult result, uint32_t comm_id, void *context)
{
  /*如果上一次请求WEB通讯失败，并且通讯ID相同，则重新发送*/
  if ((result == ECommResultFail) && (comm_id == g_comm_id_web_news))
    {
      g_comm_id_web_news = maibu_comm_request_web(g_str_news_url, "title", 0);
    }
}

/*
 * 定时器回调函数
 */
void timer_callback(date_time_t tick_time, uint32_t millis, void *context)
{
  if (g_new_install)   //新装
    {
      if (strlen(g_str_news_1) == 0)  //未请求到数据
        {
          request_news(); //发起请求
        }
      else   //已请求到数据
        {
          //重新设定定时器
          app_service_timer_unsubscribe(g_common_timer);
          g_common_timer = app_service_timer_subscribe(1 * 60 * 1000, timer_callback, NULL); //1分钟
          //清除新装标识
          g_new_install = false;
        }
    }
  else   //非新装
    {
      if ((get_system_time_seconds() - g_news_request_start_time) >= 180)   //据上次更新请求超过15分钟
        {
          request_news(); /* 请求新闻信息 */
        }
    }
}

int main(void)
{

//	simulator_init();

  /*创建显示窗口*/
  P_Window p_window = init_window();
  /*放入窗口栈显示*/
  g_window_id = app_window_stack_push(p_window);

  /*注册通讯结果回调*/
  maibu_comm_register_result_callback(news_comm_result_callback);

  //注册定时查询函数
  if (g_common_timer == 0)
    {
      g_common_timer = app_service_timer_subscribe(5 * 1000, timer_callback, NULL); //5s
    }

//	simulator_wait();

  return 0;
}


