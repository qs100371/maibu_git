#include <stdlib.h>
#include <stdio.h>

#include "maibu_sdk.h"
#include "maibu_res.h"
static int32_t g_window_id             = -1;
static SportData data;
static P_Window init_watch(void);

static GRect timeFrames[] =
{
  {{54, 6}, {54, 32}},
  {{90, 6}, {54, 32}},
  {{54, 66}, {54, 32}},
  {{90, 66}, {54, 32}},
  {{60, 142}, {21, 15}},
  {{32, 130}, {40, 28}}
};


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
  LIT_9,
  STEP,
  GOAL
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

  maibu_get_sport_data(&data, 0);

  char buff[30] = {0};

  /*创建一个窗口*/
  p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);

  uint8_t i = 0;
  //显示时间
  sprintf(buff, "%02d%02d", dt.hour, dt.min);
  for (i = 0;i <= 3;i++)
    {
      display_target_layer(p_window, &timeFrames[i], get_BIG_icon_key[buff[i] - '0']);
    }
  sprintf(buff, "%d", data.step);
  GRect temp_frame = {0};
  temp_frame = timeFrames[4];
  for (i = 0;i < strlen(buff);i++)
    {
      display_target_layer(p_window, &temp_frame, get_LIT_icon_key[buff[i] - '0']);
      temp_frame.origin.x = temp_frame.origin.x + temp_frame.size.w;
    }
  display_target_layer(p_window, &timeFrames[5], get_LIT_icon_key[10]);
  
  if (data.step >= 8000)
    {
      temp_frame.origin.y = 130;
      temp_frame.size.w = 28;
      temp_frame.size.h = 40;
      display_target_layer(p_window, &temp_frame, get_LIT_icon_key[11]);
    }
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

      g_window_id = app_window_stack_push(p_window);
    }
  /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);


  return 0;
}



