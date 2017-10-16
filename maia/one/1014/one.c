#include <stdlib.h>
#include <stdio.h>

#include "maibu_sdk.h"
#include "maibu_res.h"

/*窗口句柄*/
static uint32_t g_window = -1;
/*全局定时器*/
static int8_t g_timer_id = -1, scale = 1;
/*GPS及海拔数据结构*/
static SGpsAltitude g_gps_altitude;

/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;

/*Phone通讯ID*/
static uint32_t g_comm_id_gps = 0;

/*存放当前天气图标KEY*/
#define WEATHER_ICON_KEY	1
/*存放当前天气温度图标*/
#define WEATHER_TEMP_KEY	2

/*如果没有获取到天气图标，设置特殊天气图标*/
#define WEATHER_ICON_SPEC	0
/*如果没有获取到温度，设置特殊温度标志*/
#define WEATHER_TEMP_SPEC	125


/*获取天气数据云服务器地址*/
#define WEATHER_WEB     "http://api.openweathermap.org/data/2.5/weather?appid=6f884022975ba0c11422a597dec48ddf&units=metric&cnt=1&"

/*图片ID*/

/*no data*/
#define WEATHER_ICON_NO_DATA		0

/*clear sky*/
#define WEATHER_ICON_CLEAR_SKY		1

/*few clouds*/
#define WEATHER_ICON_FEW_CLOUDS		2

/*scattered clouds*/
#define WEATHER_ICON_SCATTERED_CLOUDS	3

/*broken clouds*/
#define WEATHER_ICON_BROKEN_CLOUDS	4

/*shower rain*/
#define WEATHER_ICON_SHOWER_RAIN	9

/*rain*/
#define WEATHER_ICON_RAIN		10

/*thunderstorm*/
#define WEATHER_ICON_THUNDERSTORM	11

/*snow*/
#define WEATHER_ICON_SNOW		13

/*mist*/
#define WEATHER_ICON_MIST		50

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

static uint32_t ap_data[25] = {0};

const static char wday_str[7][8] = {"日", "一", "二", "三", "四", "五", "六"};

static uint32_t deltastep = 0 , speed = 0, pace = 0;
static uint32_t updatetime = 0;
static SportData last_data = {0};

static P_Window init_watch(void);
void weather_init_store()
{
  /*创建保存天气图标文件*/
  app_persist_create(WEATHER_ICON_KEY, 1);

  /*先读取天气图标, 如果是第一次，设置默认值*/
  uint8_t icon_type = 0;
  int8_t icon_read_size = 0;
  icon_read_size = app_persist_read_data(WEATHER_ICON_KEY, 0, (unsigned char *) & icon_type, sizeof(int8_t));
  /*第一次使用, 显示默认值*/
  if (icon_read_size == 0)
    {
      icon_type = WEATHER_ICON_SPEC;
      app_persist_write_data_extend(WEATHER_ICON_KEY, (unsigned char *)&icon_type, sizeof(int8_t));
    }
  /*创建保存温度文件*/
  app_persist_create(WEATHER_TEMP_KEY, 1);
  /*先读取温度, 如果第一次，设置默认值*/
  int8_t temp = 0;
  int8_t temp_read_size = 0;
  temp_read_size = app_persist_read_data(WEATHER_TEMP_KEY, 0, (unsigned char *) & temp, sizeof(int8_t));
  if (temp_read_size == 0)
    {
      temp = WEATHER_TEMP_SPEC;
      app_persist_write_data_extend(WEATHER_TEMP_KEY, (unsigned char *)&temp, sizeof(int8_t));
    }
}

/*根据图标类型获取对应的天气图标*/
int32_t weather_get_icon_key(int32_t icon_type)
{

  int32_t type = RES_WEATHER_CLOUDY;
  if (icon_type == WEATHER_ICON_NO_DATA)
    {
      type = RES_WEATHER_NODATA;
    }
  else if (icon_type == WEATHER_ICON_CLEAR_SKY)
    {
      type = RES_WEATHER_SUNNY;
    }
  else if (icon_type == WEATHER_ICON_FEW_CLOUDS)
    {
      type = RES_WEATHER_CLOUDY;
    }
  else if (icon_type == WEATHER_ICON_SCATTERED_CLOUDS)
    {
      type = RES_WEATHER_CLOUDY;
    }
  else if (icon_type == WEATHER_ICON_BROKEN_CLOUDS)
    {
      type = RES_WEATHER_OVERCAST;
    }
  else if (icon_type == WEATHER_ICON_SHOWER_RAIN)
    {
      type = RES_WEATHER_RAIN_1;
    }
  else if (icon_type == WEATHER_ICON_RAIN)
    {
      type = RES_WEATHER_RAIN_2;
    }
  else if (icon_type == WEATHER_ICON_THUNDERSTORM)
    {
      type = RES_WEATHER_RAIN_3;
    }
  else if (icon_type == WEATHER_ICON_SNOW)
    {
      type = RES_WEATHER_SNOW_2;
    }
  else if (icon_type == WEATHER_ICON_MIST)
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

/*
 *--------------------------------------------------------------------------------------
 *     function:  weather_request_web
 *    parameter:
 *       return:
 *  description:  请求网络数据
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
void weather_request_web()
{
  /*拼接url请求地址, 注意url的缓存大小*/
  char url[200] = "";
  sprintf(url, "%slat=%s&lon=%s", WEATHER_WEB, g_gps_altitude.lat, g_gps_altitude.lon);
  /*拼接过滤参数，即只接受和过滤参数匹配的返回值*/
  char param[20] = "";
  sprintf(param, "%s,%s", "temp", "icon");
  /*15分钟同步一次天气数据*/
  g_comm_id_web = maibu_comm_request_web(url, param, 60 * 30);
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

/*
 *--------------------------------------------------------------------------------------
 *     function:
 *    parameter:
 *       return:
 *  description:  接受WEB数据回调
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
void weather_web_recv_callback(const uint8_t *buff, uint16_t size)
{
  int8_t icon = 0;
  int tempera = 0 ;

  /*提取温度及对应的图片ID*/
  int32_t icon_tmp = 0, tempera_tmp = 0;
  if ((-1 == maibu_get_json_int(buff, "icon", &icon_tmp)) || (-1 == maibu_get_json_int(buff, "temp", &tempera_tmp)))
    {
      return;
    }
  icon = icon_tmp;
  tempera = tempera_tmp;

  app_persist_write_data_extend(WEATHER_ICON_KEY, (unsigned char *)&icon, sizeof(int8_t));
  app_persist_write_data_extend(WEATHER_TEMP_KEY, (unsigned char *)&tempera, sizeof(int8_t));

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

  if (maibu_get_pressure(&f_temp) == 0)
    {
      char buff[20];
      sprintf(buff, "%.0f", (f_temp - 0.0)*100);

      for (i = 0;i < 24;i++)
        ap_data[i] = ap_data[i+1];
      ap_data[24] = atoi(buff);
    }

  return;
}

int32_t plot_ap_layer(P_Window p_window)
{
  uint16_t x0 = 172, y0 = 176, w = 7;

  P_Geometry p_g[1];

  int i, num = 0, min = ap_data[24], max = min;
  for (i = 24;i >= 0;i--)
    if (ap_data[i] > 0) num++;
    else
      break;

  if (num < 2) return 0;
  for (i = 23;i > 24 - num;i--)
    {
      if (ap_data[i] < min) min = ap_data[i];
      if (ap_data[i] > max) max = ap_data[i];
    }

  scale = (max - min) / 88 + 1;

  Line l[num-1];
  for (i = 0; i < num - 1; i++)
    {
      l[i].p0.x = x0 - i * w;
      l[i].p0.y = y0 - (ap_data[24-i] - min) / scale;
      l[i].p1.x = x0 - (i + 1) * w;
      l[i].p1.y = y0 - (ap_data[23-i] - min) / scale;
    }

  LineSet ls = {num - 1, l};
  Geometry lsg = {GeometryTypeLineSet, FillOutline, GColorRed, (void*)&ls};
  p_g[0] = &lsg;
  LayerGeometry lg = {1, p_g};

  /*图层1*/
  P_Layer	 layer1 = NULL;
  layer1 = app_layer_create_geometry(&lg);

  return (app_window_add_layer(p_window, layer1));
}

P_Layer get_circle_layer1(enum GColor color)
{
  LayerGeometry lg;
  P_Geometry p_geometry[1];
  memset(&lg, 0, sizeof(LayerGeometry));
  //memset(p_geometry, 0, sizeof(p_geometry));

  /*大圆心*/
  GPoint center1 = {88, 88};
  Circle c1 = {center1, 4};
  Geometry cg1 = {GeometryTypeCircle, FillArea, color, (void*)&c1};
  p_geometry[lg.num++] = &cg1;
  lg.p_g = p_geometry;

  /*图层1*/
  P_Layer	 layer1 = NULL;
  layer1 = app_layer_create_geometry(&lg);

  return (layer1);
}
P_Layer get_time_hand_layer(uint8_t min, uint8_t time_pos[][3][2], enum GColor color)
{
  GPoint p1 = {time_pos[min][0][0], time_pos[min][0][1]}, p2 = {time_pos[min][1][0], time_pos[min][1][1]}, p3 = {time_pos[min][2][0], time_pos[min][2][1]};

  /*多边形*/
  GPoint points1[3] = {p1, p2, p3};
  Polygon po1 = {3, points1};
  Geometry pg1 = {GeometryTypePolygon, FillArea, color, (void*)&po1};
  P_Geometry p_pg[1];
  LayerGeometry lg;
  memset(&lg, 0, sizeof(LayerGeometry));
  p_pg[lg.num++] = &pg1;
  lg.p_g = p_pg;


  /*图层1*/
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

  P_Layer hl = NULL, ml = NULL, cl1 = NULL, apl = NULL;
  static SportData data;
  maibu_get_sport_data(&data, 0);

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

  char buff[30] = {0};

  /*创建一个窗口*/
  p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);
  enum GColor colors[] = {GColorRed, GColorGreen, GColorBlue, GColorPurple}; //,GColorYellow

  GRect frame = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame, RES_WATCHFACE_OFFICIAL_BG);


  float f_temp = 0;
  if (maibu_get_altitude(&f_temp, 0) == 0)
    {
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
      sprintf(buff, "%d'C", (int)f_temp);
      display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
    }


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

  if (deltastep < 80)
    {
      frame.origin.x = 20;
      frame.origin.y = 120;
      frame.size.h = 20;
      frame.size.w = 136;
      sprintf(buff, "%d+%d", data.step, data.floor);
      display_target_layerText(p_window, &frame, GAlignCenter, GColorBlue, buff, U_ASCII_ARIAL_16, GColorWhite);

      f_temp = 0;
      if (maibu_get_pressure(&f_temp) == 0)
        {
          frame.origin.x = 0;
          frame.origin.y = 160;
          frame.size.h = 16;
          frame.size.w = 100;
          sprintf(buff, "%.0fPa", (f_temp - 0.0)*100);
          display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
        }

      int8_t percent = 0;
      maibu_get_battery_percent(&percent);
      frame.origin.x = 96;
      frame.origin.y = 160;
      frame.size.h = 16;
      frame.size.w = 80;
      //sprintf(buff, "%d%%", percent);
      sprintf(buff, "X%d|%d%%", scale, percent);  //显示比例
      display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);



      frame.origin.x = 68;
      frame.origin.y = 20;
      frame.size.h = 40;
      frame.size.w = 40;

      /*添加天气图标*/
      app_persist_create(WEATHER_ICON_KEY, 1);
      /*读取天气图标*/
      int8_t icon_type = 0;
      app_persist_read_data(WEATHER_ICON_KEY, 0, (unsigned char *)&icon_type, sizeof(int8_t));
      /*如果icon_type大于0，显示天气图标*/
      if (icon_type >= 0)
        {
          display_target_layer(p_window, &frame, weather_get_icon_key(icon_type));
        }

      frame.origin.x = 68;
      frame.origin.y = 60;
      frame.size.h = 20;
      frame.size.w = 40;
      int8_t temp_int = 0;
      app_persist_read_data(WEATHER_TEMP_KEY, 0, (unsigned char *)&temp_int, sizeof(int8_t));
      if (temp_int == WEATHER_TEMP_SPEC)
        {
          strcpy(buff, "--");
        }
      else
        {
          sprintf(buff, "%d'C", temp_int);
        }

      display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIAL_16, GColorBlack);
    }
  else  //步频大于80时显示
    {
      frame.origin.x = 20;
      frame.origin.y = 120;
      frame.size.h = 20;
      frame.size.w = 136;
      sprintf(buff, "%d+%d", data.step, data.floor);
      display_target_layerText(p_window, &frame, GAlignCenter, GColorBlue, buff, U_ASCII_ARIAL_20, GColorWhite);


      frame.origin.x = 0;
      frame.origin.y = 156;
      frame.size.h = 20;
      frame.size.w = 80;
      sprintf(buff, "%d'%02d\"", pace / 60, pace % 60);
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_20, GColorRed);

      frame.origin.x = 86;
      frame.origin.y = 156;
      frame.size.h = 20;
      frame.size.w = 90;

      sprintf(buff, "x%d.%d|%d.%d", speed / 10, speed % 10, data.distance / 100000, (data.distance / 10000) % 10);
      display_target_layerText(p_window, &frame, GAlignRight, GColorWhite, buff, U_ASCII_ARIAL_20, GColorRed);



      frame.origin.x = 68;
      frame.origin.y = 20;
      frame.size.h = 40;
      frame.size.w = 40;
      display_target_layer(p_window, &frame, RUN_ICON);


      frame.origin.x = 68;
      frame.origin.y = 60;
      frame.size.h = 20;
      frame.size.w = 40;

      sprintf(buff, "%d", deltastep);
      display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIAL_20, GColorRed);
    }


  plot_ap_layer(p_window);

  ml = get_time_hand_layer(dt.min, min_pos, colors[dt.min%4]);

  hl = get_time_hand_layer(dt.hour % 12 * 3 + dt.min / 20, hour_pos, colors[dt.hour%4]);

  cl1 = get_circle_layer1(GColorWhite);
  app_window_add_layer(p_window, hl);
  app_window_add_layer(p_window, ml);
  app_window_add_layer(p_window, cl1);


  if (dt.min % 10 == 0)
    if (g_timer_id == -1)
      g_timer_id = app_service_timer_subscribe(600000, timer_callback, NULL);

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

  /*初始化天气图标数据*/
  weather_init_store();

  /*创建显示表盘窗口*/
  P_Window p_window = init_watch();
  if (p_window != NULL)
    {
      /*放入窗口栈显示*/

      g_window = app_window_stack_push(p_window);
    }
  /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);

  /*注册接受WEB数据回调函数*/
  maibu_comm_register_web_callback(weather_web_recv_callback);
  /*注册接受手机数据回调函数*/
  maibu_comm_register_phone_callback((CBCommPhone)weather_phone_recv_callback);


  /*请求GPS数据*/
  g_comm_id_gps = maibu_comm_request_phone(ERequestPhoneGPSAltitude,  NULL, 0);
  /*注册通讯结果回调*/
  maibu_comm_register_result_callback(weather_comm_result_callback);

  return 0;
}



