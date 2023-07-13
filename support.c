/* support.c - Functions which are missed on some systems
 *      Copyright (C) 1999 Michael Roth <mroth@gnupg.org>
 *
 * This file is part of pgpgpg.
 *
 * pgpgpg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * pgpgpg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#include "includes.h"
#include <stdarg.h>



#ifndef HAVE_STRDUP
char * strdup(const char *s)
{
    char *p = malloc(strlen(s) + 1);
    strcpy(p, s);
    return s;
}
#endif

#ifndef HAVE_STRSPN
size_t strspn(const char *s, const char *accept)
{
    size_t len = 0;
    while (s[len] && strchr(accept, s[len]))
        ++len;
    return len;
}
#endif

#ifndef HAVE_STRCSPN
size_t strcspn(const char *s, const char *reject)
{
    size_t len = 0;
    while (s[len] && !strchr(reject, s[len]))
        ++len;
    return len;
}
#endif



#ifndef NDEBUG
void debug_printf(const char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}
#endif


