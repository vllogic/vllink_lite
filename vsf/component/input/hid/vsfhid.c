#include "vsf.h"

#define HID_LONG_ITEM(x)			((x) == 0xFE)
#define HID_ITEM_SIZE(x)			((((x) & 0x03) == 3) ? 4 : (x) & 0x03)

#define HID_ITEM_TYPE(x)			(((x) >> 2) & 0x03)
#define HID_ITEM_TYPE_MAIN			0
#define HID_ITEM_TYPE_GLOBAL		1
#define HID_ITEM_TYPE_LOCAL			2

#define HID_ITEM_TAG(x)				((x) & 0xFC)
#define HID_ITEM_INPUT				0x80
#define HID_ITEM_OUTPUT				0x90
#define HID_ITEM_FEATURE			0xB0
#define HID_ITEM_COLLECTION			0xA0
#define HID_ITEM_END_COLLECTION		0xC0
#define HID_ITEM_USAGE_PAGE			0x04
#define HID_ITEM_LOGI_MINI			0x14
#define HID_ITEM_LOGI_MAXI			0x24
#define HID_ITEM_PHY_MINI			0x34
#define HID_ITEM_PHY_MAXI			0x44
#define HID_ITEM_UNIT_EXPT			0x54
#define HID_ITEM_UNIT				0x64
#define HID_ITEM_REPORT_SIZE		0x74
#define HID_ITEM_REPORT_ID			0x84
#define HID_ITEM_REPORT_COUNT		0x94
#define HID_ITEM_PUSH				0xA4
#define HID_ITEM_POP				0xB4
#define HID_ITEM_USAGE				0x08
#define HID_ITEM_USAGE_MIN			0x18
#define HID_ITEM_USAGE_MAX			0x28

struct hid_desc_t
{
	int report_id;
	int collection;
	int report_size;
	int report_count;
	int usage_page;
	int usage_num;
	int logical_min;
	int logical_max;
	int physical_min;
	int physical_max;
	int usage_min;
	int usage_max;
	int generic_usage;
	int usages[16];
};

struct vsfhid_global_t vsfhid;

static struct vsfhid_report_t *vsfhid_get_report(struct vsfhid_dev_t *dev,
		struct hid_desc_t *desc, uint8_t type)
{
	struct vsfhid_report_t *report;

	report = sllist_get_container(dev->report_list.next, struct vsfhid_report_t,
				report_list);
	while (report)
	{
		if ((report->type == type) && (report->id == desc->report_id))
			break;
		report = sllist_get_container(report->report_list.next,
				struct vsfhid_report_t, report_list);
	}

	if (!report)
	{
		report = vsf_bufmgr_malloc(sizeof(*report));
		if (report != NULL)
		{
			memset(report, 0, sizeof(*report));
			report->report_list.next = dev->report_list.next;
			dev->report_list.next = &report->report_list;
			report->type = type;
			report->id = (desc->report_id >= 0) ? desc->report_id : -1;
			report->generic_usage = desc->generic_usage;
		}
	}
	return report;
}

static vsf_err_t vsfhid_parse_item(struct vsfhid_dev_t *dev,
		struct hid_desc_t *desc, uint8_t tag, uint32_t size, uint8_t *buf)
{
	struct vsfhid_report_t *report;
	struct vsfhid_usage_t *usage;
	uint32_t value, ival;
	int i;

	if (size == 1)		value = *buf;
	else if (size == 2)	value = *(uint16_t *)buf;
	else if (size == 4)	value = *(uint32_t *)buf;

	switch (tag)
	{
		case HID_ITEM_INPUT:
			report = vsfhid_get_report(dev, desc, HID_ITEM_INPUT);
			if (!report) return VSFERR_FAIL;

			if ((desc->usage_min != -1) && (desc->usage_max != -1))
			{
				desc->usage_num = desc->usage_max - desc->usage_min + 1;
				usage = vsf_bufmgr_malloc(sizeof(struct vsfhid_usage_t));
				if (usage == NULL) return VSFERR_FAIL;

				usage->data_flag = value;
				usage->report_size = (int32_t)desc->report_size;
				usage->report_count = (int32_t)desc->report_count;

				usage->usage_page = (uint16_t)desc->usage_page;
				usage->usage_min = (uint8_t)desc->usage_min;
				usage->usage_max = (uint8_t)desc->usage_max;
				usage->bit_offset = (int32_t)report->bitlen;
				usage->bit_length = (int32_t)(desc->report_size * desc->report_count);

				usage->logical_min = desc->logical_min;
				usage->logical_max = desc->logical_max;

				sllist_append(&report->usage_list, &usage->list);

				desc->usage_min = -1;
				desc->usage_max = -1;
			}
			else
			{
				for (i = 0; i < desc->report_count; i++)
				{
					usage = vsf_bufmgr_malloc(sizeof(struct vsfhid_usage_t));
					if (usage == NULL) return VSFERR_FAIL;

					usage->report_size = (int32_t)desc->report_size;
					usage->report_count = 1;
					usage->data_flag = value;

					usage->usage_page = (uint16_t)desc->usage_page;
					usage->usage_min = (uint8_t)desc->usages[i];
					usage->usage_max = (uint8_t)desc->usages[i];
					usage->bit_length = (int32_t)(desc->report_size * usage->report_count);
					usage->bit_offset = (int32_t)(report->bitlen + i * usage->bit_length);

					usage->logical_min = desc->logical_min;
					usage->logical_max = desc->logical_max;

					sllist_append(&report->usage_list, &usage->list);
				}
			}

			desc->usage_num = 0;
			report->bitlen += (desc->report_size * desc->report_count);
			break;

		case HID_ITEM_OUTPUT:
			report = vsfhid_get_report(dev, desc, HID_ITEM_OUTPUT);
			if (!report) return VSFERR_FAIL;

			if ((desc->usage_min != -1) && (desc->usage_max != -1))
			{
				desc->usage_num = desc->usage_max - desc->usage_min + 1;
				usage = vsf_bufmgr_malloc(sizeof(struct vsfhid_usage_t));
				if (usage == NULL) return VSFERR_FAIL;

				usage->report_size = desc->report_size;
				usage->report_count = desc->report_count;
				usage->data_flag = value;

				usage->usage_page = desc->usage_page;
				usage->usage_min = desc->usage_min;
				usage->usage_max = desc->usage_max;
				usage->bit_offset = report->bitlen;
				usage->bit_length = desc->report_size * desc->report_count;

				usage->logical_min = desc->logical_min;
				usage->logical_max = desc->logical_max;

				sllist_append(&report->usage_list, &usage->list);

				desc->usage_min = -1;
				desc->usage_max = -1;
			}
			else
			{
				for (i = 0; i < desc->usage_num; i++)
				{
					usage = vsf_bufmgr_malloc(sizeof(struct vsfhid_usage_t));
					if (usage == NULL) return VSFERR_FAIL;

					usage->report_size = desc->report_size;
					usage->report_count = desc->report_count;
					usage->data_flag = value;
					value = desc->report_size * desc->report_count / desc->usage_num;

					usage->usage_page = desc->usage_page;
					usage->usage_min = desc->usages[i];
					usage->usage_max = desc->usages[i];
					usage->bit_offset = report->bitlen + i * value;
					usage->bit_length = value;

					usage->logical_min = desc->logical_min;
					usage->logical_max = desc->logical_max;

					sllist_append(&report->usage_list, &usage->list);
				}
			}

			desc->usage_num = 0;
			report->bitlen += (desc->report_size * desc->report_count);
			break;

		case HID_ITEM_FEATURE:
			report = vsfhid_get_report(dev, desc, HID_ITEM_FEATURE);
			if (!report) return VSFERR_FAIL;
			break;

		case HID_ITEM_COLLECTION:
			desc->collection++;
			desc->usage_num = 0;
			break;

		case HID_ITEM_END_COLLECTION:
			desc->collection--;
			break;

		case HID_ITEM_USAGE_PAGE:
			desc->usage_page = value;
			break;

		case HID_ITEM_LOGI_MINI:
			if (size == 1)		ival = *(int8_t *)buf;
			else if (size == 2)	ival = *(int16_t *)buf;
			else if (size == 4)	ival = *(int32_t *)buf;
			desc->logical_min = ival;
			break;

		case HID_ITEM_LOGI_MAXI:
			if (size == 1)		ival = *(int8_t *)buf;
			else if (size == 2)	ival = *(int16_t *)buf;
			else if (size == 4)	ival = *(int32_t *)buf;
			desc->logical_max = ival;
			break;

		case HID_ITEM_PHY_MINI:
			break;

		case HID_ITEM_PHY_MAXI:
			break;

		case HID_ITEM_UNIT_EXPT:
			break;

		case HID_ITEM_UNIT:
			break;

		case HID_ITEM_REPORT_SIZE:
			desc->report_size = value;
			break;

		case HID_ITEM_REPORT_ID:
			desc->report_id = value;
			dev->report_has_id = true;
			break;

		case HID_ITEM_REPORT_COUNT:
			desc->report_count = value;
			break;

		case HID_ITEM_PUSH:
			break;

		case HID_ITEM_POP:
			break;

		case HID_ITEM_USAGE:
			if (desc->usage_num < dimof(desc->usages))
			{
				desc->usages[desc->usage_num++] = value;
				if (desc->collection == 0)
					desc->generic_usage = value;
			}
			else
				return VSFERR_NOT_ENOUGH_RESOURCES;
			break;

		case HID_ITEM_USAGE_MAX:
			desc->usage_max = value;
			break;

		case HID_ITEM_USAGE_MIN:
			desc->usage_min = value;
			break;
	}

	return VSFERR_NONE;
}

void vsfhid_free_dev(struct vsfhid_dev_t *dev)
{
	struct vsfhid_report_t *report, *report_next;
	struct sllist *usage_list, *usage_next;

	report = sllist_get_container(dev->report_list.next,
				struct vsfhid_report_t, report_list);
	while (report)
	{
		report_next = sllist_get_container(report->report_list.next,
				struct vsfhid_report_t, report_list);

		usage_list = report->usage_list.next;
		while (usage_list)
		{
			usage_next = usage_list->next;
			vsf_bufmgr_free(usage_list);
			usage_list = usage_next;
		}

		if (report->value)
			vsf_bufmgr_free(report->value);
		vsf_bufmgr_free(report);
		report = report_next;
	}
}

vsf_err_t vsfhid_parse_report(struct vsfhid_dev_t *dev, uint8_t *buf, uint32_t len)
{
	struct hid_desc_t *desc = vsf_bufmgr_malloc(sizeof(struct hid_desc_t));
	uint8_t *end = buf + len;
	int item_size;
	vsf_err_t err;

	if (desc == NULL) return VSFERR_FAIL;
	memset(desc, 0, sizeof(*desc));
	desc->report_id = -1;
	desc->usage_min = -1;
	desc->usage_max = -1;

	memset(dev, 0, sizeof(struct vsfhid_dev_t));
	while (buf < end)
	{
		if (HID_LONG_ITEM(*buf))
			item_size = *(buf + 1);
		else
		{
			item_size = HID_ITEM_SIZE(*buf);
			err = vsfhid_parse_item(dev, desc, HID_ITEM_TAG(*buf), item_size, buf + 1);
			if (err) break;
		}
		buf += (item_size + 1);
	}

	if ((desc->collection != 0) || err)
		goto free_hid_report;
	vsf_bufmgr_free(desc);
	return VSFERR_NONE;

free_hid_report:
	vsf_bufmgr_free(desc);
	vsfhid_free_dev(dev);

	return VSFERR_FAIL;
}

int32_t vsfhid_find_usage_page(struct vsfhid_dev_t *dev, uint16_t usage_page)
{
	int32_t ret = 0;
	struct vsfhid_usage_t *usage;
	struct vsfhid_report_t *report = sllist_get_container(dev->report_list.next,
				struct vsfhid_report_t, report_list);
	
	while (report != NULL)
	{
		usage = sllist_get_container(report->usage_list.next, struct vsfhid_usage_t, list);
		while (usage != NULL)
		{
			if (usage->usage_page == usage_page)
				ret++;			
			usage = sllist_get_container(usage->list.next, struct vsfhid_usage_t, list);
		}
		report = sllist_get_container(report->report_list.next,
				struct vsfhid_report_t, report_list);
	}
	return ret;
}

uint16_t vsfhid_get_max_input_size(struct vsfhid_dev_t *dev)
{
	struct vsfhid_report_t *report = sllist_get_container(dev->report_list.next,
				struct vsfhid_report_t, report_list);
	uint16_t maxsize = 0;

	while (report != NULL)
	{
		if (report->bitlen > maxsize)
			maxsize = report->bitlen;
		report = sllist_get_container(report->report_list.next,
				struct vsfhid_report_t, report_list);
	}
	return (maxsize + 7) >> 3;
}

void vsfhid_process_input(struct vsfhid_dev_t *dev, uint8_t *buf, uint32_t len)
{
	struct vsfhid_report_t *report = sllist_get_container(dev->report_list.next,
				struct vsfhid_report_t, report_list);
	struct vsfhid_event_t event;
	struct vsfhid_usage_t *usage;
	uint32_t cur_value, pre_value;
	bool reported = false;
	int16_t id = dev->report_has_id ? *buf++ : -1;
	int32_t i;

	while (report != NULL)
	{
		if ((report->type == HID_ITEM_INPUT) && (report->id == id)) break;
		report = sllist_get_container(report->report_list.next,
				struct vsfhid_report_t, report_list);
	}
	if (!report)
		return;

	if (!report->value)
	{
		report->value = vsf_bufmgr_malloc(len);
		if (!report->value) return;
		memset(report->value, 0, len);
	}

	usage = sllist_get_container(report->usage_list.next, struct vsfhid_usage_t, list);
	while (usage != NULL)
	{
		for (i = 0; i < usage->report_count; ++i)
		{
			/* get usage value */
			cur_value = buf_get_value(buf, usage->bit_offset + i * usage->report_size, usage->report_size);
			pre_value = buf_get_value(report->value, usage->bit_offset + i * usage->report_size, usage->report_size);

			/* compare and process */
			if (cur_value != (HID_USAGE_IS_REL(usage) ? 0 : pre_value))
			{
				event.usage_page = usage->usage_page;
				event.usage_id = HID_USAGE_IS_VAR(usage) ? (usage->usage_min + i) :
						(cur_value ? (uint16_t)cur_value : (uint16_t)pre_value);
				event.pre_value = pre_value;
				event.cur_value = cur_value;
				event.usage = usage;

				reported = true;
				if (vsfhid.report)
					vsfhid.report(report->generic_usage, &event);
			}
		}
		usage = sllist_get_container(usage->list.next, struct vsfhid_usage_t, list);
	}

	// just report input process end if changed reportted
	if (vsfhid.report && reported)
		vsfhid.report(report->generic_usage, NULL);
	memcpy(report->value, buf, len);
}
