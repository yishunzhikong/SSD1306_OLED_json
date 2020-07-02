
#include "dataapi.h"

//#define DATA_DEBUG

//系统设置初始化
setingData SetingData={DefaultPixel,DefaultI2cAddr,DefaultI2cDev,DefaultConfig,0,0};
//显示页面结构体链表
pageData *PageData;
//循环页面结构体
pageData *LoopPageData;
//运行的页面数据
pageData *RunPageData;
//原始json数据cJSON结构体
cJSON *ConfigJson;

/****************************************************************
* 函数名: readJsonFile
* 功能	: 读取json文件并返回cJSON结构体
* 返回	: 读取到json数据建立的cJSON结构体的指针
* 参数	: @filename: json文件路径
* 注意	: 函数会调用cJSON_Parse()函数，请自行使用cJSON_Delete()释放空间
****************************************************************/
cJSON *readJsonFile(char *filename)
{
	FILE *fp;
	long len;
	char *data;
	cJSON *json;
	
	fp=fopen(filename,"rb");
	if(fp == NULL)
	{
		printf("ERROR config:open %s error!\n",DefaultConfig);
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	len=ftell(fp);
	fseek(fp,0,SEEK_SET);
	data=(char*)malloc(len+1);
	fread(data,1,len,fp);
	data[len]='\0';
	json=cJSON_Parse(data);
	if (!json) 
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		cJSON_Delete(json);
		exit(1);
	}
	free(data);
	fclose(fp);
	return json;
}

/****************************************************************
* 函数名: writeJson
* 功能	: 写入json数据到文件
* 返回	: NONE
* 参数	: @filename: 待写入json文件路径
		  @out： 待写入数据
* 注意	: out是已格式化json数据，非cJSON结构体
****************************************************************/
void writeJson(char *filename,char *out)
{
	FILE *fp = NULL;
 
	fp = fopen(filename,"a+");
	if(fp == NULL)
	{
		fprintf(stderr,"open file failed\n");
		exit(-1);
	}
	fprintf(fp,"%s",out);
 
	if(fp != NULL)
		fclose(fp);
}

/****************************************************************
* 函数名: printJson
* 功能	: 打印cJSON结构体所有元素
* 返回	: NONE
* 参数	: @root: 待打印cJSON结构体
* 注意	: NONE
****************************************************************/
void printJson(cJSON * root)//以递归的方式打印json的最内层键值对
{
    for(int i=0; i<cJSON_GetArraySize(root); i++)   //遍历最外层json键值对
    {
        cJSON * item = cJSON_GetArrayItem(root, i);        
        if(item->type == cJSON_Object|| item->type == cJSON_Array)      //如果对应键的值仍为cJSON_Object就递归调用printJson
            printJson(item);
        else                                //值不为json对象就直接打印出键和值
        {
            printf("%s	%s\n", item->string, cJSON_Print(item));
        }
    }
}

/****************************************************************
* 函数名: printUsage
* 功能	: 打印帮助信息
* 返回	: NONE
* 参数	: @stream: 链表节点数
		  @exit_code: 返回代码
		  @program_name: 本程序名
* 注意	: NONE
****************************************************************/
void printUsage(FILE* stream, int exit_code,char *program_name)
{
	fprintf(stream, "Usage: %s options [...]\n", program_name);
    fprintf(stream, 
        "-h --help Display this usage information\n"
        "-c [path] Load configuration\n"
        "-v \n");
    exit(exit_code);
}

/****************************************************************
* 函数名: getSting
* 功能	: 从原始cJSON结构体获取系统设置信息并删除seting节点。
* 返回	: NONE
* 参数	: @conf: 原始json数据cJSON结构体
* 注意	: 本函数针对setingData结构数据，其他系统设置结构另行编写
		  本函数会调用malloc()函数，请执行使用free()释放
****************************************************************/
void getOledSting(cJSON *configjson)
{
	int len;
	cJSON *temp, *setjson;
	//读取全局设置
	if(cJSON_HasObjectItem(configjson, "seting"))
	{
		setjson = cJSON_GetObjectItem(configjson, "seting");
		//读取OLED分辨率设置
		temp = cJSON_GetObjectItem(setjson, "pixel");
		if(temp != NULL && temp->type == cJSON_Number)
		{
			SetingData.pixel = temp->valueint;
			switch(SetingData.pixel)
			{
				case OLED12864:
				break;
				case OLED12832:
				break;
				case OLED9616:
				break;
				default:
				printf("Set OLED pixel error\r\n");
				printf("Use default pixel:12864\n");
				break;
			};
#ifdef DATA_DEBUG
			printf("pixel:%d\n",SetingData.pixel);
#endif
		}
		else
		{
			printf("Set OLED pixel error\r\n");
			printf("Use default pixel:12864\n");
		}
		//读取i2c驱动设置
		temp = cJSON_GetObjectItem(setjson, "dev");
		if(temp != NULL && temp->type == cJSON_String)
		{
			len = strlen(temp->valuestring);
			SetingData.i2cdev = (char*)malloc(len + 1);
			strcpy(SetingData.i2cdev,temp->valuestring);
			SetingData.i2cdev[len]='\0';
#ifdef DATA_DEBUG
			printf("i2cdev:%s\n",SetingData.i2cdev);
#endif
		}
		else
		{
			printf("Set i2c device error\n");
			printf("Use default device:/dev/i2c-0\n");
		}
		//读取OLED地址设置
		temp = cJSON_GetObjectItem(setjson, "addr");
		if(temp != NULL && temp->type == cJSON_Number)
		{
			SetingData.oledaddr = temp->valueint;
#ifdef DATA_DEBUG
			printf("addr:%d\n",SetingData.oledaddr);
#endif
		}
		else
		{
			printf("Set OLED addr error\n");
			printf("Use default addr:0x3c\n");
		}
		//删除设置节项目
		cJSON_DeleteItemFromObject(ConfigJson,"seting");
	}
	//读取剩余显示页面数量
	SetingData.pagenum = cJSON_GetArraySize(ConfigJson);
	//创建显示页面数据结构体链表
	PageData = createPageList(SetingData.pagenum);
	//初始化运行数据
#ifdef DATA_DEBUG
		printf("page data len:%d\n",SetingData.pagenum);
#endif
}

/****************************************************************
* 函数名: getPageData
* 功能	: 从原始cJSON结构体获取OLED页面显示信息
* 返回	: NONE
* 参数	: @confjson: 原始json数据cJSON结构体
* 注意	: NONE
****************************************************************/
void getPageData(cJSON *confjson)
{
	int pagecnt;
	cJSON *pagejson, *itemjson, *datajson, *list;
	pageData *pagedata = PageData;

	for(pagecnt = 0; pagecnt < SetingData.pagenum; pagecnt ++)
	{
		//对应page节点
//		if(pagecnt == 0)
//		{
//			pagedata = PageData;
//		}
//		else if((pagedata->num + 1) == pagecnt)
//		{
//			pagedata = pagedata->next;
//		}
//		else exit(1);
		pagedata = getPageNode( PageData, pagecnt, DATANUM);
		if(pagedata == NULL)break;
		
		//设置当前页cJSON指针
		pagejson = cJSON_GetArrayItem(confjson, pagecnt);
		
		//读取页面设置
		if(cJSON_HasObjectItem(pagejson, "seting"))
		{
			itemjson = cJSON_GetObjectItem(pagejson, "seting");
			if(itemjson != NULL && itemjson->type == cJSON_Object)
			{
				datajson = cJSON_GetObjectItem(itemjson, "cycle");
				if(datajson != NULL && datajson->type == cJSON_Number)
				{
					pagedata->cycle = datajson->valueint;
#ifdef DATA_DEBUG
					printf("%s:%d,",datajson->string,pagedata->cycle);
#endif
				}
				else pagedata->cycle = 1;
				
				datajson = cJSON_GetObjectItem(itemjson, "time");
				if(datajson != NULL && datajson->type == cJSON_Number)
				{
					pagedata->time = datajson->valueint;
#ifdef DATA_DEBUG
					printf("%s:%d,",datajson->string,pagedata->time);
#endif
				}
				else pagedata->time = 1;
				
				datajson = cJSON_GetObjectItem(itemjson, "page");
				if(datajson != NULL && datajson->type == cJSON_Number)
				{
					pagedata->page = datajson->valueint;
#ifdef DATA_DEBUG
					printf("%s:%d\n",datajson->string,pagedata->page);
#endif
				}
				else pagedata->page = 0;
				
				datajson = cJSON_GetObjectItem(itemjson, "scroll");
				if(datajson != NULL && datajson->type == cJSON_Number)
				{
					pagedata->scroll = datajson->valueint;
#ifdef DATA_DEBUG
					printf("%s:%d\n",datajson->string,pagedata->page);
#endif
				}
				else pagedata->scroll = 0;
				
				datajson = cJSON_GetObjectItem(itemjson, "scrollstart");
				if(datajson != NULL && datajson->type == cJSON_Number)
				{
					pagedata->scrollstart = datajson->valueint;
#ifdef DATA_DEBUG
					printf("%s:%d\n",datajson->string,pagedata->page);
#endif
				}
				else pagedata->scrollstart = 0;
				
				datajson = cJSON_GetObjectItem(itemjson, "scrollstop");
				if(datajson != NULL && datajson->type == cJSON_Number)
				{
					pagedata->scrollstop = datajson->valueint;
#ifdef DATA_DEBUG
					printf("%s:%d\n",datajson->string,pagedata->page);
#endif
				}
				else pagedata->scrollstop = 0;
			}
			cJSON_DeleteItemFromObject(pagejson,"seting");//删除设置节项目
		}
		else
		{
			printf("ERROR %s:seting not find!\n",pagejson->string);
			exit(1);
		}
		
		//读取开始设置
		if(cJSON_HasObjectItem(pagejson,"start"))
		{
			itemjson = cJSON_GetObjectItem(pagejson, "start");
			if(itemjson != NULL && itemjson->type == cJSON_Array && cJSON_GetArraySize(itemjson) > 0)
			{
				pagedata->start = cJSON_GetArrayItem(itemjson, 0);
#ifdef DATA_DEBUG
					printf("Read start data OK\n");
#endif
			}
		}
		else pagedata->start = NULL;
		
			//读取结束设置
		if(cJSON_HasObjectItem(pagejson,"stop"))
		{
			itemjson = cJSON_GetObjectItem(pagejson, "stop");
			if(itemjson != NULL && itemjson->type == cJSON_Array && cJSON_GetArraySize(itemjson) > 0)
			{
				pagedata->stop = cJSON_GetArrayItem(itemjson, 0);
#ifdef DATA_DEBUG
					printf("Read stop data OK\n");
#endif
			}
		}
		else pagedata->stop = NULL;
		
		//读取显示内容
		if(cJSON_HasObjectItem(pagejson,"display"))
		{
			itemjson = cJSON_GetObjectItem(pagejson, "display");
			if(itemjson != NULL && itemjson->type == cJSON_Array && cJSON_GetArraySize(itemjson) > 0)
			{
				pagedata->display = cJSON_GetArrayItem(itemjson, 0);
#ifdef DATA_DEBUG
					printf("Read display data OK\n");
#endif
			}
		}
		else pagedata->display = NULL;
		//处理循环显示页面信息
		getLoopPageData();
		//遍历自建结构体
//		list = pagedata->start;
//		if(list != NULL)
//		{
//			if(cJSON_HasObjectItem(list,"type"))
//			{
//				printf("---------------------\n%s:\n",pagejson->string);
//				cJSON *test = cJSON_GetObjectItem(list,"type");
//				printf("	%s\n",test->valuestring);
//			}
//			list = list->next;
//		}
	}
}

/****************************************************************
* 函数名: createPageList
* 功能	: 创建显示页面链表
* 返回	: 显示页面链表表头
* 参数	: @number: 链表节点数
* 注意	: 函数会调用malloc()函数，请自行使用free()释放空间
****************************************************************/
void getLoopPageData(void)
{
	int pagecnt = 1;
	pageData *loopdata = PageData;
	pageData *new=NULL, *save=NULL, *head=NULL;
	while(loopdata)
	{
		loopdata = getPageNode(PageData, pagecnt, DATAPAGE);
		if(loopdata != NULL)
		{
			new = (pageData*)malloc(sizeof(pageData));
			memcpy(new,loopdata,sizeof(pageData));
			//第一个节点，确定头的位置
			if(loopdata->page == 1)
			{
				new->next = new->prev = new;
				save = new;
				head = save;
			}
			//后面的节点
			else
			{
				new->next = head;
				new->prev = save;
				save->next = new;
				save = new;
			}
			new->num = new->page;
			pagecnt ++;
			SetingData.loopnum = pagecnt;
			
			//更新头节点信息
			head->prev = new;
			LoopPageData = head;
			RunPageData = LoopPageData;
		}
		else
		{
			break;
		}
	}
	
//	//遍历
//	loopdata = LoopPageData;
//	for(pagecnt = 1; pagecnt < SetingData.loopnum; pagecnt ++)
//	{
//		printf("num:%d,page:%d,string:%d\n",loopdata->num,loopdata->page,loopdata->time);
//		loopdata = loopdata->next;
//		printf("1\n");
//	}
}

/****************************************************************
* 函数名: createPageList
* 功能	: 创建显示页面链表
* 返回	: 显示页面链表表头
* 参数	: @number: 链表节点数
* 注意	: 函数会调用malloc()函数，请自行使用free()释放空间
****************************************************************/
pageData *createPageList(int number)
{
	int cnt,i;
	pageData *new=NULL, *save=NULL, *head=NULL;
	//建立N个节点
	for(cnt = 0;cnt < number; cnt ++)
	{
		new = (pageData*)malloc(sizeof(pageData));
		//第一个节点，确定头的位置
		if(cnt == 0)
		{
//			new = (pageData*)malloc(sizeof(pageData));
	
			new->next = new->prev = new;
			new->num = 0;
			save = new;
			head = save;
		}
		//后面的节点
		else
		{
//			new  = (pageData*)malloc(sizeof(pageData));
			new->num = cnt;
			new->next = head;
			new->prev = save;
			save->next = new;
			save = new;
		}
		
		//更新头节点信息
		head->prev = new;
		
		//初始化
//		head->cycle = 0;
//		head->time = 0;;
//		head->page = 0;;
//		head->num = 0;;
//		head->start = NULL;
//		head->stop = NULL;
//		head->display = NULL;
	}
	return head;
}

void getShell(char *buf,char *cmd,int len)
{
    FILE *fp;
	if(len > BUFFSIZE - 1) len = BUFFSIZE - 1;
	memset(buf,'\0',len);
    fp = popen(cmd, "r");
	fgets(buf, len, fp);
    pclose(fp);
	buf[len] = '\0';
}

pageData *getPageNode(pageData *head, int num, orderType type)
{
	pageData *pagedata = head;
	if(type == DATANUM)
	{
		while(pagedata != NULL && pagedata->num != num)
		{
			pagedata = pagedata->next;
			if(pagedata == head)
			{
				return NULL;
			}
#ifdef DATA_DEBUG
			printf("Find pagedata node\n");
#endif
		}
	}
	else 
	if(type == DATAPAGE)
	{
		while(pagedata != NULL && pagedata->page != num)
		{
			pagedata = pagedata->next;
			if(pagedata == head)
			{
				return NULL;
			}
#ifdef DATA_DEBUG
			printf("Find pagedata node\n");
#endif
		}
	}
	return pagedata;
}

void getJsonData(char *filename)
{
	//读取配置文件
	ConfigJson = readJsonFile(filename);
	if(NULL == ConfigJson)
	{
		printf("Error before:\n%s\n", cJSON_GetErrorPtr());
		exit(1);
	}
	//系统设置
	getOledSting(ConfigJson);
	//处理显示页面信息
	getPageData(ConfigJson);
}

void oledInit(void)
{
    /* 初始化I2C总线并连接到I2C设备 */
    if(init_i2c_dev(SetingData.i2cdev, SetingData.oledaddr) != 0)
    {
		printf("(Main)i2c-2: OOPS! Something Went Wrong\r\n");
        exit(1);
    }
	
    /* 运行SDD1306初始化序列 */
	//该函数使用了calloc()分配显存空间，请勿多次引用！！！！！！！！！！！
    display_Init_seq();
}

int getJsonNumInt(cJSON *json, char *str)
{
	int pagecnt;
	if(json != NULL && cJSON_HasObjectItem(json,str))
	{
		json = cJSON_GetObjectItem(json,str);
		if(json->type == cJSON_Number)
		{
			return json->valueint;
		}
	}
	return 0;
}

double getJsonNumDouble(cJSON *json, char *str)
{
	int pagecnt;
	if(json != NULL && cJSON_HasObjectItem(json,str))
	{
		json = cJSON_GetObjectItem(json,str);
		if(json->type == cJSON_Number)
		{
			return json->valuedouble;
		}
	}
	return 0;
}
char *getJsonStr(cJSON *json, char *str)
{
	int pagecnt;
	if(json != NULL && cJSON_HasObjectItem(json,str))
	{
		json = cJSON_GetObjectItem(json,str);
		if(json->type == cJSON_String)
		{
			return json->valuestring;
		}
	}
	return NULL;
}

/****************************************************************
* 函数名: readFile
* 功能	: 读取文件
* 返回	: 读取状态：1：读取成功；0：读取失败
* 参数	: @buffer: 存放读取结果的缓存
		  @filename：需要读取的文件路径
* 注意	: NONE
****************************************************************/
int readFile(char *buffer,char *filename,char len)
{
	FILE *fp;
//	long len;
	char *data;
	
	fp=fopen(filename,"r");
	if(fp == NULL)
	{
		printf("ERROR file:open %s error!\n",filename);
		return 0;
	}
//	fseek(fp,0,SEEK_END);
//	len=ftell(fp);
	fseek(fp,0,SEEK_SET);
	if(len > BUFFSIZE) len = BUFFSIZE;
	fread(buffer,1,len,fp);
	buffer[len]='\0';
	fclose(fp);
	return 1;
}




