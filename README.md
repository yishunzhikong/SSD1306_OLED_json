# 目录
编辑中...

---
# 简介
嵌入式linux驱动SSD1306芯片的OLED，采用json文件配置显示内容

---
# 配置文件
配置文件采用json格式，使用cJSON解析，语法遵循json语法

## 开始运行
**方法1：**
将config.json和二进制ssd放入同一目录，终端进入该目录执行
```
./ssd
```
**方法2**：
将config.json放入`/xxx`目录，终端进入`ssd`所在目录执行
```
./ssd -c /xxx/config.json
```

## json简介
* JSON(JavaScript Object Notation, JS 对象简谱) 是一种轻量级的数据交换格式
* 其主要元素为`健`、`值`、`符号`
* 基本语法规则:
1. 数据在`健/值`对中
2. 数据由`逗号`分隔
3. `大括号`保存`对象`
4. `中括号`保存`数组`

## 整体说明
本程序使用分页的方式显示各种内容，json表现为多个页面数据在同一级。
配置文件整体结构如下：
```
{
	"seting":{
	},
	"test1":{
	},
	"test2":{
	}
}
```
键`"seting"`设置软硬件参数，其值为`对象`
键`"text1"`、`"text2"`设置页面显示内容，值为`对象`,名称可以自定义。

## 软硬件设置
```
{
	"seting":{
		"pixel":12832,
		"dev":"/dev/i2c-0",
		"addr":60
	}
}
```
`pixel`:分辨率，数字，可选`12864`、`12832`，默认`12864`

---

# 引用
---
 * json解析库cJSON:[DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)
 * SSD1306驱动库:[deeplyembeddedWP/SSD1306-OLED-display-driver-for-BeagleBone](https://github.com/deeplyembeddedWP/SSD1306-OLED-display-driver-for-BeagleBone)
