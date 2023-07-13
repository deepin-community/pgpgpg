/* guessarmortype.h - Header and declerations for guessarmortype.c
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


#ifndef INCLUDE_GUESSARMORTYPE_H_19990605143613
#define INCLUDE_GUESSARMORTYPE_H_19990605143613


#define ARMOR_TYPE_ERROR		0
#define ARMOR_TYPE_UNKNOWN		1
#define ARMOR_TYPE_MESSAGE		2
#define ARMOR_TYPE_SIGNED_MESSAGE	3
#define ARMOR_TYPE_SIGNATURE		4


extern int guess_armor_type(char *filename);


#endif


