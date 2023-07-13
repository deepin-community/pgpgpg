/* argvfactory.h - Creating argv vectors
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


#ifndef INCLUDE_ARGVFACTORY_H_19990218020618
#define INCLUDE_ARGVFACTORY_H_19990218020618



typedef struct
{
    char	**argv;
    size_t	used;
    size_t	size;
} ArgvFactory;



extern void argv_factory_init(ArgvFactory *af);
extern void argv_factory_add(ArgvFactory *af, char *arg);
extern void argv_factory_release(ArgvFactory *af);
extern void argv_factory_append(ArgvFactory *dest, ArgvFactory *src);


#endif

