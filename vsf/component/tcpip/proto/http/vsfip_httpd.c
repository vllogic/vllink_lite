/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "vsf.h"
#include <stdio.h>

#define VSFIP_HTTP_SERVER_SOCKETTIMEOUT					4000

static const char VSFIP_HTTP_HEAD_GET[] =				"GET ";
static const char VSFIP_HTTP_HEAD_POST[] =				"POST ";

static const struct vsfip_http_mimetype_t\
					vsfip_httpd_mimetype[VSFIP_HTTPD_MIMETYPECNT] =
{
	// can not change order of the first 2 types, they are used for post
	{"application/x-www-form-urlencoded", " "},
	{"multipart/form-data", " "},
	// random order below
	{"text/html", "htm"},
	{"text/html", "html"},
	{"image/jpeg", "jpg"},
	{"image/jpeg", "jpeg"},
	{"text/plain", "txt"},
	{"text/xml", "xml"},
	{"application/x-javascript", "js"},
	{"application/octet-stream", " "},
};

static const struct vsfile_memfile_t vsfip_http400 =
{
	.file.name = "400",
	.file.size = sizeof("HTTP/1.0 400 Bad Request\r\n\r\nBad Request"),
	.file.attr = VSFILE_ATTR_READONLY,
	.file.op = (struct vsfile_fsop_t *)&vsfile_memfs_op,
	.f.buff = "HTTP/1.0 400 Bad Request\r\n\r\nBad Request",
};

static const struct vsfile_memfile_t vsfip_http404 =
{
	.file.name = "404",
	.file.size = sizeof("HTTP/1.0 404 Not Found\r\n\r\nNot Found"),
	.file.attr = VSFILE_ATTR_READONLY,
	.file.op = (struct vsfile_fsop_t *)&vsfile_memfs_op,
	.f.buff = "HTTP/1.0 404 Not Found\r\n\r\nNot Found",
};

static char* vsfip_httpd_getnextline(char *buf)
{
	buf = strstr(buf, "\r\n");
	if (buf)
	{
		buf += 2;
		if (!*buf)
			buf = NULL;
	}
	return buf;
}

static char* vsfip_httpd_gettypestr(char *filename)
{
	char *extname = vsfile_getfileext(filename);
	if (extname)
	{
		uint8_t i;
		for (i = 0; i < VSFIP_HTTPD_MIMETYPECNT; i++)
			if (strcmp(extname, vsfip_httpd_mimetype[i].ext) == 0)
				return vsfip_httpd_mimetype[i].str;
	}
	return vsfip_httpd_mimetype[VSFIP_HTTPD_MIMETYPECNT - 1].str;
}

static uint8_t vsfip_httpd_getmimetype(char *str)
{
	uint8_t i;
	for (i = 0; i < VSFIP_HTTPD_MIMETYPECNT; i++)
		if (strncmp(str, vsfip_httpd_mimetype[i].str, strlen(vsfip_httpd_mimetype[i].str)) == 0)
			return i;
	return VSFIP_HTTPD_MIMETYPECNT - 1;
}

char* vsfip_httpd_getarg(char *src, char *name, uint32_t *valuesize)
{
	uint32_t namesize = strlen(name);
	char *end;

	while (1)
	{
		end = strchr(src, '=');
		if (end == NULL)
			return NULL;
		if (memcmp(src, name, end - src) == 0)
		{
			end++;
			src = strchr(src, '&');
			*valuesize = (src == NULL) ? strlen(end) : src - end;
			return end;
		}
		else
		{
			src = strchr(src, '&');
			if (src == NULL)
				return NULL;
			src++;
		}
	}
}

static char vsfip_httpd_hex(char c)
{
	if ((c >= '0') && (c <= '9')) return c - '0';
	if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
	if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
	return 0;
}

static vsf_err_t vsfip_httpd_parse_req(struct vsfip_httpd_service_t *service,
										struct vsf_buffer_t *buf)
{
	char *rdptr = (char *)buf->buffer, *strtmp;

	if (!memcmp(rdptr, VSFIP_HTTP_HEAD_GET, sizeof(VSFIP_HTTP_HEAD_GET) - 1))
	{
		// Get a GET Requirst
		service->req.req = VSFIP_HTTP_GET;
		// move rdptr to destfile
		rdptr += sizeof(VSFIP_HTTP_HEAD_GET) - 1;
	}
	else
	if (!memcmp(rdptr, VSFIP_HTTP_HEAD_POST, sizeof(VSFIP_HTTP_HEAD_POST) - 1))
	{
		// Get a POST Requirst
		service->req.req = VSFIP_HTTP_POST;
		// move rdptr to destfile
		rdptr += sizeof(VSFIP_HTTP_HEAD_POST) - 1;
	}
	else
		return VSFERR_NOT_SUPPORT;

	strtmp = service->req.url = rdptr;
	while (!isspace(*rdptr))
	{
		if (*rdptr == '?')
		{
			*strtmp = '\0';
			strtmp = service->req.arg = ++rdptr;
		}
		else if (*rdptr == '%')
		{
			rdptr++;
			if ((rdptr[0] != '\0') && (rdptr[1] != '\0'))
			{
				*strtmp++ = (vsfip_httpd_hex(rdptr[0]) << 4) +
								vsfip_httpd_hex(rdptr[1]);
				rdptr += 2;
			}
			else
			{
				return VSFERR_FAIL;
			}
		}
		else
		{
			*strtmp++ = *rdptr++;
		}
	}
	*rdptr = '\0';
	if (!strcmp(service->req.url, "/"))
	{
		service->req.url = service->httpd->homepage;
	}

	rdptr = vsfip_httpd_getnextline(++rdptr);
	if (rdptr != NULL)
	{
		service->req.head = rdptr;
		service->req.body = strstr(rdptr, "\r\n\r\n");
		if (service->req.body != NULL)
			service->req.body += 4;
	}
	return VSFERR_NONE;
}

static vsf_err_t vsfip_httpd_parse_post(struct vsfip_httpd_service_req_t *req)
{
	char *rdptr = req->head;

	req->post.size = 0;
	req->post.type = 0xFF;
	while (rdptr && (!req->post.size || (0xFF == req->post.type)))
	{
		if (memcmp(rdptr, "Content-", sizeof("Content-") - 1) == 0)
		{
			rdptr += sizeof("Content-") - 1;
			if (memcmp(rdptr, "Length: ", sizeof("Length: ") - 1) == 0)
			{
				rdptr += sizeof("Length: ") - 1;
				req->post.size = atoi(rdptr);
			}
			else if (memcmp(rdptr, "Type: ", sizeof("Type: ") - 1) == 0)
			{
				rdptr += sizeof("Type: ") - 1;
				req->post.type = vsfip_httpd_getmimetype(rdptr);
			}
		}
		rdptr = vsfip_httpd_getnextline(rdptr);
	}
	return !req->post.size || (0xFF == req->post.type) ?
				VSFERR_FAIL : VSFERR_NONE;
}

vsf_err_t vsfip_httpd_header_resp(struct vsfip_httpd_service_resp_t *resp,
								enum vsfip_httpd_resp_t r)
{
	char *str;

	resp->resp = r;
	resp->outbuf = VSFIP_TCPBUF_GET(VSFIP_CFG_TCP_MSS);
	if (!resp->outbuf) return VSFERR_FAIL;

	str = (char *)resp->outbuf->app.buffer;
	sprintf(str, "HTTP/1.1 %d OK\r\nServer: vsfip/1.0\r\n", (int)r);
	return VSFERR_NONE;
}

vsf_err_t vsfip_httpd_header_str(struct vsfip_httpd_service_resp_t *resp,
								const char *field, const char *value)
{
	char *str = (char *)resp->outbuf->app.buffer;
	str += strlen(str);
	sprintf(str, "%s: %s\r\n", field, value);
	return VSFERR_NONE;
}

vsf_err_t vsfip_httpd_header_u32(struct vsfip_httpd_service_resp_t *resp,
								const char *field, uint32_t value)
{
	char *str = (char *)resp->outbuf->app.buffer;
	str += strlen(str);
	sprintf(str, "%s: %d\r\n", field, value);
	return VSFERR_NONE;
}

vsf_err_t vsfip_httpd_header_end(struct vsfip_httpd_service_resp_t *resp)
{
	char *str = (char *)resp->outbuf->app.buffer;
	strcat(str, "\r\n");
	resp->outbuf->app.size = strlen((const char *)resp->outbuf->app.buffer);
	return VSFERR_NONE;
}

static struct vsfip_httpd_urlhandler_t* vsfip_httpd_get_urlhandler(
			struct vsfip_httpd_urlhandler_t *handler, char *url)
{
	while ((handler != NULL) && (handler->url != NULL))
	{
		if (strcmp(handler->url, url) == 0)
		{
			return handler;
		}
		handler++;
	}
	return NULL;
}

static vsf_err_t
vsfip_httpd_service_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfip_httpd_service_t *service =
							(struct vsfip_httpd_service_t *)pt->user_data;
	struct vsfip_httpd_t *httpd = service->httpd;
	struct vsfip_httpd_service_req_t *req = &service->req;
	struct vsfip_httpd_service_resp_t *resp = &service->resp;
	vsf_err_t err = VSFERR_NONE;

	vsfsm_pt_begin(pt);

	memset(req, 0, sizeof(*req));
	service->caller_pt.sm = pt->sm;
	service->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_tcp_recv(&service->caller_pt, evt, service->so, &req->inbuf);
	if (err > 0) return err; else if (err < 0) goto exit;

	resp->outbuf = NULL;
	resp->target_filename = NULL;

	// TODO: there is potential BUG if '\0' is out of range
	req->inbuf->app.buffer[req->inbuf->app.size] = 0;
	err = vsfip_httpd_parse_req(service, &req->inbuf->app);
	if (err < 0)
	{
		resp->targetfile = (struct vsfile_t *)&vsfip_http400;
		goto reply;
	}

#ifdef HTTPD_DEBUG
	vsf_debug("->HTTP %s %s" VSFSHELL_LINEEND,
					service->req == VSFIP_HTTP_GET ? "GET" : "POST",
					service->targetfilename);
#endif

	if (httpd->cb.onca != NULL)
	{
		service->caller_pt.user_data = httpd->cb.param;
		service->caller_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = httpd->cb.onca(&service->caller_pt, evt, service);
		if (err > 0) return err; else if (err < 0) goto exit;
	}

	switch (req->req)
	{
	case VSFIP_HTTP_GET:
		resp->target_filename = req->url;
		break;
	case VSFIP_HTTP_POST:
		if (vsfip_httpd_parse_post(req))
		{
			resp->targetfile = (struct vsfile_t *)&vsfip_http400;
			goto reply;
		}
		break;
	}

	resp->handler = vsfip_httpd_get_urlhandler(httpd->urlhandler, req->url);
	if (resp->handler != NULL)
	{
		service->caller_pt.user_data = resp->handler->param;
		service->caller_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = resp->handler->handle(&service->caller_pt, evt, service);
		if (err > 0) return err; else if (err < 0) goto exit;
	}

reply:
	// send response, can sen target_file(with outbuf) or outbuf only
	if ((resp->target_filename != NULL) && !resp->targetfile)
	{
		// jump / in "/index.htm"
		resp->target_filename += 1;
		service->caller_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile_getfile(&service->caller_pt, evt, httpd->root,
								resp->target_filename, &resp->targetfile);
		if (err > 0) return err; else if (err < 0)
		{
			resp->resp = VSFIP_HTTP_404_NOTFOUND;
			resp->targetfile = (struct vsfile_t *)&vsfip_http404;
		}
		else
		{
			resp->resp = VSFIP_HTTP_200_OK;
		}
	}

	if (resp->targetfile != NULL)
	{
#ifdef HTTPD_DEBUG
		vsf_debug("<-HTTP %d targetfile: %s" VSFSHELL_LINEEND,
					service->rsp, service->targetfile->name);
#endif

		// for 200 OK response, we can set the header
		if (!resp->outbuf && (VSFIP_HTTP_200_OK == resp->resp))
		{
			if (vsfip_httpd_header_resp(resp, VSFIP_HTTP_200_OK))
				goto exit;
			vsfip_httpd_header_u32(resp, "Content-Length",
								(uint32_t)resp->targetfile->size);
			vsfip_httpd_header_str(resp, "Content-Type",
								vsfip_httpd_gettypestr(resp->targetfile->name));
			vsfip_httpd_header_end(resp);
		}

		resp->fileoffset = 0;
		while (resp->fileoffset < resp->targetfile->size)
		{
			if (!resp->outbuf)
			{
				resp->outbuf = VSFIP_TCPBUF_GET(VSFIP_CFG_TCP_MSS);
				if (!resp->outbuf)
					goto exit;
				resp->outbuf->app.size = 0;
			}

			service->caller_pt.state = 0;
			vsfsm_pt_entry(pt);
			{
				uint32_t cursize = resp->outbuf->app.size;
				uint32_t remain = VSFIP_CFG_TCP_MSS - cursize;
				uint8_t *dest = &resp->outbuf->app.buffer[cursize];
				uint32_t rsize;

				err = vsfile_read(&service->caller_pt, evt,
					resp->targetfile, resp->fileoffset, remain, dest, &rsize);
				if (err > 0) return err; else if (err < 0)
				{
					goto exit;
				}
				resp->outbuf->app.size += rsize;
				resp->fileoffset += rsize;
			}

			service->caller_pt.state = 0;
			vsfsm_pt_entry(pt);
			err = vsfip_tcp_send(&service->caller_pt, evt, service->so,
					resp->outbuf, false);
			if (err > 0) return err;
			resp->outbuf = NULL;
			if (err < 0) goto exit;
		}

		service->caller_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile_close(&service->caller_pt, evt, resp->targetfile);
		if (err > 0) return err; else if (err < 0) goto exit;
	}
	else if (resp->outbuf != NULL)
	{
		service->caller_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfip_tcp_send(&service->caller_pt, evt, service->so,
				 resp->outbuf, false);
		if (err > 0) return err;
		resp->outbuf = NULL;
		if (err < 0) goto exit;
	}

exit:
	if (resp->outbuf != NULL)
	{
		vsfip_buffer_release(resp->outbuf);
		resp->outbuf = NULL;
	}
	if (req->inbuf != NULL)
	{
		vsfip_buffer_release(req->inbuf);
		req->inbuf = NULL;
	}

	service->caller_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_tcp_close(&service->caller_pt, evt,service->so);
	if (err > 0) return err;

	vsfip_close(service->so);
	service->so = NULL;

	vsfsm_pt_end(pt);
	return VSFERR_FAIL;
}

static vsf_err_t vsfip_httpd_attach2service(
			struct vsfip_httpd_t *httpd, struct vsfip_socket_t *acceptso)
{
	struct vsfip_httpd_service_t *service;
	uint8_t i;

	for (i = 0; i < httpd->service_num; i++)
	{
		if (httpd->service[i].so == NULL)
		{
			service = &httpd->service[i];
			memset(service, 0, sizeof(struct vsfip_httpd_service_t));

			service->so = acceptso;
			service->httpd = httpd;

			service->pt.thread = vsfip_httpd_service_thread;
			service->pt.user_data = service;
			return vsfsm_pt_init(&service->sm, &service->pt);
		}
	}
	return VSFERR_NOT_ENOUGH_RESOURCES;
}

static vsf_err_t vsfip_httpd_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfip_httpd_t *httpd = (struct vsfip_httpd_t *)pt->user_data;
	vsf_err_t err = VSFERR_NONE;
	struct vsfip_socket_t *acceptso;

	vsfsm_pt_begin(pt);

	httpd->so = vsfip_socket(AF_INET, IPPROTO_TCP);
	httpd->so->rx_timeout_ms = 0;
	httpd->so->tx_timeout_ms = VSFIP_HTTP_SERVER_SOCKETTIMEOUT;

	if ((vsfip_bind(httpd->so, httpd->sockaddr.sin_port) < 0) ||
		(vsfip_listen(httpd->so, httpd->service_num) < 0))
	{
		goto close;
	}

	httpd->daemon_pt.sm = pt->sm;
	while (httpd->isactive)
	{
		httpd->daemon_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfip_tcp_accept(&httpd->daemon_pt, evt, httpd->so, &acceptso);
		if (err > 0) return err; else if (err < 0) httpd->isactive = false; else
		{
			err = vsfip_httpd_attach2service(httpd, acceptso);
			if (err != VSFERR_NONE)
			{
				httpd->acceptso = acceptso;
				httpd->daemon_pt.state = 0;
				vsfsm_pt_entry(pt);
				err = vsfip_tcp_close(&httpd->daemon_pt, evt, httpd->acceptso);
				if (err > 0) return err;
				vsfip_close(httpd->acceptso);
				httpd->acceptso = NULL;
			}
		}
	}

	vsfip_close(httpd->so);
	httpd->so = NULL;
	vsfsm_pt_end(pt);
close:
	return VSFERR_NONE;
}

vsf_err_t vsfip_httpd_start(struct vsfip_httpd_t *httpd, uint16_t port)
{
	if (NULL == httpd)
	{
		return VSFERR_FAIL;
	}

	httpd->sockaddr.sin_port = port;
	httpd->isactive = true;
	httpd->sockaddr.sin_addr.size = 4;

	httpd->pt.thread = vsfip_httpd_thread;
	httpd->pt.user_data = httpd;
	vsfsm_pt_init(&httpd->sm, &httpd->pt);
	return VSFERR_NONE;
}

