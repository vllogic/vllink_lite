# Vllink Lite

## 简介

Vllink Lite是一款低成本高性能调试器。硬件基于GD32F350制作，最小封装为QFN28，标准版本使用GD32F350G8U6（8K RAM / 64K ROM），亦可在精简缓冲后换用GD32F350G6U6（6K RAM / 32K ROM）。

## 硬件

当前最新版本为：[Vllink Lite.R3](https://github.com/vllogic/vllink_lite/tree/000b3bc6477d7fd816e0debf9087d155adbe143d/hardware/vllink_lite.r3)，板上集成8MB SPI Flash。
![3D](./hardware/vllink_lite.r3/vllink_lite.r3.top_rotate.png)

![PCBA](./hardware/vllink_lite.r3/vllink_lite.r3.pcba.png)

## 功能

* 支持固件更新，按住按键连接Win10电脑，再使用Chrome浏览器打开更新页面即可 [WebDFU](https://devanlai.github.io/webdfu/dfu-util/)
* 提供一路CMSIS-DAP V2协议免驱（仅限Win10）接口，提供SWD及JTAG接口，已支持IAR for ARM（版本8.32.1及以上）、MDK-ARM（版本5.29及以上）、[PyOCD](https://github.com/mbedmicro/pyOCD)、[OpenOCD](https://github.com/vllogic/openocd_cmsis-dap_v2)。具体配置方式可参看[IDE使用教程](https://github.com/vllogic/vllink_lite/blob/master/doc/ide_guide.md)
* 提供一路USB CDC接口

## 特点

* 低成本，软硬件全部开源
* 优化了底层传输协议，尽量使用SPI通讯，IAR默认速率下对SRAM的读写速度可达270KB/S(SWD)或200KB/S(JTAG)，相比DAPLink，大约提升一倍。如果使用优化版的OpenOCD，读写速度可达400KB/s以上。[与主流调试器对比](https://github.com/vllogic/vllink_lite/blob/master/hardware/vllink_lite.r3/speed_test.md)

## 硬件制作

[原理图及PCBA制作资料](https://github.com/vllogic/vllink_lite/tree/master/hardware)

[固件](https://github.com/vllogic/vllink_lite/releases)

## 开发平台

* KiCAD
* IAR for ARM 8.32.3
* [GD32F3x0 AddOn](http://gd32mcu.21ic.com/documents)

## 授权

GPLv3，随便玩

## 交流

欢迎加入QQ群：512256420

## 购买渠道
* [Taobao](https://shop216739170.taobao.com/)

## TODO List

### 近期目标

1. 编写通过WebUSB对Vllink Lite进行固件更新的网页
2. 编写 ID_DAP_Vendor1 - ID_DAP_Vendor5 串口命令，支持多路串口同时收发

### 远期目标

1. 利用板上SPI Flash，研究自动化较高的离线编程器方案
2. SWO功能测试

