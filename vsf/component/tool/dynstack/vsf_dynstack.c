#include "vsf.h"

vsf_err_t vsf_dynstack_init(struct vsf_dynstack_t *stack, uint32_t item_size,
	uint32_t item_num_bitlen, uint32_t table_size_bitlen)
{
	stack->var.item_size = item_size;
	stack->var.item_num_bitlen = item_num_bitlen;
	stack->var.table_size_bitlen = table_size_bitlen;
	vsf_dynarr_init(&stack->var);
	stack->sp = 0;
	return VSFERR_NONE;
}

vsf_err_t vsf_dynstack_reset(struct vsf_dynstack_t *stack)
{
	stack->sp = 0;
	return vsf_dynarr_fini(&stack->var);
}

vsf_err_t vsf_dynstack_fini(struct vsf_dynstack_t *stack)
{
	return vsf_dynstack_reset(stack);
}

void *vsf_dynstack_get(struct vsf_dynstack_t *stack, uint32_t offset)
{
	uint16_t id;
	if (offset >= stack->sp)
		return NULL;
	id = stack->sp - offset - 1;
	return vsf_dynarr_get(&stack->var, id);
}

void *vsf_dynstack_pop(struct vsf_dynstack_t *stack, uint32_t num)
{
	if ((stack->sp >= num) && (num > 0))
	{
		void *out = vsf_dynstack_get(stack, num - 1);
		stack->sp -= num;
		return out;
	}
	return NULL;
}

vsf_err_t vsf_dynstack_push(struct vsf_dynstack_t *stack, void *item,
	uint32_t num)
{
	uint32_t stack_size = vsf_dynarr_get_size(&stack->var);
	void *in;

	if (stack_size < (stack->sp + num))
		if (vsf_dynarr_set_size(&stack->var, stack->sp + num) < 0)
			return VSFERR_FAIL;

	stack->sp += num;
	while (num)
	{
		in = vsf_dynstack_get(stack, num - 1);
		memcpy(in, item, stack->var.item_size);
		num--;
	}
	return VSFERR_NONE;
}

vsf_err_t vsf_dynstack_push_ext(struct vsf_dynstack_t *stack, void *ptr,
	uint32_t len)
{
	uint32_t *item = ptr;
	uint32_t num = (len + 3) >> 2;
	uint32_t item_size = stack->var.item_size >> 2;

	num = (num + item_size - 1) / item_size;
	while (num--)
	{
		if (vsf_dynstack_push(stack, item, 1) < 0)
			return VSFERR_FAIL;
		item += item_size;
	}
	return VSFERR_NONE;
}

vsf_err_t vsf_dynstack_pop_ext(struct vsf_dynstack_t *stack, void *ptr,
	uint32_t len)
{
	uint32_t num = (len + 3) >> 2;

	if (num)
	{
		uint32_t *item = ptr, *out;
		uint32_t item_size = stack->var.item_size >> 2;
		int i;

		item += num - 1;
		while (num)
		{
			out = vsf_dynstack_pop(stack, 1);
			if (!out)
				return VSFERR_FAIL;

			for (i = (num % item_size) ? (num % item_size) - 1 : item_size - 1;
				(i >= 0) && (num > 0); i--, num--)
			{
				*item-- = out[i];
			}
		}
	}
	return VSFERR_NONE;
}
