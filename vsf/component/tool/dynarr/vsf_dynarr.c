#include "vsf.h"

uint32_t vsf_dynarr_get_size(struct vsf_dynarr_t *dynarr)
{
	return dynarr->length;
}

vsf_err_t vsf_dynarr_set_size(struct vsf_dynarr_t *dynarr, uint32_t size)
{
	uint32_t table_buffer_num = 1UL << dynarr->table_size_bitlen;
	uint32_t buffer_item_num = 1UL << dynarr->item_num_bitlen;
	uint32_t buffer_num =
		(size + buffer_item_num - 1) >> dynarr->item_num_bitlen;
	uint32_t buffer_num_orig =
		(dynarr->length + buffer_item_num - 1) >> dynarr->item_num_bitlen;
	struct vsf_dynarr_table_t *table = sllist_get_container(dynarr->table.next,
			struct vsf_dynarr_table_t, list);
	uint32_t buffer_size = buffer_item_num * dynarr->item_size;

	struct vsf_dynarr_table_t *new_table;
	void *new_buffer;
	uint32_t table_idx, buffer_idx;
	uint32_t tblsize;

	dynarr->length = buffer_num_orig * buffer_item_num;
	if (buffer_num > buffer_num_orig)
	{
		if (!buffer_num_orig)
		{
			buffer_idx = (1UL << dynarr->table_size_bitlen) - 1;
		}
		else
		{
			buffer_num_orig--;
			buffer_num--;
			buffer_idx = buffer_num_orig & (table_buffer_num - 1);
			table_idx = buffer_num_orig >> dynarr->table_size_bitlen;

			while (table_idx--)
				table = sllist_get_container(table->list.next,
					struct vsf_dynarr_table_t, list);
		}

		buffer_num -= buffer_num_orig;
		while (buffer_num > 0)
		{
			if (++buffer_idx == (1UL << dynarr->table_size_bitlen))
			{
				tblsize = sizeof(*new_table) + (table_buffer_num << 2);
				new_table = (struct vsf_dynarr_table_t *)vsf_bufmgr_malloc(tblsize);
				if (!new_table)
					return VSFERR_NOT_ENOUGH_RESOURCES;
				memset(new_table, 0, tblsize);
				buffer_idx = 0;
			}
			else
				new_table = NULL;

			new_buffer = vsf_bufmgr_malloc(buffer_size);
			if (!new_buffer)
			{
				if (new_table)
					vsf_bufmgr_free(new_table);
				return VSFERR_NOT_ENOUGH_RESOURCES;
			}

			if (new_table)
			{
				if (!table)
					sllist_insert(dynarr->table, new_table->list);
				else
					sllist_insert(table->list, new_table->list);
				table = new_table;
			}
			table->buffer[buffer_idx] = new_buffer;
			dynarr->length += buffer_item_num;
			buffer_num--;
		}
	}
	else if (buffer_num < buffer_num_orig)
	{
		if (!buffer_num)
		{
			buffer_idx = (1UL << dynarr->table_size_bitlen) - 1;
			// dangerous operation, but working
			table = sllist_get_container(&dynarr->table,
				struct vsf_dynarr_table_t, list);
		}
		else
		{
			buffer_num_orig--;
			buffer_num--;
			buffer_idx = buffer_num & (table_buffer_num - 1);
			table_idx = buffer_num >> dynarr->table_size_bitlen;

			while (table_idx--)
				table = sllist_get_container(table->list.next,
					struct vsf_dynarr_table_t, list);
		}

		buffer_num = buffer_num_orig - buffer_num;
		while (buffer_num > 0)
		{
			if (++buffer_idx == (1UL << dynarr->table_size_bitlen))
			{
				new_table = sllist_get_container(table->list.next,
					struct vsf_dynarr_table_t, list);
				table->list.next = NULL;

				for (int i = min(buffer_num, table_buffer_num) - 1; i >= 0; i--)
				{
					vsf_bufmgr_free(new_table->buffer[i]);
					buffer_num--;
				}
				table = sllist_get_container(new_table->list.next,
					struct vsf_dynarr_table_t, list);
				vsf_bufmgr_free(new_table);
			}
			else
			{
				vsf_bufmgr_free(table->buffer[buffer_idx]);
				buffer_num--;
			}
		}
	}
	dynarr->length = size;
	return VSFERR_NONE;
}

void *vsf_dynarr_get(struct vsf_dynarr_t *dynarr, uint32_t pos)
{
	if (pos < dynarr->length)
	{
		struct vsf_dynarr_table_t *table =
			sllist_get_container(dynarr->table.next, struct vsf_dynarr_table_t,
				list);
		uint32_t item_idx, buffer_idx, table_idx;

		item_idx = pos & ((1UL << dynarr->item_num_bitlen) - 1);
		pos >>= dynarr->item_num_bitlen;
		buffer_idx = pos & ((1UL << dynarr->table_size_bitlen) - 1);
		pos >>= dynarr->table_size_bitlen;
		table_idx = pos;

		while (table_idx--)
			table = sllist_get_container(table->list.next,
				struct vsf_dynarr_table_t, list);

		if (table != NULL)
			return (void *)((uint32_t *)table->buffer[buffer_idx] +
				((item_idx * dynarr->item_size) >> 2));
	}
	return NULL;
}

vsf_err_t vsf_dynarr_init(struct vsf_dynarr_t *dynarr)
{
	dynarr->item_size = (dynarr->item_size + 3) & ~3;
	sllist_init_node(dynarr->table);
	dynarr->length = 0;
	return VSFERR_NONE;
}

vsf_err_t vsf_dynarr_fini(struct vsf_dynarr_t *dynarr)
{
	uint32_t table_buffer_num = 1UL << dynarr->table_size_bitlen;
	uint32_t buffer_item_num = 1UL << dynarr->item_num_bitlen;
	uint32_t buffer_num = (dynarr->length + buffer_item_num - 1) >> dynarr->item_num_bitlen;
	struct vsf_dynarr_table_t *table, *tmp_table;

	table = sllist_get_container(dynarr->table.next, struct vsf_dynarr_table_t,
		list);
	while (buffer_num > 0)
	{
		tmp_table = table;
		table = sllist_get_container(table->list.next, struct vsf_dynarr_table_t, list);
		for (int i = min(buffer_num, table_buffer_num); i > 0; i--)
		{
			vsf_bufmgr_free(tmp_table->buffer[i - 1]);
			buffer_num--;
		}
		vsf_bufmgr_free(tmp_table);
	}

	dynarr->table.next = NULL;
	dynarr->length = 0;
	return VSFERR_NONE;
}
