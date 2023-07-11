/* debug.h - Macros for debugging
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


#ifndef INCLUDE_DEBUG_H_19990218153513
#define INCLUDE_DEBUG_H_19990218153513


#ifndef NDEBUG
# define DEBUG(args) { fprintf(stderr, "debug: in %s at %s:%d: ", __FUNCTION__,  __FILE__, __LINE__); debug_printf args; }
#else
# define DEBUG(args) ;
#endif

#ifndef NDEBUG
extern void debug_printf(const char *, ...);
#endif

#endif

