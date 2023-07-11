/* guessarmortype.c - Guess type of armored data
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
#include "guessarmortype.h"



/*****************************************************************************
 *
 * Function : guess_armor_type
 *
 * Purpose  : Try to guess the type of armored data in a file.
 *
 * Input    : filename - The filename of the file with armored data to guess
 *                       type of.
 *
 * Output   : An integer defined as ARMOR_TYPE_xxx
 *
 * Errors   : Returns ARMOR_TYPE_ERROR when an error occured.
 *
 * Notes    : This works not very well for real PGP 2.6 armored data because
 *            PGP 2.6 don't specificy the type in the begin line very well.
 *
 *****************************************************************************/
int guess_armor_type(char *filename)
{
    FILE *		in;
    char		buffer[512];
    int			mode = 1;
    int			i;
    
    if (!( in = fopen(filename, "r") ))
        return ARMOR_TYPE_ERROR;
    
    while (mode) {
        if (!fgets(buffer, 512, in)) {
            fclose(in);
            return ARMOR_TYPE_ERROR;
        }
        
        if (mode == 1) {	/* normal check */
            if (strncmp(buffer, "-----BEGIN PGP ", 15) == 0)
                mode = 0;
            else if (strncmp(buffer, "-----END PGP ", 13) == 0) {
                fclose(in);
                return ARMOR_TYPE_ERROR;
            }
        }
        
        if (mode != 0) {
            if (strlen(buffer) && buffer[strlen(buffer)-1] == '\n')
                mode = 1;	/* line ending found, check next line */
            else
                mode = 2;	/* search for the line ending */
        }
    }
    
    fclose(in);
    
    /* 
     * A begin pgp clause was found. Strip all trailing space, tab, 
     * return and newline characters 
     */
    
    i = strlen(buffer) - 1;
    while (strchr(" \t\r\n", buffer[i]))
        buffer[i--] = 0;
    
    /*
     * Check the various PGP message types
     */
    
    if (strcmp(buffer, "-----BEGIN PGP MESSAGE-----")==0)
        return ARMOR_TYPE_MESSAGE;
    else if (strcmp(buffer, "-----BEGIN PGP SIGNED MESSAGE-----")==0)
        return ARMOR_TYPE_SIGNED_MESSAGE;
    else if (strcmp(buffer, "-----BEGIN PGP SIGNATURE-----")==0)
        return ARMOR_TYPE_SIGNATURE;
    else
        return ARMOR_TYPE_UNKNOWN;
}



