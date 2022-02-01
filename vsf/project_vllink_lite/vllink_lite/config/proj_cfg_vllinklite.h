#ifndef __PROJ_CFG_LVL2_H__
#define __PROJ_CFG_LVL2_H__

#if defined(BRD_CFG_VLLINKLITE_GD32E103)
#   define APP_CFG_SERIAL_HEADER_STR                    u"GD32E103."
#   define APP_CFG_SERIAL_HEADER_STR_LENGTH             (sizeof(APP_CFG_SERIAL_HEADER_STR) - 2)
#   define APP_CFG_SERIAL_ATTACH_UUID
#   define APP_CFG_USBD_VID                             0x1209
#   define APP_CFG_USBD_PID                             0x6666
#   define APP_CFG_USBD_BCD                             0x0100
#   define APP_CFG_USBD_VENDOR_STR                      u"Vllogic.com"
#   define APP_CFG_USBD_PRODUCT_STR                     u"Vllink Lite"
#   define APP_CFG_USBD_SERIAL_STR                      u"GD32E103"
#   define PROJ_CFG_GD32E10X_HSI48M_USB_PLL_128M_OVERCLOCK
#   if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#       define VSF_DWCOTG_DCD_CFG_FAKE_EP               ENABLED
#       define USRAPP_CFG_USBD_SPEED                    USB_SPEED_FULL
#       define APP_CFG_USBD_SPEED                       USB_DC_SPEED_FULL
#   endif
#	define APP_CFG_CMSIS_DAP_V2_SUPPORT
#	define APP_CFG_WEBUSB_SUPPORT
#	define APP_CFG_CDCEXT_SUPPORT
#       define APP_CFG_CDCEXT_EXT2USB_PKT_NUM           4
#       define APP_CFG_CDCEXT_USB2EXT_PKT_NUM           3
#       define APP_CFG_CDCEXT_PKT_SIZE                  64
//#	define APP_CFG_CDCSHELL_SUPPORT
#       define APP_CFG_CDCSHELL_EXT2USB_PKT_NUM         4
#       define APP_CFG_CDCSHELL_USB2EXT_PKT_NUM         3
#       define APP_CFG_CDCSHELL_PKT_SIZE                64
#       define APP_CFG_CDCSHELL_DAPHOST
#   define APP_CFG_CDCEXT_DATA_OUT_EP                   2
#   define APP_CFG_CDCEXT_DATA_IN_EP                    2
#   define APP_CFG_CDCEXT_NOTIFY_EP                     3
//#   define APP_CFG_CDCSHELL_DATA_OUT_EP                 3
//#   define APP_CFG_CDCSHELL_DATA_IN_EP                  3
//#   define APP_CFG_CDCSHELL_NOTIFY_EP                   5
#elif defined(BRD_CFG_VLLINKLITE_GD32F350)
#   define APP_CFG_SERIAL_HEADER_STR                    u"GD32F350."
#   define APP_CFG_SERIAL_HEADER_STR_LENGTH             (sizeof(APP_CFG_SERIAL_HEADER_STR) - 2)
#   define APP_CFG_SERIAL_ATTACH_UUID
#   define APP_CFG_USBD_VID                             0x1209
#   define APP_CFG_USBD_PID                             0x6666
#   define APP_CFG_USBD_BCD                             0x0101
#   define APP_CFG_USBD_VENDOR_STR                      u"Vllogic.com"
#   define APP_CFG_USBD_PRODUCT_STR                     u"Vllink Lite"
#   define APP_CFG_USBD_SERIAL_STR                      u"GD32F350"
#   define PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_128M_OVERCLOCK
#   if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#       define VSF_DWCOTG_DCD_CFG_FAKE_EP               ENABLED
#       define USRAPP_CFG_USBD_SPEED                    USB_SPEED_FULL
#       define APP_CFG_USBD_SPEED                       USB_DC_SPEED_FULL
#   endif
#	define APP_CFG_CMSIS_DAP_V2_SUPPORT
#	define APP_CFG_WEBUSB_SUPPORT
#	define APP_CFG_CDCEXT_SUPPORT
#       define APP_CFG_CDCEXT_PKT_SIZE                  64
//#   define APP_CFG_CDCSHELL_SUPPORT
#   define APP_CFG_CDCEXT_DATA_OUT_EP                   2
#   define APP_CFG_CDCEXT_DATA_IN_EP                    2
#   define APP_CFG_CDCEXT_NOTIFY_EP                     3
//#   define APP_CFG_CDCSHELL_DATA_OUT_EP                 3
//#   define APP_CFG_CDCSHELL_DATA_IN_EP                  3
//#   define APP_CFG_CDCSHELL_NOTIFY_EP                   5
#endif

#ifdef APP_CFG_CDCSHELL_SUPPORT
#   define VSF_KERNEL_CFG_EDA_SUPPORT_SUB_CALL          ENABLED
#   define VSF_KERNEL_CFG_EDA_SUPPORT_FSM               ENABLED
#   define VSF_USE_HEAP                                 ENABLED
#       define VSF_HEAP_CFG_MCB_MAGIC_EN                ENABLED
#       define VSF_HEAP_SIZE                            0x2000
#   define VSF_USE_PTSHELL                              ENABLED
#else
#   define VSF_KERNEL_CFG_EDA_SUPPORT_SUB_CALL          DISABLED
#   define VSF_KERNEL_CFG_EDA_SUPPORT_FSM               DISABLED
#   define VSF_USE_HEAP                                 DISABLED
#endif

#define CMSIS_DAP_V2_PACKET_SIZE                        512
#define WEBUSB_DAP_PACKET_SIZE                          64

#ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
#	define CMSIS_DAP_V2_DESC_LENGTH			            23
#	define CMSIS_DAP_V2_INTERFACE_COUNT		            1
#	define WINUSB_BOS_DESC_LENGTH			            28
#	define WINUSB_BOS_COUNT					            1
#	define FUNCTION_CMSIS_DAP_V2_SUBSET_LEN             160
#	define CMSIS_DAP_V2_STR_DESC_COUNT                  2
#else
#	define CMSIS_DAP_V2_DESC_LENGTH			            0
#	define CMSIS_DAP_V2_INTERFACE_COUNT		            0
#	define WINUSB_BOS_DESC_LENGTH			            0
#	define WINUSB_BOS_COUNT					            0
#	define FUNCTION_CMSIS_DAP_V2_SUBSET_LEN             0
#	define CMSIS_DAP_V2_STR_DESC_COUNT                  0
#endif

#ifdef APP_CFG_WEBUSB_SUPPORT
#	define WEBUSB_DESC_LENGTH				            9
#	define WEBUSB_INTERFACE_COUNT			            1
#	define WEBUSB_BOS_DESC_LENGTH			            24
#	define WEBUSB_BOS_COUNT					            1
#	define FUNCTION_WEBUSB_SUBSET_LEN                   160
#	define WEBUSB_STR_DESC_COUNT                        1
#else
#	define WEBUSB_DESC_LENGTH				            0
#	define WEBUSB_INTERFACE_COUNT			            0
#	define WEBUSB_BOS_DESC_LENGTH			            0
#	define WEBUSB_BOS_COUNT					            0
#	define FUNCTION_WEBUSB_SUBSET_LEN                   0
#	define WEBUSB_STR_DESC_COUNT                        0
#endif

#ifdef APP_CFG_CDCEXT_SUPPORT
#	define CDCEXT_DESC_LENGTH				            66
#	define CDCEXT_INTERFACE_COUNT			            2
#	define CDCEXT_STR_DESC_COUNT                        1
#else
#	define CDCEXT_DESC_LENGTH				            0
#	define CDCEXT_INTERFACE_COUNT			            0
#	define CDCEXT_STR_DESC_COUNT                        0
#endif

#ifdef APP_CFG_CDCSHELL_SUPPORT
#	define CDCSHELL_DESC_LENGTH				            66
#	define CDCSHELL_INTERFACE_COUNT			            2
#	define CDCSHELL_STR_DESC_COUNT                      1
#else
#	define CDCSHELL_DESC_LENGTH				            0
#	define CDCSHELL_INTERFACE_COUNT			            0
#	define CDCSHELL_STR_DESC_COUNT                      0
#endif

#define APP_CFG_USBD_CONFIGDESC_LENGTH                  (9 + CMSIS_DAP_V2_DESC_LENGTH + WEBUSB_DESC_LENGTH + CDCEXT_DESC_LENGTH + CDCSHELL_DESC_LENGTH)
#define APP_CFG_USBD_INTERFACE_COUNT                    (CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT + CDCEXT_INTERFACE_COUNT + CDCSHELL_INTERFACE_COUNT)
#define APP_CFG_USBD_WINUSB_DESC_LENGTH                 (10 + FUNCTION_WEBUSB_SUBSET_LEN + FUNCTION_CMSIS_DAP_V2_SUBSET_LEN)
#define APP_CFG_USBD_BOS_NUMBER                         (WINUSB_BOS_COUNT + WEBUSB_BOS_COUNT)
#define APP_CFG_USBD_BOS_DESC_LENGTH                    (5 + WINUSB_BOS_DESC_LENGTH + WEBUSB_BOS_DESC_LENGTH)
#define APP_CFG_USBD_DESC_COUNT                         (7 + CMSIS_DAP_V2_STR_DESC_COUNT + WEBUSB_STR_DESC_COUNT + CDCEXT_STR_DESC_COUNT + CDCSHELL_STR_DESC_COUNT)


/*
	DAP Config
*/
//#define DAP_PRODUCT                                     
//#define DAP_SER_NUM                                     
#define DAP_FW_VER                                      "0254" // Firmware Version
//#define DAP_DEVICE_VENDOR                               
//#define DAP_DEVICE_NAME                                 

#define DAP_SWD                                         1
#define DAP_JTAG                                        0
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


#endif // __PROJ_CFG_LVL2_H__