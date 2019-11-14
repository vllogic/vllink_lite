
#ifndef __VSF_DYNARR_H_INCLUDED__
#define __VSF_DYNARR_H_INCLUDED__

struct vsf_dynarr_table_t
{
	struct sllist list;
	void *buffer[0];
};

struct vsf_dynarr_t
{
	// public
	uint32_t item_size;
	uint32_t item_num_bitlen;
	uint32_t table_size_bitlen;

	// private
	struct sllist table;
	uint32_t length;
};

vsf_err_t vsf_dynarr_init(struct vsf_dynarr_t *dynarr);
vsf_err_t vsf_dynarr_fini(struct vsf_dynarr_t *dynarr);
uint32_t vsf_dynarr_get_size(struct vsf_dynarr_t *dynarr);
vsf_err_t vsf_dynarr_set_size(struct vsf_dynarr_t *dynarr, uint32_t size);
void *vsf_dynarr_get(struct vsf_dynarr_t *dynarr, uint32_t pos);

#endif	// __VSF_DYNARR_H_INCLUDED__
