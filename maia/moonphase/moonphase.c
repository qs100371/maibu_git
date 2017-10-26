#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maibu_sdk.h"
#include "maibu_res.h"

/*窗口句柄*/
static uint32_t g_window = -1;

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
const static char lunar_day[30][8] = {"初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十", "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十", "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"};
const static int32_t moon_phases[30] = {MOON1, MOON2, MOON2, MOON2, MOON2, MOON2, MOON2, MOON3, MOON4, MOON4, MOON4, MOON4, MOON4, MOON4, MOON5, MOON5, MOON6, MOON6, MOON6, MOON6, MOON6, MOON6, MOON7, MOON8, MOON8, MOON8, MOON8, MOON8, MOON8, MOON8};

const static char wday_str[7][8] = {"日", "一", "二", "三", "四", "五", "六"};


static P_Window init_watch();


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

uint16_t get_lunar_day()
{
  SLunarData sld = {0};
  maibu_get_lunar_calendar(NULL, &sld);
  char buff[20] = {0};
  sprintf(buff, "%s", sld.day);
  int i = 0;
  for (i = 0;i < 30;i++)
    if (strcmp(buff, lunar_day[i]) == 0)
      return i;
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
static P_Window init_watch()
{
  P_Window p_window = NULL;
  P_Layer p_layer = NULL;

  P_Layer hl = NULL, ml = NULL, cl1 = NULL, apl = NULL;
  

  char buff[30] = {0};

  /*创建一个窗口*/
  p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);
  enum GColor colors[] = {GColorRed, GColorGreen, GColorBlue, GColorPurple}; //,GColorYellow

  GRect frame = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame, RES_WATCHFACE_OFFICIAL_BG);

  //显示月相

  frame.origin.x = 68;
  frame.origin.y = 102;
  frame.size.h = 40;
  frame.size.w = 40;
  display_target_layer(p_window, &frame, moon_phases[get_lunar_day()]);//


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

  ml = get_time_hand_layer(dt.min, min_pos, colors[dt.min%4]);

  hl = get_time_hand_layer(dt.hour % 12 * 3 + dt.min / 20, hour_pos, colors[dt.hour%4]);

  cl1 = get_circle_layer1(GColorWhite);
  app_window_add_layer(p_window, hl);
  app_window_add_layer(p_window, ml);
  app_window_add_layer(p_window, cl1);

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

  
  /*创建显示表盘窗口*/
  P_Window p_window = init_watch();
  if (p_window != NULL)
    {
      /*放入窗口栈显示*/

      g_window = app_window_stack_push(p_window);
    }
  /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);

  return 0;
}



