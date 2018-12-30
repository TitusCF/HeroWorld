/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2014 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "libproto.h"
#include "stringbuffer.h"

/**
 * A buffer that will be expanded as content is added to it.
 */
struct StringBuffer {
    /**
     * The string buffer. The first {@link #pos} bytes contain the collected
     * string. Its size is at least {@link #size} bytes.
     */
    char *buf;

    /**
     * The current length of {@link #buf}. The invariant <code>pos <
     * size</code> always holds; this means there is always enough room to
     * attach a trailing \0 character.
     */
    size_t pos;

    /**
     * The allocation size of {@link #buf}.
     */
    size_t size;
};


/**
 * Make sure that at least <code>len</code> bytes are available in the passed
 * string buffer.
 *
 * @param sb The string buffer to modify.
 *
 * @param len The number of bytes to allocate.
 */
static void stringbuffer_ensure(StringBuffer *sb, size_t len);


StringBuffer *stringbuffer_new(void) {
    StringBuffer *sb;

    sb = malloc(sizeof(*sb));
    if (sb == NULL) {
        fatal(OUT_OF_MEMORY);
    }

    sb->size = 256;
    sb->buf = malloc(sb->size);
    sb->pos = 0;
    return sb;
}

void stringbuffer_delete(StringBuffer *sb) {
    free(sb->buf);
    free(sb);
}

char *stringbuffer_finish(StringBuffer *sb) {
    char *result;

    sb->buf[sb->pos] = '\0';
    result = sb->buf;
    free(sb);
    return result;
}

sstring stringbuffer_finish_shared(StringBuffer *sb) {
    char *str;
    sstring result;

    str = stringbuffer_finish(sb);
    result = add_string(str);
    free(str);
    return result;
}

void stringbuffer_append_string(StringBuffer *sb, const char *str) {
    size_t len;

    len = strlen(str);
    stringbuffer_ensure(sb, len+1);
    memcpy(sb->buf+sb->pos, str, len);
    sb->pos += len;
}

void stringbuffer_append_printf(StringBuffer *sb, const char *format, ...) {
    size_t size;

    size = 100;                 /* arbitrary guess */
    for (;;) {
        int n;
        va_list arg;

        stringbuffer_ensure(sb, size);

        va_start(arg, format);
        n = vsnprintf(sb->buf+sb->pos, size, format, arg);
        va_end(arg);

        if (n > -1 && (size_t)n < size) {
            sb->pos += (size_t)n;
            break;
        }

        if (n > -1) {
            size = n+1;         /* precisely what is needed */
        } else {
            size *= 2;          /* twice the old size */
        }
    }
}

void stringbuffer_append_stringbuffer(StringBuffer *sb, const StringBuffer *sb2) {
    stringbuffer_ensure(sb, sb2->pos+1);
    memcpy(sb->buf+sb->pos, sb2->buf, sb2->pos);
    sb->pos += sb2->pos;
}

static void stringbuffer_ensure(StringBuffer *sb, size_t len) {
    char *tmp;
    size_t new_size;

    if (sb->pos+len <= sb->size) {
        return;
    }

    new_size = sb->pos+len+256;
    tmp = realloc(sb->buf, new_size);
    if (tmp == NULL) {
        fatal(OUT_OF_MEMORY);
    }
    sb->buf = tmp;
    sb->size = new_size;
}

size_t stringbuffer_length(StringBuffer *sb) {
    return sb->pos;
}
