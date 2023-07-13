/* pgpopts.h - Parse pgp long options
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


#ifndef INCLUDE_PGPOPTS_H_19990224160832
#define INCLUDE_PGPOPTS_H_19990224160832


typedef struct
{
    char	k, g, a, x, v, c, r, e, s, d, b, p, f, w, m, t;
    char	*passphrase;
    char	*outputfile;
    char	*localuserid;
    char	*charset;
    char	*comment;
    char	*pubring;
    char	*secring;
    int		batchmode;
    int		asciiarmor;
    int		textmode;
    int		clearsign;
    int		verbose;
    int		compress;
    int		encrypttoself;
    char 	*completesneeded;
    char	*marginalsneeded;
    char	*certdepth;
    int		force;
    int		stdinisterminal;
    char	**args;
    size_t	args_size;
    size_t	args_used;
    int		passphrase_pipe[2];
} Pgpopts;


extern void pgpopts_init(Pgpopts *opts);
extern char ** pgpopts_build_argv(Pgpopts *pgpopts);
extern int pgpopts_parse(Pgpopts *pgpopts, int argc, char *argv[]);


#endif

