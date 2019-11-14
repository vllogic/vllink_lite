
#ifndef __VSF_ERR_H_INCLUDED__
#define __VSF_ERR_H_INCLUDED__

#include "vsf_basetype.h"

typedef vsf_int_t vsf_err_t;

#define VSFERR_NOT_READY				1
#define VSFERR_NONE						0
#define VSFERR_NOT_SUPPORT				-1
#define VSFERR_NOT_AVAILABLE			-3
#define VSFERR_NOT_ACCESSABLE			-4
#define VSFERR_NOT_ENOUGH_RESOURCES		-5
#define VSFERR_FAIL						-6
#define VSFERR_INVALID_PARAMETER		-7
#define VSFERR_INVALID_RANGE			-8
#define VSFERR_INVALID_PTR				-9
#define VSFERR_IO						-10
#define VSFERR_BUG						-11
#define VSFERR_UNKNOWN					-100

#endif	// __VSF_ERR_H_INCLUDED__
