#include <stdlib.h>
#include <stdio.h>

#include "maibu_sdk.h"
#include "maibu_res.h"
//////
static uint8_t g_auto_weather_flag     = 1;//标志是否自动查询天气
static char    get_city_context[35]    = {0x01};//协议ID

static uint32_t g_comm_id_get_city      = 0;//获取城市的通讯ID
static uint32_t g_comm_id_web_weather   = 0;//获取天气的通讯ID


static char    g_city[20]              = {0};
static char    g_city_temp[20]         = {0};

static int32_t g_temperature           = 0;
//static int32_t g_curPm                 = -1;
static int32_t g_level				   = 0;

static char weather_info[24] 	       = {0};


#define CITY_EMPTY					3

#define WEATHER_KEY			"code,temp,pm2_5,info,aqi_level"

#define WEATHER_URL		"http://maibu.cc/ThirdPartyData/weather/getRealTimeWeather.do?city="


#define WEATHER_CLOUDY		"多云"
#define WEATHER_FOG_1		"雾"
#define WEATHER_FOG_2		"霾"
#define WEATHER_FOG_3		"沙"
#define WEATHER_OVERCAST	"阴"
#define WEATHER_RAIN		"雨"
#define WEATHER_SNOW		"雪"
#define WEATHER_SUNNY		"晴"

#define JSON_ERROR			-1
#define CITY_PRESIST_DATA_KEY 1

static char level_aqi_str[7][8] = {"无", "优", "良", "轻度", "中度", "重度", "严重"};
static enum GColor weathercolor[7] = { GColorWhite, GColorGreen, GColorYellow, GColorPurple, GColorPurple, GColorRed, GColorRed};
/////
static int32_t g_window_id = -1, g_timer_id = -1, g_walktime_id = -1, g_step_id = -1, g_distance_id = -1;
static uint8_t watch_mode = 0;//0,正常模式,1,运动模式
static uint32_t deltastep = 0 , speed = 0, walk_step = 0, start_time = 123455, walk_time = 600, distance = 998, pace = 0 , last_pace_time = 0;

static uint32_t pressed_time = 0, pressed_count = 0, show_pace_flag = 0, pace_index = 0;

static SportData data0, data1;
static P_Window init_watch(void);
static void request_get_city_info(void);
static GRect timeFrames[] =
{
  {{54, 12}, {48, 32}},
  {{90, 12}, {48, 32}},
  {{54, 72}, {48, 32}},
  {{90, 72}, {48, 32}},
  {{60, 142}, {21, 15}},
  {{32, 130}, {40, 28}}
};

static GRect sportFrames[] =
{
  {{0, 0}, {60, 120}}, //运动时间  30号字{{120, 4}, {16, 56}},
  {{120, 8}, {20, 56}}, //时间  20号字
  {{120, 32}, {20, 56}},
  {{0, 60}, {44, 88}},  //速度  30号字
  {{0, 102}, {16, 88}},  //速度标签  16号字
  {{88, 60}, {44, 88}},  //距离   30号字
  {{88, 102}, {16, 88}},   //距离标签  16号字
  {{0, 130}, {46, 88}},   //步频
  {{92, 120}, {45, 80}},   //阶段图像
  {{0, 100}, {30, 176}}   //配速显示
};
static char stage_str[6][8] = {"  ", "热身", "燃脂", "心肺", "耐力", "极限"};//热身、燃脂、心肺、耐力、极限
static enum GColor speedcolor[6] = { GColorWhite, GColorCyan, GColorGreen, GColorYellow, GColorPurple, GColorRed};

//小号数字数组
const static uint32_t get_LIT_icon_key[] =
{
  LIT_0,
  LIT_1,
  LIT_2,
  LIT_3,
  LIT_4,
  LIT_5,
  LIT_6,
  LIT_7,
  LIT_8,
  LIT_9
};
//大号数字数组
const static uint32_t get_BIG_icon_key[] =
{
  BIG_0,
  BIG_1,
  BIG_2,
  BIG_3,
  BIG_4,
  BIG_5,
  BIG_6,
  BIG_7,
  BIG_8,
  BIG_9
};

const static uint32_t stage[] =
{
  STAGE0,
  STAGE1,
  STAGE2,
  STAGE3,
  STAGE4,
  STAGE5,
};

//重新载入并刷新窗口所有图层
void window_reloading(void)
{
  /*根据窗口ID获取窗口句柄*/
  P_Window p_old_window = (P_Window) app_window_stack_get_window_by_id(g_window_id);
  if (NULL != p_old_window)
    {
      P_Window p_window = init_watch();
      if (NULL != p_window)
        {
          g_window_id = app_window_stack_replace_window(p_old_window, p_window);
        }
    }

}
//请求天气数据回调
static void weather_info_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[20] = {0};

  uint32_t temp_num = 0;
  maibu_get_json_int(buff, "code", &temp_num);
  if (temp_num != 0)
    {
      return;
    }
  g_temperature = 0;
  //g_curPm = -1;
  g_level = 0;
  maibu_get_json_int(buff, "temp", &g_temperature);
  //maibu_get_json_int(buff, "pm2_5", &g_curPm);
  maibu_get_json_int(buff, "aqi_level", &g_level);
  if (maibu_get_json_str(buff, "info", buffer, sizeof(weather_info)) != JSON_ERROR)
    {//防止数据为空导致的数据清空
      memset(weather_info, 0, sizeof(weather_info));
      maibu_get_json_str(buff, "info", weather_info, sizeof(weather_info));
    }
  else
    {
      return;
    }
  //刷新界面
  if (watch_mode == 1)
    window_reloading();
}

//请求天气数据
static void request_weather_info(char * city_name)
{
  char url[256] = {0};
  sprintf(url, "%s", WEATHER_URL);

  if (city_name != NULL)
    {
      int i = 0, j = 0;
      for (i;(city_name[i] != '\0') && (j < sizeof(url)) ;++i)
        {
          sprintf(url, "%s%%%x", url, (unsigned char)city_name[i]);
        }

    }
  else if (strlen(city_name) < CITY_EMPTY)
    {
      request_get_city_info();
      return;
    }

  maibu_comm_register_web_callback(weather_info_callback);
  g_comm_id_web_weather = maibu_comm_request_web(url, WEATHER_KEY, 15 * 60); //* get_front_or_back_flag()

}


//请求城市数据
static void request_get_city_info(void)
{
  g_comm_id_get_city = maibu_comm_request_phone(ERequestPhoneSelfDefine, (void *) & get_city_context, 34);
}


//请求城市数据回调
static void get_city_info_callback(enum ERequestPhone  type, void * context, uint16_t context_size)
{
  if (type == ERequestPhoneSelfDefine)
    {
      if (*(int16_t *)context == 1)
        {
          int8_t *context_city_name_point = (int8_t *)context + 4;

          memcpy(g_city, context_city_name_point, 20);
          g_city[19] = '\0';

          request_weather_info(g_city);
        }
    }
}
static void weather_comm_result_callback(enum ECommResult result, uint32_t comm_id, void *context)
{
  /*如果上一次请求GPS通讯失败，并且通讯ID相同，则重新发送*/
  if (result == ECommResultFail)
    {
      if (comm_id == g_comm_id_get_city)
        {
          if (strlen(g_city) >= CITY_EMPTY)
            {
              request_weather_info(g_city);
            }
          else
            {
              g_comm_id_get_city = maibu_comm_request_phone(ERequestPhoneSelfDefine, (void *) & get_city_context, 34);
            }
        }

      /*如果上一次请求WEB通讯失败，并且通讯ID相同，则重新发送*/
      if (comm_id == g_comm_id_web_weather)
        {
          request_weather_info(g_city);

        }
    }

}

static void watchapp_comm_callback(enum ESyncWatchApp type, uint8_t *buf, uint16_t len)
{

  if (type == ESyncWatchAppUpdateParam)
    {
      if ((len >= 4) && (len <= 20))
        {
          g_auto_weather_flag = 0;

          memset(weather_info, 0, sizeof(weather_info));

          memcpy(g_city, buf, len);
          g_city[len] = '\0';

          app_persist_write_data_extend(CITY_PRESIST_DATA_KEY, g_city, sizeof(g_city));
          //根据城市信息获取天气信息
          request_weather_info(g_city);

        }
      else//恢复自动查询城市
        {
          app_persist_delete_data(CITY_PRESIST_DATA_KEY);

          g_auto_weather_flag = 1;

          memset(weather_info, 0, sizeof(weather_info));
          memset(g_city, 0, sizeof(g_city));
          request_get_city_info();
        }

    }
}


/*创建并显示图层，需要坐标值，得到icon_key的数组，数组需要的参数值，P_Window*/
static int32_t display_target_layer(P_Window p_window, GRect *temp_p_frame, int32_t bmp_array_name_key)
{
  int32_t g_layer_id_temporary;

  GBitmap bmp_point;
  P_Layer temp_P_Layer = NULL;

  res_get_user_bitmap(bmp_array_name_key, &bmp_point);
  LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, GAlignLeft};
  temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);

  if (temp_P_Layer != NULL)
    {
      app_layer_set_bg_color(temp_P_Layer, GColorBlack);
      g_layer_id_temporary = app_window_add_layer(p_window, temp_P_Layer);
    }

  return g_layer_id_temporary;
}

/*创建并显示文本图层*/
int32_t display_target_layerText(P_Window p_window, const GRect  *temp_p_frame, enum GAlign how_to_align, enum GColor color, char * str, uint8_t font_type, enum GColor fcolor)
{
  LayerText temp_LayerText = {0};
  temp_LayerText.text = str;
  temp_LayerText.frame = *temp_p_frame;
  temp_LayerText.alignment = how_to_align;
  temp_LayerText.font_type = font_type;
  temp_LayerText.foregroundColor = fcolor;

  P_Layer p_layer = app_layer_create_text(&temp_LayerText);

  if (p_layer != NULL)
    {
      app_layer_set_bg_color(p_layer, color);
      return app_window_add_layer(p_window, p_layer);
    }
  return 0;
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
      window_reloading();
    }
}
static void timer_callback(date_time_t tick_time, uint32_t millis, void* context)
{

  //计算

  if (start_time == 0)
    {
      start_time = millis / 1000;
      maibu_get_sport_data(&data0, 0);
      data1 = data0;
      last_pace_time = 0;
      show_pace_flag = 0;
      pace_index = 0;
    }
  else
    {
      if (tick_time->hour == 0 && tick_time->min == 0)  //到第二天了，关闭运动模式
        {
          watch_mode = 0;
          app_service_timer_unsubscribe(g_timer_id);
          window_reloading();
          return;
        }

      uint8_t reload_flag = 0;
      static SportData tmp_data;

      walk_time = millis / 1000 - start_time;
      maibu_get_sport_data(&tmp_data, 0);
      distance = (tmp_data.distance - data0.distance) / 100;  //米
      walk_step = tmp_data.step - data0.step;
      if (walk_time % 60 == 0)      //计算时速，步频小于80时速设为0
        {
          deltastep = tmp_data.step - data1.step;
          if (deltastep >= 80)
            speed = (tmp_data.distance - data1.distance) * 600 / 100000;  //10倍的时速
          else
            speed = 0;
          data1 = tmp_data;
          reload_flag = 1;
        }
      //计算配速
      if (show_pace_flag > 1)
        {
          show_pace_flag--;
        }
      else if (show_pace_flag == 1)
        {
          reload_flag = 1;
          show_pace_flag = 0;
        }

      if (distance / 1000 > pace_index && show_pace_flag == 0)
        {
          pace = walk_time - last_pace_time;
          show_pace_flag = 60;
          pace_index++;
          last_pace_time = walk_time;
          reload_flag = 1;
          maibu_service_vibes_pulse(VibesPulseTypeLong, 2);
        }

      //全部刷新
      if (reload_flag)
        {
          window_reloading();
          return;
        }

      //刷新部分界面
      P_Window p_window = (P_Window) app_window_stack_get_window_by_id(g_window_id);
      if (p_window == NULL)return;
      char buff[20] = {0};

      sprintf(buff, "%02d:%02d", walk_time / 60, walk_time % 60);
      P_Layer p_layer = NULL;
      p_layer = app_window_get_layer_by_id(p_window, g_walktime_id);
      if (p_layer == NULL) return;
      app_layer_set_text_text(p_layer, buff);

      sprintf(buff, "%d", walk_step);
      p_layer = app_window_get_layer_by_id(p_window, g_step_id);
      if (p_layer == NULL) return;
      app_layer_set_text_text(p_layer, buff);

      sprintf(buff, "%d.%02d", distance / 1000, distance % 1000 / 10);
      p_layer = app_window_get_layer_by_id(p_window, g_distance_id);
      if (p_layer == NULL) return;
      app_layer_set_text_text(p_layer, buff);

      app_window_update(p_window);
    }

}

/*定义后退按键事件*/
void scroll_select_back(void *context)
{
  if (pressed_count == 0)
    {
      pressed_time = app_get_time_number();
      pressed_count++;
    }
  else  if (app_get_time_number() - pressed_time < 3)
    pressed_count++;
  else
    pressed_count = 0;

  if (pressed_count >= 3)
    {
      pressed_count = 0;
      if (watch_mode == 0)
        {
          watch_mode = 1;
          start_time = 0;
          walk_step = 0;
          distance = 0;
          walk_time = 0;
          deltastep = 0;
          speed = 0;
          pace = 0;
          g_timer_id = app_service_timer_subscribe(1000, timer_callback, NULL);
        }
      else
        {
          watch_mode = 0;
          app_service_timer_unsubscribe(g_timer_id);

        }
      window_reloading();
      return;
    }
  return;

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
  char buff[30] = {0};
  GRect temp_frame = {0};
  int i = 0;
  /*创建一个窗口*/
  p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);
  static SportData data;
  maibu_get_sport_data(&data, 0);


  /////显示表盘
  if (watch_mode == 0)   //正常模式
    {

      //显示时间
      sprintf(buff, "%02d%02d", dt.hour, dt.min);
      for (i = 0;i <= 3;i++)
        {
          display_target_layer(p_window, &timeFrames[i], get_BIG_icon_key[buff[i] - '0']);
        }

      sprintf(buff, "%d", data.step);
      temp_frame = timeFrames[4];
      for (i = 0;i < strlen(buff);i++)
        {
          display_target_layer(p_window, &temp_frame, get_LIT_icon_key[buff[i] - '0']);
          temp_frame.origin.x = temp_frame.origin.x + temp_frame.size.w;
        }
      display_target_layer(p_window, &timeFrames[5], STEP);

      /* if (data.step >= 8000)
        {
          temp_frame.origin.y = 130;
          temp_frame.size.w = 28;
          temp_frame.size.h = 40;
          display_target_layer(p_window, &temp_frame, GOAL);
        } */
    }
  else      //运动模式
    {
      if (strlen(weather_info) > 0)
        {
          memset(buff, 0, sizeof(buff));
          if (g_level == 0)
            sprintf(buff, "%s,%d'C", weather_info, g_temperature);
          else
            sprintf(buff, "%s%d'C,%s", weather_info, g_temperature, level_aqi_str[g_level]);
          display_target_layerText(p_window, &sportFrames[0], GAlignTop, GColorBlack, buff, U_GBK_SIMSUN_16, weathercolor[g_level]);
        }
      sprintf(buff, "%02d:%02d", walk_time / 60, walk_time % 60);
      g_walktime_id = display_target_layerText(p_window, &sportFrames[0], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_30, GColorWhite);

      sprintf(buff, "%02d:%02d", dt.hour, dt.min);
      display_target_layerText(p_window, &sportFrames[1], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_20, GColorWhite);
      sprintf(buff, "%d", walk_step);
      g_step_id = display_target_layerText(p_window, &sportFrames[2], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_20, GColorWhite);

      sprintf(buff, "%d.%d", speed / 10, speed % 10);
      display_target_layerText(p_window, &sportFrames[3], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_30, GColorWhite);

      sprintf(buff, "km/h");
      display_target_layerText(p_window, &sportFrames[4], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_16, GColorWhite);

      sprintf(buff, "%d.%02d", distance / 1000, distance % 1000 / 10);
      g_distance_id = display_target_layerText(p_window, &sportFrames[5], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_30, GColorWhite);

      sprintf(buff, "km");
      display_target_layerText(p_window, &sportFrames[6], GAlignCenter, GColorBlack, buff, U_ASCII_ARIAL_16, GColorWhite);

      //if (deltastep < 80 && start_time > 0) maibu_service_vibes_pulse(VibesPulseTypeShort, 1);

      i = deltastep > 79 ? (deltastep - 60) / 20 : 0;
      if (i > 5) i = 5;
      sprintf(buff, "%d", deltastep);
      display_target_layerText(p_window, &sportFrames[7], GAlignTop, GColorBlack, buff, U_ASCII_ARIAL_30, speedcolor[i]);
      sprintf(buff, "%s", stage_str[i]);
      display_target_layerText(p_window, &sportFrames[7], GAlignBottom, GColorBlack, buff, U_GBK_SIMSUN_16, speedcolor[i]);

      display_target_layer(p_window, &sportFrames[8], stage[i]);


      //绘制网格
      Geometry *p_g1[1];
      Line l[3] =
      {
        {{0, 60}, {176, 60}},
        {{0, 120}, {176, 120}},
        {{88, 60}, {88, 120}}
      };
      LineSet ls = {3, l};
      Geometry lg = {GeometryTypeLineSet, FillOutline, speedcolor[i], (void*)&ls};
      p_g1[0] = &lg;
      LayerGeometry cell_struct = {1, p_g1};
      p_layer = app_layer_create_geometry(&cell_struct);
      app_window_add_layer(p_window, p_layer);
      //显示上公里配速
      if (show_pace_flag > 0 && pace > 240 && pace < 1200)
        {
          Geometry *p_g1[1];
          GPoint box[4] = { {0, 100}, {176, 100}, {176, 130}, {0, 130}};
          Polygon background   = {4, box};
          Geometry geometry = {GeometryTypePolygon, FillArea, GColorYellow, (void*)&background};
          p_g1[0] = &geometry;
          LayerGeometry pace_struct = {1, p_g1};
          p_layer = app_layer_create_geometry(&pace_struct);
          app_window_add_layer(p_window, p_layer);

          sprintf(buff, "上公里配速");
          display_target_layerText(p_window, &sportFrames[9], GAlignLeft, GColorYellow, buff, U_GBK_SIMSUN_16, GColorBlack);

          sprintf(buff, "%d'%02d\"", pace / 60, pace % 60);
          display_target_layerText(p_window, &sportFrames[9], GAlignRight, GColorYellow, buff, U_ASCII_ARIAL_30, GColorBlack);
        }

    }



  /*添加按键事件*/
  app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);

  return p_window;
}

static void init_register_handle(void)
{
  //创建空间储存城市配置
  app_persist_create(CITY_PRESIST_DATA_KEY, sizeof(g_city));
  //读取城市配置
  memset(g_city_temp, 0, sizeof(g_city_temp));
  app_persist_read_data(CITY_PRESIST_DATA_KEY, 0, g_city_temp, sizeof(g_city_temp));


  //注册手机设置回调函数
  maibu_comm_register_watchapp_callback(watchapp_comm_callback);

  //注册接受请求城市数据回调函数
  maibu_comm_register_phone_callback(get_city_info_callback);


  /*注册通讯结果回调*/
  maibu_comm_register_result_callback(weather_comm_result_callback);

  //判断是否自动查询
  if (strlen(g_city_temp) == 0)
    {
      g_auto_weather_flag = 1;
      request_get_city_info();
    }
  else
    {
      g_auto_weather_flag = 0;
      memcpy(g_city, g_city_temp, sizeof(g_city_temp));
      request_weather_info(g_city);
    }

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
  watch_mode = 1;
  /*创建显示表盘窗口*/
  P_Window p_window = init_watch();
  if (p_window != NULL)
    {
      /*放入窗口栈显示*/

      g_window_id = app_window_stack_push(p_window);
    }
  /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);
  init_register_handle();

  return 0;
}



