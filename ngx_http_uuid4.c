/* (C) 2015 Cybozu.  All rights reserved. */

#include <ngx_http.h>
#include "mt19937/mt64.h"

#define UUID_STR_LENGTH 36
#define SEED_LENGTH 312

static int mt_initialized = 0;

static ngx_int_t initialize_mt(ngx_http_request_t *r) {
    unsigned long long  seed[SEED_LENGTH];
    FILE *f = fopen("/dev/urandom", "r");
    if (!f) { ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "!fopen"); return NGX_ERROR; }
    size_t n = fread(seed, sizeof(unsigned long long), SEED_LENGTH, f);
    if (n < SEED_LENGTH) { ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "fread < SEED_LENGTH"); fclose(f); return NGX_ERROR; }
    fclose(f);
    init_by_array64(seed, SEED_LENGTH);
    return NGX_OK;
}

static ngx_int_t ngx_http_uuid4_func(ngx_http_request_t *r, ngx_str_t *val) {
    if (!mt_initialized) {
        if (initialize_mt(r) != NGX_OK) { ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "initialize_mt != NGX_OK"); return NGX_ERROR; }
        mt_initialized = 1;
    }
    uint64_t upper = (uint64_t)genrand64_int64();
    uint64_t lower = (uint64_t)genrand64_int64();
    upper &= ~((1ULL << 12) | (1ULL << 13) | (1ULL << 15));
    upper |= (1ULL << 14);
    lower &= ~(1ULL << 62);
    lower |= (1ULL << 63);
    val->len = UUID_STR_LENGTH;
    if (!(val->data = ngx_pnalloc(r->pool, UUID_STR_LENGTH))) { ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "!ngx_pnalloc"); return NGX_ERROR; }
    ngx_snprintf(val->data, UUID_STR_LENGTH, "%08uxL-%04uxL-%04uxL-%04uxL-%012uxL", upper >> 32, (upper >> 16) & 0xFFFFULL, upper & 0xFFFFULL, lower >> 48, lower & 0xFFFFFFFFFFFFULL);
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
