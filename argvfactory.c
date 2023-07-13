/* argvfactory.c - Creating argv vectors
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
#include "argvfactory.h"




/*****************************************************************************
 *
 * Function : argv_factory_init
 *
 * Purpose  : Initializes an ArgvFactory structure
 *
 * Input    : af - A pointer to an uninitialized ArgvFactory structure to
 *                 initialize.
 *
 *****************************************************************************/
void argv_factory_init(ArgvFactory *af)
{
    assert(af != NULL);
    af->size = 64;
    af->used = 0;
    af->argv = (char **)malloc(af->size * sizeof(char *));
    assert(af->argv != NULL);
    *af->argv = NULL;
}




/*****************************************************************************
 *
 * Function : argv_factory_release
 *
 * Purpose  : Releases all memory associated with an ArgvFactory structure.
 *
 * Input    : af - A pointer to the ArgvFactory structure to release.
 *
 * Notes    : Don't use `af' after releasing.
 *
 *****************************************************************************/
void argv_factory_release(ArgvFactory *af)
{
    assert(af != NULL);
    assert(af->argv != NULL);
    assert(af->used+1 <= af->size);
    assert(af->argv[af->used] == NULL);
    
    /* FIXME: Free the strings in the argv vector !!! */
    
    free(af->argv);
}




/*****************************************************************************
 *
 * Function : argv_factory_add
 *
 * Purpose  : Adds a argument to the argv vector managed by a ArgvFactory
 *            structure.
 *
 * Input    : af  - A pointer to an ArgvFactory which manages an argv vector.
 *            arg - The string to add to the argv vector managed by an
 *                  ArgvFactory. The string will be duplicated.
 *
 * Notes    : You could reuse or free the memory pointed to by `arg' after the
 *            call to argv_factory_add().
 *
 *****************************************************************************/
void argv_factory_add(ArgvFactory *af, char *arg)
{
    assert(af != NULL);
    assert(af->argv != NULL);
    assert(af->used+1 <= af->size);
    assert(af->argv[af->used] == NULL);
    assert(arg != NULL);
    
    if (af->used + 1 == af->size)
    {
        af->size *= 2;
        af->argv = realloc(af->argv, af->size * sizeof(char *));
        assert(af->argv != NULL);
    }
    
    af->argv[af->used++] = strdup(arg);
    af->argv[af->used] = NULL;
    
    assert(af->argv[af->used-1]);
}




/*****************************************************************************
 *
 * Function : argv_factory_append
 *
 * Purpose  : Concatenate a second argv vector managed by an ArgvFactory to the
 *            first argv vector managed by an ArgvFactory structure.
 *
 * Input    : dest - The ArgvFactory to which the `src' ArgvFactorys argv vector
 *                   will be appended.
 *            src  - The ArgvFactory which manage the argv vector which will be
 *                   appended to the `dest' ArgvFactory argv vector. The strings
 *                   in the `src' ArgvFactory argv vector will be duplicated.
 *
 * Notes    : You can reuse or free the ArgvFactory `src' after a call to
 *            argv_factory_append().
 *
 *****************************************************************************/
void argv_factory_append(ArgvFactory *dest, ArgvFactory *src)
{
    char **argv;
    
    assert(dest != NULL);
    assert(dest->argv != NULL);
    assert(dest->used+1 <= dest->size);
    assert(dest->argv[dest->used] == NULL);
    assert(src != NULL);
    assert(src->argv != NULL);
    assert(src->used+1 <= src->size);
    assert(src->argv[src->used] == NULL);
    
    for (argv=src->argv; *argv; ++argv)
    {
        if (dest->used + 1 == dest->size)
        {
            dest->used *= 2;
            dest->argv = realloc(dest->argv, dest->size * sizeof(char *));
            assert(dest->argv != NULL);
        }
        
        dest->argv[dest->used++] = strdup(*argv);
        dest->argv[dest->used] = NULL;
        
        assert(dest->argv[dest->used-1]);
    }
}


