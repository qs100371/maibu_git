
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maibu_res.h"
#include "maibu_sdk.h"

#define MAX_WORDS

#define ESSEY_KEY	100

static uint32_t g_window = 0;


const static char wday[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const static char wmon[][6] = {"Jan.", "Feb.", "Mar.", "Apr.", "May.", "June.", "July.", "Aug.", "Sept.", "Oct.", "Nov.", "Dec."};
static unsigned char essay[80] = "六级单词";
static int8_t bg_index = 0;
static uint16_t offset = 0, word_index = 0;

//图片ID数组
static int32_t bg_array[] =
{
  RES_BG000,
  RES_BG001,
  RES_BG002
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
      window_reloading();
    }
}
void scroll_select_back(void *context)
{
  int i, num = 89;
  uint16_t tmp_offset = 0, tmp_index = 0;
  if (num > word_index)
    {
      tmp_offset = offset;
      tmp_index = word_index;
    }
  unsigned char buff[100] = {0};


  do
    {
      memset(buff, 0, 100);
      maibu_read_user_file(MY_FILE_KEY, tmp_offset, buff, 100);
      for (i = 0; i < strlen(buff) ; i++)
        if (buff[i] == '\n')
          {
            tmp_index++;
            if (tmp_index == num)
              {
                offset = tmp_offset + i + 1;
                window_reloading();
                return;
              }


          }
      tmp_offset += 100;
    }
  while (tmp_index < num);

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

P_Window init_watch(void)
{
  P_Window p_window = NULL;
  P_Layer p_layer = NULL;
  p_window = app_window_create();
  if (NULL == p_window)
    {
      return NULL;
    }
  char str[20] = "";
  struct date_time dt;
  app_service_get_datetime(&dt);

  bg_index = dt.min % 3;


  GRect frame = {{0, 0}, {176, 176}};
  GBitmap bitmap;
  res_get_user_bitmap(bg_array[bg_index], &bitmap);
  LayerBitmap lb = {bitmap, frame, GAlignCenter};
  p_layer = app_layer_create_bitmap(&lb);
  app_window_add_layer(p_window, p_layer);

  GRect frame_time = {{0, 68}, {26, 88}};
  sprintf(str, "%02d:%02d", dt.hour, dt.min);
  display_target_layerText(p_window, &frame_time, GAlignLeft, GColorWhite, str, U_ASCII_ARIAL_24, GColorBlack);

  GRect frame_day = {{0, 94}, {16, 88}};
  sprintf(str, "%s%d,%s", wmon[dt.mon-1], dt.mday, wday[dt.wday]);
  display_target_layerText(p_window, &frame_day, GAlignLeft, GColorWhite, str, U_ASCII_ARIAL_16, GColorBlack);

  Geometry *geometry[1];
  LayerGeometry layer_geometry;
  memset(geometry, 0, sizeof(geometry));

  Line l = {{0, 94}, {88, 94}};
  Geometry lg = {GeometryTypeLine, FillOutline, GColorRed, (void*)&l};
  geometry[0] = &lg;

  layer_geometry.num = 1;
  layer_geometry.p_g = geometry;

  p_layer = app_layer_create_geometry(&layer_geometry);
  app_window_add_layer(p_window, p_layer);


  uint16_t i;
  memset(essay, 0, sizeof(essay));
  maibu_read_user_file(MY_FILE_KEY, offset, essay, 80);
  for (i = 0; essay[i] != '\n' && i < strlen(essay) ; i++);
  if (essay[i] == '\n')
    {
      offset += i + 1;
      word_index ++;
    }
  else
    {
      offset = 0;
      word_index = 0;
    }
  essay[i] = '\0';

  app_persist_write_data_extend(ESSEY_KEY, essay, strlen(essay));
  LayerScroll ls1 = {{{0, 110}, {66, 176}}, ESSEY_KEY, strlen(essay) , U_GBK_SIMSUN_20, 1, GColorBlack, 0};
  p_layer = app_layer_create_scroll(&ls1);
  app_layer_set_bg_color(p_layer, GColorWhite);
  app_window_add_layer(p_window, p_layer);

  GRect frame_index = {{88, 160}, {16, 88}};
  sprintf(str, "-%d-", word_index);
  display_target_layerText(p_window, &frame_index, GAlignRight, GColorWhite, str, U_ASCII_ARIAL_16, GColorBlack);

  app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);
  /*注册一个事件通知回调，当有时间改变是，立即更新时间*/
  maibu_service_sys_event_subscribe(watch_time_change);

  return p_window;
}


int main()
{
  //simulator_init();
  app_persist_create(ESSEY_KEY, 80);

  P_Window p_window = init_watch();
  g_window = app_window_stack_push(p_window);
  //simulator_wait();
  return 0;
}
























