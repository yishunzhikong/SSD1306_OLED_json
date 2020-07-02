/*
 * Main.c
 * 
 * 创建于	: 2020.02.20
 * 作者	: yishunjian
 * 描述	: 通过json配置文件配置OLED显示内容
*/

/* 库文件 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

/* 头文件 */
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "cJSON.h"
#include "dataapi.h"

#define MAIN_DEBUG

/* flags */
typedef struct timeFlag
{
	bool CycleFlag;
	bool TimeFlag;
}timeFlag;


/* Oh Compiler-Please leave me as is */
static unsigned char flag = 0;
static timeFlag TimeFlag={1,0}; 
static int RunPageNum = 0;
//锁存自由页面运行状态
static bool RunPageFlag = 0;
static int RunLoopNum = 1;
static bool scrollLatch = 0;

void setTimer(void);
void timerHandler(int num);
void oledRun(char *conf);
void pageDisplay(pageData *pagedata);
void pageStart(pageData *pagedata);
void pageStop(pageData *pagedata);

///* 报警信号处理程序 */
//void ALARMhandler(int sig)
//{
//    /* Set flag */
//    flag = 5;
//}

int main(int argc, char **argv)
{
	int ArgcCnt=0;
	unsigned int i,j;
	unsigned int pagelen;
	FILE *ConfigFile;
	//////////////////////////////////////////////////////////////////////////////////////////////////
	cJSON *json_page, *json_start, *json_stop, *json_display, *json_class, *json_data;
	unsigned int ItemCnt,PageCnt;
	
//  /* 初始化I2C总线并连接到I2C设备 */
//  if(init_i2c_dev(I2C_DEV0_PATH, SSD1306_OLED_ADDR) != 0)
//  {
//		printf("(Main)i2c-2: OOPS! Something Went Wrong\r\n");
//      exit(1);
//  }
	
    /* 注册报警处理程序 */
//    signal(SIGALRM, ALARMhandler);
	/* 注册100ms定时器 */
	setTimer();

//    /* 运行SDD1306初始化序列 */
//    display_Init_seq();
	
	if (argc < 2) 
	{
		oledRun(SetingData.config);
	}
	else
	{
		for(ArgcCnt = 1; ArgcCnt < argc; ArgcCnt ++)
		{
			if(!strcmp(argv[1],"-h"))
			{
				;//
			}
			else if(!strcmp(argv[ArgcCnt],"-c"))
			{
				ArgcCnt ++;
				if(argv[ArgcCnt][0] == '-')
				{
					printf("unrecognized option:%s",argv[ArgcCnt]);
					exit(1);
				}
				else
				{
					if(access(argv[ArgcCnt], R_OK) != 0)
					{
						printf("%s does not have read permission or it doesn't exist\r\n",argv[ArgcCnt]);
						exit(1);	
					}
					else
					{
						//*************************************************************
						printf("%s Loading succeeded！\r\n",argv[ArgcCnt]);
						oledRun(argv[ArgcCnt]);
					}
				}
			}
			else if(!strcmp(argv[1],"-clean"))
			{
				getJsonData(SetingData.config);
				oledInit();
				clearDisplay();//
				Display();
			}
			else
			{
				printf("unrecognized option:%s\r\n",argv[ArgcCnt]);
				exit(1);
			}
		}
	}
	exit(0);
}

void setTimer(void)
{
  
	// Get system call result to determine successful or failed  
	int res = 0;  
	
	// Register printMsg to SIGALRM   
	signal(SIGALRM, timerHandler);  
	
	struct itimerval tick;  
	
	// Initialize struct    
	memset(&tick, 0, sizeof(tick));  
	
	// Timeout to run function first time    
	tick.it_value.tv_sec = 0;  // sec  
	tick.it_value.tv_usec = 100000; // micro sec.  
	// Interval time to run function   
	tick.it_interval.tv_sec = 0;  
	tick.it_interval.tv_usec = 100000;  
   
	if (setitimer(ITIMER_REAL, &tick, NULL)) 
	{
		printf("Set timer failed!!/n");
	} 
}

void timerHandler(int num)
{
	static int cyclecnt = 1, timecnt = 1;
	
	if(cyclecnt < RunPageData->cycle)
	{
		cyclecnt ++;
	}
	else if(RunPageData->cycle != 0)
	{
//		printf("cycle ");
		cyclecnt = 1;
		TimeFlag.CycleFlag = 1;
	}
	
	if(timecnt < RunPageData->time)
	{
		timecnt ++;
	}
	else if(RunPageData->time != 0)
	{
//		printf("time\n");
		timecnt = 1;
		TimeFlag.TimeFlag = 1;
	}
}

void oledRun(char *conf)
{
	getJsonData(conf);
	oledInit();
	
	while(1)
	{
		if(TimeFlag.CycleFlag)
		{
			TimeFlag.CycleFlag = 0;
			clearDisplay();
			pageDisplay(RunPageData);
			Display();
		}
		if(TimeFlag.TimeFlag)
		{
			TimeFlag.TimeFlag = 0;
			stopscroll();
			scrollLatch = 0;
				
			if(RunLoopNum < SetingData.loopnum - 1)
			{
				RunLoopNum ++;
			}
			else RunLoopNum = 1;
			if(RunLoopNum != 0)
			{
				RunPageData = getPageNode( LoopPageData, RunLoopNum, DATAPAGE);
				if(RunPageData == NULL)break;
			}
		}
		if(!scrollLatch)
		{
			scrollLatch = 1;
			printf("scroll\n");//////////////////////////////////
			switch(RunPageData->scroll)
			{
				case 1:
					startscrollright( \
					RunPageData->scrollstart, \
					RunPageData->scrollstop);
//					scrollLatch = 1;
				break;
				case 2:
					startscrollleft( \
					RunPageData->scrollstart, \
					RunPageData->scrollstop);
//					scrollLatch = 1;
				break;
				case 3:
					startscrolldiagright( \
					RunPageData->scrollstart, \
					RunPageData->scrollstop);
//					scrollLatch = 1;
				break;
				case 4:
					startscrolldiagleft( \
					RunPageData->scrollstart, \
					RunPageData->scrollstop);
//					scrollLatch = 1;
				break;
				default:
				break;
			}
		}
		if(RunLoopNum == 0 && RunPageFlag == 0)
		{
			;
		}
		pageStart(PageData);
		pageStop(RunPageData);
		usleep(100000);
	}
	
//	//释放空间
//	free(PageData);
//	cJSON_Delete(ConfigJson);//释放json空间
	exit(0);
}

void pageDisplay(pageData *pagedata)
{
	cJSON *datajson, *dispjson;
	long temp;
	char buffer[BUFFSIZE];
	char cmd[BUFFSIZE];
	char *p;
	time_t rawtime;

	//display
	if(pagedata->display)
	{
		dispjson = pagedata->display;
		while(dispjson != NULL && cJSON_GetArraySize(dispjson) > 0)
		{
			datajson = cJSON_GetObjectItem(dispjson,"type");
			if(datajson->type == cJSON_String && getJsonNumInt(dispjson,"en"))
			{
				if(strcmp(datajson->valuestring,"base") == 0)
				{
					switch(getJsonNumInt(dispjson,"func"))
					{
						case 0:
							time( &rawtime );
							p = getJsonStr(dispjson,"class");
							if(p == NULL) strftime(buffer, BUFFSIZE, "%Y-%m-%d", localtime(&rawtime));
							else strftime(buffer, BUFFSIZE, p, localtime(&rawtime));
							setTextSize(getJsonNumInt(dispjson,"size"));
							setTextColor(getJsonNumInt(dispjson,"color"));
							setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
							print_strln(buffer);
						break;
						case 1:
							if(readFile( buffer, TempPath,5) == 0) break;
							setTextSize(getJsonNumInt(dispjson,"size"));
							setTextColor(getJsonNumInt(dispjson,"color"));
							setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
							temp = atol(buffer);
							sprintf(buffer,"%f",(float)temp/1000);
							buffer[getJsonNumInt(dispjson,"base")] = '\0';
							print_str(buffer);
						break;
						case 2:
							getShell(buffer,FreqPath,5);
							setTextSize(getJsonNumInt(dispjson,"size"));
							setTextColor(getJsonNumInt(dispjson,"color"));
							setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
							if(getJsonNumInt(dispjson,"class"))
							{
								temp = atol(buffer);
								sprintf(buffer,"%f",(float)temp/1000);
							}
							buffer[getJsonNumInt(dispjson,"base")] = '\0';
							print_str(buffer);
						break;
						case 3:
							sprintf(cmd, \
							"ifconfig %s|grep 'inet addr:'|cut -d: -f2|awk '{print $1}'", \
							getJsonStr(dispjson,"port"));
							getShell(buffer,cmd,16);
							setTextSize(getJsonNumInt(dispjson,"size"));
							setTextColor(getJsonNumInt(dispjson,"color"));
							setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
							buffer[getJsonNumInt(dispjson,"base")] = '\0';
							print_str(buffer);
						break;
						default:
						break;
					}
				}
//				else if(strcmp(datajson->valuestring,"scroll") == 0)
//				{
//					if(scrollLatch == 0)
//					{
//						if(strcmp(getJsonStr(dispjson,"path"),"r") == 0)
//						{
//							startscrollright(getJsonNumInt(dispjson,"start"), \
//							getJsonNumInt(dispjson,"stop"));
//							scrollLatch = 1;
//						}
//						else if(strcmp(getJsonStr(dispjson,"path"),"l") == 0)
//						{
//							startscrollleft(getJsonNumInt(dispjson,"start"), \
//							getJsonNumInt(dispjson,"stop"));
//							scrollLatch = 1;
//						}
//						else if(strcmp(getJsonStr(dispjson,"path"),"dr") == 0)
//						{
//							startscrolldiagright(getJsonNumInt(dispjson,"start"), \
//							getJsonNumInt(dispjson,"stop"));
//							scrollLatch = 1;
//						}
//						else if(strcmp(getJsonStr(dispjson,"path"),"dl") == 0)
//						{
//							startscrolldiagleft(getJsonNumInt(dispjson,"start"), \
//							getJsonNumInt(dispjson,"stop"));
//							scrollLatch = 1;
//						}
//					}
//				}
				else if(strcmp(datajson->valuestring,"pixel") == 0)
				{
					drawPixel(getJsonNumInt(dispjson,"x0"), \
					getJsonNumInt(dispjson,"y0"), \
					getJsonNumInt(dispjson,"color"));
				}
				else if(strcmp(datajson->valuestring,"line") == 0)
				{
					drawLine(getJsonNumInt(dispjson,"x0"), \
					getJsonNumInt(dispjson,"y0"), \
					getJsonNumInt(dispjson,"x1"), \
					getJsonNumInt(dispjson,"y1"), \
					getJsonNumInt(dispjson,"color"));
				}
				else if(strcmp(datajson->valuestring,"rect") == 0)
				{
					if(getJsonNumInt(dispjson,"fill") == 0)
					{
						drawRect(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"w"), \
						getJsonNumInt(dispjson,"h"), \
						getJsonNumInt(dispjson,"color"));
					}
					else
					{
						fillRect(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"w"), \
						getJsonNumInt(dispjson,"h"), \
						getJsonNumInt(dispjson,"color"));
					}
				}
				else if(strcmp(datajson->valuestring,"circle") == 0)
				{
					if(getJsonNumInt(dispjson,"fill") == 0)
					{
						drawCircle(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"r"), \
						getJsonNumInt(dispjson,"color"));
					}
					else
					{
						fillCircle(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"r"), \
						getJsonNumInt(dispjson,"color"));
					}
				}
				else if(strcmp(datajson->valuestring,"trle") == 0)
				{
					if(getJsonNumInt(dispjson,"fill") == 0)
					{
						drawTriangle(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"x1"), \
						getJsonNumInt(dispjson,"y1"), \
						getJsonNumInt(dispjson,"x2"), \
						getJsonNumInt(dispjson,"y2"), \
						getJsonNumInt(dispjson,"color"));
					}
					else
					{
						fillTriangle(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"x1"), \
						getJsonNumInt(dispjson,"y1"), \
						getJsonNumInt(dispjson,"x2"), \
						getJsonNumInt(dispjson,"y2"), \
						getJsonNumInt(dispjson,"color"));
					}
				}
				else if(strcmp(datajson->valuestring,"r_rect") == 0)
				{
					if(getJsonNumInt(dispjson,"fill") == 0)
					{
						drawRoundRect(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"w"), \
						getJsonNumInt(dispjson,"h"), \
						getJsonNumInt(dispjson,"r"), \
						getJsonNumInt(dispjson,"color"));
					}
					else
					{
						fillRoundRect(getJsonNumInt(dispjson,"x0"), \
						getJsonNumInt(dispjson,"y0"), \
						getJsonNumInt(dispjson,"w"), \
						getJsonNumInt(dispjson,"h"), \
						getJsonNumInt(dispjson,"r"), \
						getJsonNumInt(dispjson,"color"));
					}
				}
				else if(strcmp(datajson->valuestring,"bmp") == 0)
				{
					printf("%s in development ...\n",datajson->valuestring);
				}
				else if(strcmp(datajson->valuestring,"str") == 0)
				{
					setTextSize(getJsonNumInt(dispjson,"size"));
					setTextColor(getJsonNumInt(dispjson,"color"));
					setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
					print_str(getJsonStr(dispjson,"data"));
				}
				else if(strcmp(datajson->valuestring,"num") == 0)
				{
					//memset(buffer,'\0',BUFFSIZE);
					setTextSize(getJsonNumInt(dispjson,"size"));
					setTextColor(getJsonNumInt(dispjson,"color"));
					setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
					if(strcmp(getJsonStr(dispjson,"func"),"ui") == 0)
					{
						sprintf(buffer,"%d",getJsonNumInt(dispjson,"data"));
					}
					else if(strcmp(getJsonStr(dispjson,"func"),"fl") == 0)
					{
						sprintf(buffer,"%f",getJsonNumDouble(dispjson,"data"));
					}
					buffer[getJsonNumInt(dispjson,"base")] = '\0';
					print_str(buffer);
				}
				else if(strcmp(datajson->valuestring,"cmd") == 0)
				{
					getShell(buffer,getJsonStr(dispjson,"data"),getJsonNumInt(dispjson,"base")+1);
					setTextSize(getJsonNumInt(dispjson,"size"));
					setTextColor(getJsonNumInt(dispjson,"color"));
					setCursor(getJsonNumInt(dispjson,"x0"),getJsonNumInt(dispjson,"y0"));
					print_str(buffer);
				}
				else
				{
					printf("ERROR display: %s is not supported\n",datajson->valuestring);
				}
			}
			dispjson = dispjson->next;
		}
	}
}

void pageStart(pageData *pagedata)
{
//	int pagecnt;
//	cJSON *datajson, *startjson;
//	char cmdbuf[200];
//	
//	printf("---------------------------------------------\n");///////////////////////////
//	//对应page节点
//
//	for(pagecnt = 0; pagecnt < SetingData.pagenum; pagecnt ++)
//	{
//		pagedata = getPageNode( PageData, pagecnt, DATANUM);
//		if(pagedata == NULL)break;
//		//start
//		startjson = pagedata->start;
//		while(startjson != NULL && cJSON_GetArraySize(startjson) > 0)
//		{
//			datajson = cJSON_GetObjectItem(startjson,"type");
//			if(datajson->type == cJSON_String)
//			{
//				if(strcmp(datajson->valuestring,"cmd") == 0)
//				{
//					datajson = cJSON_GetObjectItem(startjson,"data");
//					getShell(cmdbuf,datajson->valuestring);
//					printf("temp:%s",cmdbuf);
//				}
//				else
//				{
//					printf("ERROR start: %s is not supported\n",datajson->valuestring);
//				}
//			}
//			startjson = startjson->next;
//		}
//	}
}

void pageStop(pageData *pagedata)
{
//	cJSON *datajson, *stopjson;
//	char cmdbuf[200];
//	long cmdnum;
//
//	//stop
//	if(pagedata->stop)
//	{
//		stopjson = pagedata->stop;
//		while(stopjson != NULL && cJSON_GetArraySize(stopjson) > 0)
//		{
//			datajson = cJSON_GetObjectItem(stopjson,"type");
//			if(datajson->type == cJSON_String)
//			{
//				if(strcmp(datajson->valuestring,"cmd") == 0)
//				{
//					datajson = cJSON_GetObjectItem(stopjson,"data");
//					getShell(cmdbuf,datajson->valuestring);
//					cmdnum = atol(cmdbuf);
//				}
//				else
//				{
//					printf("ERROR stop: %s is not supported\n",datajson->valuestring);
//				}
//			}
//		}
//	}
}


