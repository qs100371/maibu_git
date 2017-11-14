/*
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

#define SCROLL_KEY	1

/*获取云服务器地址*/
#define WEB_URL     "http://api.sqqbd.com/qsbk/qsbk/"

static int32_t g_app_window_id = -1;


/*Web通讯ID*/
static uint32_t g_comm_id_web = 0;

static int32_t dsi[3];
static int8_t g_i = 1;
static int8_t g_z = 0;
static int32_t g_id = 0;



void request_web()
{
  char url[200] = "";
  sprintf(url, "%s%d?i=%d", WEB_URL, g_id, g_i);
  g_comm_id_web = maibu_comm_request_web(url, "id,i,t,z", 0);

}


/*创建并显示图片图层，需要坐标值，得到icon_key的数组，数组需要的参数值，P_Window*/
int32_t display_target_layer(P_Window p_window, GRect *temp_p_frame, int32_t bmp_id)
{
  GBitmap bmp_point;

  res_get_user_bitmap(bmp_id, &bmp_point);
  LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, GAlignCenter};
  P_Layer temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);

  if (temp_P_Layer != NULL)
    {
      app_layer_set_bg_color(temp_P_Layer, GColorBlack);
      return app_window_add_layer(p_window, temp_P_Layer);
    }

  return 0;
}
/*定义向上按键事件*/
void select_up(void *context)
{
  if (g_i > 1)
    {
      g_i--;
      g_i--;
      request_web();
    }
}


/*定义向下按键事件*/
void select_down(void *context)
{
  if (g_z == 0)
    {
      //g_i++;
      request_web();
    }
}


/*定义后退按键事件*/
void select_back(void *context)
{
  P_Window p_window = (P_Window)context;
  app_window_stack_pop(p_window);
}

/*选择按键*/
void click_select(void *context)
{
  g_id = 0;
  g_i = 0;
  request_web();
}


P_Window zw_create_bg_window2()
{
  P_Window p_window = app_window_create();
  if (NULL == p_window)
    {
      return NULL;
    }
  GRect frame_bg = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame_bg, RES_BITMAP_BG2);
  app_plug_status_bar_create(p_window, NULL, NULL, NULL);
  app_plug_status_bar_add_time(p_window);

  int16_t total_bytes = app_persist_get_data_size(SCROLL_KEY);

  /*初始化滚动结构体, 依次为滚动图层显示框架, 内容文件KEY，需要显示的内容大小、字体字号*/
  LayerScroll ls1 = {{{5, 20}, {156, 166}}, SCROLL_KEY, total_bytes, U_GBK_SIMSUN_20, 2, GColorBlack};
  P_Layer	 layer1 = app_layer_create_scroll(&ls1);
  app_layer_set_bg_color(layer1, GColorWhite);
  app_window_add_layer(p_window, layer1);
  //setInfoLayer(p_window, 0);
  /*添加按键事件，实现上下翻页功能*/

  app_window_click_subscribe(p_window, ButtonIdDown, select_down);
  app_window_click_subscribe(p_window, ButtonIdUp, select_up);
  app_window_click_subscribe(p_window, ButtonIdBack, select_back);
  app_window_click_subscribe(p_window, ButtonIdSelect, click_select);

  return (p_window);
}

void upRequestWeb()
{
  P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_app_window_id);
  if (NULL == p_window)
    {
      return;
    }

  P_Window p_new_window = zw_create_bg_window2();

  g_app_window_id = app_window_stack_replace_window(p_window, p_new_window);

}



P_Window init_window()
{
  P_Window p_window = app_window_create();
  if (NULL == p_window)
    {
      return NULL;
    }

  /*添加完成图片图层*/
  GRect frame_bg = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame_bg, RES_BITMAP_BG1);
  //setInfoLayer(p_window, 0);
  return (p_window);

}

void weather_web_recv_callback(const uint8_t *buff, uint16_t size)
{
  char buffer[150] = "";
  maibu_get_json_str(buff, "t", buffer, 150);
  maibu_get_json_int(buff, "i", &dsi[0]);
  maibu_get_json_int(buff, "z", &dsi[2]);
  maibu_get_json_int(buff, "id", &dsi[1]);
  g_i = dsi[0];
  g_id = dsi[1];
  g_z = dsi[2];

  uint8_t i = 0;
  char *j = NULL;
  do
    {
      j = strstr(buffer + i, "\\n");
      if (j != NULL)
        {
          *j = ' ';
          *(j + 1) = ' ';
        }

      i ++;
    }
  while (j != NULL);
  j = NULL;
  do
    {
      j = strstr(buffer + i, "\\u0001");
      if (j != NULL)
        {
          *j = ' ';
          *(j + 1) = ' ';
          *(j + 2) = ' ';
          *(j + 3) = ' ';
          *(j + 4) = ' ';
          *(j + 5) = ' ';
        }
      i ++;
    }
  while (j != NULL);
  j = NULL;
  do
    {
      j = strstr(buffer + i, "\\ur");
      if (j != NULL)
        {
          *j = ' ';
          *(j + 1) = ' ';
        }
      i ++;
    }
  while (j != NULL);

  j = buffer;
  for (i = 0;i < strlen(buffer);i++)
    {
      if (buffer[i] == ' ')
        continue;
      j++;
      *j = buffer[i];
    }
  *j = '\0';

  app_persist_write_data_extend(SCROLL_KEY, buffer, strlen(buffer));

  upRequestWeb();
}
static void comm_result_callback(enum ECommResult result, uint32_t comm_id, void *context)
{
  /*如果上一次请求WEB通讯失败，并且通讯ID相同，则重新发送*/
  if ((result == ECommResultFail) && (comm_id == g_comm_id_web))
    {
      request_web();
    }
}

int main()
{
//simulator_init();

  P_Window p_window = init_window();
  if (p_window != NULL)
    {
      g_app_window_id = app_window_stack_push(p_window);
    }

  app_persist_create(SCROLL_KEY, 200);
  maibu_comm_register_web_callback(weather_web_recv_callback);
  //注册通讯结果回调
  maibu_comm_register_result_callback(comm_result_callback);
  app_window_click_subscribe(p_window, ButtonIdBack, select_back);
  app_window_click_subscribe(p_window, ButtonIdSelect, click_select);
  g_i--;
  uint32_t t = app_get_time_number();
  while (app_get_time_number() - t < 2)
    {
      //等待2秒
    }
  request_web();

//simulator_wait();
  return 0;
}




