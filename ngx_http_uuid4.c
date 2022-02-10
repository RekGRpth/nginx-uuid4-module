/* (C) 2015 Cybozu.  All rights reserved. */

#include <ndk_set_var.h>
#include <ngx_http.h>
#include <uuid/uuid.h>

#define UUID_STR_LENGTH 36

static ngx_int_t ngx_http_uuid4_func(ngx_http_request_t *r, ngx_str_t *val) {
    if (!(val->data = ngx_pnalloc(r->pool, UUID_STR_LENGTH + 1))) { ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "!ngx_pnalloc"); return NGX_ERROR; }
    val->len = UUID_STR_LENGTH;
    uuid_t uu;
    uuid_generate_random(uu);
    uuid_unparse(uu, (char *)val->data);
    return NGX_OK;
}

static ngx_command_t ngx_http_uuid4_commands[] = {
  { .name = ngx_string("set_uuid4"),
    .type = NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    .set = ndk_set_var_value,
    .conf = NGX_HTTP_LOC_CONF_OFFSET,
    .offset = 0,
    .post = &(ndk_set_var_t){ NDK_SET_VAR_BASIC, ngx_http_uuid4_func, 0, NULL } },
    ngx_null_command
};

static ngx_http_module_t ngx_http_uuid4_ctx = {
    .preconfiguration = NULL,
    .postconfiguration = NULL,
    .create_main_conf = NULL,
    .init_main_conf = NULL,
    .create_srv_conf = NULL,
    .merge_srv_conf = NULL,
    .create_loc_conf = NULL,
    .merge_loc_conf = NULL
};

ngx_module_t ngx_http_uuid4_module = {
    NGX_MODULE_V1,
    .ctx = &ngx_http_uuid4_ctx,
    .commands = ngx_http_uuid4_commands,
    .type = NGX_HTTP_MODULE,
    .init_master = NULL,
    .init_module = NULL,
    .init_process = NULL,
    .init_thread = NULL,
    .exit_thread = NULL,
    .exit_process = NULL,
    .exit_master = NULL,
    NGX_MODULE_V1_PADDING
};
