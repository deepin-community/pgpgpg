/* mygetopt.h - Simple getopt() like function
 *      Copyright (C) 1999 Michael Roth <mroth@gnupg.de>
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


#ifndef INCLUDE_MYGETOPT_H_19990527175842
#define INCLUDE_MYGETOPT_H_19990527175842


extern char *mynextchar;
extern char *myoptarg;
extern int myoptind;
extern int myoptopt;


extern int mygetopt(int argc, char * const argv[], const char *optstring);


#endif
