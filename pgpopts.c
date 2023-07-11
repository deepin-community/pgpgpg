/* pgpopts.c - Parse pgp long options
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
#include "argvfactory.h"
#include "defaultkey.h"
#include "mygetopt.h"
#include "guessarmortype.h"





/*****************************************************************************
 *
 * Function : pgpopts_init
 *
 * Purpose  : Initialize a Pgpopts structure for future usage.
 *
 * Input    : opts - A pointer to an unitialized Pgpopts structure to initialize.
 *
 *****************************************************************************/
void pgpopts_init(Pgpopts *opts)
{
    assert(opts != NULL);
    
    memset(opts, 0, sizeof(Pgpopts));
    opts->clearsign = 1;
    opts->force = -1;
    opts->compress = -1;
    
    opts->args_size = 64;
    opts->args_used = 0;
    opts->args = (char **)malloc(opts->args_size * sizeof(char));
    
    opts->passphrase_pipe[0] = -1;
    opts->passphrase_pipe[1] = -1;
}





enum
{
    PGP_LONG_OPT_IGNORE	= -1,
    PGP_LONG_OPT_NOSUPPORT = -2,
    PGP_LONG_OPT_UNKNOWN = -3,
    PGP_LONG_OPT_LOCALID = 0,		/* myname */
    PGP_LONG_OPT_CHARSET,		/* charset */
    PGP_LONG_OPT_BATCHMODE,		/* batchmode */
    PGP_LONG_OPT_ARMOR,			/* armor */
    PGP_LONG_OPT_TEXTMODE,		/* textmode */
    PGP_LONG_OPT_CLEARSIGN,		/* clearsig */
    PGP_LONG_OPT_VERBOSE,		/* verbose */
    PGP_LONG_OPT_COMPRESS,		/* compress */
    PGP_LONG_OPT_ENCRYPTTOSELF,		/* encrypttoself */
    PGP_LONG_OPT_COMPLETESNEEDED,	/* completes_needed */
    PGP_LONG_OPT_MARGINALSNEEDED,	/* marginals_needed */
    PGP_LONG_OPT_CERTDEPTH,		/* cert_depth */
    PGP_LONG_OPT_COMMENT,		/* comment */
    PGP_LONG_OPT_FORCE,			/* force */
    PGP_LONG_OPT_PUBRING,		/* pubring */
    PGP_LONG_OPT_SECRING,		/* secring */
    
    PGP_LONG_OPT_COUNT
};




static struct { char *keyword; int minlen; int	index; } mapping[] =
{
    { "armor",		5, PGP_LONG_OPT_ARMOR },
    { "armorlines",	6, PGP_LONG_OPT_IGNORE },
    { "autosign",	2, PGP_LONG_OPT_IGNORE },	/* FIXME: is this `autosign' on creating keys ? */
    { "bakring",	3, PGP_LONG_OPT_IGNORE },
    { "batchmode",	3, PGP_LONG_OPT_BATCHMODE },
    { "cert_depth",	2, PGP_LONG_OPT_CERTDEPTH },
    { "charset",	2, PGP_LONG_OPT_CHARSET },
    { "clearsig",	2, PGP_LONG_OPT_CLEARSIGN },
    { "comment",	4, PGP_LONG_OPT_COMMENT },
    { "completes_needed", 5, PGP_LONG_OPT_COMPLETESNEEDED },
    { "compress",	5, PGP_LONG_OPT_COMPRESS },
    { "encrypttoself",	1, PGP_LONG_OPT_ENCRYPTTOSELF },
    { "force",		1, PGP_LONG_OPT_FORCE },
    { "interactive",	1, PGP_LONG_OPT_IGNORE },
    { "keepbinary",	1, PGP_LONG_OPT_NOSUPPORT },
    { "language",	2, PGP_LONG_OPT_IGNORE },
    { "legal_kludge",	2, PGP_LONG_OPT_IGNORE },
    { "marginals_needed", 3, PGP_LONG_OPT_MARGINALSNEEDED },	/* pgp needs at least 3 chars !?!? */
    { "myname",		2, PGP_LONG_OPT_LOCALID },
    { "nomanual",	1, PGP_LONG_OPT_IGNORE },
    { "pager",		2, PGP_LONG_OPT_IGNORE },
    { "pubring",	2, PGP_LONG_OPT_PUBRING },
    { "randseed",	1, PGP_LONG_OPT_IGNORE },
    { "secring",	2, PGP_LONG_OPT_SECRING },
    { "showpass",	2, PGP_LONG_OPT_IGNORE },
    { "textmode",	2, PGP_LONG_OPT_TEXTMODE },
    { "tmp",		2, PGP_LONG_OPT_IGNORE },
    { "tzfix",		2, PGP_LONG_OPT_IGNORE },
    { "verbose",	1, PGP_LONG_OPT_VERBOSE },
    
    { NULL, 0, PGP_LONG_OPT_UNKNOWN }
};




/*****************************************************************************
 *
 * Function : fetch_bool
 *
 * Purpose  : Determines the boolean value from an long option.
 *
 * Input    : keyword - The keyword to which the argument `value' was given.
 *            value   - The value of the keyword. Either Null, "true" or "false".
 *
 * Output   : Returns 0 for a false boolean and 1 for a true boolean value.
 *
 * Errors   : Returns -1 if an error occoured.
 *
 *****************************************************************************/
static int fetch_bool(char *keyword, char *value)
{
    assert(keyword);
    
    if (!value)
        return 1;
    else if (!strcasecmp(value, "on"))
        return 1;
    else if (!strcasecmp(value, "off"))
        return 0;
    else
    {
        fprintf(stderr, "Error: Invalid boolean value `%s' for long option `%s'.\n", value, keyword);
        return -1;
    }
}



static int parse_longopt(Pgpopts *opts, const char *s)
{
    char	*keyword;
    size_t	keyword_len;
    char	*value;
    char	*end;
    int		i;
    
    assert(opts != NULL);
    assert(s != NULL);
    
    keyword = strdup(s);
    value = keyword ? strchr(keyword, '=') : NULL;
    
    if (value)
        *value++ = 0;
    
    /* skip whitespaces from keyword and terminate */
    keyword += strspn(keyword, " \t");
    if (*keyword)
        for (end=keyword+strlen(keyword)-1; *end==' ' || *end=='\t'; *end--=0);
    
    /* skip whitspaces and quotes from value */
    if (value)
    {
        value += strspn(value, " \t");
        
        if (*value == '"')
        {
            end = strchr(value+1, '"'); /* yes, pgp works like this... */
            
            if (!end)
            {
                fprintf(stderr, "Error: Unterminated long option value string: '%s'\n", value);
                return -1;
            }
            
            ++value;
            *end = 0;
        }
        else 
            if (*value)
                for (end=value+strlen(value)-1; *end==' ' || *end=='\t'; *end--=0);
    }
    
    keyword_len = strlen(keyword);
    
    DEBUG(("longopts: keyword: '%s'", keyword))
    DEBUG(("longopts: value '%s'", value ? value : "- -- NO VALUE GIVEN -- -"))
    
    /* search keyword index */
    for(i=0; mapping[i].keyword; ++i)
        if (mapping[i].minlen <= keyword_len  &&  !strncasecmp(mapping[i].keyword, keyword, keyword_len))
            break;
    
    /* process long option */
    switch(mapping[i].index)
    {
        case PGP_LONG_OPT_BATCHMODE:
            if ((opts->batchmode = fetch_bool(keyword, value)) == -1)
                return -1;
            break;
        
        case PGP_LONG_OPT_FORCE:
            if ((opts->force = fetch_bool(keyword, value)) == -1)
                return -1;
            break;
        
        case PGP_LONG_OPT_LOCALID:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `myname' requires an argument.\n");
                return -1;
            }
            opts->localuserid = value;
            break;
        
        case PGP_LONG_OPT_ENCRYPTTOSELF:
            if ((opts->encrypttoself = fetch_bool(keyword, value)) == -1)
                return -1;
            break;
        
        case PGP_LONG_OPT_MARGINALSNEEDED:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `marginals_needed' requires an argument.\n");
                return -1;
            }
            opts->marginalsneeded = value;
            break;
        
        case PGP_LONG_OPT_COMPLETESNEEDED:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `completes_needed' requires an argument.\n");
                return -1;
            }
            opts->completesneeded = value;
            break;
        
        case PGP_LONG_OPT_CERTDEPTH:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `certdepth' requires an argument.\n");
                return -1;
            }
            opts->certdepth = value;
            break;
        
        case PGP_LONG_OPT_VERBOSE:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `verbose' requires an argument.\n");
                return -1;
            }
            opts->verbose = atoi(value);
            break;
        
        case PGP_LONG_OPT_COMPRESS:
            if ((opts->compress = fetch_bool(keyword, value)) == -1)
                return -1;
            break;
        
        case PGP_LONG_OPT_TEXTMODE:
            if ((opts->textmode = fetch_bool(keyword, value)) == -1)
                return -1;
            break;
        
        case PGP_LONG_OPT_ARMOR:
            if ((opts->asciiarmor = fetch_bool(keyword, value)) == -1)
                return -1;
            break;
        
        case PGP_LONG_OPT_CLEARSIGN:
            if ((opts->clearsign = fetch_bool(keyword, value)) == -1)
                return -1;
             break;
        
        case PGP_LONG_OPT_COMMENT:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `comment' requires an argument.\n");
                return -1;
            }
            opts->comment = value;
            break;
        
        case PGP_LONG_OPT_SECRING:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `secring' requires an argument.\n");
                return -1;
            }
            opts->secring = value;
            break;
        
        case PGP_LONG_OPT_PUBRING:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `pubring' requires an argument.\n");
                return -1;
            }
            opts->pubring = value;
            break;
        
        case PGP_LONG_OPT_CHARSET:
            if (!value)
            {
                fprintf(stderr, "Error: Long option `charset' requires an argument.\n");
                return -1;
            }
            if (!strcasecmp(value, "noconv"))
                opts->charset = NULL;
            else if (!strcasecmp(value, "latin1"))
                opts->charset = "iso-8859-1";
            else if (!strcasecmp(value, "koi8"))
                opts->charset = "koi8-r";
            else
            {
                fprintf(stderr, "Error: Charset `%s' not supported.\n", value);
                return -1;
            }
            break;
        
        case PGP_LONG_OPT_UNKNOWN:
            fprintf(stderr, "Error: Unknown long option `%s'\n", keyword);
            return -1;
        
        case PGP_LONG_OPT_NOSUPPORT:
            fprintf(stderr, "Error: Sorry, but long option `%s' is not supported.\n", mapping[i].keyword);
            return -1;
        
        case PGP_LONG_OPT_IGNORE:
            fprintf(stderr, "Warning: Long option `%s' is not translated/used.\n", mapping[i].keyword);
            return 0;
        
        default:
            fprintf(stderr, "BUG: The long option `%s' is currently not supported.\n"
                            "     Please submit a bug report to <mroth@gnupg.org>\n", mapping[i].keyword);
            return -1;
    }
    
    return 0;
}





/*****************************************************************************
 *
 * Function : add_argument
 *
 * Purpose  : Adds an argument to a Pgpopts structure. Arguments are files.
 *
 * Input    : pgpopts - The Pgpopts to add the argument `arg' to.
 *            arg     - The string to add as argument to the `pgpopts'.
 *
 * Notes    : Don't reuse or free `arg' after calling add_argument()!! It will
 *            not duplicated!!
 *
 *****************************************************************************/
static void add_argument(Pgpopts *pgpopts, char *arg)
{
    assert(pgpopts != NULL);
    assert(arg != NULL);
    assert(pgpopts->args != NULL);
    assert(pgpopts->args_used < pgpopts->args_size);
    assert(pgpopts->args[pgpopts->args_used] == NULL);
    
    if (pgpopts->args_used + 1 == pgpopts->args_size)
    {
        pgpopts->args_size *= 2;
        pgpopts->args = (char **)realloc(pgpopts->args, pgpopts->args_size * sizeof(char *));
        assert(pgpopts->args != NULL);
    }
    
    pgpopts->args[pgpopts->args_used++] = arg;
    pgpopts->args[pgpopts->args_used] = NULL;
}




/*****************************************************************************
 *
 * Function : check_opts_used
 *
 * Purpose  : Runs some consistence checks on the Pgpopts structure to 
 *            determine if all given command line options are correctly
 *            translated to GnuPG options.
 *
 * Input    : pgpopts - A Pgpopts structure to check.
 *
 * Output   : Returns 0 if no inconsistence are found, otherwise some 
 *            different value.
 *
 *****************************************************************************/
static int check_opts_used(Pgpopts *pgpopts)
{
    int err = 0;
    
    assert(pgpopts != NULL);
    
    err += pgpopts->k ? 1 : 0;
    err += pgpopts->g ? 1 : 0;
    /* err += pgpopts->a ? 1 : 0; */
    err += pgpopts->x ? 1 : 0;
    /* err += pgpopts->v ? 1 : 0; */
    err += pgpopts->c ? 1 : 0;
    err += pgpopts->r ? 1 : 0;
    err += pgpopts->e ? 1 : 0;
    err += pgpopts->s ? 1 : 0;
    err += pgpopts->d ? 1 : 0;
    err += pgpopts->b ? 1 : 0;
    err += pgpopts->p ? 1 : 0;
    err += pgpopts->f ? 1 : 0;
    /* err += pgpopts->w ? 1 : 0; */
    /* err += pgpopts->m ? 1 : 0; */
    err += pgpopts->t ? 1 : 0;
    
    /* err += pgpopts->outputfile ? 1 : 0; */
    /* err += pgpopts->remote_userid_file ? 1 : 0; */
    /* err += pgpopts->passphrase ? 1 : 0; */
    /* err += pgpopts->localuserid ? 1 : 0; */
    
    return err;
}



static void build_asciiarmor(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->a || pgpopts->asciiarmor)
        argv_factory_add(af, "--armor");
    else
        argv_factory_add(af, "--no-armor");
}



static void build_textmode(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->t)
        argv_factory_add(af, "--textmode");
    pgpopts->t = 0;
}



static void build_force(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->force == 1)
        argv_factory_add(af, "--yes");
    else if (pgpopts->force == 0)
        argv_factory_add(af, "--no");
}



static void build_batchmode(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->batchmode)
    {
        argv_factory_add(af, "--batch");
        argv_factory_add(af, "--always-trust");
        build_force(af, pgpopts);
    }
    else
        argv_factory_add(af, "--no-batch");
}


static void build_comment(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->comment)
    {
        argv_factory_add(af, "--comment");
        argv_factory_add(af, pgpopts->comment);
    }
}


static void build_compress(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->compress == 0)
    {
        argv_factory_add(af, "-z");
        argv_factory_add(af, "0");
    }
    else if (pgpopts->compress == 1)
    {
        argv_factory_add(af, "-z");
        argv_factory_add(af, "6");
    }
}


static void build_certdepth(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->completesneeded)
    {
        argv_factory_add(af, "--completes-needed");
        argv_factory_add(af, pgpopts->completesneeded);
    }
    if (pgpopts->marginalsneeded)
    {
        argv_factory_add(af, "--marinals-needed");
        argv_factory_add(af, pgpopts->marginalsneeded);
    }
    if (pgpopts->certdepth)
    {
        argv_factory_add(af, "--max-cert-depth");
        argv_factory_add(af, pgpopts->certdepth);
    }
}


static void build_localuser(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->localuserid)
    {
        argv_factory_add(af, "--local-user");
        argv_factory_add(af, pgpopts->localuserid);
    }
}


static void build_charset(ArgvFactory *af, Pgpopts *pgpopts)
{
    assert(af != NULL);
    assert(pgpopts != NULL);
    
    if (pgpopts->charset)
    {
        argv_factory_add(af, "--charset");
        argv_factory_add(af, pgpopts->charset);
    }
}


static int  build_input(ArgvFactory *af, Pgpopts *opts)
{
    int implicit_filter = !opts->stdinisterminal && opts->args_used == 0;
    
    if (!opts->f && !implicit_filter && opts->args_used==0)
    {
        fprintf(stderr, "Error: Filename required.\n");
        return -1;
    }
    
    if (!opts->f && !implicit_filter)
        argv_factory_add(af, opts->args[0]);
    
    opts->f = 0;
    
    return 0;
}


static void build_output(ArgvFactory *af, Pgpopts *opts)
{
    int implicit_filter = !opts->stdinisterminal && opts->args_used == 0;
    
    DEBUG(("opts->stdinisterminal: %d", opts->stdinisterminal))
    DEBUG(("opts->args_used: %d", opts->args_used))
    DEBUG(("implicit_filter: %d", implicit_filter))
    
    if (opts->f || implicit_filter)
    {
        argv_factory_add(af, "--set-filename");
        argv_factory_add(af, "stdin");
    }
    
    if (opts->outputfile)
    {
        argv_factory_add(af, "--output");
        argv_factory_add(af, opts->outputfile);
    }
    else
        if (!opts->f)
            if (implicit_filter)
            {
                argv_factory_add(af, "--output");
                argv_factory_add(af, opts->a || opts->asciiarmor ? "stdin.asc" : "stdin.pgp");
            }
            else if (opts->args_used != 0)	/* Hint: if this condition is not given, it will result in an error on build_input() */
            {
                char *s = (char *)malloc((strlen(opts->args[0])+5) * sizeof(char));
                
                strcpy(s, opts->args[0]);
                
                if (opts->a || opts->asciiarmor)	/* ascii armored output */
                    strcat(s, ".asc");
                else if (opts->s && opts->b)		/* detached sign */
                    strcat(s, ".sig");
                else					/* binary output */
                    strcat(s, ".pgp");
                
                argv_factory_add(af, "--output");
                argv_factory_add(af, s);
            }
}


static void build_passfd(ArgvFactory *af, Pgpopts *opts)
{
    if (opts->passphrase_pipe[0] != -1)
    {
        char *arg = (char *)malloc(16);
        sprintf(arg, "%d", opts->passphrase_pipe[0]);
        argv_factory_add(af, "--passphrase-fd");
        argv_factory_add(af, arg);
    }
}


#if 0
static void build_passfd_pgpbug(ArgvFactory *af, Pgpopts *opts)
{
    /* Work arround a PGP 2.6x Bug */
    if (opts->passphrase_pipe[0] == -1  &&  getenv("PGPPASSFD"))
    {
        argv_factory_add(af, "--passphrase-fd");
        argv_factory_add(af, "0");
    }
}
#endif



static void build_keyrings(ArgvFactory *af, Pgpopts *opts)
{
    if (opts->pubring)
    {
        argv_factory_add(af, "--no-default-keyring");
        argv_factory_add(af, "--keyring");
        argv_factory_add(af, opts->pubring);
    }
    
    if (opts->secring)
    {
        argv_factory_add(af, "--secret-keyring");
        argv_factory_add(af, opts->secring);
    }
}




/*****************************************************************************
 *
 * Function : pgpopts_build_argv
 *
 * Purpose  : Contructs an argv style array of GnuPG options out of a Pgpopts
 *            structure.
 *
 * Input    : pgpopts - A Pgpopts structure to build the GnuPG options from.
 *
 * Output   : An argv style vector or NULL on error.
 *
 * Errors   : On en error NULL is returned.
 *
 * Notes    : PGP 2.6 contains a lot of bugs in the command line option parsing.
 *            However, many tools and scripts which use PGP assume this bugs
 *            so we will try to simulate this PGP bugs.
 *
 *****************************************************************************/
char ** pgpopts_build_argv(Pgpopts *pgpopts)
{
    Pgpopts	opts;
    ArgvFactory	af;
    
    assert(pgpopts != NULL);
    
    opts = *pgpopts;	/* don't modify pgpopts */
    
    argv_factory_init(&af);
    argv_factory_add(&af, GPG_PATH);
    
    if (opts.k)
    {
        /* key commands */
        --opts.k;
        
        build_batchmode(&af, &opts);
        build_force(&af, &opts);
        build_certdepth(&af, &opts);
        
        if (opts.g == 1)			/* -kg create new key */
        {
            argv_factory_add(&af, "--gen-key");
            opts.g = 0;
            
            if (opts.args_used > 0)
                fprintf(stderr, "Info: The optional keybits argument '%s' is not used.\n", opts.args[0]);
        }
        else if (opts.x == 0 && opts.a == 1)	/* -ka import key */
        {
            argv_factory_add(&af, "--import");
            opts.a = 0;
            if (!opts.f)
            {
                if (opts.args_used == 0)
                {
                    fprintf(stderr, "Error: Keyfile required.\n");
                    return NULL;
                }
                
                if (opts.args_used > 1)
                {
                    fprintf(stderr, "BUG: Keyring argument not supported.\n");
                    return NULL;
                }
                
                argv_factory_add(&af, opts.args[0]);
            }
            else
                opts.f = 0;	/* yes, on -kaf, PGP just ignores all arguments */
        }
        else if (opts.x == 1)			/* -kx export key */
        {
            build_asciiarmor(&af, &opts);
            argv_factory_add(&af, "--export");
            opts.x = 0;
            if (opts.args_used == 0)
            {
                fprintf(stderr, "Error: User-ID required for exporting a key.\n");
                return NULL;
            }
            if (opts.f)
            {
                opts.f = 0;
                if (opts.args_used == 1)
                    argv_factory_add(&af, opts.args[0]);
                else
                {
                    fprintf(stderr, "BUG: Keyring argument not supported.\n");
                    return NULL;
                }
            }
            else
            {
                if (opts.args_used < 2)
                {
                    fprintf(stderr, "Error: File to store exported key required.\n");
                    return NULL;
                }
                
                if (opts.args_used > 2)
                {
                    fprintf(stderr, "BUG: Keyring argument not supported.\n");
                    return NULL;
                }
                
                argv_factory_add(&af, "--output");
                argv_factory_add(&af, opts.args[1]);
                argv_factory_add(&af, opts.args[0]);
            }
        }
        else if (opts.v)
        {
            if (opts.v >= 2)
                argv_factory_add(&af, "--verbose");
            
            if (opts.c == 0)
                argv_factory_add(&af, "--list-keys");	/* -kv / -kvv */
            else
            {
                argv_factory_add(&af, "--fingerprint");	/* -kvc / -kvvc */
                opts.c = 0;
            }
            
            if (opts.args_used == 1)
                argv_factory_add(&af, opts.args[0]);
            
            if (opts.args_used >= 2)
            {
                fprintf(stderr, "BUG: Keyring argument not supported.\n");
                return NULL;
            }
        }
        else if (opts.c)			/* -kc check key signatures */
        {
            argv_factory_add(&af, "--check-sigs");
            opts.c = 0;
            
            if (opts.args_used == 1)
                argv_factory_add(&af, opts.args[0]);
            
            if (opts.args_used >= 2)
            {
                fprintf(stderr, "BUG: Keyring argument not supported.\n");
                return NULL;
            }
        }
        else if (opts.r == 1 && opts.s == 0)	/* -kr delete key */
        {
            opts.r = 0;
            argv_factory_add(&af, "--delete-key");
            
            if (opts.args_used == 0)
            {
                fprintf(stderr, "Error: User ID required for deleting a key.\n");
                return NULL;
            }
            
            if (opts.args_used >= 2)
            {
                fprintf(stderr, "BUG: Keyring argument not supported.\n");
                return NULL;
            }
            
            argv_factory_add(&af, opts.args[0]);
        }
        else if (opts.r == 1 && opts.s == 1)	/* -krs remove signature from key */
        {
            opts.r = 0;
            opts.s = 0;
            
            fprintf(stderr, "BUG: GnuPG doesn't support removing a key signature.\n");
            return NULL;
        }
        else if (opts.r == 0 && opts.s == 1)	/* -ks add signature to key */
        {
            opts.s = 0;
            argv_factory_add(&af, "--edit-key");
            
            if (opts.args_used == 0)
            {
                fprintf(stderr, "Error: User ID required for signing a key.\n");
                return NULL;
            }
            
            if (opts.args_used >= 2)
            {
                fprintf(stderr, "BUG: Keyring argument not supported.\n");
                return NULL;
            }
            
            argv_factory_add(&af, opts.args[0]);
            argv_factory_add(&af, "sign");
            argv_factory_add(&af, "quit");
            
        }
        else if (opts.e)			/* -ke edit key */
        {
            argv_factory_add(&af, "--edit-key");
            opts.e = 0;
            
            if (opts.args_used == 0)
            {
                fprintf(stderr, "Error: User ID required for editing a key.\n");
                return NULL;
            }
            
            if (opts.args_used >= 2)
            {
                fprintf(stderr, "BUG: Keyring argument not supported.\n");
                return NULL;
            }
            
            argv_factory_add(&af, opts.args[0]);
        }
        else if (opts.d)			/* -kd create key revocation certificate */
        {
            argv_factory_add(&af, "--gen-revoke");
            opts.d = 0;
            
            if (opts.args_used == 0)
            {
                fprintf(stderr, "Error: User ID required for generating key revokation certificate.\n");
                return NULL;
            }
            
            if (opts.args_used >= 2)
            {
                fprintf(stderr, "BUG: Keyring argument not supported.\n");
                return NULL;
            }
            
            argv_factory_add(&af, opts.args[0]);
        }
        else
        {
            fprintf(stderr, "Invalid key command option\n");
            return NULL;
        }
    }
    else if (opts.s==0 && opts.e==0 && opts.c==0)
    {
        build_batchmode(&af, &opts);
        
        if (opts.a || opts.asciiarmor)			/* store ascii armored */
        {
            build_comment(&af, &opts);
            build_compress(&af, &opts);
            build_charset(&af, &opts);
            build_output(&af, &opts);
            argv_factory_add(&af, "--armor");
            argv_factory_add(&af, "--store");
            if (build_input(&af, &opts))
                return NULL;
        }
        else						/* decrypt, verify */
        {
            opts.d = 0;	/* FIXME: How to handle these flags?? */
            opts.p = 0;
            
            build_certdepth(&af, &opts);
            build_keyrings(&af, &opts);
            
            /*
             * Try to be a little bit intelligent and try to determine if the
             * user would like to decrypt or verify a message a verify a
             * detached signature.
             */
            
            if (opts.args_used == 0)
            {
                /*
                 * It's easy: Decrypt or verify a message from stdin
                 */
                
                build_passfd(&af, &opts);
                
                if (opts.outputfile)
                {
                    argv_factory_add(&af, "--output");
                    argv_factory_add(&af, opts.outputfile);
                }
                
                if (opts.f)
                {
                    opts.f = 0;
                    argv_factory_add(&af, "--decrypt");
                }
                else if (opts.stdinisterminal)
                {
                    fprintf(stderr, "Error: Filename for decryption needed.\n");
                    return NULL;
                }
            }
            else if (opts.args_used >= 2)
            {
                /*
                 * It's (hopefully) easy: Verify a cleartext signature. But this
                 *  only works, if the user didn't made a misstake on command line
                 *  options. The bad thing is, we can't check if the user made a
                 *  misstake or signature verification failed...
                 */
                
                argv_factory_add(&af, "--verify");
                argv_factory_add(&af, opts.args[0]);
                argv_factory_add(&af, opts.args[1]);
            }
            else
            {
                /*
                 * It's complicated: We must try to guess if the user would like 
                 * to decrypt a file or verify a detached signature. This process
                 * doesn't work if the detached signature doesn't end with `.sig'
                 */
                
                char *file_base = strdup(opts.args[0]);
                char *file_ext  = strrchr(file_base, '.');
                
                if (file_ext)
                    *file_ext++ = 0;
                
                if (file_ext && !strcasecmp(file_ext, "sig"))
                {
                    /*
                     * Yes, the user would like to check a detached signature.
                     * We asume the correctly named data file will exists.
                     */
                    
                    argv_factory_add(&af, "--verify");
                    argv_factory_add(&af, opts.args[0]);
                    argv_factory_add(&af, file_base);
                }
                else if(file_ext && !strcasecmp(file_ext, "asc"))
                {
                    /*
                     * The given file is armored data. We try to determine the type of
                     * armored data. However, this will not work correctly with PGP 2.6
                     * generated data.
                     */
                    
                    switch (guess_armor_type(opts.args[0])) 
                    {
                        case ARMOR_TYPE_SIGNATURE:
                            /*
                             * Yes, a detached signature. Assume their is a file to check
                             * this signature against it, without the .asc extension.
                             */
                            argv_factory_add(&af, "--verify");
                            argv_factory_add(&af, opts.args[0]);
                            argv_factory_add(&af, file_base);
                            break;
                        
                        case ARMOR_TYPE_SIGNED_MESSAGE:
                            /*
                             * This is a clear sign message. Check the signatures.
                             */
                            if (opts.outputfile)
                            {
                                argv_factory_add(&af, "--output");
                                argv_factory_add(&af, opts.outputfile);
                            }
                            if (opts.f)
                            {
                                opts.f = 0;
                                argv_factory_add(&af, "--decrypt");
                            }
                            argv_factory_add(&af, opts.args[0]);
                            break;
                        
                        default:
                            /*
                             * This armored data could be either a signed/encrypted or
                             * a detached signature.
                             * If a file without the extension .asc exists assume the user
                             * would like to check a detached signature.
                             */
                            if (access(file_base, R_OK)==0)
                            {
                                /* Assume a detached signature */
                                argv_factory_add(&af, "--verify");
                                argv_factory_add(&af, opts.args[0]);
                                argv_factory_add(&af, file_base);
                            }
                            else
                            {
                                if (opts.outputfile)
                                {
                                    argv_factory_add(&af, "--output");
                                    argv_factory_add(&af, opts.outputfile);
                                }
                                
                                if (opts.f) 
                                {
                                    opts.f = 0;
                                    argv_factory_add(&af, "--decrypt");
                                }
                                
                                argv_factory_add(&af, opts.args[0]);
                                
                                /* FIXME: should we close stdin ??? */
                            }
                    }
                }
                else
                {
                    /*
                     * We are not sure what type the input is.
                     * Hopefully this isn't a renamed detached signature...
                     */
                    
                    int null_fd;
                    
                    if (opts.outputfile)
                    {
                        argv_factory_add(&af, "--output");
                        argv_factory_add(&af, opts.outputfile);
                    }
                    
                    if (opts.f)
                    {
                        opts.f = 0;
                        argv_factory_add(&af, "--decrypt");
                    }
                    
                    argv_factory_add(&af, opts.args[0]);
                    
                    /*
                     * Redirect stdin to /dev/null to prevent waiting for data
                     * on stdin when "decrypting" a detached signature.
                     */
                    
                    if ((null_fd = open("/dev/null", O_RDWR)) == -1)
                    {
                        fprintf(stderr, "Can't open /dev/null\n");
                        return NULL;
                    }
                    close(0);
                    dup2(null_fd, 0);
                    close(null_fd);
                }
            }
        }
    }
    else
    {
        int textmode = opts.t;
        
        build_textmode(&af, &opts);
        build_asciiarmor(&af, &opts);
        build_batchmode(&af, &opts);
        build_force(&af, &opts);
        build_comment(&af, &opts);
        build_compress(&af, &opts);
        build_charset(&af, &opts);
        build_passfd(&af, &opts);
        build_keyrings(&af, &opts);
        
        if (opts.c==1 && opts.e==0 && opts.s==0)	/* encrypt symmetric */
        {
            opts.c = 0;
            build_output(&af, &opts);
            argv_factory_add(&af, "--symmetric");
            if (build_input(&af, &opts))
                return NULL;
        }
        else if (opts.c==0 && opts.e==1)		/* encrypt asymmetric*/
        {
            int i;
            
            opts.e = 0;
            build_certdepth(&af, &opts);
            build_output(&af, &opts);
            
            for (i = !opts.f && opts.args_used != 0 ? 1 : 0; opts.args[i]; ++i)
            {
                argv_factory_add(&af, "--recipient");
                argv_factory_add(&af, opts.args[i]);
            }
            
            if (opts.encrypttoself)
            {
                argv_factory_add(&af, "--encrypt-to");
                if (opts.localuserid)
                    argv_factory_add(&af, opts.localuserid);
                else
                {
                    char *id = get_defaultkey();
                    if (!id)
                    {
                        fprintf(stderr, "Error: Can't get default key user id.\n");
                        return NULL;
                    }
                    argv_factory_add(&af, id);
                }
            }
            
            if (opts.s)
            {
                opts.s = 0;
                build_localuser(&af, &opts);
                argv_factory_add(&af, "--sign");
            }
            
            argv_factory_add(&af, "--encrypt");
            
            if (build_input(&af, &opts))
                return NULL;
            
            opts.f = 0;
        }
        else if (opts.c==0 && opts.e==0 && opts.s==1)	/* sign */
        {
            build_localuser(&af, &opts);
            build_output(&af, &opts);
            
            opts.s = 0; /* build_output() uses this flag too */
            
            if (opts.clearsign && (opts.a || opts.asciiarmor) && textmode && opts.b==0)
            {
                argv_factory_add(&af, "--escape-from");
                argv_factory_add(&af, "--clearsign");
            }
            else if (opts.b==1)				/* detached sign */
            {
                argv_factory_add(&af, "--detach-sign");
                opts.b = 0;
            }
            else
                argv_factory_add(&af, "--sign");
            
            if (build_input(&af, &opts))
                return NULL;
        }
        else						/* unknown commands */
        {
            fprintf(stderr, "Error: Unknown command option combination.\n");
            return NULL;
        }
    }
    
    if (check_opts_used(&opts))
    {
        fprintf(stderr, "Error: Invalid options\n");
        return NULL;
    }
    
    return af.argv;
}




#define OPT_INITIAL             "hkesabcdpo:@:z:u:fwmt"
#define OPT_KEY                 "gafxcresdvu:"
#define OPT_ENCRYPT             "astfo:@:z:u:wm"
#define OPT_SIGN                "aetfbo:z:u:wm"
#define OPT_SIGN_ENCRYPT        "atfo:@:z:u:wm"
#define OPT_SYMMETRIC           "atfo:z:u:wm"
#define OPT_DECRYPT             "pdfo:@:z:"




/*****************************************************************************
 *
 * Function : pgpopts_parse
 *
 * Purpose  : Parses a command line options style argv vector with PGP 2.6 like
 *            options and fills the Pgpopts structure.
 *
 * Input    : pgpopts - The Pgpopts structure to fill.
 *            argc    - The count of valid entries in the `argv' vector.
 *            argv    - The vector with the command line options.
 *
 * Output   : Returns 0 if all gone right.
 *
 *****************************************************************************/
int pgpopts_parse(Pgpopts *pgpopts, int argc, char *argv[])
{
    int result;
    char *opt_format;
    
    assert(pgpopts != NULL);
    assert(argc >= 0);
    assert(argv != NULL);
    
    /* initial format */
    opt_format = OPT_INITIAL;
    while ( (result=mygetopt(argc, argv, opt_format)) != EOF )
        switch(result)
        {
            case 1:
                if (*myoptarg == '+')
                {
                    if (parse_longopt(pgpopts, myoptarg+1))
                        return -1;
                }
                else
                    add_argument(pgpopts, myoptarg);
                break;
            
            case 'k':
                opt_format = OPT_KEY;
                pgpopts->k++;
                break;
                
            case 's':
                pgpopts->s++;
                if (!pgpopts->k)
                    opt_format = pgpopts->e ? OPT_SIGN_ENCRYPT : OPT_SIGN;
                break;
            
            case 'e':
                pgpopts->e++;
                if (!pgpopts->k)
                    opt_format = pgpopts->s ? OPT_SIGN_ENCRYPT : OPT_ENCRYPT;
                break;
            
            case 'c':
                pgpopts->c++;
                if (!pgpopts->k)
                    opt_format = OPT_SYMMETRIC;
                break;
            
            case 'd':
                pgpopts->d++;
                opt_format = OPT_DECRYPT;
                break;
            
            case 'p':
                pgpopts->p++;
                opt_format = OPT_DECRYPT;
                break;
            
            case 'z':
                if (!pgpopts->passphrase)	/* yes, PGP uses the first -z option */
                    pgpopts->passphrase = myoptarg;
                break;
            
            case 'o': pgpopts->outputfile = myoptarg; break;
            case 'u': pgpopts->localuserid = myoptarg; break;
            
            case 'g': pgpopts->g++; break;
            case 'a': pgpopts->a++; break;
            case 'x': pgpopts->x++; break;
            case 'v': pgpopts->v++; break;
            case 'r': pgpopts->r++; break;
            case 'b': pgpopts->b++; break;
            case 'f': pgpopts->f++; break;
            case 'w': pgpopts->w++; break;
            case 'm': pgpopts->m++; break;
            case 't': pgpopts->t++; break;
            
            case '@':
                fprintf(stderr, "BUG: Option `-@' not supported.\n");
                return -1;
            
            case '?':
                printf("Invalid option: -%c\n", myoptopt);
                return -1;
                break;
            
            case 'h':
                printf("Help not supported.\n");
                return -1;
                break;
            
            default:
                fprintf(stderr, "BUG: Unhandled option: -%c\n", result);
                return -1;
        }
    
    return 0;
}








