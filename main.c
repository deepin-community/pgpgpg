/* main.c - pgpgpg main function
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
#include "pgpopts.h"


/* PGP exit codes */
#define PGPERR_OK		0
#define PGPERR_FILE_INVALID	1
#define PGPERR_FILE_NOT_FOUND	2
#define PGPERR_FILE_UNKNOWN	3
#define PGPERR_BATCH		4
#define PGPERR_ARGUMENT		5
#define PGPERR_INTERRUPT	6
#define PGPERR_MEM		7
#define PGPERR_KEY_GENERATE	10
#define PGPERR_KEY_NOT_EXIST	11
#define PGPERR_KEY_IMPORT	12
#define PGPERR_KEY_EXPORT	13
#define PGPERR_KEY_EDIT		14
#define PGPERR_KEY_VIEW		15
#define PGPERR_KEY_REMOVE	16
#define PGPERR_KEY_CHECK	17
#define PGPERR_KEY_SIGN		18
#define PGPERR_KEY_SIGN_REMOVE	19
#define PGPERR_ENCR_SIGN	20
#define PGPERR_ENCR_RSA		21
#define PGPERR_ENCR_ENCRYPT	22
#define PGPERR_ENCR_COMPRESS	23
#define PGPERR_DECR_SIGN	30
#define PGPERR_DECR_RSA		31
#define PGPERR_DECR_DECRYPT	32
#define PGPERR_DECR_COMPRESS	33
#define PGPERR_UNKNOWN		255	/* that's my own code */





/*****************************************************************************
 *
 * Function : main
 *
 * Purpose  : Parses the command line options, translate the PGP options to
 *           GnuPG options and call GnuPG.
 *
 * Input    : argc/argv command line options
 *
 *****************************************************************************/
int main(int argc, char *argv[])
{
    Pgpopts	pgpopts;
    char	**gpg_argv;
    pid_t	child_pid;
    int		child_status;
    int		i, result;
    
#ifndef NDEBUG
    for (i=0; argv[i]; ++i)
    {
        fprintf(stderr, "%s", argv[i]);

        if (argv[i+1])
            fprintf(stderr, " ");
        else
            fprintf(stderr, "\n");
    }
#endif
    
    pgpopts_init(&pgpopts);
    
    if (pgpopts_parse(&pgpopts, argc, argv))
        return PGPERR_UNKNOWN;
    
    pgpopts.stdinisterminal = isatty(0);
    
    /* passphrase stuff */
    if (pgpopts.passphrase)	/* -z passphrase have the highest priority */
    {
        if (pipe(pgpopts.passphrase_pipe))
        {
            perror("Error: Can't setup pipe for passphrase");
            return PGPERR_UNKNOWN;
        }
    }
    else
    {
        char *s;
        
        if ((s = getenv("PGPPASSFD")))
            pgpopts.passphrase_pipe[0] = atoi(s);
        else if ((s = getenv("PGPPASS")))
        {
            pgpopts.passphrase = s;
            
            if (pipe(pgpopts.passphrase_pipe))
            {
                perror("Error: Can't setup pipe for passphrase");
                return PGPERR_UNKNOWN;
            }
        }
    }
    
    if (!(gpg_argv = pgpopts_build_argv(&pgpopts)))
        return 1;
    
#ifndef NDEBUG
    for (i=0; gpg_argv[i]; ++i)
    {
        fprintf(stderr, "%s", gpg_argv[i]);
        
        if (gpg_argv[i+1])
            fprintf(stderr, " ");
        else
            fprintf(stderr, "\n");
    }
#endif
    
    /* execute gnupg */
    switch( (child_pid=fork()) )
    {
        case -1:
            perror("Error: Can't execute gnupg");
            return PGPERR_UNKNOWN;
        
        case 0:		/* child */
            execv(gpg_argv[0], gpg_argv);
            perror("Can't execute gpg (Gnu Privacy Guard)");
            return PGPERR_UNKNOWN;
        
        default:	/* parent */
            if (pgpopts.passphrase)	/* FIXME: partly writes, signals, and so on */
            {
                write(pgpopts.passphrase_pipe[1], pgpopts.passphrase, strlen(pgpopts.passphrase));
                write(pgpopts.passphrase_pipe[1], "\n", 1);
                close(pgpopts.passphrase_pipe[1]);
            }
            waitpid(child_pid, &child_status, 0);
    }
    
    /* process PGP exit codes */
    if (!WIFEXITED(child_status))
    {
        fprintf(stderr, "Error: Execution of `gpg' failed (Unknown reason).\n");
        return PGPERR_UNKNOWN;
    }
    
    switch(WEXITSTATUS(child_status))
    {
        case 0:
            result = PGPERR_OK;
            break;
        
        case 1:
            result = PGPERR_DECR_SIGN;
            fprintf(stderr, "WARNING: Bad signature, doesn't match file contents!\n");
            break;
        
        case 2:
            result = PGPERR_KEY_NOT_EXIST;
            fprintf(stderr, "WARNING: Can't find the right public key-- can't check signature integrity.\n");
            break;
        
        default:
            result = 0; /* FIXME... */
            break;
    }
    
    return result;
}




