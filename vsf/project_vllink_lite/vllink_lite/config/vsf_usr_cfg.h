#ifndef __TOP_APP_CFG_H__
#define __TOP_APP_CFG_H__

#if 0   // Debug mode
//  Need set '--no_unaligned_access'
#   define __VSF_DEBUG__
#   define __OOC_DEBUG__
#   define ASSERT(...)                     if (!(__VA_ARGS__)) {while(1);};
#else
#   define __VSF_RELEASE__
//#   define __OOC_RELEASE__
#   define __OOC_DEBUG__
#   define ASSERT(...)
#endif

#ifdef __GNUC__
#	include "compile_definitions.h"
#endif

#define VSF_CFG_USER_HEADER             "../vsf_private/user_vsf.h"
#define VSF_OSA_DRIVER_HEADER           "./driver.h"        // fake include

// software independent components, if not used, compiler will optimize
#define VSF_USE_FIFO                                    ENABLED
#define VSF_USE_JSON                                    DISABLED

#define VSF_OS_CFG_PRIORITY_NUM                         2
#define VSF_OS_CFG_MAIN_MODE                            VSF_OS_CFG_MAIN_MODE_IDLE

#define VSF_KERNEL_CFG_SUPPORT_SYNC                     ENABLED
#   define VSF_KERNEL_CFG_SUPPORT_SYNC_IRQ              ENABLED
#   define VSF_KERNEL_CFG_SUPPORT_BITMAP_EVENT          ENABLED
#   define VSF_KERNEL_CFG_SUPPORT_MSG_QUEUE             ENABLED
#define VSF_KERNEL_CFG_EDA_SUPPORT_PT                   ENABLED
#define VSF_KERNEL_CFG_EDA_SUPPORT_TIMER                ENABLED
#   define VSF_KERNEL_CFG_CALLBACK_TIMER                ENABLED

#define VSF_KERNEL_CFG_ALLOW_KERNEL_BEING_PREEMPTED     ENABLED
#   define VSF_KERNEL_CFG_SUPPORT_DYNAMIC_PRIOTIRY      DISABLED
#   define VSF_OS_CFG_EVTQ_BITSIZE                      4

#define VSF_USE_KERNEL_SIMPLE_SHELL                     ENABLED
#define VSF_KERNEL_CFG_SUPPORT_THREAD                   DISABLED

#define VSF_USE_USB_DEVICE                              ENABLED
#   define VSF_USBD_CFG_USE_EDA                         ENABLED
#   define VSF_USBD_USE_CDC                             ENABLED
#   define VSF_USBD_USE_CDCACM                          ENABLED
#   define VSF_USBD_CFG_HW_PRIORITY                     vsf_arch_prio_2
#   define VSF_USBD_CFG_EDA_PRIORITY                    vsf_prio_1
#   define VSF_USBD_CFG_RAW_MODE                        DISABLED

#define VSF_USE_TRACE                                   DISABLED

#define VSF_USE_STREAM                                  DISABLED
#define VSF_USE_SIMPLE_STREAM                           ENABLED

#include "proj_cfg.h"
#include "brd_cfg.h"


/*
	DAP Config
*/
//#define DAP_PRODUCT                                     
//#define DAP_SER_NUM                                     
#define DAP_FW_VER                                      "0254" // Firmware Version
//#define DAP_DEVICE_VENDOR                               
//#define DAP_DEVICE_NAME                                 

#define DAP_SWD                                         1
#define DAP_JTAG                                        1
#define DAP_JTAG_DEV_CNT                                8
#define DAP_DEFAULT_PORT                                1
#define DAP_DEFAULT_SWJ_CLOCK                           4000000
#define DAP_CTRL_PACKET_SIZE                            64
#define DAP_BULK_PACKET_SIZE                            512
#define DAP_HID_PACKET_SIZE                             64
#if DAP_BULK_PACKET_SIZE > DAP_HID_PACKET_SIZE
#   define DAP_PACKET_SIZE                              DAP_BULK_PACKET_SIZE
#else
#   define DAP_PACKET_SIZE                              DAP_HID_PACKET_SIZE
#endif
#define DAP_PACKET_COUNT                                4
#define TIMESTAMP_CLOCK                                 1000000			// 1M
#define SWO_UART                                        1
#define SWO_STREAM                                      0
#define SWO_UART_MAX_BAUDRATE                           3200000
#define SWO_UART_MIN_BAUDRATE                           2000
#define SWO_MANCHESTER                                  0
#define SWO_BUFFER_SIZE                                 512
#define VENDOR_UART                                     0
#define VENDOR_UART_BUFFER_SIZE                         256


#endif

