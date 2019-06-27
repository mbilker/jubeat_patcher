#define LOG_MODULE "util-options"

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"
#include "defs.h"
#include "fs.h"
#include "hex.h"
#include "log.h"
#include "mem.h"
#include "options.h"
#include "str.h"

static struct util_options_opts* util_options_get(int argc, char** argv,
    const struct util_options_defs* option_defs);
static bool util_options_init_from_file(struct util_options_opts* options,
    const struct util_options_defs* option_defs, char* file_path);
static bool util_options_write_default_options_file(
        const struct util_options_defs* option_defs, char* file_path);
static void util_options_print(const char* fmt, ...);

static FILE* util_options_usage_print_out;

void util_options_set_usage_output(FILE* out)
{
    util_options_usage_print_out = out;
}

struct util_options_opts* util_options_init(int argc, char** argv,
        const struct util_options_defs* option_defs)
{
    struct util_options_opts* options;

    options = util_options_get(argc, argv, option_defs);

    if (!options) {
        exit(0);
    }

    return options;
}

struct util_options_opts* util_options_init_from_cmdline(
        const struct util_options_defs* option_defs)
{
    int argc;
    char **argv;
    struct util_options_opts* options;

    args_recover(&argc, &argv);

    options = util_options_get(argc, argv, option_defs);

    args_free(argc, argv);

    if (!options) {
        exit(0);
    }

    return options;
}

int util_options_get_int(const struct util_options_opts* opts,
        const char* name)
{
    for (uint32_t i = 0; i < opts->entries; i++) {

        if (!strcmp(opts->defs->defs[i]->name, name)) {

            if (opts->defs->defs[i]->type != UTIL_OPTIONS_TYPE_INT) {
                log_warning("Option %s is not of type int", name);
                return 0;
            }

            if (opts->values[i].avail) {
                return opts->values[i].value.i;
            } else {
                return opts->defs->defs[i]->default_value.i;
            }
        }
    }

    log_warning("Could not find int option %s", name);
    return 0;
}

const char* util_options_get_str(const struct util_options_opts* opts,
        const char* name)
{
    for (uint32_t i = 0; i < opts->entries; i++) {

        if (!strcmp(opts->defs->defs[i]->name, name)) {

            if (opts->defs->defs[i]->type != UTIL_OPTIONS_TYPE_STR) {
                log_warning("Option %s is not of type str", name);
                return 0;
            }

            if (opts->values[i].avail) {
                return opts->values[i].value.str;
            } else {
                return opts->defs->defs[i]->default_value.str;
            }
        }
    }

    log_warning("Could not find str option %s", name);
    return NULL;
}

bool util_options_get_bool(const struct util_options_opts* opts,
        const char* name)
{
    for (uint32_t i = 0; i < opts->entries; i++) {

        if (!strcmp(opts->defs->defs[i]->name, name)) {

            if (opts->defs->defs[i]->type != UTIL_OPTIONS_TYPE_BOOL) {
                log_warning("Option %s is not of type bool", name);
                return false;
            }

            if (opts->values[i].avail) {
                return opts->values[i].value.b;
            } else {
                return opts->defs->defs[i]->default_value.b;
            }
        }
    }

    log_warning("Could not find bool option %s", name);
    return false;
}

const uint8_t* util_options_get_bin(const struct util_options_opts* opts,
        const char* name, size_t* length)
{
    for (uint32_t i = 0; i < opts->entries; i++) {

        if (!strcmp(opts->defs->defs[i]->name, name)) {

            if (opts->defs->defs[i]->type != UTIL_OPTIONS_TYPE_BIN) {
                log_warning("Option %s is not of type bin", name);
                return 0;
            }

            if (opts->values[i].avail) {
                if (length) {
                    *length = opts->values[i].value.bin.len;
                }

                return opts->values[i].value.bin.data;
            } else {
                if (length) {
                    *length = opts->defs->defs[i]->default_value.bin.len;
                }

                return opts->defs->defs[i]->default_value.bin.data;
            }
        }
    }

    log_warning("Could not find bin option %s", name);
    return NULL;
}

void util_options_print_usage(const struct util_options_defs* option_defs)
{
    util_options_print(
        "%s\nSpecify options as arguments or uss reboot_connection.conf to "
        "specify a config file containing key=value per line entries\n",
        option_defs->usage_header);

    for (uint32_t i = 0; i < option_defs->ndefs; i++) {

        switch (option_defs->defs[i]->type) {
            case UTIL_OPTIONS_TYPE_INT:
            {
                util_options_print(
                    "    -%c [int]   %s: %s\n"
                    "        default: %d\n",
                    option_defs->defs[i]->param, option_defs->defs[i]->name,
                    option_defs->defs[i]->description,
                    option_defs->defs[i]->default_value.i);

                break;
            }

            case UTIL_OPTIONS_TYPE_STR:
            {
                util_options_print(
                    "    -%c [str]   %s: %s\n"
                    "        default: %s\n",
                    option_defs->defs[i]->param, option_defs->defs[i]->name,
                    option_defs->defs[i]->description,
                    option_defs->defs[i]->default_value.str);

                break;
            }

            case UTIL_OPTIONS_TYPE_BOOL:
            {
                util_options_print(
                    "    -%c [bool]  %s: %s\n"
                    "        default: %d\n",
                    option_defs->defs[i]->param, option_defs->defs[i]->name,
                    option_defs->defs[i]->description,
                    option_defs->defs[i]->default_value.b);

                break;
            }

            case UTIL_OPTIONS_TYPE_BIN:
            {
                size_t len = option_defs->defs[i]->default_value.bin.len * 2 + 1;
                char* buf = xmalloc(len);

                hex_encode_uc(option_defs->defs[i]->default_value.bin.data,
                    option_defs->defs[i]->default_value.bin.len, buf, len);

                util_options_print(
                    "    -%c [bin]  %s: %s\n"
                    "        default (len %Iu): %s\n",
                    option_defs->defs[i]->param, option_defs->defs[i]->name,
                    option_defs->defs[i]->description,
                    option_defs->defs[i]->default_value.bin.len,
                    buf);

                free(buf);

                break;
            }

            default:
            {
                util_options_print(
                    "    -%c [unkn]  %s: %s\n"
                    "        default: -invalid-\n",
                    option_defs->defs[i]->param, option_defs->defs[i]->name,
                    option_defs->defs[i]->description);

                break;
            }
        }
    }
}

void util_options_free(struct util_options_opts* opts)
{
    for (uint32_t i = 0; i < opts->entries; i++) {

        switch (opts->defs->defs[i]->type) {
            case UTIL_OPTIONS_TYPE_STR:
            {
                free(opts->values[i].value.str);
                break;
            }

            case UTIL_OPTIONS_TYPE_BIN:
            {
                free(opts->values[i].value.bin.data);
                break;
            }

            default:
                break;
        }
    }

    free(opts->values);
    free(opts);
}

// if NULL, usage is printed
static struct util_options_opts* util_options_get(
        int argc, char** argv, const struct util_options_defs* option_defs)
{
    bool success;
    struct util_options_opts* options =
        (struct util_options_opts*)
            xmalloc(sizeof(struct util_options_opts));

    if (option_defs == NULL) {
        options->defs = NULL;
        options->values = NULL;
        options->entries = 0;

        return options;
    }

    options->defs = option_defs;
    options->entries = option_defs->ndefs;

    options->values = (struct util_options_value*) malloc(
        sizeof(struct util_options_value) * options->entries);

    memset(options->values, 0,
        sizeof(struct util_options_value) * options->entries);

    log_misc("Processing options file reboot_connection.conf");
    success = util_options_init_from_file(options, option_defs,
          "reboot_connection.conf");

    if (!success) {
        util_options_print_usage(option_defs);
        util_options_free(options);
        return NULL;
    } else {
        return options;
    }
}

static bool util_options_init_from_file(struct util_options_opts* options,
        const struct util_options_defs* option_defs, char* file_path)
{
    char* pos_lines;
    char* pos_key_val;
    char* ctx_lines;
    char* ctx_key_val;
    char* data;
    size_t len;

    if (!file_load(file_path, (void**) &data, &len, true)) {
        /* If file does not exist, create one with default configuration
           values */
        if (path_exists(file_path)) {
            log_warning("Loading options file %s failed, faulty or corrupted "
                "configuration", file_path);
        } else {
            /* Create a default config file if the usage param is set. This
               indicates that we are currently not on the inject process
               but already on the inject library. We want all config params
               of the inject lib */
            if (option_defs->usage_param) {
                log_info("Options file %s does not exist, creating default...",
                    file_path);

                if (!util_options_write_default_options_file(option_defs,
                        file_path)) {
                    log_warning("Creating default options file %s failed",
                        file_path);
                }
            }
        }

        return false;
    }

    pos_lines = strtok_r(data, "\n", &ctx_lines);

    while (pos_lines != NULL) {
        char* pos_line_dup;
        char* key = NULL;
        char* val = NULL;
        int cnt = 0;

        /* ignore comments and empty lines */
        if (strlen(pos_lines) > 0 && pos_lines[0] != '#') {
            pos_line_dup = str_dup(pos_lines);
            pos_key_val = strtok_r(pos_line_dup, "=", &ctx_key_val);

            log_misc("Line: %s", pos_lines);

            while (pos_key_val != NULL) {

                if (cnt == 0) {
                    key = pos_key_val;
                } else if (cnt == 1) {
                    val = pos_key_val;
                }

                pos_key_val = strtok_r(NULL, "=", &ctx_key_val);
                cnt++;
            }

            /* Key requiured, value can be NULL */
            if (cnt != 1 && cnt != 2) {
                log_warning("Invalid options line %s in options file %s",
                    pos_lines, file_path);
                free(pos_line_dup);
                free(data);
                return false;
            }

            log_misc("Key: %s, Value: %s", key, val);

            /* search for key */
            for (uint32_t j = 0; j < options->entries; j++) {

                if (!strcmp(options->defs->defs[j]->name, key)) {

                    switch (options->defs->defs[j]->type) {
                        case UTIL_OPTIONS_TYPE_INT:
                        {
                            if (val) {
                                options->values[j].value.i = atoi(val);
                            } else {
                                options->values[j].value.i = 0;
                            }

                            break;
                        }

                        case UTIL_OPTIONS_TYPE_STR:
                        {
                            if (val) {
                                options->values[j].value.str = str_dup(val);
                            } else {
                                options->values[j].value.str = NULL;
                            }

                            break;
                        }

                        case UTIL_OPTIONS_TYPE_BOOL:
                        {
                            if (val) {
                                if (atoi(val) <= 0) {
                                    options->values[j].value.b = false;
                                } else {
                                    options->values[j].value.b = true;
                                }
                            } else {
                                options->values[j].value.b = false;
                            }


                            break;
                        }

                        case UTIL_OPTIONS_TYPE_BIN:
                        {
                            if (val) {
                                size_t len = strlen(val);
                                len = len / 2 + len % 2;

                                options->values[j].value.bin.len = len;
                                options->values[j].value.bin.data =
                                    xmalloc(len);

                                if (!hex_decode(
                                        options->values[j].value.bin.data,
                                        options->values[j].value.bin.len, val,
                                        strlen(val))) {
                                    log_warning("Decoding binary data for key "
                                        "%s failed",
                                        options->defs->defs[j]->name);
                                    options->values[j].value.bin.len = 0;
                                    options->values[j].value.bin.data = NULL;
                                }
                            } else {
                                options->values[j].value.bin.len = 0;
                                options->values[j].value.bin.data = NULL;
                            }

                            break;
                        }

                        default:
                            continue;
                    }

                    log_misc("Options file param: -%c %s (type %d): %s",
                        options->defs->defs[j]->param,
                        options->defs->defs[j]->name,
                        options->defs->defs[j]->type, val ? val : "NULL");

                    options->values[j].avail = true;
                    break;
                }
            }

            free(pos_line_dup);
        }

        pos_lines = strtok_r(NULL, "\n", &ctx_lines);
    }

    free(data);

    return true;
}

static bool util_options_write_default_options_file(
        const struct util_options_defs* option_defs, char* file_path)
{
    FILE* file;

    file = fopen(file_path, "wb");

    if (file == NULL) {
        return false;
    }

     for (uint32_t i = 0; i < option_defs->ndefs; i++) {

        switch (option_defs->defs[i]->type) {
            case UTIL_OPTIONS_TYPE_INT:
            {
                fprintf(file, "# [int]: %s\n",
                    option_defs->defs[i]->description);
                fprintf(file, "%s=%d\n\n", option_defs->defs[i]->name,
                    option_defs->defs[i]->default_value.i);

                break;
            }

            case UTIL_OPTIONS_TYPE_STR:
            {
                fprintf(file, "# [str]: %s\n",
                    option_defs->defs[i]->description);

                if (option_defs->defs[i]->default_value.str) {
                    fprintf(file, "%s=%s\n\n", option_defs->defs[i]->name,
                        option_defs->defs[i]->default_value.str);
                } else {
                    fprintf(file, "%s=\n\n", option_defs->defs[i]->name);
                }

                break;
            }

            case UTIL_OPTIONS_TYPE_BOOL:
            {
                fprintf(file, "# [bool (0/1)]: %s\n",
                    option_defs->defs[i]->description);
                fprintf(file, "%s=%d\n\n", option_defs->defs[i]->name,
                    option_defs->defs[i]->default_value.b);

                break;
            }

            case UTIL_OPTIONS_TYPE_BIN:
            {
                fprintf(file, "# [bin]: %s\n",
                    option_defs->defs[i]->description);

                if (option_defs->defs[i]->default_value.bin.data) {
                    size_t len =
                        option_defs->defs[i]->default_value.bin.len * 2 + 1;
                    char* buf = xmalloc(len);

                    hex_encode_uc(option_defs->defs[i]->default_value.bin.data,
                        option_defs->defs[i]->default_value.bin.len, buf, len);


                    fprintf(file, "%s=%s\n\n", option_defs->defs[i]->name, buf);

                    free(buf);
                }

                break;
            }

            default:
            {
                log_warning("Invalid value type %d found",
                    option_defs->defs[i]->type);
                break;
            }
        }
    }

    fclose(file);
    return true;
}

static void util_options_print(const char* fmt, ...)
{
    char buffer[8192];
    va_list ap;

    va_start(ap, fmt);

    if (util_options_usage_print_out) {
        vfprintf(util_options_usage_print_out, fmt, ap);
    } else {
        _vsnprintf(buffer, sizeof(buffer), fmt, ap);
        OutputDebugString(buffer);
    }

    va_end(ap);
}
