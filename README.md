## 说明
1. 本说明基于正点原子Apollo开发板，MCU为STM32H743IIT6;
2. 由于本开发板的外置`Flash`类型为`Nand Flash`，它不支持通过内存映射的方式访问(即直接通过`0~4G`的地址进行访问)，所以这里是通过将`GUI`的一些图片资源存储在`Nand Flash`里面(通过`Flash`下载算法实现数据的下载功能)；然后程序初始化时，将`Nand Flash`里的数据读取到外部`SDRAM`里面，然后`TouchGFX`从`SDRAM`中按需获取所需要的数据(需要通过链接文件指定数据的地址)。

## 时钟配置
1. 外部高速晶振HSE为25MHz，外部低速晶振LSE为32.768KHz;
2. 要配置到最高主频480MHz，需要在cubeMx中的RCC配置页面将`Product revision`改为`rev V`;
3. 主时钟配置参考：25 / 5 * 192 / 2 = 480MHz；
4. FMC时钟频率配置为260MHz；
5. LTDC时钟配置为50MHz；


## LED
1. `LED0`控制管脚为`PB1`，低电平亮（硬件上无上下拉）；

## 外部 SDRAM W9825G6KH-6 (4Mwords * 4Banks * 16bit = 32MBytes)
1. 支持的最大速度：166MHz/CL3 or 133MHz/CL2; (CL=CAS Latency)
2. 通过 SDClockPeriod 对 FMC 时钟进行分频，最终决定 SDRAM 的主频；
3. 实测设置 FMC=260MHz, SDClockPeriod=FMC_SDRAM_CLOCK_PERIOD_2, CL=3，RPD>=1 时，SDRAM 读写测试通过；（推荐）
4. 实测设置 FMC=260MHz, SDClockPeriod=FMC_SDRAM_CLOCK_PERIOD_2, CL=2 时，SDRAM 读写测试失败；
5. 实测设置 FMC=300MHz, SDClockPeriod=FMC_SDRAM_CLOCK_PERIOD_2, CL=3 时，SDRAM 读写测试失败；
6. 地址线(13根)：A0-A12 （A0-A12 为行地址线，A0-A8为列地址线）
7. 时序参数计算：基于 SDRAM_CLK=260MHz/2=130MHz 计算可得，每个 tck = 7.69ns

|cubeMx参数名|datasheet 缩写|W9825G6KH-6 参数举例|说明|
|---|---|:---:|---|
|Load mode register to active delay| T_RSC or T_MRD | 2 tck | 2 tck |
|Exit self-refresh delay| T_XSR | > 72ns | 72ns / 7.67ns = 10 tck |
|Self-refresh time| T_RAS | > 42ns | 42ns / 7.67ns = 6 tck |
|SDRAM common row cycle delay| T_RC | > 60ns | 60ns / 7.67ns = 8 tck |
|Write recovery time| T_WR | 2 tck | 2 tck（全部配置完后cubeMx会报错，至少为4）|
|SDRAM common row precharge delay| T_RCD or T_RP | > 15ns | 15ns / 7.67ns = 2 tck |
|Row to column delay| T_RC or T_RCD | > 15ns | 15ns / 7.67ns = 2 tck |
|||||
8. 不需要开启`FMC`中断；
9. `FMC`初始化后，需要对`SDRAM`进行`初始化`。由于`SDRAM`是`动态RAM`，需要`定时刷新`，所以初始化时有一个重要的`刷新率`需要设置。根据`datasheet`的描述`8K Refresh Cycles/64 mS`可知，每`64ms`需要刷新`8192(8K)`个`cycles`，即每个刷新间隔时间为`64ms / 8192 = 7.8us`，而SDRAM的时钟是130MHz，所以至少每经过`7.8us / (1/130MHz) = 1014`个SDRAM时钟，就需要对SDRAM中的数据进行刷新一次，而为了保证数据的可靠性，我们一般会设置一个比这个值稍微小一点的刷新率，一般小`20`就可以了。
10. 一个有`4`个`Bank`：`Bank1`地址为`0xC0000000`，`Bank2`地址为`0xC0800000`，`Bank3=0xC1000000`，`Bank4=0xC1800000`；
11. 如果使能了`MCU`的`MPU`功能，则相应的区域也需要配置其`MPU`功能。

## 外部 Nand Flash H27U4G8F2ETR-BI (4Gbit 8位) 
> * 原理图上写的是 MT29F4G08，实际上贴的是 H27U4G8F2ETR-BI；
> * 使用这个`Nand Flash`需要设置相应的`MPU`区域，否则会读写失败！
1. 这个芯片没有 CLK 引脚，无需考虑最大的 FMC 时钟问题；只需考虑`cubeMx`上的配置相关时序参数即可；
2. 容量：（2048+64）Bytes * 64 pages * 4096 blocks = 512MBytes
3. `FMC_NWAIT(PD6)`管脚需要上拉；
3. 相关时序参数设置（计算方法参考`AN4570(page 34)`，通过`DS12110`可知`T_su(D-NOE) > 8ns`）（`datasheet`显示`HCLK`是最高主频的一半）：

|cubeMx名称|datasheet缩写|参数值|
|---|---|---|
|CLE low to RE low dalay in HCLK cycles| T_CLR | 10ns/(1/240MHz) = 3HCLK |
|ALE low to RE low dalay in HCLK cycles| T_AR | 10ns/(1/240MHz) = 3HCLK |
|setup time| (SET+1)*T_HCLK > max(T_CS, T_CLS, T_ALS, T_CLR, T_AR) - T_WP | [max(20,12,12,10,10)-12]/(1/240MHz)-1 = 1HCLK|
|wait time| (WAIT+1)*T_HCLK > MAX(T_WP, T_RP) and (WAIT+1)*T_HCLK > T_REA + T_su(D-NOE) | max(12, 12)/(1/240MHz)-1 = 2HCLK and (20+8)/(1/240MHz)-1 = 6HCLK |
|hold time| (HOLD+1)*T_HCLK > MAX(T_CH, T_CLH, T_ALH) | max(5,5,5)/(1/240MHz)-1 = 1HCLK |
|Hi-Z time| (HIZ+1)*T_HCLK > MAX(T_CS, T_ALS, T_CLS) + (T_WP - T_DS) | (max(20, 12, 12) + (12-12))/(1/240MHz)-1 = 4HCLK |



## 触摸屏配置 GT911
> GT911 的参数不能随便写，每个屏的参数都是不一样的！只有特定几个寄存器可以自由配置。
1. 可以通过在LCD显示屏上切换排阻选择不同的触摸屏驱动芯片，现有的驱动芯片的 IIC 接口的 GT911；
2.  以下管脚在硬件上都没有上下拉电阻：

|原理图信号名|驱动芯片管脚|MCU管脚编号|说明|
|---|---|---|---|
|TP_CS|CT_RST|PI8| 需软件上拉，拉低复位GT911 |
|TP_SCK|CT_SCL|PH6| PH6不支持硬件IIC功能，需模拟IIC实现，需MCU上拉|
|TP_PEN|CT_INT|PH7|PH7配置成上升沿中断触发，可配置MCU下拉|
|TP_MOSI|CT_SDA|PI3|PI3不支持硬件IIC功能，需模拟IIC实现，需MCU上拉|
|TP_MISO|NC|PG3| / |
||||

## 显示屏 LCD
1. 背光控制`LCD_BL: PB5`，无外部上下拉；
2. MCU和LCD显示屏的连接方式只支持RGB565格式；
3. 正点原子的`7寸1024*600`的显示屏参数如下：

|cubeMx名称|缩写|参数值|
|---|---|---|
|Horizontal Synchronization Width | HSync | 20 pixels |
|Horizontal Back Porch | HBP  | 140 pixels|
|Active Width | / | 1024 pixels|
|Horizontal Front Porch | HFP | 160 pixels |
|Total Width|/|20+140+1024+160=1344 pixels|
|Vertical Synchronization Height | VSync | 3 lines |
|Vertical Back Porch | VBP  | 20 lines |
|Active Height | / | 600 lines|
|Vertical Front Porch | VFP | 12 lines |
|Total Height| / | 3+20+600+12=635 lines |
|||

4. LTDC模块的时钟：假设需要以`60Hz`的屏幕刷新率显示`RGB565`的图像，则`LTDC时钟`应该为`1344*635*60=51.2MHz`；如果将`RGB565(2Bytes)`改为`ARGB888(4Bytes)`，则时钟频率需要翻倍；
5. `LCD`的所有管脚速度必须配置为`Very High`；
6. 需要开启`LTDC global interrupt`，但是优先级要低一些，以免影响其他任务的实时性；

## 开启 TouchGFX 组件
1. 