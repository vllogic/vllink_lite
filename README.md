# Vllink Lite

## 简介

Vllink Lite是一款低成本高性能调试器。硬件基于GD32F350/GD32E103制作，此两款MCU的所有规格型号都可用于调试器主控，区别主要在于RAM、ROM的大小对于性能及功能的影响。推荐使用GD32E103TB，以获得最佳体验。

## 硬件

#### 目前提供三套原理图，分别是：

* [Vllink Lite.R4.GD32F350xx](https://github.com/vllogic/vllink_lite/tree/master/hardware/vllink_lite.r4.gd32f350xx) 此方案适合作为成本敏感的集成式调试器。
![3D](./hardware/vllink_lite.r4.gd32f350xx/vllink_lite.r4.gd32f350xx.top45.png)
![BOTTOM](./hardware/vllink_lite.r4.gd32f350xx/vllink_lite.r4.gd32f350xx.bottom.png)

* [Vllink Lite.R4.GD32E103Tx](https://github.com/vllogic/vllink_lite/tree/master/hardware/vllink_lite.r4.gd32e103tx) 相比GD32F350，提供更大RAM与ROM，可外扩SPI FLASH **[IO模拟方式]** ，用于离线编程。

* [Vllink Lite.R4.GD32E103Cx](https://github.com/vllogic/vllink_lite/tree/master/hardware/vllink_lite.r4.gd32e103Cx) 相比GD32E103Tx，换用LQFP封装，并支持硬件SPI连接SPI FLASH。

#### 硬件实拍图
![PCBA](./hardware/vllink_lite.r3/vllink_lite.r3.pcba.png)
![PCBA](./hardware/vllink_lite.r4.gd32e103tx/vllink_lite.r4.gd32e103tx.pcba.png)

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
* IAR for ARM 8.40.2 + [GD32F3x0/GD32E10x AddOn](http://www.gd32mcu.com/cn/download)
* GNU Arm Embedded Toolchain + Cmake + Ninja

## 授权

GPLv3，随便玩

## 交流

欢迎加入QQ群：512256420

## 购买渠道
* 5元打样 + 淘宝买MCU

## TODO List
1. 支持Swd/Jtag Host
2. 支持RAMIO，即通过调试口访问目标芯片的特定RAM，实现数据交互，类似RTT(J-Link)及Nuconsole(Nu-Link)
3. 支持CDCShell，用以访问第二路串口或者RAMIO

## 远景
1. WebUSB接口
2. 简易脚本及自动化配置功能，配合网页端工具，通过载入IAR或Keil的芯片描述工具，自动完成对应芯片的离线编程工具配置
