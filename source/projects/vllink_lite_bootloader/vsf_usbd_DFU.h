#ifndef __VSF_USBD_DFU_H__
#define __VSF_USBD_DFU_H__

#define DFU_BLOCK_SIZE		1024

struct vk_usbd_dfu_t
{
	uint8_t state;
	uint32_t op_addr;
	uint32_t op_size;

	uint8_t reply[64];
	uint8_t block[DFU_BLOCK_SIZE];
};
typedef struct vk_usbd_dfu_t vk_usbd_dfu_t;

extern const vk_usbd_class_op_t vk_usbd_dfu_class;

#endif  // __VSF_USBD_DFU_H__
