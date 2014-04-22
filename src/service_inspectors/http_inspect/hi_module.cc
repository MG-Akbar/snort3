/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// hi_module.cc author Russ Combs <rucombs@cisco.com>

#include "hi_module.h"

#include <string>

#include "hi_ui_config.h"
#include "hi_events.h"
#include "hi_cmd_lookup.h"
#include "hi_ui_iis_unicode_map.h"
#include "utils/util.h"

//-------------------------------------------------------------------------
// http_inspect module
//-------------------------------------------------------------------------

// these are shared
static const Parameter hi_umap_params[] =
{
    { "code_page", Parameter::PT_INT, "0:", "1252",
      "help" },

    { "map_file", Parameter::PT_STRING, nullptr, nullptr,
      "help" },

    { nullptr, Parameter::PT_MAX, nullptr, nullptr, nullptr }
};

static const Parameter hi_decode_params[] =
{
    { "b64_decode_depth", Parameter::PT_INT, "-1:65535", "0",
      "single packet decode depth" },

    { "bitenc_decode_depth", Parameter::PT_INT, "-1:65535", "0",
      "single packet decode depth" },

    { "max_mime_mem", Parameter::PT_INT, "3276:", "838860",
      "single packet decode depth" },

    { "qp_decode_depth", Parameter::PT_INT, "-1:65535", "0",
      "single packet decode depth" },

    { "uu_decode_depth", Parameter::PT_INT, "-1:65535", "0",
      "single packet decode depth" },

    { nullptr, Parameter::PT_MAX, nullptr, nullptr, nullptr }
};

static const Parameter hi_global_params[] =
{
    { "compress_depth", Parameter::PT_INT, "1:65535", "1460",
      "maximum amount of packet payload to decompress" },

    { "decode", Parameter::PT_TABLE, nullptr, hi_decode_params,
      "help" },

    { "decompress_depth", Parameter::PT_INT, "1:65535", "2920",
      "maximum amount of decompressed data to process" },

    { "detect_anomalous_servers", Parameter::PT_BOOL, nullptr, "false",
      "inspect non-configured ports for HTTP - bad idea" },

    { "max_gzip_mem", Parameter::PT_INT, "3276:", "838860",
      "total memory used for decompression across all active sessions" },

    { "memcap", Parameter::PT_INT, "2304:", "150994944",
      "limit of memory used for logging extra data" },

    //{ "mime", Parameter::PT_TABLE, nullptr, hi_mime_params,
    //  "help" },

    { "proxy_alert", Parameter::PT_BOOL, nullptr, "false",
      "alert on proxy usage for servers without allow_proxy_use" },

    { "unicode_map", Parameter::PT_TABLE, nullptr, hi_umap_params,
      "help" },

    { nullptr, Parameter::PT_MAX, nullptr, nullptr, nullptr }
};

static const RuleMap hi_global_rules[] =
{
    { HI_CLIENT_ASCII, HI_CLIENT_ASCII_STR },
    { HI_CLIENT_DOUBLE_DECODE, HI_CLIENT_DOUBLE_DECODE_STR },
    { HI_CLIENT_U_ENCODE, HI_CLIENT_U_ENCODE_STR },
    { HI_CLIENT_BARE_BYTE, HI_CLIENT_BARE_BYTE_STR },
    { HI_CLIENT_BASE36, HI_CLIENT_BASE36_STR },
    { HI_CLIENT_UTF_8,  HI_CLIENT_UTF_8_STR },
    { HI_CLIENT_IIS_UNICODE, HI_CLIENT_IIS_UNICODE_STR },
    { HI_CLIENT_MULTI_SLASH, HI_CLIENT_MULTI_SLASH_STR },
    { HI_CLIENT_IIS_BACKSLASH,  HI_CLIENT_IIS_BACKSLASH_STR },
    { HI_CLIENT_SELF_DIR_TRAV, HI_CLIENT_SELF_DIR_TRAV_STR },
    { HI_CLIENT_DIR_TRAV, HI_CLIENT_DIR_TRAV_STR },
    { HI_CLIENT_APACHE_WS, HI_CLIENT_APACHE_WS_STR },
    { HI_CLIENT_IIS_DELIMITER, HI_CLIENT_IIS_DELIMITER_STR },
    { HI_CLIENT_NON_RFC_CHAR, HI_CLIENT_NON_RFC_CHAR_STR },
    { HI_CLIENT_OVERSIZE_DIR, HI_CLIENT_OVERSIZE_DIR_STR },
    { HI_CLIENT_LARGE_CHUNK, HI_CLIENT_LARGE_CHUNK_STR },
    { HI_CLIENT_PROXY_USE, HI_CLIENT_PROXY_USE_STR },
    { HI_CLIENT_WEBROOT_DIR, HI_CLIENT_WEBROOT_DIR_STR },
    { HI_CLIENT_LONG_HDR, HI_CLIENT_LONG_HDR_STR },
    { HI_CLIENT_MAX_HEADERS, HI_CLIENT_MAX_HEADERS_STR },
    { HI_CLIENT_MULTIPLE_CONTLEN, HI_CLIENT_MULTIPLE_CONTLEN_STR },
    { HI_CLIENT_CHUNK_SIZE_MISMATCH, HI_CLIENT_CHUNK_SIZE_MISMATCH_STR },
    { HI_CLIENT_INVALID_TRUEIP, HI_CLIENT_INVALID_TRUEIP_STR },
    { HI_CLIENT_MULTIPLE_HOST_HDRS, HI_CLIENT_MULTIPLE_HOST_HDRS_STR },
    { HI_CLIENT_LONG_HOSTNAME, HI_CLIENT_LONG_HOSTNAME_STR },
    { HI_CLIENT_EXCEEDS_SPACES, HI_CLIENT_EXCEEDS_SPACES_STR },
    { HI_CLIENT_CONSECUTIVE_SMALL_CHUNKS, HI_CLIENT_CONSECUTIVE_SMALL_CHUNKS_STR },
    { HI_CLIENT_UNBOUNDED_POST, HI_CLIENT_UNBOUNDED_POST_STR },
    { HI_CLIENT_MULTIPLE_TRUEIP_IN_SESSION, HI_CLIENT_MULTIPLE_TRUEIP_IN_SESSION_STR },
    { HI_CLIENT_BOTH_TRUEIP_XFF_HDRS, HI_CLIENT_BOTH_TRUEIP_XFF_HDRS_STR },
    { HI_CLIENT_UNKNOWN_METHOD, HI_CLIENT_UNKNOWN_METHOD_STR },
    { HI_CLIENT_SIMPLE_REQUEST, HI_CLIENT_SIMPLE_REQUEST_STR },
    { HI_CLIENT_UNESCAPED_SPACE_URI, HI_CLIENT_UNESCAPED_SPACE_URI_STR },
    { HI_CLIENT_PIPELINE_MAX, HI_CLIENT_PIPELINE_MAX_STR },
    { 0, nullptr }
};

HttpGlobalModule::HttpGlobalModule() :
    Module(GLOBAL_KEYWORD, hi_global_params, hi_global_rules)
{
    config = nullptr;
}

HttpGlobalModule::~HttpGlobalModule()
{
    if ( config )
        delete config;
}

HTTPINSPECT_GLOBAL_CONF* HttpGlobalModule::get_data()
{
    HTTPINSPECT_GLOBAL_CONF* tmp = config;
    config = nullptr;
    return tmp;
}

bool HttpGlobalModule::set(const char*, Value& v, SnortConfig*)
{
    if ( v.is("b64_decode_depth") )
        config->decode_conf.b64_depth = v.get_long();

    else if ( v.is("bitenc_decode_depth") )
        config->decode_conf.bitenc_depth = v.get_long();

    else if ( v.is("code_page") )
        config->iis_unicode_codepage = v.get_long();

    else if ( v.is("compress_depth") )
        config->compr_depth = v.get_long();

    else if ( v.is("decompress_depth") )
        config->decompr_depth = v.get_long();

    else if ( v.is("detect_anomalous_servers") )
        config->anomalous_servers = v.get_bool();

    else if ( v.is("map_file") )
        config->iis_unicode_map_filename = SnortStrdup(v.get_string());

    else if ( v.is("max_gzip_mem") )
        config->max_gzip_mem = v.get_long();

    else if ( v.is("max_mime_mem") )
        config->decode_conf.max_mime_mem = v.get_long();

    else if ( v.is("memcap") )
        config->memcap = v.get_long();

    else if ( v.is("proxy_alert") )
        config->proxy_alert = v.get_bool();

    else if ( v.is("qp_decode_depth") )
        config->decode_conf.qp_depth = v.get_long();

    else if ( v.is("uu_decode_depth") )
        config->decode_conf.uu_depth = v.get_long();

    else
        return false;

    return true;
}

bool HttpGlobalModule::begin(const char*, int, SnortConfig*)
{
    if ( !config )
        config = new HTTPINSPECT_GLOBAL_CONF;

    return true;
}

bool HttpGlobalModule::end(const char* fqn, int, SnortConfig*)
{
    if ( strcmp(fqn, GLOBAL_KEYWORD) )
        return true;

    if ( config->iis_unicode_map_filename )
    {
        hi_ui_parse_iis_unicode_map(
            &config->iis_unicode_map,
            config->iis_unicode_map_filename,
            config->iis_unicode_codepage);
    }
    else
    {
        get_default_unicode_map(
            config->iis_unicode_map,
            config->iis_unicode_codepage);
    }
    return true;
}

//-------------------------------------------------------------------------
// http_server module
//-------------------------------------------------------------------------

static const char* profiles = "none | all | apache | iis | iis_40 | iis_50";

// FIXIT refactor params to create a profile table so that user can define
// different profiles (like above) and use those.  rename existing profile
// to profile_type.

static const Parameter hi_server_params[] =
{
    { "allow_proxy_use", Parameter::PT_BOOL, nullptr, "false",
      "don't alert on proxy use for this server" },

    { "apache_whitespace", Parameter::PT_BOOL, nullptr, "true",
      "don't alert if tab is used in lieu of space characters" },

    { "ascii", Parameter::PT_BOOL, nullptr, "true",
      "enable decoding ASCII like %2f to /" },

    { "bare_byte", Parameter::PT_BOOL, nullptr, "false",
      "decode non-standard, non-ASCII character encodings" },

    { "chunk_length", Parameter::PT_INT, "1:", "500000",
      "alert on chunk lengths greater than specified" },

    { "client_flow_depth", Parameter::PT_INT, "-1:1460", "300",
      "raw request payload to inspect" },

    { "directory", Parameter::PT_BOOL, nullptr, "true",
      "normalize . and .. sequences out of URI" },

    { "double_decode", Parameter::PT_BOOL, nullptr, "false",
      "iis specific extra decoding" },

    { "enable_cookies", Parameter::PT_BOOL, nullptr, "false",
      "extract cookies" },

    { "enable_xff", Parameter::PT_BOOL, nullptr, "false",
      "log True-Client-IP and X-Forwarded-For headers with unified2 alerts as extra data" },

    { "extended_ascii_uri", Parameter::PT_BOOL, nullptr, "false",
      "help" },

    { "extended_response_inspection", Parameter::PT_BOOL, nullptr, "false",
      "extract resonse headers" },

    { "http_methods", Parameter::PT_STRING, nullptr, nullptr,
      "request methods allowed in addition to GET and POST" },

    { "iis_backslash", Parameter::PT_BOOL, nullptr, "false",
      "normalize directory slashes" },

    { "iis_delimiter", Parameter::PT_BOOL, nullptr, "true",
      "allow use of non-standard delimiter" },

    { "iis_unicode", Parameter::PT_BOOL, nullptr, "false",
      "enable unicode code point mapping using unicode_map settings" },

    { "iis_unicode_map", Parameter::PT_TABLE, nullptr, hi_umap_params,
      "help" },

    { "inspect_gzip", Parameter::PT_BOOL, nullptr, "false",
      "enable gzip decompression of compressed bodies" },

    { "inspect_uri_only", Parameter::PT_BOOL, nullptr, "false",
      "disable all detection except for uricontent" },

    { "log_hostname", Parameter::PT_BOOL, nullptr, "false",
      "enable logging of Hostname with unified2 alerts as extra data" },

    { "log_uri", Parameter::PT_BOOL, nullptr, "false",
      "enable logging of URI with unified2 alerts as extra data" },

    { "max_header_length", Parameter::PT_INT, "0:65535", "0",
      "maximum allowed client request header field" },

    { "max_headers", Parameter::PT_INT, "0:1024", "0",
      "maximum allowd client request headers" },

    { "max_spaces", Parameter::PT_INT, "0:65535", "200",
      "help" },

    { "multi_slash", Parameter::PT_BOOL, nullptr, "true",
      "normalize out consecutive slashes in URI" },

    { "no_pipeline_req", Parameter::PT_BOOL, nullptr, "false",
      "don't inspect pipelined requests after first (still does general detection)" },

    { "non_rfc_chars", Parameter::PT_BIT_LIST, "255", "false",
      "alert on given non-RFC chars being present in the URI" },

    { "non_strict", Parameter::PT_BOOL, nullptr, "true",
      "allows HTTP 0.9 processing" },

    { "normalize_cookies", Parameter::PT_BOOL, nullptr, "false",
      "help" },

    { "normalize_headers", Parameter::PT_BOOL, nullptr, "false",
      "help" },

    { "normalize_javascript", Parameter::PT_BOOL, nullptr, "false",
      "normalize javascript between <script> tags" },

    { "max_javascript_whitespaces", Parameter::PT_INT, "0:", "200",
      "maximum number of consecutive whitespaces" },

    { "normalize_utf", Parameter::PT_BOOL, nullptr, "false",
      "help" },

    { "oversize_dir_length", Parameter::PT_INT, "0:", "0",
      "alert if a URL has a directory longer than this limit" },

    { "ports", Parameter::PT_BIT_LIST, "65535", "80",
      "ports to inspect" },

    { "post_depth", Parameter::PT_INT, "-1:65535", "-1",
      "amount of POST data to inspect" },

    { "profile", Parameter::PT_ENUM, profiles, "none",
      "set defaults appropriate for selected server" },

    { "server_flow_depth", Parameter::PT_INT, "-1:65535", "300",
      "response payload to inspect; includes headers with extended_response_inspection" },

    { "small_chunk_count", Parameter::PT_INT, "0:255", "0",
      "alert if more than this limit of consecutive chunks are below small_chunk_length" },

    { "small_chunk_length", Parameter::PT_INT, "0:255", "0",
      "alert if more than small_chunk_count consecutive chunks below this limit" },

    { "tab_uri_delimiter", Parameter::PT_BOOL, nullptr, "false",
      "help" },

    { "u_encode", Parameter::PT_BOOL, nullptr, "false",
      "decode %uXXXX character sequences" },

    { "unicode_map", Parameter::PT_TABLE, nullptr, hi_umap_params,
      "help" },

    { "unlimited_decompress", Parameter::PT_INT, nullptr, "false",
      "decompress across multiple packets" },

    { "utf_8", Parameter::PT_BOOL, nullptr, "true",
      "decode UTF-8 unicode sequences in URI" },

    { "webroot", Parameter::PT_BOOL, nullptr, "true",
      "alert on directory traversals past the top level (web server root)" },

    { "whitespace_chars", Parameter::PT_BIT_LIST, "255", "false",
      "help" },

    { nullptr, Parameter::PT_MAX, nullptr, nullptr, nullptr }
};

static const RuleMap hi_server_rules[] =
{
    { HI_ANOM_SERVER, HI_ANOM_SERVER_STR },
    { HI_SERVER_INVALID_STATCODE, HI_SERVER_INVALID_STATCODE_STR },
    { HI_SERVER_NO_CONTLEN, HI_SERVER_NO_CONTLEN_STR },
    { HI_SERVER_UTF_NORM_FAIL, HI_SERVER_UTF_NORM_FAIL_STR },
    { HI_SERVER_UTF7, HI_SERVER_UTF7_STR },
    { HI_SERVER_DECOMPR_FAILED, HI_SERVER_DECOMPR_FAILED_STR },
    { HI_SERVER_CONSECUTIVE_SMALL_CHUNKS, HI_SERVER_CONSECUTIVE_SMALL_CHUNKS_STR },
    { HI_CLISRV_MSG_SIZE_EXCEPTION, HI_CLISRV_MSG_SIZE_EXCEPTION_STR },
    { HI_SERVER_JS_OBFUSCATION_EXCD, HI_SERVER_JS_OBFUSCATION_EXCD_STR },
    { HI_SERVER_JS_EXCESS_WS, HI_SERVER_JS_EXCESS_WS_STR },
    { HI_SERVER_MIXED_ENCODINGS, HI_SERVER_MIXED_ENCODINGS_STR },
    { 0, nullptr }
};

HttpServerModule::HttpServerModule() :
    Module(SERVER_KEYWORD, hi_server_params, hi_server_rules)
{ 
    server = nullptr;
}

HttpServerModule::~HttpServerModule()
{
    if ( server )
        delete server;
}

HTTPINSPECT_CONF* HttpServerModule::get_data()
{
    HTTPINSPECT_CONF* tmp = server;
    server = nullptr;
    return tmp;
}

bool HttpServerModule::set(const char*, Value& v, SnortConfig*)
{
    if ( v.is("allow_proxy_use") )
        server->allow_proxy = v.get_bool();

    else if ( v.is("apache_whitespace") )
        server->apache_whitespace.on = v.get_bool();

    else if ( v.is("ascii") )
        server->ascii.on = v.get_bool();

    else if ( v.is("bare_byte") )
        server->bare_byte.on = v.get_bool();

    else if ( v.is("chunk_length") )
        server->chunk_length = v.get_long();

    else if ( v.is("client_flow_depth") )
        server->client_flow_depth = v.get_long();

    else if ( v.is("code_page") )
        server->iis_unicode_codepage = v.get_long();

    else if ( v.is("directory") )
        server->directory.on = v.get_bool();

    else if ( v.is("double_decode") )
        server->double_decoding.on = v.get_bool();

    else if ( v.is("enable_cookies") )
        server->enable_cookie = v.get_bool();

    else if ( v.is("extended_ascii_uri") )
        server->extended_ascii_uri = v.get_bool();

    else if ( v.is("extended_response_inspection") )
        server->inspect_response = v.get_bool();

    else if ( v.is("enable_xff") )
        server->enable_xff = v.get_bool();

    else if ( v.is("http_methods") )
    {
        std::string tok;
        v.set_first_token();

        while ( v.get_next_token(tok) )
        {
            char* s = SnortStrdup(tok.c_str());
            http_cmd_lookup_add(server->cmd_lookup, s, strlen(s), (HTTP_CMD_CONF*)s);
        }
    }
    else if ( v.is("iis_backslash") )
        server->iis_backslash.on = v.get_bool();

    else if ( v.is("iis_delimiter") )
        server->iis_delimiter.on = v.get_bool();

    else if ( v.is("iis_unicode") )
        server->iis_unicode.on = v.get_bool();

    else if ( v.is("inspect_gzip") )
        server->extract_gzip = v.get_bool();

    else if ( v.is("inspect_uri_only") )
        server->uri_only = v.get_bool();

    else if ( v.is("log_hostname") )
        server->log_hostname = v.get_bool();

    else if ( v.is("log_uri") )
        server->log_uri = v.get_bool();

    else if ( v.is("map_file") )
        server->iis_unicode_map_filename = SnortStrdup(v.get_string());

    else if ( v.is("max_header_length") )
        server->max_hdr_len = v.get_long();

    else if ( v.is("max_headers") )
        server->max_headers = v.get_long();

    else if ( v.is("max_javascript_whitespaces") )
        server->max_js_ws = v.get_long();

    else if ( v.is("max_spaces") )
        server->max_spaces = v.get_long();

    else if ( v.is("multi_slash") )
        server->multiple_slash.on = v.get_bool();

    else if ( v.is("no_pipeline_req") )
        server->no_pipeline = v.get_bool();

    else if ( v.is("non_rfc_chars") )
        v.get_bits(server->non_rfc_chars);

    else if ( v.is("non_strict") )
        server->non_strict = v.get_bool();

    else if ( v.is("normalize_cookies") )
        server->normalize_cookies = v.get_bool();

    else if ( v.is("normalize_headers") )
        server->normalize_headers = v.get_bool();

    else if ( v.is("normalize_javascript") )
        server->normalize_javascript = v.get_bool();

    else if ( v.is("normalize_utf") )
        server->normalize_utf = v.get_bool();

    else if ( v.is("oversize_dir_length") )
        server->long_dir = v.get_long();

    else if ( v.is("ports") )
        v.get_bits(server->ports);

    else if ( v.is("post_depth") )
        server->post_depth = v.get_long();

    else if ( v.is("profile") )
        server->profile = (PROFILES)v.get_long();

    else if ( v.is("server_flow_depth") )
        server->server_flow_depth = v.get_long();

    else if ( v.is("small_chunk_count") )
        server->small_chunk_length.num = v.get_long();

    else if ( v.is("small_chunk_length") )
        server->small_chunk_length.size = v.get_long();

    else if ( v.is("tab_uri_delimiter") )
        server->tab_uri_delimiter = v.get_bool();

    else if ( v.is("u_encode") )
        server->u_encoding.on = v.get_bool();

    else if ( v.is("unlimited_decompress") )
        server->unlimited_decompress = v.get_bool();

    else if ( v.is("utf_8") )
        server->utf_8.on = v.get_bool();

    else if ( v.is("webroot") )
        server->webroot.on = v.get_bool();

    else if ( v.is("whitespace_chars") )
        v.get_bits(server->whitespace);

    else
        return false;

    return true;
}

bool HttpServerModule::begin(const char*, int, SnortConfig*)
{
    if ( !server )
        server = new HTTPINSPECT_CONF;

    return true;
}

bool HttpServerModule::end(const char* fqn, int, SnortConfig*)
{
    if ( strcmp(fqn, SERVER_KEYWORD) )
        return true;
    
    if ( server->iis_unicode_map_filename )
    {
        hi_ui_parse_iis_unicode_map(
            &server->iis_unicode_map,
            server->iis_unicode_map_filename,
            server->iis_unicode_codepage);
    }
    return true;
}
