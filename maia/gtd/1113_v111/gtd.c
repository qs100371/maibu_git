#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maibu_sdk.h"
#include "maibu_res.h"

#define TASK_LIST_MAX_SIZE 20
/*窗口句柄*/
static int32_t g_window = -1;

/*任务项结构体*/
typedef struct tag_STaskItem
  {
    uint8_t id;	//任务类型ID  0 日程,1 有限定日期的待办,2 长期任务,3 节日或节气
    int32_t date;	//任务日期 月*100+日
    int32_t time;	//任务时间 时*100+分
    char title[24];	//任务标题
    uint8_t alarmed; //0 未提醒,1 已提醒
  }STaskItem;

/*任务列表结构体*/
typedef struct tag_STaskList
  {
    uint8_t nums;	//当前任务项个数
    STaskItem list[TASK_LIST_MAX_SIZE];	//任务项列表
  }STaskList;

/*任务列表*/
static STaskList g_s_task_list = {0};

const static char wday_str[7][8] = {"日", "一", "二", "三", "四", "五", "六"};
const static int32_t ids[] = {ID0, ID1, ID2, ID3};

P_Window init_watch();
void window_reloading(void);



void timer_callback(date_time_t tick_time, uint32_t millis, void *context)
{
  int8_t i = 0;
  NotifyParam	param;
  /* if(maibu_get_phone_type() == PhoneTypeIOS)
  {
  	return;
  } */
  struct date_time dt;
  app_service_get_datetime(&dt);

  for (i = 0; i < g_s_task_list.nums; i++)
    {
      if (strlen(g_s_task_list.list[i].title) == 0)
        {
          continue;
        }
      if ((g_s_task_list.list[i].date == dt.mon*100 + dt.mday || g_s_task_list.list[i].date == 0) && ((g_s_task_list.list[i].id == 0) || (g_s_task_list.list[i].id == 1)))
        {
          int32_t time = g_s_task_list.list[i].time / 100 * 60 + g_s_task_list.list[i].time % 100;
          int32_t now = dt.hour * 60 + dt.min;
          if ((g_s_task_list.list[i].alarmed == 0) && (((time - now == 15) && (g_s_task_list.list[i].date == dt.mon*100 + dt.mday)) || ((time - now == 0) && (g_s_task_list.list[i].date == 0))))
            //如果没有提醒过，并且是日程提前15分钟，每日提醒准时，则提醒
            {
              g_s_task_list.list[i].alarmed = 1;
              memset(&param, 0, sizeof(NotifyParam));
              res_get_user_bitmap(ALARM,  &param.bmp);
              sprintf(param.main_title, "%s", "日程提醒");
              sprintf(param.sub_title, "%d:%02d %s", g_s_task_list.list[i].time / 100, g_s_task_list.list[i].time % 100, g_s_task_list.list[i].title);
              param.pulse_type = VibesPulseTypeMiddle;
              param.pulse_time = 6;
              maibu_service_sys_notify(&param);
              return;
            }
          if ((g_s_task_list.list[i].date == dt.mon*100 + dt.mday) && (now - time > 5)) //(g_s_task_list.list[i].alarmed == 1)
            //如果已过期10分钟，则删除
            memset(&g_s_task_list.list[i], 0, sizeof(STaskItem));
          else if (g_s_task_list.list[i].date == 0 && now - time > 5)
            //每日提醒，过期10分则清除已提醒标记
            g_s_task_list.list[i].alarmed = 0;
        }

      window_reloading();
    }
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


void watch_time_change(enum SysEventType type, void *context)
{
  /*时间更改*/
  if (type == SysEventTypeTimeChange)
    {
      window_reloading();
    }
}

/*创建并显示图层，需要坐标值，得到icon_key的数组，数组需要的参数值，P_Window*/
int32_t display_target_layer(P_Window p_window, GRect *temp_p_frame, int32_t bmp_array_name_key)
{
  GBitmap bmp_point;
  res_get_user_bitmap(bmp_array_name_key, &bmp_point);
  LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, GAlignLeft};
  P_Layer temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);

  if (temp_P_Layer != NULL)
    {
      app_layer_set_bg_color(temp_P_Layer, GColorBlack);
      return app_window_add_layer(p_window, temp_P_Layer);
    }

  return 0;
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

void watchapp_comm_callback(enum ESyncWatchApp type, uint8_t *buf, uint16_t len)
{

  if (type == ESyncWatchAppUpdateParam)
    {
      /* char buffer[200];
      if (len < sizeof(buffer))
        {
          memcpy(buffer, buf, len);
          buffer[len] = '\0';
        }
      else
        {
          memcpy(buffer, buf, sizeof(buffer) - 1);
          buffer[sizeof(buffer) - 1] = '\0';
        } */

      uint8_t i = 0, count = 0;
      memset(&g_s_task_list, 0, sizeof(STaskList));

      while (i < strlen(buf))
        {
          uint8_t j = i;
          for (; (i < strlen(buf) && buf[i] != '\n'); i++);

          if (buf[j] == '-')
            {
              if (i - j - 13 > 0)
                {
                  g_s_task_list.list[count].id = 1;
                  g_s_task_list.list[count].date = (buf[j+1] - '0') * 1000 + (buf[j+2] - '0') * 100 + (buf[j+4] - '0') * 10 + (buf[j+5] - '0');
                  g_s_task_list.list[count].time = (buf[j+7] - '0') * 1000 + (buf[j+8] - '0') * 100 + (buf[j+10] - '0') * 10 + (buf[j+11] - '0');

                  memcpy(g_s_task_list.list[count].title, buf + j + 13, i - j - 13);
                }
            }
          else if (buf[j] >= '0' && buf[j] <= '9')
            {
              if (i - j - 12 > 0)
                {
                  g_s_task_list.list[count].id = 0;
                  g_s_task_list.list[count].date = (buf[j] - '0') * 1000 + (buf[j+1] - '0') * 100 + (buf[j+3] - '0') * 10 + (buf[j+4] - '0');
                  g_s_task_list.list[count].time = (buf[j+6] - '0') * 1000 + (buf[j+7] - '0') * 100 + (buf[j+9] - '0') * 10 + (buf[j+10] - '0');
                  memcpy(g_s_task_list.list[count].title, buf + j + 12, i - j - 12);
                }
            }
          else
            {
              if (i - j > 0)
                {
                  g_s_task_list.list[count].id = 2;
                  memcpy(g_s_task_list.list[count].title, buf + j , i - j);
                }
            }
          if (strlen(g_s_task_list.list[count].title) > 0)
            count++;
          else
            memset(&g_s_task_list.list[count], 0, sizeof(STaskItem));
          i++;
          if (count == TASK_LIST_MAX_SIZE)
            break;
        }
      g_s_task_list.nums = count;

      window_reloading();
    }
}

P_Window init_watch()
{
  char buff[24] = {0};
  P_Window p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);

  GRect frame = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame, RES_BG);

  frame.origin.x = 0;
  frame.origin.y = 0;
  frame.size.h = 30;
  frame.size.w = 40;
  sprintf(buff, "%2d", dt.mday);//
  display_target_layerText(p_window, &frame, GAlignRight, GColorBlue, buff, U_ASCII_ARIAL_30, GColorWhite);

  frame.origin.x = 42;
  frame.origin.y = 0;
  frame.size.h = 15;
  frame.size.w = 120;
  sprintf(buff, "%d.%d,周%s", dt.year, dt.mon, wday_str[dt.wday]);//dt.mday
  display_target_layerText(p_window, &frame, GAlignLeft, GColorBlue, buff, U_ASCII_ARIAL_16, GColorWhite);

  frame.origin.x = 42;
  frame.origin.y = 16;
  frame.size.h = 15;
  frame.size.w = 60;
  sprintf(buff, "%d:%02d", dt.hour, dt.min);
  display_target_layerText(p_window, &frame, GAlignLeft, GColorBlue, buff, U_ASCII_ARIALBD_16, GColorWhite);

  /* if (g_s_task_list.nums == 0)
    {
      frame.origin.x = 0;
      frame.origin.y = 75;
      frame.size.h = 16;
      frame.size.w = 176;
      sprintf(buff, "日程未设置!");
      display_target_layerText(p_window, &frame, GAlignCenter, GColorWhite, buff, U_ASCII_ARIALBD_16, GColorRed);
    }
  else
    { */
  uint8_t count = 0;
  static uint8_t i = 0;
  for (; i < g_s_task_list.nums && count < 4; i++)
    {
      if (strlen(g_s_task_list.list[i].title) == 0)
        {
          continue;
        }

      strcpy(buff, g_s_task_list.list[i].title);
      frame.origin.x = 5;
      frame.origin.y = 33 + count * 36;
      frame.size.h = 16;
      frame.size.w = 166;
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_GBK_SIMSUN_16, GColorBlack);

      frame.origin.x = 5;
      frame.origin.y = 50 + count * 36;
      frame.size.h = 16;
      frame.size.w = 32;
      display_target_layer(p_window, &frame, ids[g_s_task_list.list[i].id]);

      if (g_s_task_list.list[i].id == 2)	//为2则需要显示为近期
        {
          strcpy(buff, "近期");
        }
      else if (g_s_task_list.list[i].id == 0 && g_s_task_list.list[i].date == 0)
        {
          sprintf(buff, "每日%d:%02d", g_s_task_list.list[i].time / 100, g_s_task_list.list[i].time % 100);
        }
      else if (g_s_task_list.list[i].date == dt.mon*100 + dt.mday)
        {
          if (g_s_task_list.list[i].id == 3)
            strcpy(buff, "今日");
          else
            sprintf(buff, "今日%d:%02d", g_s_task_list.list[i].time / 100, g_s_task_list.list[i].time % 100);
        }
      else if ((g_s_task_list.list[i].date != dt.mon*100 + dt.mday) && (g_s_task_list.list[i].id == 3))
        sprintf(buff, "%d月%d日", g_s_task_list.list[i].date / 100, g_s_task_list.list[i].date % 100);
      else
        {
          sprintf(buff, "%d月%d日%d:%02d", g_s_task_list.list[i].date / 100, g_s_task_list.list[i].date % 100, g_s_task_list.list[i].time / 100, g_s_task_list.list[i].time % 100);
        }
      frame.origin.x = 40;
      frame.origin.y = 50 + count * 36;
      frame.size.h = 16;
      frame.size.w = 136;
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorRed);

      count++;

    }
  i = (i == g_s_task_list.nums) ? 0 : i;
  /* } */
  return p_window;
}



int main()
{
  /* if (g_s_task_list.nums == 0)
    {
      g_s_task_list.list[0].id = 0;
      g_s_task_list.list[0].date = 1108;
      g_s_task_list.list[0].time = 1000;
      sprintf(g_s_task_list.list[0].title, "相亲");
      g_s_task_list.list[0].alarmed = 0;

      g_s_task_list.list[1].id = 1;
      g_s_task_list.list[1].date = 1111;
      g_s_task_list.list[1].time = 1100;
      sprintf(g_s_task_list.list[1].title, "领证");
      g_s_task_list.list[1].alarmed = 0;

      g_s_task_list.list[2].id = 2;
      g_s_task_list.list[2].date = 0;
      g_s_task_list.list[2].time = 0;
      sprintf(g_s_task_list.list[2].title, "看电影");
      g_s_task_list.list[2].alarmed = 0;

      g_s_task_list.list[3].id = 2;
      g_s_task_list.list[3].date = 0;
      g_s_task_list.list[3].time = 0;
      sprintf(g_s_task_list.list[3].title, "结婚");
      g_s_task_list.list[3].alarmed = 0;

      g_s_task_list.list[4].id = 1;
      g_s_task_list.list[4].date = 1109;
      g_s_task_list.list[4].time = 2359;
      sprintf(g_s_task_list.list[4].title, "买房");
      g_s_task_list.list[4].alarmed = 0;

      g_s_task_list.nums = 5;
    }*/


  /*创建显示表盘窗口*/
  P_Window p_window = init_watch();
  if (p_window != NULL)
    {
      g_window = app_window_stack_push(p_window);
    }

  maibu_service_sys_event_subscribe(watch_time_change);
  maibu_comm_register_watchapp_callback(watchapp_comm_callback);
  app_service_timer_subscribe(30000, timer_callback, (void *)p_window);

  return 0;
}



