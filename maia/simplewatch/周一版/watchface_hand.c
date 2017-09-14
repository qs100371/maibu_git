
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "maibu_res.h"
#include "maibu_sdk.h"

/*表盘中心位置*/
#define CX 88
#define CY 88

static uint32_t g_window = 0;

static bool backispressed = false;
static uint32_t lastmin_distance = 0, lastmin_steps = 0;
const static char wday[7][8] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const uint8_t min_pos[60][2] =
{
  {87, 8},
  {96, 8},
  {104, 9},
  {112, 11},
  {120, 14},
  {127, 18},
  {135, 23},
  {141, 28},
  {147, 34},
  {152, 40},
  {157, 47},
  {161, 55},
  {164, 63},
  {166, 71},
  {167, 79},
  {168, 88},
  {167, 96},
  {166, 104},
  {164, 112},
  {161, 120},
  {157, 128},
  {152, 135},
  {147, 141},
  {141, 147},
  {135, 152},
  {127, 157},
  {120, 161},
  {112, 164},
  {104, 166},
  {96, 167},
  {87, 167},
  {79, 167},
  {71, 166},
  {63, 164},
  {55, 161},
  {47, 157},
  {40, 152},
  {34, 147},
  {28, 141},
  {23, 135},
  {18, 127},
  {14, 120},
  {11, 112},
  {9, 104},
  {8, 96},
  {8, 87},
  {8, 79},
  {9, 71},
  {11, 63},
  {14, 55},
  {18, 47},
  {23, 40},
  {28, 34},
  {34, 28},
  {40, 23},
  {48, 18},
  {55, 14},
  {63, 11},
  {71, 9},
  {79, 8}
};

const uint8_t hour_pos[36][2] =
{
  {87, 38},
  {96, 38},
  {105, 41},
  {112, 44},
  {120, 49},
  {126, 55},
  {131, 62},
  {134, 70},
  {137, 79},
  {138, 88},
  {137, 96},
  {134, 105},
  {131, 113},
  {126, 120},
  {120, 126},
  {112, 131},
  {105, 134},
  {96, 137},
  {87, 137},
  {79, 137},
  {70, 134},
  {62, 131},
  {55, 126},
  {49, 120},
  {44, 112},
  {41, 105},
  {38, 96},
  {38, 87},
  {38, 79},
  {41, 70},
  {44, 62},
  {49, 55},
  {55, 49},
  {63, 44},
  {70, 41},
  {79, 38}
};

P_Window init_watch(void);



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

void watch_time_change(enum SysEventType type, void *context)
{
  /*时间更改*/
  if (type == SysEventTypeTimeChange)
    {
      if (backispressed) backispressed = false;
      window_reloading();
    }
}

void select_back(void *context)
{
  P_Window p_window = (P_Window)context;

  P_Layer p_layer = NULL;

  if (backispressed)
    {
      window_reloading();
      return;
    }

  backispressed = true;
  struct date_time dt;
  app_service_get_datetime(&dt);

  uint16_t totalsteps = 0, totaldistance = 0;
  uint8_t i;
  SportData data;
  int day = dt.wday;
  if (day == 0) day = 7;
  for (i = 0;i < day;i++)
    {

      maibu_get_sport_data(&data, i);
      totalsteps += data.step / 1000;
      totaldistance += data.distance / 100000;
    }

  char str[20] = "";

  GRect frame = {{3, 56}, {60, 170}};
  GBitmap bitmap;
  res_get_user_bitmap(RES_MSGBOX_BG, &bitmap);
  LayerBitmap lb = {bitmap, frame, GAlignCenter};
  p_layer = app_layer_create_bitmap(&lb);
  app_window_add_layer(p_window, p_layer);

  GRect frame1 = {{4, 60}, {20, 166}};
  sprintf(str, "From Mon,I walked");
  LayerText lt1 = {str, frame1, GAlignCenter, GColorWhite, U_ASCII_ARIAL_16};
  p_layer = app_layer_create_text(&lt1);
  app_layer_set_bg_color(p_layer, GColorBlue);
  app_window_add_layer(p_window, p_layer);
  app_window_update(p_window);

  GRect frame2 = {{4, 90}, {20, 166}};
  sprintf(str, "%dk steps,%dkm.", totalsteps, totaldistance);
  LayerText lt2 = {str, frame2, GAlignCenter, GColorWhite, U_ASCII_ARIAL_16};
  p_layer = app_layer_create_text(&lt2);
  app_layer_set_bg_color(p_layer, GColorBlue);
  app_window_add_layer(p_window, p_layer);
  app_window_update(p_window);

  return;

}

P_Window init_watch(void)
{
  P_Window p_window = NULL;
  p_window = app_window_create();
  if (NULL == p_window)
    {
      return NULL;
    }
  SpeedInfo info;
  maibu_get_speed_info(&info);
  SportData data;
  maibu_get_sport_data(&data, 0);
  bool inwalking = (data.step - lastmin_steps) >= 80 && lastmin_steps > 0 ? true : false;


  GRect frame = {{0, 0}, {176, 176}};
  GBitmap bitmap;
  res_get_user_bitmap(RES_BG, &bitmap);
  LayerBitmap lb = {bitmap, frame, GAlignCenter};
  P_Layer p_layer = app_layer_create_bitmap(&lb);
  app_window_add_layer(p_window, p_layer);

  struct date_time dt;
  app_service_get_datetime(&dt);

  char str[20] = "";

  GRect frame_day = {{0, 16}, {26, 176}};
  sprintf(str, "%d/%d,%s", dt.mon, dt.mday, wday[dt.wday]);
  LayerText lt_day = {str, frame_day, GAlignCenter, GColorBlack, U_ASCII_ARIAL_20};
  p_layer = app_layer_create_text(&lt_day);
  app_layer_set_bg_color(p_layer, GColorWhite);
  app_window_add_layer(p_window, p_layer);

  if (inwalking)  //info.flag
    {
      GRect frame_time = {{0, 42}, {42, 176}};
      sprintf(str, "%02d:%02d", dt.hour, dt.min);
      LayerText lt_time = {str, frame_time, GAlignCenter, GColorBlack, U_ASCII_ARIAL_42};
      p_layer = app_layer_create_text(&lt_time);
      app_layer_set_bg_color(p_layer, GColorWhite);
      app_window_add_layer(p_window, p_layer);
    }

  /*添加运动数据图层*/
  GRect frame_step = {{20, 104}, {26, 60}};

  sprintf(str, "%d", data.step);
  LayerText lt_step = {str, frame_step, GAlignCenter, GColorWhite, U_ASCII_ARIAL_20};
  p_layer = app_layer_create_text(&lt_step);
  app_layer_set_bg_color(p_layer, GColorBlue);
  app_window_add_layer(p_window, p_layer);



  GRect frame_distance = {{96, 104}, {26, 60}};
  if (!inwalking) //!info.flag
    sprintf(str, "%dkm", data.distance / 100000);
  else
    {
      int speed = (data.distance - lastmin_distance) * 60 / 10000; //10倍时速
      sprintf(str, "%d.%dkph", speed / 10, speed % 10); //%d.%d
    }
  LayerText lt_distance = {str, frame_distance, GAlignCenter, GColorWhite, U_ASCII_ARIAL_20};
  p_layer = app_layer_create_text(&lt_distance);
  app_layer_set_bg_color(p_layer, GColorBlue);
  app_window_add_layer(p_window, p_layer);

  if (inwalking)  //info.flag
    {
      //显示前一分钟配速
      int pace = 6000000 / (data.distance - lastmin_distance); //配速 秒为单位
      GRect frame_pace = {{0, 130}, {26, 176}};
      sprintf(str, "%d'%d\"", pace / 60, pace % 60);  //%d'%d\"
      LayerText lt_pace = {str, frame_pace, GAlignCenter, GColorWhite, U_ASCII_ARIAL_20};
      p_layer = app_layer_create_text(&lt_pace);
      app_layer_set_bg_color(p_layer, GColorBlue);
      app_window_add_layer(p_window, p_layer);
    }


  Geometry *geometry[3];
  uint8_t num = 0;
  LayerGeometry layer_geometry;
  memset(geometry, 0, sizeof(geometry));

  uint8_t hour = dt.hour > 11 ? dt.hour - 12 : dt.hour;
  Line lh = {{CX, CY}, {hour_pos[hour*3+dt.min/20][0], hour_pos[hour*3+dt.min/20][1]}};
  Geometry lg1 = {GeometryTypeLine, FillOutline, GColorBlack, (void*)&lh};
  geometry[num++] = &lg1;

  Line lm = {{CX, CY}, {min_pos[dt.min][0], min_pos[dt.min][1] }};
  Geometry lg2 = {GeometryTypeLine, FillOutline, GColorBlack, (void*)&lm};
  geometry[num++] = &lg2;

  if (inwalking)
    {
      uint16_t speed = info.real_time_speed * 36 / 100;  //speed*10
      Line ls = {{speed*176 / 80, 0}, {speed*176 / 80, 20}};
      Geometry lg3 = {GeometryTypeLine, FillOutline, GColorBlue, (void*)&ls};
      geometry[num++] = &lg3;

    }



  layer_geometry.num = num;
  layer_geometry.p_g = geometry;

  p_layer = app_layer_create_geometry(&layer_geometry);
  app_window_add_layer(p_window, p_layer);

  if (backispressed) backispressed = false;
  lastmin_distance = data.distance;
  lastmin_steps = data.step;

  app_window_click_subscribe(p_window, ButtonIdBack, select_back);

  /*注册一个事件通知回调，当有时间改变是，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);

  return p_window;
}

int main()
{
  //simulator_init();

  P_Window p_window = init_watch();
  g_window = app_window_stack_push(p_window);
  //simulator_wait();
  return 0;
}















