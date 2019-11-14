#ifndef __VSFHID_H_INCLUDED__
#define __VSFHID_H_INCLUDED__

#define HID_USAGE_PAGE_GENERIC		0x01
#define HID_USAGE_PAGE_SIM_CTRLS	0x02
#define HID_USAGE_PAGE_KEYBOARD 	0x07
#define HID_USAGE_PAGE_LED		 	0x08
#define HID_USAGE_PAGE_BUTTON		0x09

// HID_USAGE_PAGE_GENERIC (Generic Desktop Page)
#define HID_USAGE_ID_X				0x30
#define HID_USAGE_ID_Y				0x31
#define HID_USAGE_ID_Z				0x32
#define HID_USAGE_ID_Rx				0x33
#define HID_USAGE_ID_Ry				0x34
#define HID_USAGE_ID_Rz				0x35
#define HID_USAGE_ID_Slider			0x36
#define HID_USAGE_ID_Dial			0x37
#define HID_USAGE_ID_Wheel			0x38
#define HID_USAGE_ID_Hat			0x39

#define HID_USAGE_ID_GENERIC_MIN	HID_USAGE_ID_X
#define HID_USAGE_ID_GENERIC_MAX	HID_USAGE_ID_Hat

#define HID_USAGE_ID_KEYBOARD_MIN	0
#define HID_USAGE_ID_KEYBOARD_BITS	224
#define HID_USAGE_ID_KEYBOARD_MAX	255

#define HID_USAGE_ID_BUTTON_MIN		1
#define HID_USAGE_ID_BUTTON_MAX		32

struct vsfhid_usage_t
{
	uint16_t usage_page;
	uint8_t usage_min;
	uint8_t usage_max;
	int32_t logical_min;
	int32_t logical_max;
	int32_t bit_offset;
	int32_t bit_length;
	int32_t report_size;
	int32_t report_count;
	uint32_t data_flag;
	struct sllist list;
};

struct vsfhid_event_t
{
	uint16_t usage_page;
	uint16_t usage_id;
	int32_t pre_value;
	int32_t cur_value;
	struct vsfhid_usage_t *usage;
};

#define HID_USAGE_IS_CONST(usage)	((usage)->data_flag & 1)
#define HID_USAGE_IS_DATA(usage)	!HID_USAGE_IS_CONST(usage)
#define HID_USAGE_IS_VAR(usage)		((usage)->data_flag & 2)
#define HID_USAGE_IS_ARRAY(usage)	!HID_USAGE_IS_VAR(usate)
#define HID_USAGE_IS_REL(usage)		((usage)->data_flag & 4)
#define HID_USAGE_IS_ABS(usage)		!HID_USAGE_IS_REL(usage)

#define HID_GENERIC_DESKTOP_UNDEFINEED				0
#define HID_GENERIC_DESKTOP_POINTER					1
#define HID_GENERIC_DESKTOP_MOUSE					2
#define HID_GENERIC_DESKTOP_JOYSTICK				4
#define HID_GENERIC_DESKTOP_GAMEPAD					5
#define HID_GENERIC_DESKTOP_KEYBOARD				6
#define HID_GENERIC_DESKTOP_KEYPAD					7
#define HID_GENERIC_DESKTOP_MULTIAXIS_CONTROLLER	8

struct vsfhid_report_t
{
	uint8_t type;
	int16_t id;
	uint16_t bitlen;
	uint16_t generic_usage;

	uint8_t *value;
	struct sllist usage_list;
	struct sllist report_list;
};

struct vsfhid_dev_t
{
	struct sllist report_list;
	bool report_has_id;
};

struct vsfhid_global_t
{
	void (*report)(uint16_t generic_usage, struct vsfhid_event_t *event);
};
extern struct vsfhid_global_t vsfhid;

vsf_err_t vsfhid_parse_report(struct vsfhid_dev_t *dev, uint8_t *buf, uint32_t len);
int32_t vsfhid_find_usage_page(struct vsfhid_dev_t *dev, uint16_t usage_page);
void vsfhid_process_input(struct vsfhid_dev_t *dev, uint8_t *buf, uint32_t len);
uint16_t vsfhid_get_max_input_size(struct vsfhid_dev_t *dev);
void vsfhid_free_dev(struct vsfhid_dev_t *dev);

#endif		// __VSFHID_H_INCLUDED__
