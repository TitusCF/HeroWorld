/**
 * @file
 * This code was grabbed from the Netrek source and modified to work with
 * Crossfire.
 */

#include <global.h>
#include <sproto.h>
#ifndef WIN32 /* ---win32 : remove unix headers */
#include <sys/ioctl.h>
#endif /* win32 */
#ifdef hpux
#include <sys/ptyio.h>
#endif

#ifndef WIN32 /* ---win32 : remove unix headers */
#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#endif /* win32 */

/**
 * Check if a player and/or host is banned. Wildcards can be used.
 *
 * @param login
 * player name to check; NULL to check only the host name.
 * @param host
 * host name to check.
 *
 * @return
 * 1=player/host is banned; 0=player/host is not banned
 */
int checkbanned(const char *login, const char *host) {
    FILE *bannedfile;
    char buf[MAX_BUF];
    char log_buf0[160], host_buf[64], line_buf[160];
    char *indexpos;
    int num1;
    int hits = 0;               /* Hits==2 means we're banned */
    int loop;

    /* Inverse ban feature: if a line is prefixed by a ~, then we will
     * immediately return "check passed" if it matches. This allow to ban a
     * network, but let a part of it still connect.
     */
    int inverse_ban = 0;

    for (loop = 0; loop < 2; loop++) {  /* have to check both ban files now */
        /* First time through look for BANFILE */

        if (loop == 0) {
            snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, BANFILE);
            bannedfile = fopen(buf, "r");
            if (bannedfile == NULL) {
                LOG(llevDebug, "Could not find file Banned file\n");
                continue;
            }
        }

        /* Second time through look for BANISHFILE */

        if (loop == 1) {
            snprintf(buf, sizeof(buf), "%s/%s", settings.localdir, BANISHFILE);
            bannedfile = fopen(buf, "r");
            if (bannedfile == NULL) {
                LOG(llevDebug, "Could not find file Banish file\n");
                return(0);
            }
        }

        /* Do the actual work here checking for banned IPs */

        while (fgets(line_buf, 160, bannedfile) != NULL) {
            char *log_buf = log_buf0;

            inverse_ban = 0;
            hits = 0;

            /* Split line up */
            if (*line_buf == '#' || *line_buf == '\n')
                continue;

            indexpos = strrchr(line_buf, '@');
            if (indexpos == NULL) {
                LOG(llevDebug, "Bad line in banned file\n");
                continue;
            }

            /* copy login name into log_buf */
            num1 = indexpos-line_buf;
            strncpy(log_buf, line_buf, num1);
            log_buf[num1] = '\0';

            /* copy host name into host_buf */
            strncpy(host_buf, indexpos+1, sizeof(host_buf)-1);
            host_buf[sizeof(host_buf)-1] = '\0';

            /* Cut off any extra spaces on the host buffer */
            indexpos = host_buf;
            while (!isspace(*indexpos)) {
                indexpos++;
            }
            *indexpos = '\0';

            if (*log_buf == '~') {
                log_buf++;
                inverse_ban = 1;
            }

            /*
              LOG(llevDebug, "Login: <%s>; host: <%s>\n", login, host);
              LOG(llevDebug, "    Checking Banned <%s> and <%s>.\n", log_buf, host_buf);
            */

            if (*log_buf == '*')
                hits = 1;
            else if (login != NULL && strcmp(login, log_buf) == 0)
                hits = 1;

            if (hits == 1) {
                if (*host_buf == '*') { /* Lock out any host */
                    hits++;

                    /* break out now. otherwise hits will get reset to one */
                    break;
                } else if (strstr(host, host_buf) != NULL) { /* Lock out subdomains (eg, "*@usc.edu" */
                    hits++;

                    /* break out now. otherwise hits will get reset to one */
                    break;
                } else if (strcmp(host, host_buf) == 0) { /* Lock out specific host */
                    hits++;

                    /* break out now. otherwise hits will get reset to one */
                    break;
                }
            }
        } /* loop for one file */

        fclose(bannedfile);

        if (hits >= 2) {
            return(!inverse_ban);
        }
    }

    return(0);
}
