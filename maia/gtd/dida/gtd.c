#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maibu_sdk.h"
#include "maibu_res.h"


/*窗口句柄*/
static uint32_t g_window = -1;
/*全局定时器*/
//static int8_t g_timer_id = -1;

/*滴答通讯协议*/
#define PROTOCOL_UPDATE_LIST			0x01
#define PROTOCOL_CHECK_LIST				0x02
#define PROTOCOL_GET_CHECK_LIST			0x03
#define PROTOCOL_UPDATE_LIST_ACK		0x21
#define PROTOCOL_CHECK_LIST_ACK			0x22
#define PROTOCOL_GET_LIST_ACK			0x23


/*通讯相关*/

/*通讯ID：更新任务应答*/
static int32_t g_comm_id_update_list_ack = -1;
static int32_t g_comm_id_check_ok = -1;
static int32_t g_comm_id_get_list_ack = -1;

/*Linkid*/
static char g_link_id[10] = "";

/*上一个check的任务ID*/
static int32_t g_task_id = -1;


/*应答错误码*/
static int8_t g_error_code_update_ack = 0;

/*任务项结构体*/
typedef struct tag_STaskItem
  {
    uint32_t id;	//任务ID
    uint32_t date;	//任务日期时间
    char title[24];	//任务标题
  }STaskItem;

/*任务项信息*/
typedef struct tag_STaskInfo
  {
    int8_t checked;	//是否已check	0 未check 1 checking  2 checked
    int8_t used;	//是否已使用	0 未使用 1 已使用
    int8_t alarm;	//是否已提醒，只针对Android
    STaskItem item;
  }STaskInfo;


#define TASK_LIST_MAX_SIZE	5
#define NOT_USED	0
#define USED	1
#define NOT_HAVE_CHECKED	0
#define CHECKING		1
#define HAVE_CHECKED		2
#define NOT_HAVE_ALARM		0
#define HAVE_ALARM		1


/*任务列表结构体*/
typedef struct tag_STaskInfoList
  {
    uint8_t nums;	//当前任务项个数
    STaskInfo list[TASK_LIST_MAX_SIZE];	//任务项列表
  }STaskInfoList;

/*任务列表*/
static STaskInfoList g_s_task_list = {0};


void window_reloading(void);

const static char wday_str[7][8] = {"日", "一", "二", "三", "四", "五", "六"};

static P_Window init_watch();

/*
 *--------------------------------------------------------------------------------------
 *     function:  send_get_task_list_ack
 *    parameter:
 *       return:
 *  description:  发送获取任务列表应答
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
int32_t send_get_task_list_ack()
{
  uint8_t ack[30] = {0xdd, 0x01, 0x23};
  int8_t i = 0, count = 0;

  for (i = 0; i < TASK_LIST_MAX_SIZE; i++)
    {
      if (g_s_task_list.list[i].checked == CHECKING)
        {
          memcpy(&ack[4+sizeof(int32_t)*count], (char *)&g_s_task_list.list[i].item.id,  sizeof(int32_t));
          count++;
        }
    }
  ack[3] = sizeof(int32_t) * count;

  return (maibu_comm_send_msg(g_link_id, ack, ack[3] + 4));
}
/*
 *--------------------------------------------------------------------------------------
 *     function:  send_check_task
 *    parameter:
 *       return:
 *  description:  发送完成任务命令
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
/* int32_t send_check_task(int32_t task_id)
{
  uint8_t data[8] = {0xdd, 0x01, 0x02, 0x04};
  memcpy(&data[4], &task_id, sizeof(int32_t));

  return (maibu_comm_send_msg(g_link_id, data, sizeof(data)));
} */

/*
 *--------------------------------------------------------------------------------------
 *     function:  send_update_task_list_ack
 *    parameter:
 *       return:
 *  description:  发送更新任务列表应答包
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
int32_t send_update_task_list_ack()
{
  uint8_t ack[5] = {0xdd, 0x01, 0x21, 0x01, g_error_code_update_ack};

  return (maibu_comm_send_msg(g_link_id, ack, sizeof(ack)));
}
/*
 *--------------------------------------------------------------------------------------
 *     function:  msg_recv_callback
 *    parameter:
 *       return:
 *  description:  处理来此手机第三方APP协议数据
 * 	  other:  协议格式：{(固定标志,0xDD), (协议版本,0x01)，(命令ID,1字节),(命令结构内容，命令ID不同，内容不同)}
 *--------------------------------------------------------------------------------------
 */
void msg_recv_callback(const char *link_id, const uint8_t *buff, uint16_t size)
{

  memset(g_link_id, 0, sizeof(g_link_id));
  strcpy(g_link_id, link_id);

  /*是否滴答协议*/
  if ((size < 3) || (0xDD != buff[0]) || (0x01 != buff[1]))
    {
      return;
    }

  /*如果是更新任务列表*/
  if (buff[2] == PROTOCOL_UPDATE_LIST)
    {
      /*长度不对，或者数据结构不对*/
      if ((size < 4) || (size != (4 + buff[3])))
        {
          /*发送错误应答*/
          g_error_code_update_ack = -2;
          g_comm_id_update_list_ack = send_update_task_list_ack();
          return;
        }

      /*保存任务列表,之前的信息全部清空*/
      memset(&g_s_task_list, 0, sizeof(STaskInfoList));
      g_s_task_list.nums = buff[3] / sizeof(STaskItem);
      int i = 0;
      for (i = 0; i < g_s_task_list.nums; i++)
        {
          memcpy(&g_s_task_list.list[i].item, &buff[4 + sizeof(STaskItem)*i], sizeof(STaskItem));
          g_s_task_list.list[i].used = USED;
        }

      /*发送成功应答*/
      g_error_code_update_ack = 0;
      g_comm_id_update_list_ack = send_update_task_list_ack();


    }/*完成任务命令应答*/
   else if (buff[2] == PROTOCOL_CHECK_LIST_ACK)
    {


      int32_t id_checked = 0;
      int8_t id_nums = 0;
      id_nums = buff[3] / sizeof(int32_t);
      int8_t i = 0, j = 0;
      for (i = 0; i < id_nums; i++)
        {
          memcpy(&id_checked, &buff[4+sizeof(int32_t)*i], sizeof(int32_t));
          for (j = 0; j < TASK_LIST_MAX_SIZE; j++)
            {
              if (g_s_task_list.list[j].item.id == id_checked)
                {
                  memset(&g_s_task_list.list[j], 0, sizeof(STaskInfo));
                }
            }
        }


    }
  else if (buff[2] == PROTOCOL_GET_CHECK_LIST)
    {
      g_comm_id_get_list_ack = send_get_task_list_ack();
    }
 
  //更新窗口, 获取已check任务列表也会更新菜单，但是必须在收到通讯成功应答后
  if ((buff[2] == PROTOCOL_CHECK_LIST_ACK) || (buff[2] == PROTOCOL_UPDATE_LIST))
    {
      window_reloading();
    }

  return;
}


/*
 *--------------------------------------------------------------------------------------
 *     function:  msg_result_callback
 *    parameter:
 *       return:
 *  description:  通讯结果回调
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
void msg_result_callback(enum ECommResult status, uint32_t comm_id, void *context)
{

  /*如果更新任务应答发送失败，则重发*/
  if ((ECommResultFail == status) && (comm_id == g_comm_id_update_list_ack))
    {
      send_update_task_list_ack();
    }

  /*如果任务完成发送失败，则重发*/
  if ((ECommResultFail == status) && (comm_id == g_comm_id_check_ok))
    {

    }

  /*如果任务完成发送失败，则重发*/
  if ((ECommResultFail == status) && (comm_id == g_comm_id_get_list_ack))
    {
      send_get_task_list_ack();
    }/*成功，则清除*/
  else if ((ECommResultSuccess == status) && (comm_id == g_comm_id_get_list_ack))
    {


    }
  
  
  return;
}


/* static void timer_callback(date_time_t tick_time, uint32_t millis, void *context)
{
	int8_t i = 0;
	NotifyParam	param;

	if(maibu_get_phone_type() == PhoneTypeIOS)
	{
		return;
	}

	for (i = 0; i < TASK_LIST_MAX_SIZE; i++)
	{
		if ((g_s_task_list.list[i].used == NOT_USED) || (g_s_task_list.list[i].alarm == HAVE_ALARM))
		{
			continue;
		}


		uint32_t seconds = app_get_time(tick_time);



		//如果没有提醒过，并且是在120秒以内，则提醒
		if (120 > (seconds - g_s_task_list.list[i].item.date))
		{

			g_s_task_list.list[i].alarm = HAVE_ALARM;
			memset(&param, 0, sizeof(NotifyParam));
			res_get_user_bitmap(RES_BITMAP_WATCHAPP_DIDA_ALARM,  &param.bmp);
			sprintf(param.main_title, "%s", "滴答清单");
			strcpy(param.sub_title, g_s_task_list.list[i].item.title);
			param.pulse_type = VibesPulseTypeMiddle;
			param.pulse_time = 6;
			maibu_service_sys_notify(&param);

		}
	}


}
 */


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

static P_Window init_watch()
{
  P_Window p_window = NULL;
  P_Layer p_layer = NULL;


  char buff[40] = {0};
  /*创建一个窗口*/
  p_window = app_window_create();
  if (NULL == p_window) return NULL;
  struct date_time dt;
  app_service_get_datetime(&dt);
  //enum GColor colors[] = {GColorRed, GColorGreen, GColorBlue, GColorPurple}; //,GColorYellow

  GRect frame = {{0, 0}, {176, 176}};
  display_target_layer(p_window, &frame, RES_BG);

  int i;
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
  sprintf(buff, "%02d:%02d", dt.hour, dt.min);
  display_target_layerText(p_window, &frame, GAlignLeft, GColorBlue, buff, U_ASCII_ARIAL_16, GColorWhite);

  uint8_t count = 0;
  for (i = 0; i < TASK_LIST_MAX_SIZE && count < 4; i++)
    {
      if (g_s_task_list.list[i].used == NOT_USED)
        {
          continue;
        }

      //非常重要, 要清空
      memset(buff, 0, sizeof(buff));

      memcpy(buff, g_s_task_list.list[i].item.title, sizeof(g_s_task_list.list[i].item.title));

      frame.origin.x = 5;
      frame.origin.y = 33 + count * 36;
      frame.size.h = 16;
      frame.size.w = 166;
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_GBK_SIMSUN_16, GColorBlack);


      /*添加菜单项中时间元素*/
      struct date_time t;
      if (0 == g_s_task_list.list[i].item.date)	//为0则需要显示为全天
        {
          strcpy(buff, "全天");
        }
      else
        {
          app_get_dt_by_num(g_s_task_list.list[i].item.date, &t);
          sprintf(buff, "%02d:%02d", t.hour, t.min);
        }


      frame.origin.x = 5;
      frame.origin.y = 50 + count * 36;
      frame.size.h = 16;
      frame.size.w = 166;
      display_target_layerText(p_window, &frame, GAlignLeft, GColorWhite, buff, U_ASCII_ARIAL_16, GColorRed);

      count++;
    }

  return p_window;
}



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
  
  /*注册接受数据回调函数*/
  maibu_comm_register_msg_callback((CBCommMsg)msg_recv_callback);
  /*注册通讯结果状态回调*/
  maibu_comm_register_result_callback(msg_result_callback);
  /*注册定时器, Android任务到时提醒*/
  //app_service_timer_subscribe(30000, timer_callback, (void *)p_window);

  return 0;
}



