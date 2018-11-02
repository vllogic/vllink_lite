# Vllink Lite
## 简介
Vllink Lite是一款超低成本高性能调试器。硬件基于GD32F350制作，最小封装为QFN28，最小RAM/ROM占用仅为6/32KB，无需外部晶振。

## 成本
最小方案的Vllink Lite硬件设计中，除了阻容及接口器件外，仅需一颗LDO和一颗GD32F350G6U6，此芯片在淘宝GD32旗舰店有售，当前零售价为5.75￥。

## 功能
* 提供一路CMSIS-DAP协议免驱接口，支持SWD，JTAG，SWO接口，支持主流IDE
* 提供一路Vlink协议接口，支持SWD，JTAG接口，目前需配合OpenOCD使用
* 提供一路USB CDC接口，最高波特率可达8M

## 性能
截止20181102，已测试并完善SWO及CDC接口，JTAG部分稳定性欠佳。以下测试数据是在OpenOCD（Vllink协议）<==> Vllink Lite <==> STM32F411（SRAM区域）环境下获得的。

| 时钟频率 | 读速度（KiB/s） | 写速度（KiB/s） |
| --------| -----:  | -----:  |
| 2000    | 128.672 | 131.903 |
| 4000    | 213.449 | 223.565 |
| 8000    | 323.268 | 347.579 |
| 16000   | 402.034 | 425.731 |
| 32000   | 438.233 | 456.286 |

测试记录：
```
Open On-Chip Debugger
> adapter_khz 2000
adapter speed: 2000 kHz
> dump_image 128kB_ram.bin 0x20000000 0x20000
dumped 131072 bytes in 0.994774s (128.672 KiB/s)
> load_image 128kB_ram.bin 0x20000000
131072 bytes written at address 0x20000000
downloaded 131072 bytes in 0.970408s (131.903 KiB/s)
> adapter_khz 4000
adapter speed: 4000 kHz
> dump_image 128kB_ram.bin 0x20000000 0x20000
dumped 131072 bytes in 0.599675s (213.449 KiB/s)
> load_image 128kB_ram.bin 0x20000000
131072 bytes written at address 0x20000000
downloaded 131072 bytes in 0.572540s (223.565 KiB/s)
> adapter_khz 8000
adapter speed: 8000 kHz
> dump_image 128kB_ram.bin 0x20000000 0x20000
dumped 131072 bytes in 0.395956s (323.268 KiB/s)
> load_image 128kB_ram.bin 0x20000000
131072 bytes written at address 0x20000000
downloaded 131072 bytes in 0.368262s (347.579 KiB/s)
> adapter_khz 16000
adapter speed: 16000 kHz
> dump_image 128kB_ram.bin 0x20000000 0x20000
dumped 131072 bytes in 0.318381s (402.034 KiB/s)
> load_image 128kB_ram.bin 0x20000000
131072 bytes written at address 0x20000000
downloaded 131072 bytes in 0.300659s (425.731 KiB/s)
> adapter_khz 32000
adapter speed: 32000 kHz
> dump_image 128kB_ram.bin 0x20000000 0x20000
dumped 131072 bytes in 0.292082s (438.233 KiB/s)
> load_image 128kB_ram.bin 0x20000000
131072 bytes written at address 0x20000000
downloaded 131072 bytes in 0.280526s (456.286 KiB/s)
```

## 制作
`./hardware/`路径下有相关演示板的原理图及PCBA制作资料。
`./firmware/`路径下有相关演示板的运行固件。

## Openocd
Vllink驱动遵循OpenOCD GPL协议，可编译运行：[OpenOCD-Source](https://github.com/vllogic/openocd-vllink)。

或直接下载预编译的可执行文件：[OpenOCD-Releases](https://github.com/vllogic/openocd-vllink/releases)。

## 授权
Vllink Lite的硬件设计图纸及固件可自由使用及分发，也可以在保证固件完整性的情况下进行商业使用及销售。

## 交流
欢迎加入QQ群：512256420
