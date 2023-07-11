/* mygetopt.c - Simple getopt() like function
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


#include "includes.h"
#include "mygetopt.h"



char *mynextchar = NULL;
char *myoptarg = NULL;
int myoptind = 1;
int myoptopt = 0;



int mygetopt(int argc, char * const argv[], const char *optstring)
{
    char *option;
    
    /*
    DEBUG(("mygetopt called: optstring: '%s'  mynextchar: '%s'  myoptarg: '%s'  myoptind: %d", 
            optstring, mynextchar, myoptarg, myoptind))
    */
    
    if (myoptind >= argc)
        return EOF;
    
    if (!mynextchar)
    {
        mynextchar = argv[myoptind];
        
        if (*mynextchar == '-')
        {
            mynextchar++;
        }
        else
        {
            myoptarg = mynextchar;
            myoptind++;
            mynextchar = NULL;
            return 1;
        }
    }
    
    if (*mynextchar == ':')
    {
        myoptopt = ':';
        return '?';
    }
    
    if ((option=strchr(optstring, *mynextchar)))
    {
        if (option[1] == ':')
        {
            /* Option with an argument */
            
            if (mynextchar[1])
            {
                myoptarg = mynextchar + 1;
                mynextchar = NULL;
                myoptind++;
                return *option;
            }
            else
            {
                if (++myoptind >= argc)
                {
                    myoptopt = *option;
                    return ':';
                }
                
                myoptarg = argv[myoptind++];
                mynextchar = NULL;
                return *option;
            }
        }
        else
        {
            /* Option without an argument */
            
            mynextchar++;
            if (!*mynextchar)
            {
                mynextchar = NULL;
                myoptind++;
            }
            
            return *option;
        }
    }
    
    myoptopt = *mynextchar;
    return '?';
}







