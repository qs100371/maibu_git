#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maibu_sdk.h"
#include "maibu_res.h"

//显示界面定制
#define APP_SETUP_KEY 666
static uint8_t show_status, show_ap, show_alt, show_weather, show_step, show_speed, show_floor; //分别对应状态、气压、海拔、天气、计步、速度
//删除月相功能
/*窗口句柄*/
static int32_t g_window = -1;
/*全局定时器*/
static int8_t g_timer_id = -1, ap_scale = 1, alt_scale = 1;

// weather source 1

/*GPS及海拔数据结构*/
static SGpsAltitude g_gps_altitude;

/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;

/*Phone通讯ID*/
static uint32_t g_comm_id_gps = 0;

static int32_t ow_icon = 0, ow_temp = 125;


/*获取天气数据云服务器地址*/
#define WEATHER_WEB     "http://api.openweathermap.org/data/2.5/weather?appid=6f884022975ba0c11422a597dec48ddf&units=metric&cnt=1&"


//weather source 2
static uint8_t g_auto_weather_flag     = 1;//标志是否自动查询天气
static char    get_city_context[35]    = {0x01};//协议ID

static uint32_t g_comm_id_get_city = 0, g_comm_id_web_weather   = 0;//获取城市的通讯ID 获取天气的通讯ID

static char    g_city[20] = {0}, g_city_temp[20] = {0}, weather_info[24] = {0};

static int32_t g_temperature = 0, g_level  = 0;

#define WEATHER_KEY			"code,temp,info,aqi_level"

#define WEATHER_URL		"http://maibu.cc/ThirdPartyData/weather/getRealTimeWeather.do?city="

#define CITY_PRESIST_DATA_KEY 100

const static int32_t level_aqi_pic[6] = {AQI1, AQI2, AQI3, AQI4, AQI5, AQI6};
//static enum GColor weathercolor[7] = {GColorBlack, GColorGreen, GColorGreen, GColorBlue, GColorPurple, GColorRed, GColorBlack};
/////
static void init_register_handle(void);
static void request_get_city_info(void);
/////
void window_reloading(void);
static uint8_t hour_pos[36][3][2] =
{
//0:
  {{88, 49}, {83, 88}, {94, 88}},
//0:20:
  {{95, 50}, {83, 87}, {94, 90}},
//0:40:
  {{102, 51}, {83, 87}, {94, 90}},

//1:
  {{109, 54}, {84, 86}, {93, 91}},
//1:20:
  {{115, 60}, {86, 84}, {93, 91}},
//1:40:
  {{119, 64}, {86, 84}, {93, 91}},

//2:
  {{123, 69}, {86, 84}, {91, 93}},
//2:20:
  {{126, 76}, {87, 83}, {90, 94}},
//2:40:
  {{127, 82}, {88, 83}, {90, 94}},

//3:
  {{127, 88}, {88, 83}, {88, 94}},
//3:20:
  {{127, 94}, {90, 83}, {88, 94}},
//3:40:
  {{126, 99}, {90, 83}, {87, 93}},

//4:
  {{123, 106}, {90, 83}, {84, 93}},
//4:20:
  {{119, 112}, {91, 84}, {84, 91}},
//4:40:
  {{115, 116}, {91, 84}, {84, 91}},

//5:
  {{109, 121}, {93, 84}, {84, 90}},
//5:20:
  {{102, 124}, {93, 86}, {83, 88}},
//5:40:
  {{95, 126}, {93, 86}, {83, 88}},

//6:
  {{88, 127}, {94, 87}, {83, 87}},
//6:20:
  {{80, 126}, {94, 90}, {83, 87}},
//6:40:
  {{73, 124}, {94, 90}, {84, 86}},

//7:
  {{66, 121}, {94, 90}, {84, 84}},
//7:20:
  {{61, 116}, {93, 93}, {84, 84}},
//7:40:
  {{57, 112}, {91, 93}, {84, 84}},

//8:
  {{53, 106}, {91, 93}, {84, 84}},
//8:20:
  {{50, 99}, {90, 94}, {84, 84}},
//8:40:
  {{49, 94}, {87, 94}, {87, 83}},

//9:
  {{49, 88}, {87, 94}, {87, 83}},
//9:20:
  {{49, 82}, {87, 94}, {87, 83}},
//9:40:
  {{50, 76}, {84, 93}, {88, 83}},

//10:
  {{53, 69}, {84, 93}, {90, 83}},
//10:20:
  {{57, 64}, {84, 93}, {90, 83}},
//10:40:
  {{61, 60}, {84, 91}, {91, 84}},

//11:
  {{66, 54}, {83, 90}, {93, 84}},
//11:20:
  {{73, 51}, {83, 90}, {93, 86}},
//11:40:
  {{80, 50}, {83, 90}, {94, 87}},

};


static uint8_t min_pos[60][3][2] =
{
//0分：
  {{88, 18}, {84, 87}, {93, 87}},
//1分：
  {{95, 17}, {86, 86}, {93, 86}},
//2分：
  {{104, 18}, {86, 86}, {94, 86}},
//3分：
  {{110, 20}, {86, 86}, {94, 87}},
//4分：
  {{117, 24}, {86, 84}, {93, 88}},
//5分：
  {{124, 27}, {86, 84}, {94, 88}},
//6分：
  {{130, 31}, {86, 86}, {93, 91}},
//7分：
  {{135, 35}, {86, 86}, {93, 91}},
//8分：
  {{141, 42}, {87, 84}, {91, 93}},
//9分：
  {{145, 47}, {87, 84}, {91, 93}},
//10分：
  {{149, 53}, {88, 84}, {91, 93}},
//11分：
  {{153, 60}, {87, 84}, {91, 93}},
//12分：
  {{156, 66}, {88, 84}, {90, 93}},
//13分：
  {{157, 73}, {90, 84}, {90, 93}},
//14分：
  {{159, 82}, {90, 84}, {90, 93}},
//15分：
  {{159, 88}, {88, 84}, {88, 94}},
//16分：
  {{160, 95}, {91, 84}, {91, 93}},
//17分：
  {{157, 104}, {90, 84}, {90, 93}},
//18分：
  {{156, 110}, {90, 84}, {88, 93}},
//19分：
  {{153, 117}, {90, 84}, {87, 93}},
//20分：
  {{149, 124}, {91, 86}, {88, 94}},
//21分：
  {{145, 130}, {91, 86}, {86, 91}},
//22分：
  {{141, 135}, {91, 86}, {87, 93}},
//23分：
  {{135, 141}, {91, 84}, {86, 91}},
//24分：
  {{130, 145}, {93, 86}, {86, 91}},
//25分：
  {{123, 149}, {93, 86}, {87, 94}},
//26分：
  {{117, 152}, {93, 87}, {84, 91}},
//27分：
  {{110, 156}, {93, 88}, {84, 90}},
//28分：
  {{104, 157}, {93, 90}, {84, 90}},
//29分：
  {{95, 159}, {93, 90}, {84, 90}},
//30分：
  {{88, 159}, {84, 88}, {94, 91}},
//31分：
  {{80, 159}, {93, 90}, {84, 90}},
//32分：
  {{73, 157}, {93, 90}, {84, 90}},
//33分：
  {{66, 156}, {93, 91}, {84, 90}},
//34分：
  {{60, 153}, {93, 91}, {84, 88}},
//35分：
  {{53, 149}, {93, 90}, {84, 88}},
//36分：
  {{47, 145}, {91, 93}, {84, 87}},
//37分：
  {{40, 141}, {90, 94}, {86, 86}},
//38分：
  {{35, 135}, {91, 93}, {86, 86}},
//39分：
  {{31, 130}, {91, 93}, {84, 87}},
//40分：
  {{27, 124}, {88, 94}, {84, 86}},
//41分：
  {{22, 117}, {86, 94}, {84, 87}},
//42分：
  {{20, 110}, {87, 94}, {86, 86}},
//43分：
  {{18, 104}, {86, 93}, {88, 84}},
//44分：
  {{17, 95}, {86, 93}, {86, 84}},
//45分：
  {{18, 88}, {88, 93}, {88, 84}},
//46分：
  {{17, 82}, {87, 94}, {86, 86}},
//47分：
  {{18, 73}, {86, 93}, {86, 84}},
//48分：
  {{20, 66}, {88, 93}, {84, 83}},
//49分：
  {{22, 60}, {84, 91}, {88, 84}},
//50分：
  {{27, 53}, {88, 93}, {87, 83}},
//51分：
  {{31, 47}, {86, 91}, {90, 84}},
//52分：
  {{35, 42}, {84, 91}, {90, 84}},
//53分：
  {{40, 35}, {86, 91}, {90, 84}},
//54分：
  {{47, 31}, {84, 90}, {91, 84}},
//55分：
  {{53, 27}, {91, 84}, {84, 90}},
//56分：
  {{60, 24}, {84, 88}, {91, 84}},
//57分：
  {{66, 20}, {93, 84}, {84, 87}},
//58分：
  {{73, 18}, {84, 86}, {93, 86}},
//59分：
  {{80, 17}, {84, 86}, {93, 86}},

};
//const static char lunar_day[30][8] = {"初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十", "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十", "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"};
//const static int32_t moon_phases[30] = {MOON1, MOON2, MOON2, MOON2, MOON2, MOON2, MOON3, MOON3, MOON4, MOON4, MOON4, MOON4, MOON4, MOON4, MOON5, MOON5, MOON6, MOON6, MOON6, MOON6, MOON6, MOON7, MOON7, MOON8, MOON8, MOON8, MOON8, MOON8, MOON8, MOON8};
static int32_t ap_data[25] = {0}, altitude_data[25];// ap_max = 0, ap_min = 0, alt_max = 0, alt_min = 0;

static char stage_str[6][8] = {"  ", "热身", "燃脂", "心肺", "耐力", "极限"};//热身、燃脂、心肺、耐力、极限
static enum GColor speedcolor[6] = { GColorBlack, GColorBlack, GColorGreen, GColorBlue, GColorPurple, GColorRed};

const static char wday_str[7][8] = {"日", "一", "二", "三", "四", "五", "六"};

static int32_t deltastep = 0 , speed = 0, pace = 0;
static uint32_t updatetime = 0;
static SportData last_data = {0};

static P_Window init_watch();
//
//请求天气数据回调
static void weather_info_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[20] = {0};

  int32_t temp_num = 0;
  maibu_get_json_int(buff, "code", &temp_num);
  if (temp_num != 0)
    {
      memset(weather_info, 0, sizeof(weather_info));
      return;
    }
  g_temperature = 0;
  g_level = 0;
  memset(weather_info, 0, sizeof(weather_info));
  maibu_get_json_int(buff, "temp", &g_temperature);
  maibu_get_json_int(buff, "aqi_level", &g_level);
  maibu_get_json_str(buff, "info", weather_info, sizeof(weather_info));
  window_reloading();
}

//请求天气数据
static void request_weather_info(char * city_name)
{
  char url[200] = {0};
  sprintf(url, "%s%s", WEATHER_URL, city_name);

  maibu_comm_register_web_callback(weather_info_callback);
  g_comm_id_web_weather = maibu_comm_request_web(url, WEATHER_KEY, 1800); //15 * 60* get_front_or_back_flag()

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

//

/*根据图标类型获取对应的天气图标*/
int32_t weather_get_icon_key(int32_t icon_type)
{

  int32_t type = RES_WEATHER_CLOUDY;
  if (icon_type == 0)
    {
      type = RES_WEATHER_NODATA;
    }
  else if (icon_type == 1)
    {
      type = RES_WEATHER_SUNNY;
    }
  else if (icon_type == 2 || icon_type == 3)
    {
      type = RES_WEATHER_CLOUDY;
    }
  else if (icon_type == 4)
    {
      type = RES_WEATHER_OVERCAST;
    }
  else if (icon_type == 9 || icon_type == 10 || icon_type == 11)
    {
      type = RES_WEATHER_RAIN;
    }
  else if (icon_type == 13)
    {
      type = RES_WEATHER_SNOW;
    }
  else if (icon_type == 50)
    {
      type = RES_WEATHER_FOG;
    }
  else
    {
      type = RES_WEATHER_CLOUDY;
    }
  return type;
}



//重新载入并刷新窗口所有图层
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

void weather_web_recv_callback(const uint8_t *buff, uint16_t size)
{
  int32_t icon_tmp = 0, tempera_tmp = 0;
  if ((-1 == maibu_get_json_int(buff, "icon", &icon_tmp)) || (-1 == maibu_get_json_int(buff, "temp", &tempera_tmp)))
    {
      return;
    }
  ow_icon = icon_tmp;
  ow_temp = tempera_tmp;
  window_reloading();
}
void weather_request_web()
{
  /*拼接url请求地址, 注意url的缓存大小*/
  char url[200] = "";
  sprintf(url, "%slat=%s&lon=%s", WEATHER_WEB, g_gps_altitude.lat, g_gps_altitude.lon);
  /*30分钟同步一次天气数据*/
  maibu_comm_register_web_callback(weather_web_recv_callback);
  g_comm_id_web = maibu_comm_request_web(url, "temp,icon", 1800);
}


void weather_phone_recv_callback(enum ERequestPhone type, void *context)
{

  /*如果不是GPS海拔数据， 退出*/
  if (type != ERequestPhoneGPSAltitude)
    {
      return;
    }
  /*提取经度纬度*/
  memcpy(&g_gps_altitude, (SGpsAltitude *)context, sizeof(SGpsAltitude));
  /*请求Web获取天气数据*/
  weather_request_web();
}
static void weather_comm_result_callback2(enum ECommResult result, uint32_t comm_id, void *context)
{
  /*如果上一次请求GPS通讯失败，并且通讯ID相同，则重新发送*/
  if (result == ECommResultFail)
    {
      if (comm_id == g_comm_id_get_city)
        {
          if (strlen(g_city) >= 4)
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

static void init_register_handle(void)
{
  //创建空间储存城市配置
  app_persist_create(CITY_PRESIST_DATA_KEY, sizeof(g_city));
  //读取城市配置
  memset(g_city_temp, 0, sizeof(g_city_temp));
  app_persist_read_data(CITY_PRESIST_DATA_KEY, 0, g_city_temp, sizeof(g_city_temp));

  //注册接受请求城市数据回调函数
  maibu_comm_register_phone_callback(get_city_info_callback);

  /*注册通讯结果回调*/
  maibu_comm_register_result_callback(weather_comm_result_callback2);

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
////



static void watch_time_change(enum SysEventType type, void *context)
{
  /*时间更改*/
  if (type == SysEventTypeTimeChange)
    {
      window_reloading();
    }
}

void weather_comm_result_callback(enum ECommResult result, uint32_t comm_id, void *context)
{
  /*如果上一次请求GPS通讯失败，并且通讯ID相同，则重新发送*/
  if ((result == ECommResultFail) && (comm_id == g_comm_id_gps))
    {
      g_comm_id_gps = maibu_comm_request_phone(ERequestPhoneGPSAltitude,  NULL, 0);
    }

  /*如果上一次请求WEB通讯失败，并且通讯ID相同，则重新发送*/
  if ((result == ECommResultFail) && (comm_id == g_comm_id_web))
    {
      weather_request_web();
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

static void timer_callback(date_time_t tick_time, uint32_t millis, void *context)
{
  float f_temp = 0;
  int i;
  char buff[20];

  if (maibu_get_altitude(&f_temp, 0) == 0)  //海拔单位 米
    {
      //sprintf(buff, "%.0f", (f_temp - 0.0)*10);
      for (i = 0;i < 24;i++)
        altitude_data[i] = altitude_data[i+1];
      altitude_data[24] = (int)f_temp;//atoi(buff)
    }
  f_temp = 0;
  if (maibu_get_pressure(&f_temp) == 0)       //气压单位10Pa
    {
      sprintf(buff, "%.0f", (f_temp - 0.0)*10);
      for (i = 0;i < 24;i++)
        ap_data[i] = ap_data[i+1];
      ap_data[24] = atoi(buff);
    }
  ap_data[24] = ap_data[24] + altitude_data[24];  //校正气压
  return;
}

P_Layer plot_data(int32_t data[], int32_t ZERO, int8_t *scale, enum GColor color)
{
  uint16_t x0 = 172, y0 = 176, w = 7;
  int i, num = 0;
  int32_t  min = data[24], max = min;
  for (i = 24;i >= 0;i--)
    if (data[i] != ZERO) num++;
    else
      break;

  if (num < 2) return NULL;
  for (i = 23;i > 24 - num;i--)
    {
      if (data[i] < min) min = data[i];
      if (data[i] > max) max = data[i];
    }
  *scale = (max - min) / 100 + 1;

  Line l[num-1];
  for (i = 0; i < num - 1; i++)
    {
      l[i].p0.x = x0 - i * w;
      l[i].p0.y = y0 - (data[24-i] - min) / *scale;
      l[i].p1.x = x0 - (i + 1) * w;
      l[i].p1.y = y0 - (data[23-i] - min) / *scale;
    }

  LineSet ls = {num - 1, l};
  Geometry lsg = {GeometryTypeLineSet, FillOutline, color, (void*)&ls};
  P_Geometry p_g[1];
  p_g[0] = &lsg;
  LayerGeometry lg = {1, p_g};

  P_Layer	layer1 = NULL;
  layer1 = app_layer_create_geometry(&lg);
  return (layer1);
}

P_Layer get_circle_layer(enum GColor color)
{
  GPoint center1 = {88, 88};
  Circle c1 = {center1, 4};
  Geometry cg1 = {GeometryTypeCircle, FillArea, color, (void*)&c1};
  P_Geometry p_g[1];
  p_g[0] = &cg1;
  LayerGeometry lg = {1, p_g};

  P_Layer	 layer1 = NULL;
  layer1 = app_layer_create_geometry(&lg);

  return (layer1);
}
P_Layer get_time_hand_layer(uint8_t min, uint8_t time_pos[][3][2], enum GColor color)
{
  GPoint p1 = {time_pos[min][0][0], time_pos[min][0][1]}, p2 = {time_pos[min][1][0], time_pos[min][1][1]}, p3 = {time_pos[min][2][0], time_pos[min][2][1]};

  GPoint points1[3] = {p1, p2, p3};
  Polygon po1 = {3, points1};
  Geometry pg1 = {GeometryTypePolygon, FillArea, color, (void*)&po1};

  P_Geometry p_g[1];
  p_g[0] = &pg1;
  LayerGeometry lg = {1, p_g};

  P_Layer	 layer1 = NULL;
  layer1 = app_layer_create_geometry(&lg);


  return layer1;
}
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

/* uint16_t get_lunar_day()
{
  SLunarData sld = {0};
  maibu_get_lunar_calendar(NULL, &sld);
  char buff[20] = {0};
  sprintf(buff, "%s", sld.day);
  int i = 0;
  for (i = 0;i < 30;i++)
    if (strcmp(buff, lunar_day[i]) == 0)
      return i;
} */

static void watchapp_comm_callback(enum ESyncWatchApp type, uint8_t *buf, uint16_t len)
{

  if (type == ESyncWatchAppUpdateParam)
    {
      if (len >= 7)
        {
          if (buf[0] == '0')
            show_status = 0;
          else
            show_status = 1;
          if (buf[1] == '0')
            show_ap = 0;
          else
            show_ap = 1;
          if (buf[2] == '0')
            show_alt = 0;
          else
            show_alt = 1;
          if (buf[3] == '0')
            show_weather = 0;
          else if (buf[3] == '1')
            show_weather = 1;
          else
            show_weather = 2;
          if (buf[4] == '0')
            show_step = 0;
          else
            show_step = 1;
          if (buf[5] == '0')
            show_speed = 0;
          else
            show_speed = 1;
          if (buf[6] == '0')
            show_floor = 0;
          else
            show_floor = 1;
          int i;
          if (buf[7] == 'c' || buf[7] == 'C')  //清空气压海拔数据
            {
              memset(ap_data, 0, sizeof(ap_data));
              for (i = 0;i < 25;i++)
                altitude_data[i] = -10000;
              ap_scale = 1;
              alt_scale = 1;
            }
          if ((len > 8) && buf[8] == '\n')
            {
              g_auto_weather_flag = 0;
              memset(weather_info, 0, sizeof(weather_info));
              for (i = 9;*(buf + i) == ' ';i++);
              memcpy(g_city, buf + i, len - i);
              g_city[len-i] = '\0';
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

          char buff[10] = {0};
          sprintf(buff, "%07d", show_status*1000000 + show_ap*100000 + show_alt*10000 + show_weather*1000 + show_step*100 + show_speed*10 + show_floor);
          app_persist_write_data_extend(APP_SETUP_KEY, buff, sizeof(buff));
        }
      else
        {
          app_persist_delete_data(APP_SETUP_KEY);
          show_status = 0;
          show_ap = 0;
          show_alt = 0;
          show_weather = 2;
          show_step = 1;
          show_speed = 1;
          show_floor = 0;
        }
      window_reloading();
    }
}


static P_Window init_watch()
{
  P_Window p_window = NULL;
  P_Layer p_layer = NULL;

  P_Layer hl = NULL, ml = NULL, cl1 = NULL, apl = NULL, altl = NULL;
  static SportData data;
  maibu_get_sport_data(&data, 0);

  if (show_speed)
    {
      //计算速度、配速
      uint32_t updatetime1 = app_get_time_number();

      if (updatetime1 - updatetime == 60)
        {
          deltastep = data.step - last_data.step;
          if (deltastep >= 80)
            {
              pace = 6000000 / (data.distance - last_data.distance);
              speed = (data.distance - last_data.distance) * 60 / 10000;
            }
          last_data = data;
          updatetime = updatetime1;
        }
      else if (updatetime1 - updatetime > 60)
        {
          deltastep = 0;
          pace = 0;
          speed = 0;
          last_data = data;
          updatetime = updatetime1;
        }
      //
    }

  char buff[30] = {0};
  /*创建一个窗口*/
  p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);
  enum GColor colors[] = {GColorRed, GColorGreen, GColorBlue, GColorPurple}; //,GColorYellow

  GRect frame = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame, RES_WATCHFACE_OFFICIAL_BG);

  int i;
  float f_temp = 0;
  if (show_status)
    {
      if (maibu_get_altitude(&f_temp, 0) == 0)
        {
          /* sprintf(buff, "%.0f", (f_temp - 0.0)*10);
          i = atoi(buff);
          sprintf(buff, "%d.%dm", i / 10, i % 10); */
          frame.origin.x = 0;
          frame.origin.y = 0;
          frame.size.h = 16;
          frame.size.w = 60;
          sprintf(buff, "%dm", (int)f_temp);
          display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
        }

      f_temp = 0;
      if (maibu_get_temperature(&f_temp) == 0)
        {
          frame.origin.x = 116;
          frame.origin.y = 0;
          frame.size.h = 16;
          frame.size.w = 60;
          i = (int)f_temp;
          i = i > 23 ? (2 * i - 36) : (i * 1.5 - 18);
          sprintf(buff, "%d'C", i);
          display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
        }
    }
  /* if (show_mp)
    {
      //显示月相

      frame.origin.x = 24;
      frame.origin.y = 68;
      frame.size.h = 40;
      frame.size.w = 40;
      display_target_layer(p_window, &frame, moon_phases[get_lunar_day()]);
    } */

  frame.origin.x = 112;
  frame.origin.y = 80;
  frame.size.h = 16;
  frame.size.w = 20;
  sprintf(buff, "%2d", dt.mday);
  display_target_layerText(p_window, &frame, GAlignRight, GColorBlue, buff, U_ASCII_ARIAL_16, GColorWhite);

  frame.origin.x = 132;
  frame.origin.y = 80;
  frame.size.h = 16;
  frame.size.w = 16;
  sprintf(buff, "%s", wday_str[dt.wday]);
  display_target_layerText(p_window, &frame, GAlignLeft, GColorRed, buff, U_GBK_SIMSUN_16, GColorWhite);

  if (show_step)
    {
      frame.origin.x = 20;
      frame.origin.y = 116;
      frame.size.h = 20;
      frame.size.w = 136;
      if (show_floor)
        sprintf(buff, "%d+%d", data.step, data.floor);
      else
        sprintf(buff, "%d", data.step);
      display_target_layerText(p_window, &frame, GAlignCenter, GColorBlue, buff, U_ASCII_ARIAL_16, GColorWhite);
    }

  if (deltastep < 80)
    {

      if (show_status)
        {
          f_temp = 0;
          if (maibu_get_pressure(&f_temp) == 0)
            {
              frame.origin.x = 0;
              frame.origin.y = 160;
              frame.size.h = 16;
              frame.size.w = 100;
              sprintf(buff, "%dhPa", (int)f_temp);
              display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
            }

          int8_t percent = 0;
          maibu_get_battery_percent(&percent);
          frame.origin.x = 96;
          frame.origin.y = 160;
          frame.size.h = 16;
          frame.size.w = 80;
          sprintf(buff, "%d%%", percent);
          display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
        }

      if (show_weather == 1)
        {
          frame.origin.x = 68;
          frame.origin.y = 20;
          frame.size.h = 40;
          frame.size.w = 40;
          display_target_layer(p_window, &frame, weather_get_icon_key(ow_icon));

          frame.origin.x = 58;
          frame.origin.y = 60;
          frame.size.h = 20;
          frame.size.w = 60;
          if (ow_temp == 125)
            {
              strcpy(buff, "--");
            }
          else
            {
              sprintf(buff, "%d'C", ow_temp);
            }

          display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
        }
      else if (show_weather == 2)
        {
          if (strlen(weather_info) > 0)
            {
              frame.origin.x = 68;
              frame.origin.y = 44;
              frame.size.h = 20;
              frame.size.w = 40;
              sprintf(buff, "%d'C", g_temperature);
              display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
              if (g_level > 0)
                {
                  frame.origin.x = 88;
                  frame.origin.y = 24;
                  frame.size.h = 20;
                  frame.size.w = 40;
                  display_target_layer(p_window, &frame, level_aqi_pic[g_level-1]);

                  frame.origin.x = 28;
                  frame.origin.y = 24;
                  frame.size.h = 20;
                  frame.size.w = 60;
                  display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, weather_info, U_GBK_SIMSUN_16, GColorBlack);
                }
              else
                {
                  frame.origin.x = 58;
                  frame.origin.y = 24;
                  frame.size.h = 20;
                  frame.size.w = 60;
                  display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, weather_info, U_GBK_SIMSUN_16, GColorBlack);
                }
            }
        }
    }
  else  //步频大于80时显示
    {
      i = (deltastep - 60) / 20 ;
      if (i > 5) i = 5;

      if (show_speed)
        {
          frame.origin.x = 0;
          frame.origin.y = 156;
          frame.size.h = 20;
          frame.size.w = 80;
          sprintf(buff, "%d'%02d\"", pace / 60, pace % 60);
          display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_20, speedcolor[i]);

          frame.origin.x = 86;
          frame.origin.y = 156;
          frame.size.h = 20;
          frame.size.w = 90;

          sprintf(buff, "x%d.%d|%d.%d", speed / 10, speed % 10, data.distance / 100000, (data.distance / 10000) % 10);
          display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, buff, U_ASCII_ARIAL_20, speedcolor[i]);



          frame.origin.x = 68;
          frame.origin.y = 20;
          frame.size.h = 10;
          frame.size.w = 40;
          display_target_layer(p_window, &frame, STAGE);


          frame.origin.x = 68;
          frame.origin.y = 30;
          frame.size.h = 16;
          frame.size.w = 40;

          sprintf(buff, "%s", stage_str[i]);
          display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIAL_16, speedcolor[i]);

          frame.origin.x = 68;
          frame.origin.y = 48;
          frame.size.h = 20;
          frame.size.w = 40;

          sprintf(buff, "%d", deltastep);
          display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIAL_20, speedcolor[i]);
        }
    }

  if (show_ap)
    {
      frame.origin.x = 136;
      frame.origin.y = 116;
      frame.size.h = 16;
      frame.size.w = 40;
      sprintf(buff, "x%d", ap_scale);
      //sprintf(buff, "%d", altitude_data[24]);
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorRed);
      apl = plot_data(ap_data, 0, &ap_scale, GColorRed);
      app_window_add_layer(p_window, apl);
    }
  if (show_alt)
    {
      frame.origin.x = 136;
      frame.origin.y = 136;
      frame.size.h = 16;
      frame.size.w = 40;

      sprintf(buff, "x%d", alt_scale);
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlue);
      altl = plot_data(altitude_data, -10000, &alt_scale, GColorBlue);
      app_window_add_layer(p_window, altl);

    }

  ml = get_time_hand_layer(dt.min, min_pos, colors[dt.min%4]);

  hl = get_time_hand_layer(dt.hour % 12 * 3 + dt.min / 20, hour_pos, colors[dt.hour%4]);

  cl1 = get_circle_layer(GColorWhite);
  app_window_add_layer(p_window, hl);
  app_window_add_layer(p_window, ml);
  app_window_add_layer(p_window, cl1);

  if (g_timer_id == -1 && dt.min % 10 == 0) //
    {
      for (i = 0;i < 25;i++)
        altitude_data[i] = -10000;
      g_timer_id = app_service_timer_subscribe(600000, timer_callback, NULL);//600000
    }

  return p_window;
}


int main()
{
  //读取配置文件
  char buff[10] = {0};
  app_persist_create(APP_SETUP_KEY, sizeof(buff));
  app_persist_read_data(APP_SETUP_KEY, 0, buff, sizeof(buff));
  if (strlen(buff) >= 7)
    {
      if (buff[0] == '0')
        show_status = 0;
      else
        show_status = 1;
      if (buff[1] == '0')
        show_ap = 0;
      else
        show_ap = 1;
      if (buff[2] == '0')
        show_alt = 0;
      else
        show_alt = 1;
      if (buff[3] == '0')
        show_weather = 0;
      else if (buff[3] == '1')
        show_weather = 1;
      else
        show_weather = 2;
      if (buff[4] == '0')
        show_step = 0;
      else
        show_step = 1;
      if (buff[5] == '0')
        show_speed = 0;
      else
        show_speed = 1;
      if (buff[6] == '0')
        show_floor = 0;
      else
        show_floor = 1;

    }
  else
    {
      show_status = 0;
      show_ap = 0;
      show_alt = 0;
      show_weather = 2;
      show_step = 1;
      show_speed = 1;
      show_floor = 0;
    }

  /*创建显示表盘窗口*/
  P_Window p_window = init_watch();
  if (p_window != NULL)
    {
      /*放入窗口栈显示*/

      g_window = app_window_stack_push(p_window);
    }
  /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);

  maibu_comm_register_watchapp_callback(watchapp_comm_callback);
  if (show_weather == 1)
    {
      /*注册接受手机数据回调函数*/
      maibu_comm_register_phone_callback((CBCommPhone)weather_phone_recv_callback);
      /*请求GPS数据*/
      g_comm_id_gps = maibu_comm_request_phone(ERequestPhoneGPSAltitude,  NULL, 0);
      /*注册通讯结果回调*/
      maibu_comm_register_result_callback(weather_comm_result_callback);
    }
  else if (show_weather == 2)
    {
      init_register_handle();
    }
  return 0;
}



