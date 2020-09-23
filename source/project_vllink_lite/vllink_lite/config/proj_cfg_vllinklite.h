#ifndef __PROJ_CFG_LVL2_H__
#define __PROJ_CFG_LVL2_H__

#if defined(BRD_CFG_VLLINKLITE_GD32E103)
#   define APP_CFG_SERIAL_HEADER_STR                    u"GD32E103."
#   define APP_CFG_SERIAL_HEADER_STR_LENGTH             (sizeof(APP_CFG_SERIAL_HEADER_STR) - 2)
#   define FIRMWARE_AREA_ADDR                           0x08003000
#   define FIRMWARE_AREA_SIZE_MAX                       (128 * 1024 - 12 * 1024)
#   define FIRMWARE_SP_ADDR			                    (0x20000000 + 4)
#   define FIRMWARE_SP_SIZE_MAX		                    (8 * 1024 - 4)
//#   define PROJ_CFG_CORE_INIT_TINY
#   if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#       define VSF_DWCOTG_DCD_CFG_FAKE_EP               ENABLED
#       define USRAPP_CFG_USBD_SPEED                    USB_SPEED_FULL
#       define APP_CFG_USBD_SPEED                       USB_DC_SPEED_FULL
#   endif
#	define APP_CFG_CMSIS_DAP_V2_SUPPORT
#	define APP_CFG_WEBUSB_SUPPORT
#	define APP_CFG_CDCEXT_SUPPORT
#       define APP_CFG_CDCEXT_PKT_SIZE                  64
#	define APP_CFG_CDCSHELL_SUPPORT
#       define APP_CFG_CDCSHELL_PKT_SIZE                64
#elif defined(BRD_CFG_VLLINKLITE_GD32F350)
#   define APP_CFG_SERIAL_HEADER_STR                    u"GD32F350."
#   define APP_CFG_SERIAL_HEADER_STR_LENGTH             (sizeof(APP_CFG_SERIAL_HEADER_STR) - 2)
#   define FIRMWARE_AREA_ADDR                           0x08003000
#   define FIRMWARE_AREA_SIZE_MAX                       (64 * 1024 - 12 * 1024)
#   define FIRMWARE_SP_ADDR			                    (0x20000000 + 4)
#   define FIRMWARE_SP_SIZE_MAX		                    (32 * 1024 - 4)
//#   define PROJ_CFG_CORE_INIT_TINY
#   if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#       define VSF_DWCOTG_DCD_CFG_FAKE_EP               ENABLED
#       define USRAPP_CFG_USBD_SPEED                    USB_SPEED_FULL
#       define APP_CFG_USBD_SPEED                       USB_DC_SPEED_FULL
#   endif
#	define APP_CFG_CMSIS_DAP_V2_SUPPORT
#	define APP_CFG_WEBUSB_SUPPORT
#	define APP_CFG_CDCEXT_SUPPORT
#       define APP_CFG_CDCEXT_PKT_SIZE                  32
#	define APP_CFG_CDCSHELL_SUPPORT
#       define APP_CFG_CDCSHELL_PKT_SIZE                32
#endif

#define APP_CFG_USBD_VID                                0xABCD  // 0x1209
#define APP_CFG_USBD_PID                                0x6666
#define APP_CFG_USBD_BCD                                0x0100
#define APP_CFG_USBD_EP0_SIZE                           64
#define APP_CFG_USBD_VENDOR_STR                         u"vsf"
#define APP_CFG_USBD_PRODUCT_STR                        u"vsf_dfu"
#define APP_CFG_USBD_SERIAL_STR                         u"0000"
#define APP_CFG_USBD_WEBUSB_URL                         "vllogic.github.io/webdfu/"


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

#endif // __PROJ_CFG_LVL2_H__