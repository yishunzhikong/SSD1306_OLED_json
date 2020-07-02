#ifndef DATAAPI_H_
#define DATAAPI_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "I2C.h"
#include "SSD1306_OLED.h"
#include "cJSON.h"

/* 默认设置 */
#define	DefaultPixel	12864					//像素
#define	DefaultI2cAddr	0x3c					//OLED地址
#define	DefaultI2cDev	"/dev/i2c-0"			//驱动文件地址
#define DefaultConfig	"/etc/oled/config.json"	//配置文件地址
#define TempPath		"/sys/class/thermal/thermal_zone0/temp"	//配置文件地址
#define FreqPath		"cat /sys/devices/system/cpu/cpu[04]/cpufreq/cpuinfo_cur_freq"
#define BUFFSIZE		200

/* 可用分辨率 */
typedef enum pixelType
{
	OLED12864=12864,OLED12832=12832,OLED9616=9616,OLEDNC=3
}pixelType;

/*  */
typedef enum orderType
{
	DATANUM,DATAPAGE
}orderType;

/* 页面数据结构 */
typedef struct pageData
{
	int cycle;
	int time;
	int page;
	int num;
	int scroll;
	int scrollstart;
	int scrollstop;
	cJSON *start;
	cJSON *stop;
	cJSON *display;
	struct pageData *prev;
	struct pageData *next;
}pageData;

/* 系统数据结构 */	
typedef struct sysSetData
{
	pixelType pixel;		//像素
	unsigned char oledaddr;	//OLED地址
	char *i2cdev;			//驱动文件地址
	char *config;			//配置文件地址
	int pagenum;			//页面数量
	int loopnum;			//页面数量
}setingData;

extern setingData SetingData;
extern pageData *PageData;
extern pageData *LoopPageData;
extern pageData *RunPageData;
extern cJSON *ConfigJson;

extern cJSON *readJsonFile(char *filename);
extern void writeJson(char *filename,char *out);
extern void printJson(cJSON * root);
extern void print_usage(FILE* stream, int exit_code,char *program_name);
extern void getOledSting(cJSON *conf);
extern void getPageData(cJSON *confjson);
extern void getLoopPageData(void);
extern pageData *createPageList(int num);
extern void getShell(char *buf,char *cmd,int len);
extern pageData *getPageNode(pageData *pagedata, int num, orderType type);
extern void getJsonData(char *filename);
extern void oledInit(void);
int getJsonNumInt(cJSON *json, char *str);
double getJsonNumDouble(cJSON *json, char *str);
char *getJsonStr(cJSON *json, char *str);
int readFile(char *buffer,char *filename,char len);
#endif