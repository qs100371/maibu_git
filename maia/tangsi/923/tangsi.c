/*
 * =====================================================================================
 *
 *     Modulename:
 *       Filename:
 *
 *    Description:
 *    Corporation:
 *
 *         Author:
 *        Created:
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

#define SCROLL_KEY	0x2017

#define PER_CHN_CHARACTER_WIDTH  16

#define PER_ROW_MAX_CHN_CHARACTER_NUM  10
#define PER_PAGE_MAX_ROW_NUM  9

#define ONE_PAGE_MAX_SIZE (PER_ROW_MAX_CHN_CHARACTER_NUM*PER_PAGE_MAX_ROW_NUM*3)
#define MAX_PAGE  300

static uint16_t page_offset_buff[MAX_PAGE] = {0};
static int32_t page = 0;

static int32_t g_window_id = -1;

static uint32_t g_text_offset = 0;
static uint32_t g_next_page_text_offset = 0;

static uint8_t g_page_end_flag = 0;

const char ascii_start32_width[] =
{
  3,
  3,
  4,
  7,
  7,
  11,
  9,
  3,
  5,
  4,
  5,
  7,
  3,
  4,
  3,
  3,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  3,
  3,
  7,
  7,
  7,
  7,
  13,
  7,
  8,
  9,
  9,
  8,
  7,
  9,
  9,
  3,
  6,
  9,
  8,
  9,
  9,
  9,
  8,
  9,
  9,
  8,
  7,
  9,
  7,
  7,
  7,
  7,
  4,
  3,
  3,
  5,
  7,
  4,
  7,
  7,
  6,
  7,
  7,
  4,
  7,
  7,
  3,
  3,
  7,
  3,
  7,
  7,
  7,
  7,
  5,
  7,
  3,
  7,
  5,
  9,
  5,
  5,
  5,
  4,
  3,
  5,
  8
};


extern uint32_t get_res_size(uint32_t res_key);

int enc_get_utf8_size(unsigned char input)
{

  int num = 0, i = 0;

  for (i = 0; i < 8; i++)
    {
      if (((input >> (7 - i)) & 0x01))
        {
          num++;
        }
      else
        {
          break;
        }
    }

  return (num > 0) ? num : 0;
}

int my_utf_unicode_change_utf2unicode(const  char* pInput, short pInput_len)
{

  int utfbytes = enc_get_utf8_size(*pInput);

  if (utfbytes > pInput_len)
    {

      return -1;
    }

  if (utfbytes == 0)
    {
      utfbytes++;
    }
  else if (utfbytes == 1)
    {
      utfbytes = 0;
    }

  return utfbytes;
}

P_Window init_window(void);

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

/*创建并显示图片图层，需要坐标值，得到icon_key的数组，数组需要的参数值，P_Window*/
int32_t display_target_layer(P_Window p_window, GRect *temp_p_frame, enum GAlign how_to_align, enum GColor black_or_white, uint32_t bmp_knowed_key)
{


  GBitmap bmp_point;
  P_Layer temp_P_Layer = NULL;

  res_get_user_bitmap(bmp_knowed_key, &bmp_point);
  LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, how_to_align};
  temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);

  if (temp_P_Layer != NULL)
    {
      app_layer_set_bg_color(temp_P_Layer, black_or_white);
      return app_window_add_layer(p_window, temp_P_Layer);
    }

  return 0;
}


/*创建并显示文本图层*/
int32_t display_target_layerText(P_Window p_window, GRect  *temp_p_frame, enum GAlign how_to_align, enum GColor color, char * str, uint8_t font_type, enum GColor fcolor)
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


void scroll_select_back(void *context)

{

  P_Window p_window = (P_Window)context;

  if (NULL != p_window)

    {

      app_window_stack_pop(p_window);

    }

}

void scroll_select_select(void *context)

{

  P_Window p_window = (P_Window)context;

  if (NULL != p_window)

    {
      page = 0;
	  g_text_offset = page_offset_buff[0];
      window_reloading();
    }
}


void scroll_select_up(void *context)
{
  P_Window p_window = (P_Window)context;
  if (p_window == NULL)
    {
      return;
    }
  page = page - 1 < 0 ? 0 : page - 1 ;

  g_next_page_text_offset = g_text_offset;
  g_text_offset = page_offset_buff[page];



  window_reloading();

}


void scroll_select_down(void *context)
{
  P_Window p_window = (P_Window)context;

  if (p_window == NULL)
    {
      return;
    }



  if (page >= MAX_PAGE - 1)
    {

      return;
    }

  if (g_page_end_flag == 0)
    {
      page_offset_buff[page] = g_text_offset;
      g_text_offset = g_next_page_text_offset;
      page = page + 1 >= MAX_PAGE ? MAX_PAGE - 1 : page + 1 ;

    }


  window_reloading();

}

static void time_change(enum SysEventType type, void *context)
{

  /*时间更改*/
  if (type == SysEventTypeTimeChange)
    {
      window_reloading();
    }
}


P_Window init_window(void)
{
  //创建储存空间
  app_persist_create(SCROLL_KEY, ONE_PAGE_MAX_SIZE);

  P_Window p_window = app_window_create();
  if (NULL == p_window)
    {
      return NULL;
    }


  /*创建背景图层*/
  GRect temporary_frame = {{0, 0}, {176, 176}};
  P_GRect temp_p_frame = &temporary_frame;
  display_target_layer(p_window, temp_p_frame, GAlignCenter, GColorBlack, BG);

  char buffer[20] = {0};
  struct date_time t;
  app_service_get_datetime(&t);

  /*添加小时分钟图层*/
  temporary_frame.origin.x = 0;
  temporary_frame.origin.y = 0;
  temporary_frame.size.h = 16;
  temporary_frame.size.w = 162;

  sprintf(buffer, "---%d---", page);
  display_target_layerText(p_window, temp_p_frame, GAlignLeft, GColorYellow, buffer, U_ASCII_ARIAL_16, GColorRed);
  sprintf(buffer, "%02d:%02d", t.hour, t.min);
  display_target_layerText(p_window, temp_p_frame, GAlignRight, GColorYellow, buffer, U_ASCII_ARIAL_16, GColorRed);


  char text_read[ONE_PAGE_MAX_SIZE] = {0};


  //计算占满整页的所用偏移
  uint32_t read_size = sizeof(text_read) > (get_res_size(MY_FILE_KEY) - g_text_offset) ? (get_res_size(MY_FILE_KEY) - g_text_offset) : sizeof(text_read);

  uint32_t ret_read_long = maibu_read_user_file(MY_FILE_KEY, g_text_offset, text_read, read_size);

  if (ret_read_long <= 0)
    {
      return NULL;
    }

  uint32_t readed_offset = 0;
  uint32_t step = 0;
  uint32_t width = 0;
  uint32_t row = 0;
  uint32_t row_9th_offset = 0;

  for (readed_offset = 0;readed_offset < strlen(text_read);readed_offset = readed_offset + step)
    {

      int ret = my_utf_unicode_change_utf2unicode(&text_read[readed_offset], strlen(text_read) - readed_offset);
      if (ret == 1)
        {
          if (text_read[readed_offset] == 10)
            {

              width = 0;
              row++;
              if (row == PER_PAGE_MAX_ROW_NUM - 1)
                {
                  row_9th_offset = readed_offset + 1;
                }
              if (row >= PER_PAGE_MAX_ROW_NUM)
                {
                  break;
                }
            }
          else  if (text_read[readed_offset] >= 32)
            {

              if (width + ascii_start32_width[text_read[readed_offset] - 32] <= PER_ROW_MAX_CHN_CHARACTER_NUM*PER_CHN_CHARACTER_WIDTH)
                {
                  width = width + ascii_start32_width[text_read[readed_offset] - 32];
                }
              else
                {

                  width = ascii_start32_width[text_read[readed_offset] - 32];
                  row++;
                  if (row == PER_PAGE_MAX_ROW_NUM - 1)
                    {
                      row_9th_offset = readed_offset;
                    }
                  if (row >= PER_PAGE_MAX_ROW_NUM)
                    {
                      break;
                    }

                }
            }
        }
      else if (ret == 3)
        {
          if (width + PER_CHN_CHARACTER_WIDTH <= PER_ROW_MAX_CHN_CHARACTER_NUM*PER_CHN_CHARACTER_WIDTH)
            {
              width = width + PER_CHN_CHARACTER_WIDTH;
            }
          else
            {

              width = PER_CHN_CHARACTER_WIDTH;
              row++;
              if (row == PER_PAGE_MAX_ROW_NUM - 1)
                {
                  row_9th_offset = readed_offset;
                }
              if (row >= PER_PAGE_MAX_ROW_NUM)
                {
                  break;
                }

            }
        }
      else if (ret == 0)
        {
          break;
        }
      step = ret;

    }

  g_next_page_text_offset = g_text_offset + row_9th_offset;
  app_persist_write_data_extend(SCROLL_KEY, text_read, readed_offset);

  LayerScroll ls1 = {{{0, 16}, {160, 162}}, SCROLL_KEY, readed_offset, U_ASCII_ARIAL_16, 2, GColorBlue};
  P_Layer layer = app_layer_create_scroll(&ls1);
  app_layer_set_bg_color(layer, GColorWhite);

  app_window_add_layer(p_window, layer);

  if (g_text_offset + readed_offset >= get_res_size(MY_FILE_KEY))
    {
      g_page_end_flag = 1;
    }
  else
    {
      g_page_end_flag = 0;
    }

  /*添加按键事件，实现上下翻页功能*/
  app_window_click_subscribe(p_window, ButtonIdSelect, scroll_select_select);
  app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);
  app_window_click_subscribe(p_window, ButtonIdDown, scroll_select_down);
  app_window_click_subscribe(p_window, ButtonIdUp, scroll_select_up);



  /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
  maibu_service_sys_event_subscribe(time_change);
  return p_window;
}


int main(void)
{
  /*创建显示窗口*/
  P_Window p_window = init_window();
  if (NULL == p_window)
    {
      return -1;
    }
  /*放入窗口栈显示*/
  g_window_id = app_window_stack_push(p_window);
  return 0;
}




