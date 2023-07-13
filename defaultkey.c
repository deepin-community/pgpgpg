/* defaultkey.c - fetch the default secret key id
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
 * Function : get_defaultkey()
 *
 * Purpose  : Runs GnuPG (Gnu Privacy Guard) to determine the first key in the
 *            secret keyring which is also known as the default key.
 *
 * Output   : Returns the long GnuPG keyid of the default keyid.
 *
 * Errors   : Returns Null if the defaultkey couldn't be determined.
 *
 *****************************************************************************/
char * get_defaultkey(void)
{
    enum	{ buffer_size = 1024 };
    char	buffer[buffer_size];
    ArgvFactory	af;
    pid_t	child_pid;
    int		child_status;
    int		child_pipe[2];
    int		dev_null_fd;
    FILE	*in;
    char	*defaultid = NULL;
    int		err = 0;
    
    argv_factory_init(&af);
    argv_factory_add(&af, GPG_PATH);
    argv_factory_add(&af, "--with-colons");
    argv_factory_add(&af, "--batch");
    argv_factory_add(&af, "--quiet");
    argv_factory_add(&af, "--no-verbose");
    argv_factory_add(&af, "--list-secret-keys");
    
    if (pipe(child_pipe))
    {
        perror("Can't setup a pipe for gpg.");
        return NULL;
    }
    
    if ((dev_null_fd = open("/dev/null", O_RDWR)) == -1)
    {
        perror("Can't open /dev/null for redirect");
        return NULL;
    }
    
    switch ((child_pid = fork()))
    {
        case  -1:
            perror("Can't fork to execute gpg");
            return NULL;
        
        case 0:		/* child */
            err += close(0) == -1 ? 1 : 0;
            err += close(1) == -1 ? 1 : 0;
            err += close(2) == -1 ? 1 : 0;
            err += dup2(dev_null_fd, 0) == -1 ? 1 : 0;
            err += dup2(child_pipe[1], 1) == -1 ? 1 : 0;
            err += dup2(dev_null_fd, 2) == -1 ? 1 : 0;
            err += close(child_pipe[0]) == -1 ? 1 : 0;
            err += close(child_pipe[1]) == -1 ? 1 : 0;
            err += close(dev_null_fd) == -1 ? 1 : 0;
            if (err)
            {
                fprintf(stderr, "Error in redirecting stdin, stdout and stderr. Can't execute gpg.");
                return NULL;
            }
            execv(af.argv[0], af.argv);
            perror("Can't execute gpg (execv failed).");
            return NULL;
        
        default:	/* parent */
            close(child_pipe[1]);
            close(dev_null_fd);
            if (!(in = fdopen(child_pipe[0], "r")))	/* much easier to use a FILE */
            {
                perror("Can't associate stream with pipe.");
                return NULL;
            }
            while (fgets(buffer, buffer_size, in))
            {
                if (strncmp(buffer, "sec:", 4))
                {
                    /* skip remaining line */
                    while (!strchr(buffer, '\n') && fgets(buffer, buffer_size, in));
                }
                else
                {
                    /* the first secret key is the default key; skip 4 colon seperated fields */
                    int i;
                    defaultid = buffer;
                    for(i=0; i<4; ++i)
                        defaultid = defaultid ? strchr(defaultid+1, ':') : NULL;
                    
                    if (defaultid)
                    {
                        char *end;
                        
                        --defaultid;
                        defaultid[0] = '0';
                        defaultid[1] = 'x';
                        
                        if (!(end = strchr(defaultid, ':')))
                        {
                            fprintf(stderr, "Wrong keylist format.\n");
                            defaultid = NULL;
                            break;
                        }
                        *end = 0;
                        
                        defaultid = strdup(defaultid);
                        break;
                    }
                    else
                    {
                        fprintf(stderr, "Wrong keylist format.\n");
                        break;
                    }
                }
            }
            fclose(in);
            
            /* terminate child */
            kill(child_pid, SIGINT);
            waitpid(child_pid, &child_status, 0);
    }
    
    argv_factory_release(&af);
    
    return defaultid;
}



