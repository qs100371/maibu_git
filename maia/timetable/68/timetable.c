#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "maibu_sdk.h"
#include "maibu_res.h"

//保存上课时间，课程代码和课程表的KEY ID
#define TIME_KEY 1
#define COURSE_KEY 2
#define TIMETABLE_KEY 3

/*标志位*/

//窗口ID
static int32_t g_window_id = -1;
/*全局定时器*/
static int8_t g_timer_id = -1;


//表盘的记录的当天日期，用于比较日期是否变为第二天
static uint8_t g_today_num = 0;


static int8_t  g_time_bmp_num[4]= {0};

//课程表更新标志和是否正在显示2日课程表标记
static int8_t  show2day=0;//updatetime=0,

static int   duration=0,classtime[12],coursecount=0,classcount=0,today_course[12];
static char  courses[12][10];

/*初始数据*/


//字符串
//年月日 周
char g_str_ymd[18] = {""};
//当前
char g_courses[3][20] = {"无课","无课","无课"};


static char wday[7][8]= {"周日","周一","周二","周三","周四","周五","周六"};



//图片ID数组
int32_t bmp_array_name[12] =
{
	RES_BITMAP_NUMBER_0,
	RES_BITMAP_NUMBER_1,
	RES_BITMAP_NUMBER_2,
	RES_BITMAP_NUMBER_3,
	RES_BITMAP_NUMBER_4,
	RES_BITMAP_NUMBER_5,
	RES_BITMAP_NUMBER_6,
	RES_BITMAP_NUMBER_7,
	RES_BITMAP_NUMBER_8,
	RES_BITMAP_NUMBER_9,
	RES_BITMAP_BG,
	RES_BITMAP_BG2
};


P_Window init_window(void);
void data_handler_per_day();
void data_handler_per_minute();
void scroll_select_back(void *context);

//重新载入并刷新窗口所有图层
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
int32_t display_target_layer(P_Window p_window,GRect *temp_p_frame,enum GAlign how_to_align,enum GColor black_or_white,int32_t bmp_array_name_key[],int bmp_id_number)
{

	GBitmap bmp_point;
	P_Layer temp_P_Layer = NULL;

	res_get_user_bitmap(bmp_array_name_key[bmp_id_number], &bmp_point);
	LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, how_to_align};
	temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);
	if(temp_P_Layer != NULL)
	{
		app_layer_set_bg_color(temp_P_Layer, black_or_white);
		return app_window_add_layer(p_window, temp_P_Layer);
	}

	return 0;
}


/*创建并显示文本图层*/
int32_t display_target_layerText(P_Window p_window,const GRect  *temp_p_frame,enum GAlign how_to_align,enum GColor color,char * str,uint8_t font_type,enum GColor fcolor)
{
	LayerText temp_LayerText = {0};
	temp_LayerText.text = str;
	temp_LayerText.frame = *temp_p_frame;
	temp_LayerText.alignment = how_to_align;
	temp_LayerText.font_type = font_type;
	temp_LayerText.foregroundColor = fcolor;
	
	P_Layer p_layer = app_layer_create_text(&temp_LayerText);
	
	if(p_layer != NULL)
	{
		app_layer_set_bg_color(p_layer, color);
		return app_window_add_layer(p_window, p_layer);
	}
	return 0;
}

void delblank(char *str)
{
	int i,j,k;
	//去掉后置空格
	for(i=strlen(str)-1;*(str+i)==' ';i--);
	*(str+i+1)='\0';

	//去掉前置空格
	for(i=0;*(str+i)==' ';i++);//i个空格
	for(j=0;j<strlen(str)-i;j++) *(str+j)=*(str+j+i);
	*(str+j)='\0';

	//去掉重复的空格
	int l=0;
	do{
	for(i=l;*(str+i)!=' ';i++);
	for(j=i;*(str+j)==' '&&*(str+j+1)==' ';j++);//j-i是连续空格
	for(k=i+1;k<strlen(str)+j-i+1;k++) *(str+k)=*(str+k+j-i);
	*(str+k)='\0';
	l=i+1;
	}while(l<strlen(str));

} 

void watchapp_comm_callback(enum ESyncWatchApp type, uint8_t *context, uint16_t context_len)
{
	if(type == ESyncWatchAppUpdateParam)
	{

		if(context_len > 0)
		{
			char tmpstr[100]= {0};
			int i,j;
			for(i=0; context[i]!='\n'; i++);
			memset(tmpstr,0,100);
			memcpy(tmpstr,context,i);
			//tmpstr[i]='\0';
			delblank(tmpstr);
			app_persist_write_data_extend(TIME_KEY,tmpstr,strlen(tmpstr)); //保存第一行

			for(j=i+1; context[j]!='\n'; j++);
			memset(tmpstr,0,100);
			memcpy(tmpstr,context+i+1,j - i-1);
			//tmpstr[j-i-1]='\0';
			delblank(tmpstr);
			app_persist_write_data_extend(COURSE_KEY,tmpstr,strlen(tmpstr));//保存第二行

			for(i=j+1; (context[i]!=' '||context[i]!='\n')&&i<context_len; i++);
			memset(tmpstr,0,100);
			memcpy(tmpstr,context+j+1,i-j-1);
			//tmpstr[i-j-1]='\0';
			delblank(tmpstr);
			app_persist_write_data_extend(TIMETABLE_KEY,tmpstr,strlen(tmpstr));//保存第三行

			data_handler_per_day();

			data_handler_per_minute();
			window_reloading();
		}
	}
}




P_Window init_window(void)
{

	P_Window p_window = app_window_create();
	if (NULL == p_window)
	{
		return NULL;
	}

	//添加背景图层
	GRect bgframe = {{0,0},{176,176}};
	display_target_layer(p_window,&bgframe,GAlignCenter,GColorBlack,bmp_array_name,10);

	//添加日期周文本图层
	GRect ymdframe = {{0,4},{16,176}};
	display_target_layerText(p_window,&ymdframe,GAlignCenter,GColorWhite,g_str_ymd,U_ASCII_ARIAL_16,GColorBlack);

	//添加时间图片图层
	GRect timeframe = {{32,37},{47,26}};//{{23,27},{34,19}}

	uint8_t ii;
	for(ii = 0; ii <=3 ; ii++)
	{
		display_target_layer(p_window,&timeframe,GAlignLeft,GColorBlack,bmp_array_name,g_time_bmp_num[ii]);
		timeframe.origin.x = timeframe.origin.x + 26;
		if(ii == 1)
		{
			timeframe.origin.x = timeframe.origin.x + 12;
		}
	}
	/*添加按键事件*/
	app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);
	///
	if((app_persist_get_data_size(TIME_KEY)==0)||(app_persist_get_data_size(COURSE_KEY)==0)||(app_persist_get_data_size(TIMETABLE_KEY)==0))
	{
		memset(g_courses,0,sizeof(g_courses));
		sprintf(g_courses[0],"课程未设置");
	}

	//添加课程1
	GRect course1frame = {{42,93},{20,110}};
	display_target_layerText(p_window,&course1frame,GAlignLeft,GColorRed,g_courses[0],U_ASCII_ARIAL_20,GColorWhite);

	//添加课程2
	GRect course2frame = {{42,122},{20,110}};
	display_target_layerText(p_window,&course2frame,GAlignLeft,GColorWhite,g_courses[1],U_ASCII_ARIAL_20,GColorBlack);

	//添加课程3
	GRect course3frame = {{42,150},{20,110}};
	display_target_layerText(p_window,&course3frame,GAlignLeft,GColorWhite,g_courses[2],U_ASCII_ARIAL_20,GColorBlack);
	show2day=0;


	return p_window;
}



void data_handler_per_minute()
{
	//获取时间数据
	struct date_time datetime_perminute;
	app_service_get_datetime(&datetime_perminute);
	g_today_num = datetime_perminute.mday;

	g_time_bmp_num[0] = datetime_perminute.hour/10;
	g_time_bmp_num[1] = datetime_perminute.hour%10;

	g_time_bmp_num[2] = datetime_perminute.min/10;
	g_time_bmp_num[3] = datetime_perminute.min%10;
	//updatetime--;
	//if (updatetime>0) return;

	int i,timenow=datetime_perminute.hour*60+datetime_perminute.min;
	//memset(g_courses,0,sizeof(g_courses));
	//timenow=660;
	if(classcount==0)
	{
		sprintf(g_courses[0],"无课");
		sprintf(g_courses[1],"无课");
		sprintf(g_courses[2],"无课");
		return;
	}

	if(timenow<classtime[0])
	{
		//updatetime=classtime[0]-timenow;
		if(classcount>=2)
		{
			sprintf(g_courses[0],"无课");
			sprintf(g_courses[1],"%02d:%02d %s",classtime[0]/60,classtime[0]%60,courses[today_course[0]]+1);
			sprintf(g_courses[2],"%02d:%02d %s",classtime[1]/60,classtime[1]%60,courses[today_course[1]]+1);
		}
		else
		{
			sprintf(g_courses[0],"无课");
			sprintf(g_courses[1],"%02d:%02d %s",classtime[0]/60,classtime[0]%60,courses[today_course[0]]+1);
			sprintf(g_courses[2],"无课");
		}
	}
	else if(timenow>classtime[classcount-1]+duration)
	{
		//updatetime=23*60+59-timenow;
		sprintf(g_courses[0],"无课");
		sprintf(g_courses[1],"无课");
		sprintf(g_courses[2],"无课");

	}
	else
	{
		for(i=0; i<classcount; i++)
		{
			if(timenow>classtime[i]+duration&&timenow<classtime[i+1])  //现在是第i节和第i+1节之间
			{
				//updatetime=classtime[i+1]-timenow;
				if(classcount-i>=3)
				{
					sprintf(g_courses[0],"无课");
					sprintf(g_courses[1],"%02d:%02d %s",classtime[i+1]/60,classtime[i+1]%60,courses[today_course[i+1]]+1);
					sprintf(g_courses[2],"%02d:%02d %s",classtime[i+2]/60,classtime[i+2]%60,courses[today_course[i+2]]+1);
				}
				else
				{
					sprintf(g_courses[0],"无课");
					sprintf(g_courses[1],"%02d:%02d %s",classtime[i+1]/60,classtime[i+1]%60,courses[today_course[i+1]]+1);
					sprintf(g_courses[2],"无课");
				}
				return;


			}

			if(timenow>=classtime[i]&&timenow<=classtime[i]+duration)
			{
				//updatetime=classtime[i]+duration-timenow;
				if (i<=classcount-3)  //现在是第i节
				{
					sprintf(g_courses[0],"%02d:%02d %s",classtime[i]/60,classtime[i]%60,courses[today_course[i]]+1);
					sprintf(g_courses[1],"%02d:%02d %s",classtime[i+1]/60,classtime[i+1]%60,courses[today_course[i+1]]+1);
					sprintf(g_courses[2],"%02d:%02d %s",classtime[i+2]/60,classtime[i+2]%60,courses[today_course[i+2]]+1);
				}
				else if (i==classcount-2)
				{
					sprintf(g_courses[0],"%02d:%02d %s",classtime[i]/60,classtime[i]%60,courses[today_course[i]]+1);
					sprintf(g_courses[1],"%02d:%02d %s",classtime[i+1]/60,classtime[i+1]%60,courses[today_course[i+1]]+1);
					sprintf(g_courses[2],"无课");
				}
				else
				{
					sprintf(g_courses[0],"%02d:%02d %s",classtime[i]/60,classtime[i]%60,courses[today_course[i]]+1);
					sprintf(g_courses[1],"无课");
					sprintf(g_courses[2],"无课");
				}
				return;
			}
		}
	}
}

void data_handler_per_day()
{

	//获取时间月日周数据
	struct date_time datetime_perday;
	app_service_get_datetime(&datetime_perday);

	sprintf(g_str_ymd, "%d.%d.%d %s", datetime_perday.year,datetime_perday.mon,datetime_perday.mday,wday[datetime_perday.wday]);

	//获取每一节课的时长和起点
	if(app_persist_get_data_size(TIME_KEY)==0) return;
	char buff[100]= {0},tmp[20]= {0};
	app_persist_read_data(TIME_KEY,0,buff,app_persist_get_data_size(TIME_KEY));
	int i,j,k;
	for(i=0; buff[i]!=' '; i++);
	memcpy(tmp,buff,i);
	duration=atoi(tmp);

	classcount=0;  //(strlen(buff)-i)/5;

	do
	{
		for(j=i+1; buff[j]!=' '&&j<strlen(buff); j++);
		memset(tmp,0,20);
		memcpy(tmp,buff+i+1,j-i-1);
		//tmp[j-i-1]='\0';
		//if(	strlen(tmp)==5)
//	{
		classtime[classcount]=((tmp[0]-'0')*10+(tmp[1]-'0'))*60+(tmp[3]-'0')*10+(tmp[4]-'0');

		classcount++;
		//}

		i=j;
	}
	while(j<strlen(buff));

	//取课程名
	if(app_persist_get_data_size(COURSE_KEY)==0) return;
	memset(buff,0,sizeof(buff));
	app_persist_read_data(COURSE_KEY,0,buff,app_persist_get_data_size(COURSE_KEY));

	i=0;
	coursecount=0;
	memset(courses,0,sizeof(courses));
	do
	{
		for(j=i; buff[j]!=' '&&j<strlen(buff); j++);

		memcpy(courses[coursecount],buff+i,j-i);
		//courses[coursecount][j-i]='\0';
		coursecount++;
		i=j+1;
	}
	while(j<strlen(buff));



	//获取datetime_perday.wday当天的课程
	k=datetime_perday.wday;
	if(k==0)k=7;   //周日为7
	if(app_persist_get_data_size(TIMETABLE_KEY)==0) return;
	memset(buff,0,sizeof(buff));
	app_persist_read_data(TIMETABLE_KEY,0,buff,app_persist_get_data_size(TIMETABLE_KEY));
	//if(strlen(buff)/classcount!=7)return;
	j=(k-1)*classcount;
	for(i=0; j<k*classcount; i++) //取得当天每一节课的名称id,包括空白
	{
		today_course[i]=buff[j]-'a'>=coursecount?100:buff[j]-'a';

		j++;
	}
	if(classcount==0) return;
	//去掉空白
	//for(i=classcount-1;today_course[i]==100&&i>0;i--)
	//	classcount--;
	//sprintf(tmp,"%d",classcount);
	//maibu_print_log(tmp);

	int l=0;
	for(i=0; i<classcount; i++)
	{
		if(today_course[i]!=100)
		{
			today_course[l]=today_course[i];
			classtime[l]=classtime[i];
			l++;
		}

	}
	classcount=l;
	//sprintf(tmp,"%d",classcount);
	//maibu_print_log(tmp);
	//updatetime=0;

}


/*定义后退按键事件*/
void scroll_select_back(void *context)
{
	char buff[100]= {0},tmp[20]= {0};
	int i,j,k;
	//maibu_get_os_version(tmp,20);
	//maibu_print_log(strstr(tmp,"alpha"));
	//if(!strstr(tmp,"增强版")) return;

	if(show2day==1)
	{
		window_reloading();
		show2day=0;
		return;
	}
	P_Window p_window = app_window_create();
	/* if (NULL == p_window)
	{
		return ;
	} */
	///////获取三天的课程表

	//获取时间月日周数据
	struct date_time today;
	app_service_get_datetime(&today);

	//获取每一节课的时长和起点
	if(app_persist_get_data_size(TIME_KEY)==0) return;

	app_persist_read_data(TIME_KEY,0,buff,app_persist_get_data_size(TIME_KEY));
	int classcount1,classtime1[12],course3day[2][12];
	for(i=0; buff[i]!=' '; i++);

	classcount1=0;  //

	do
	{
		for(j=i+1; buff[j]!=' '&&j<strlen(buff); j++);
		memset(tmp,0,20);
		memcpy(tmp,buff+i+1,j-i-1);
		//tmp[j-i-1]='\0';
		//if(	strlen(tmp)==5)
//	{
		classtime1[classcount1]=((tmp[0]-'0')*10+(tmp[1]-'0'))*60+(tmp[3]-'0')*10+(tmp[4]-'0');

		classcount1++;
		//}

		i=j;
	}
	while(j<strlen(buff));


	//获取当天的课程
	k=today.wday;
	if(k==0)k=7;   //周日为7
	if(app_persist_get_data_size(TIMETABLE_KEY)==0) return;
	memset(buff,0,sizeof(buff));
	app_persist_read_data(TIMETABLE_KEY,0,buff,app_persist_get_data_size(TIMETABLE_KEY));
	//if(strlen(buff)/classcount!=7)return;
	j=(k-1)*classcount1;
	for(i=0; j<k*classcount1; i++) //取得当天每一节课的名称id,包括空白
	{
		course3day[0][i]=buff[j]-'a'<coursecount?buff[j]-'a':100;

		j++;
	}

	k=k+1>7?1:k+1;
	j=(k-1)*classcount1;
	for(i=0; j<k*classcount1; i++) //取得第二天每一节课的名称id,包括空白课id为100
	{
		course3day[1][i]=buff[j]-'a'<coursecount?buff[j]-'a':100;

		j++;
	}





	//添加背景图层
	GRect bgframe ={{0,0},{176,176}};
	display_target_layer(p_window,&bgframe,GAlignCenter,GColorBlack,bmp_array_name,11);

	//添加日期周文本图层
	//GRect tmpframe ={{18,4},{12,92}};
	//display_target_layerText(p_window,&tmpframe,GAlignCenter,GColorWhite,g_str_ymd,U_ASCII_ARIAL_12);
	GRect tmpframe;
	//绘制表格//行宽和行高   4列，最多10行
	int x0=13,y0=8,w=50,h;
	h=classcount1>9?16:160/classcount1;
    enum GColor colors[]={GColorRed,GColorGreen,GColorBlue,GColorPurple}; //,GColorYellow,GColorCyan


	//输出课程表
	for(i=0; i<classcount1; i++)
	{
		tmpframe.origin.x=x0+3;
		tmpframe.origin.y=y0+i*h;
		tmpframe.size.h=h;
		tmpframe.size.w=2*w;
		if(course3day[0][i]!=100)
			sprintf(tmp,"%02d:%02d   %s",classtime1[i]/60,classtime1[i]%60,courses[course3day[0][i]]+1);
		else
			sprintf(tmp,"%02d:%02d",classtime1[i]/60,classtime1[i]%60);
		display_target_layerText(p_window,&tmpframe,GAlignLeft,GColorWhite,tmp,U_ASCII_ARIAL_16,colors[i%4]);
	}

	for(i=0; i<classcount1; i++)
	{
		if(course3day[1][i]!=100)
		{
			tmpframe.origin.x=x0+2*w+3;
			tmpframe.origin.y=y0+i*h;
			tmpframe.size.h=h;
			tmpframe.size.w=w;
			display_target_layerText(p_window,&tmpframe,GAlignLeft,GColorWhite,courses[course3day[1][i]]+1,U_ASCII_ARIAL_16,colors[i%4]);
		}
	}
	//绘制网格
	Geometry *p_geometry_array[2];

	GPoint box_points[4] = { {x0,y0},{x0+3*w,y0},{x0+3*w,y0+classcount1*h},{x0,y0+classcount1*h}};
	Polygon box = {4, box_points};
	Geometry geometry = {GeometryTypePolygon, FillOutline,GColorBlack,(void*)&box};
	p_geometry_array[0] = &geometry;

	k=1;

	Line l[classcount1+1];
	for(i=0; i<2; i++)
	{
		l[i].p0.x =x0+(i+1)*w;
		l[i].p0.y=y0;
		l[i].p1.x=x0+(i+1)*w;
		l[i].p1.y=y0+classcount1*h;
	}
	for(j=0; j<classcount1-1; j++)
	{
		l[i].p0.x =x0;
		l[i].p0.y=y0+(j+1)*h;
		l[i].p1.x=x0+3*w;
		l[i].p1.y=y0+(j+1)*h;
		i++;
	}
	LineSet ls= {classcount1+1,l};
	Geometry lg = {GeometryTypeLineSet, FillOutline, GColorBlack, (void*)&ls};
	p_geometry_array[1] = &lg;

	LayerGeometry cell_struct = {2, p_geometry_array};

	P_Layer p_layer = app_layer_create_geometry(&cell_struct);

	app_window_add_layer(p_window, p_layer);


	P_Window p_old_window = (P_Window)app_window_stack_get_window_by_id(g_window_id);
	//if (NULL != p_old_window)
	//{
	g_window_id = app_window_stack_replace_window(p_old_window, p_window);
	show2day=1;
	/*添加按键事件*/
	app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);


	/*放入窗口栈显示*/
	//g_window_id=app_window_stack_push(p_window);

	return;

}




void  time_change (enum SysEventType type, void *context)
{
	/*时间更改*/
	if (type == SysEventTypeTimeChange)
	{
		struct date_time datetime_now_change;
		app_service_get_datetime(&datetime_now_change);

		if(g_today_num != datetime_now_change.mday)
		{
			data_handler_per_day();
		}


		data_handler_per_minute();

		window_reloading();
	}

}

static void timer_callback(date_time_t tick_time, uint32_t millis, void *context)
{
	P_Window p_window = (P_Window)context;
	if(app_persist_get_data_size(TIME_KEY)==0) return;
    //日期变更，更新课程表
	if((g_today_num != tick_time->mday)||p_window==NULL)
		{
			data_handler_per_day();
		}
	g_today_num=tick_time->mday;
	
	int i,timenow=tick_time->hour*60+tick_time->min;
	for(i=0;i<classcount;i++)
	{
		if(classtime[i]-timenow==15)
		{
			//弹出窗口
			NotifyParam	param;
			memset(&param, 0, sizeof(NotifyParam));
			res_get_user_bitmap(RES_BITMAP_REMIND,  &param.bmp);	
			sprintf(param.main_title, "时间到");
			//char tmp[30];
			sprintf(param.sub_title,"%s十五分钟后开始!",courses[today_course[i]]+1);
			//strcpy(param.sub_title,tmp);
			param.pulse_type = VibesPulseTypeLong;
			param.pulse_time = 2;
			maibu_service_sys_notify(&param);
		}
	}
	
	return;
	
	
}

static void maibu_app_init()
{
	//if(g_timer_id ==-1)
	g_timer_id =app_service_timer_subscribe(60000, timer_callback, NULL);
	//注册手机设置回调函数
	
	maibu_comm_register_watchapp_callback(watchapp_comm_callback);
}


int main(void)
{
	//simulator_init();
	//创建储存空间
	app_persist_create(TIME_KEY,50);
	app_persist_create(COURSE_KEY,100);
	app_persist_create(TIMETABLE_KEY,100);

	/* char tmp1[]="100 08:00 10:00 14:00 16:00 19:00";
	app_persist_write_data_extend(TIME_KEY,tmp1,strlen(tmp1));
	char tmp2[]="a高数 b精读 c泛读 d马原 e毛概 f电工 g大物";
	app_persist_write_data_extend(COURSE_KEY,tmp2,strlen(tmp2));
	char tmp3[]="abcdeabcdeabcdexxxxbabcdeabcdeabcde";
	app_persist_write_data_extend(TIMETABLE_KEY,tmp3,strlen(tmp3)); */

	/* char tmp1[]="40 07:55 08:45 10:10 11:00 14:10 15:05";
	app_persist_write_data_extend(TIME_KEY,tmp1,strlen(tmp1));
	char tmp2[]="语文 数学 地方 品生 体育 音乐 美术 阅读";
	app_persist_write_data_extend(COURSE_KEY,tmp2,strlen(tmp2));
	char tmp3[]="cbaegdbadeafadeacxbacahxabfgexxxxxxxxxxxxx";
	app_persist_write_data_extend(TIMETABLE_KEY,tmp3,strlen(tmp3));  */

	/*注册一个事件通知回调，当有改变时，改变表盘显示数据*/
	maibu_service_sys_event_subscribe(time_change);

	time_change(SysEventTypeTimeChange,NULL);

	P_Window p_window = init_window();

	/*放入窗口栈显示*/
	g_window_id = app_window_stack_push(p_window);

	/*添加按键事件，实现上下翻页功能*/
	//app_window_click_subscribe(p_window, ButtonIdBack, scroll_select_back);

	//全局定时器，弹窗提醒
	if(g_timer_id ==-1)
		g_timer_id =app_service_timer_subscribe(60000, timer_callback,NULL );//(void *)p_window
	//注册手机设置回调函数
	maibu_comm_register_watchapp_callback(watchapp_comm_callback);

	//simulator_wait();
	return 0;

}

