
#ifndef __VSF_DYNSTACK_H_INCLUDED__
#define __VSF_DYNSTACK_H_INCLUDED__

struct vsf_dynstack_t
{
	// public
	struct vsf_dynarr_t var;
	uint32_t sp;
};

vsf_err_t vsf_dynstack_init(struct vsf_dynstack_t *stack, uint32_t item_size,
	uint32_t item_num_bitlen, uint32_t table_size_bitlen);
vsf_err_t vsf_dynstack_reset(struct vsf_dynstack_t *stack);
vsf_err_t vsf_dynstack_fini(struct vsf_dynstack_t *stack);
void *vsf_dynstack_get(struct vsf_dynstack_t *stack, uint32_t offset);
void *vsf_dynstack_pop(struct vsf_dynstack_t *stack, uint32_t num);
vsf_err_t vsf_dynstack_push(struct vsf_dynstack_t *stack, void *item,
	uint32_t num);
vsf_err_t vsf_dynstack_push_ext(struct vsf_dynstack_t *stack, void *ptr,
	uint32_t len);
vsf_err_t vsf_dynstack_pop_ext(struct vsf_dynstack_t *stack, void *ptr,
	uint32_t len);

#endif	// __VSF_DYNSTACK_H_INCLUDED__
