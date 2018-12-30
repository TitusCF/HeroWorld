/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2014 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file
 * Player-related functions, including those used during the login and creation phases.
 * @todo describe login/creation functions/cycles.
 */

#include <global.h>
#include <assert.h>
#ifndef WIN32 /* ---win32 remove headers */
#include <pwd.h>
#endif
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <sounds.h>
#include <living.h>
#include <object.h>
#include <spells.h>
#include <skills.h>
#include <shared/newclient.h>

static archetype *get_player_archetype(archetype *at);

static void kill_player_not_permadeath(object *op);

static void kill_player_permadeath(object *op);

static int action_makes_visible(object *op);

/**
 * Find a player by her full name.
 *
 * @param plname
 * name to find.
 * @return
 * matching player, or NULL if no match.
 */
player *find_player(const char *plname) {
    return find_player_options(plname, 0, NULL);
}

/**
 * Find a player.
 * @param plname name of the player to search for. Can be partial.
 * @param options combination of @ref FIND_PLAYER_xxx "FIND_PLAYER_xxx" flags.
 * @param map optional map the player must be on (adjacent maps are ok too).
 * @return matching player, NULL if none or more than one.
 */
player *find_player_options(const char *plname, int options, const mapstruct *map) {
    player *pl;
    player *found = NULL;
    size_t namelen = strlen(plname);
    char name[MAX_BUF];

    for (pl = first_player; pl != NULL; pl = pl->next) {
        if ((options & FIND_PLAYER_NO_HIDDEN_DM) && (QUERY_FLAG(pl->ob, FLAG_WIZ) && pl->ob->contr->hidden))
            continue;

        if (map != NULL && pl->ob->map != map)
            continue;

        if (!(options & FIND_PLAYER_PARTIAL_NAME)) {
            query_name(pl->ob, name, sizeof(name));
            if (!strcmp(name, plname))
                return pl;
            continue;
        }

        if (strlen(pl->ob->name) < namelen)
            continue;

        if (!strcmp(pl->ob->name, plname))
            return pl;

        if (!strncasecmp(pl->ob->name, plname, namelen)) {
            if (found)
                return NULL;

            found = pl;
        }
    }
    return found;
}

/**
 * Find a player by a partial name.
 *
 * @param plname
 * name to match.
 * @return
 * matching player if only one matching, or one perfectly matching, NULL if no match or more than one.
 */
player *find_player_partial_name(const char *plname) {
    return find_player_options(plname, FIND_PLAYER_PARTIAL_NAME, NULL);
}

/**
 * Return a player for a socket structure.
 * @param ns socket to search for.
 * @return NULL if no player, player else.
 */
player* find_player_socket(const socket_struct *ns) {
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next) {
        if (&pl->socket == ns)
            return pl;
    }
    return NULL;
}

/**
 * Sends the message of the day to the player.
 *
 * @param op
 * player to send to.
 */
void display_motd(const object *op) {
    char buf[MAX_BUF];
    char motd[HUGE_BUF];
    FILE *fp;
    int size;

    snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, settings.motd);
    fp = fopen(buf, "r");
    if (fp == NULL) {
        return;
    }
    motd[0] = '\0';
    size = 0;
    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        strncat(motd+size, buf, HUGE_BUF-size);
        size += strlen(buf);
    }
    draw_ext_info(NDI_UNIQUE|NDI_GREEN, 0, op, MSG_TYPE_MOTD, MSG_SUBTYPE_NONE,
                  motd);
    fclose(fp);
}

/**
 * Send the rules to a player.
 *
 * @param op
 * player to send rules to.
 */
void send_rules(const object *op) {
    char buf[MAX_BUF];
    char rules[HUGE_BUF];
    FILE *fp;
    int size;

    snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, settings.rules);
    fp = fopen(buf, "r");
    if (fp == NULL) {
        return;
    }
    rules[0] = '\0';
    size = 0;
    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        if (size+strlen(buf) >= HUGE_BUF) {
            LOG(llevDebug, "Warning, rules size is > %d bytes.\n", HUGE_BUF);
            break;
        }
        strncat(rules+size, buf, HUGE_BUF-size);
        size += strlen(buf);
    }
    draw_ext_info(NDI_UNIQUE|NDI_GREEN, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_RULES,
                  rules);
    fclose(fp);
}

/**
 * Send the news to a player.
 *
 * @param op
 * player to send to.
 */
void send_news(const object *op) {
    char buf[MAX_BUF];
    char news[HUGE_BUF];
    char subject[MAX_BUF];
    FILE *fp;
    int size;

    snprintf(buf, sizeof(buf), "%s/%s", settings.confdir, settings.news);
    fp = fopen(buf, "r");
    if (fp == NULL)
        return;
    news[0] = '\0';
    subject[0] = '\0';
    size = 0;
    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        if (*buf == '%') { /* send one news */
            if (size > 0)
                draw_ext_info_format(NDI_UNIQUE|NDI_GREEN, 0, op,
                                     MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_NEWS,
                                     "%s:\n%s",
                                     subject, news); /*send previously read news*/
            strcpy(subject, buf+1);
            strip_endline(subject);
            size = 0;
            news[0] = '\0';
        } else {
            if (size+strlen(buf) >= HUGE_BUF) {
                LOG(llevDebug, "Warning, one news item has size > %d bytes.\n", HUGE_BUF);
                break;
            }
            strncat(news+size, buf, HUGE_BUF-size);
            size += strlen(buf);
        }
    }

    draw_ext_info_format(NDI_UNIQUE|NDI_GREEN, 0, op,
                         MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_NEWS,
                         "%s:\n%s",
                         subject, news);
    fclose(fp);
}

/**
 * Is the player name valid.
 *
 * @param cp
 * name to test.
 * @return
 * 0 if invalid, 1 if valid.
 */
int playername_ok(const char *cp) {
    /* Don't allow - or _ as first character in the name */
    if (*cp == '-' || *cp == '_')
        return 0;

    for (; *cp != '\0'; cp++)
        if (!((*cp >= 'a' && *cp <= 'z') || (*cp >= 'A' && *cp <= 'Z'))
        && *cp != '-'
        && *cp != '_')
            return 0;
    return 1;
}

/**
 * Create a player's object, initialize a player's structure.
 *
 * This no longer sets the player map.  Also, it now updates
 * all the pointers so the caller doesn't need to do that.
 * Caller is responsible for setting the correct map.
 *
 * Redo this to do both get_player_ob and get_player.
 * Hopefully this will be bugfree and simpler.
 *
 * @param p
 * if NULL, a new player structure is created, else p is recycled.
 * @return
 * initialized player structure.
 */
player *get_player(player *p) {
    object *op = arch_to_object(get_player_archetype(NULL));
    int i;

    if (!p) {
        player *tmp;

        p = (player *)malloc(sizeof(player));
        if (p == NULL)
            fatal(OUT_OF_MEMORY);

        /* This adds the player in the linked list.  There is extra
         * complexity here because we want to add the new player at the
         * end of the list - there is in fact no compelling reason that
         * that needs to be done except for things like output of
         * 'who'.
         */
        tmp = first_player;
        while (tmp != NULL && tmp->next != NULL)
            tmp = tmp->next;
        if (tmp != NULL)
            tmp->next = p;
        else
            first_player = p;

        p->next = NULL;
        /* This only needs to be done on initial creation of player
         * object.  the callers of get_player() will copy over the
         * socket structure to p->socket, and if this is an existing
         * player object, that has been done.  The call to
         * roll_stats() below will try to send spell information to
         * the client - if this is non zero (eg, garbage from not
         * being cleared), that will cause problems.  So just clear
         * it, and no spell data is sent.
         */
        p->socket.monitor_spells = 0;
    } else {
        /* Only needed when reusing existing player. */
        clear_player(p);
    }

    /* Clears basically the entire player structure except
     * for next and socket.
     */
    memset((void *)((char *)p+offsetof(player, maplevel)), 0, sizeof(player)-offsetof(player, maplevel));

    /* There are some elements we want initialized to non zero value -
     * we deal with that below this point.
     */
    p->party = NULL;
    p->rejoin_party = party_rejoin_if_exists;
    p->unapply = unapply_nochoice;
    p->swap_first = -1;

#ifdef AUTOSAVE
    p->last_save_tick = 9999999;
#endif

    strcpy(p->savebed_map, first_map_path);  /* Init. respawn position */

    op->contr = p; /* this aren't yet in archetype */
    p->ob = op;
    op->speed_left = 0.5;
    op->speed = 1.0;
    op->direction = 5;     /* So player faces south */
    op->stats.wc = 2;
    op->run_away = 25; /* Then we panic... */

    roll_stats(op);
    player_set_state(p, ST_ROLL_STAT);
    clear_los(op);

    p->gen_sp_armour = 10;
    p->last_speed = -1;
    p->shoottype = range_none;
    p->bowtype = bow_normal;
    p->petmode = pet_normal;
    p->listening = 10;
    p->last_weapon_sp = -1;
    p->peaceful = 1;   /* default peaceful */
    p->do_los = 1;
    p->no_shout = 0;   /* default can shout */
    p->language = 0;
    p->unarmed_skill = NULL;
    p->ticks_played = 0;

    strncpy(p->title, op->arch->clone.name, sizeof(p->title)-1);
    p->title[sizeof(p->title)-1] = '\0';
    op->race = add_string(op->arch->clone.race);

    CLEAR_FLAG(op, FLAG_READY_SKILL);

    /* we need to clear these to -1 and not zero - otherwise,
     * if a player quits and starts a new character, we wont
     * send new values to the client, as things like exp start
     * at zero.
     */
    for (i = 0; i < NUM_SKILLS; i++) {
        p->last_skill_exp[i] = -1;
        p->last_skill_ob[i] = NULL;
    }
    for (i = 0; i < NROFATTACKS; i++) {
        p->last_resist[i] = -1;
    }
    p->last_stats.exp = -1;
    p->last_weight = (uint32)-1;

    p->socket.update_look = 0;
    p->socket.look_position = 0;
    return p;
}

/**
 * This loads the first map an puts the player on it.
 *
 * @param op
 * player to put on map.
 */
void set_first_map(object *op) {
    strcpy(op->contr->maplevel, first_map_path);
    op->x = -1;
    op->y = -1;
    enter_exit(op, NULL);
}

/**
 * This copies the data from the socket into
 * the player structure.
 * Originally written to separate this logic from
 * add_player() so new character method could use it,
 * but that did not work out, but still a good thing
 * to have this separate.
 * @param p
 * The target player object to copy the data into
 * @param ns
 * the socket structure to copy.
 */
static void set_player_socket(player *p, socket_struct *ns) {
    memcpy(&p->socket, ns, sizeof(socket_struct));

    /* The memcpy above copies the reference to faces sent.  So we need to clear
     * that pointer in ns, otherwise we get a double free.
     */
    ns->faces_sent = NULL;

    if (p->socket.faces_sent == NULL)
        fatal(OUT_OF_MEMORY);

    /* Needed because the socket we just copied over needs to be cleared.
     * Note that this can result in a client reset if there is partial data
     * on the incoming socket.
     */
    SockList_ResetRead(&p->socket.inbuf);


}

/**
 * Tries to add player on the connection passwd in ns.
 *
 * Player object is created and put on the first map, rules/news/motd are sent.
 *
 * @param ns
 * connection.
 * @param flags
 * flag values are define in player.h:
 * ADD_PLAYER_NEW -  If set, go right to new character creation - used in case
 *    of new account code.  We don't display motd and other bits in this case either.
 * ADD_PLAYER_NO_MAP - Do not set up map information - used in character login
 *    method >2 where we do not use the same starting map.
 * @return player
 * returns pointer to newly created player structure.
 */
player *add_player(socket_struct *ns, int flags) {
    player *p;

    p = get_player(NULL);
    set_player_socket(p, ns);

    CLEAR_FLAG(p->ob, FLAG_FRIENDLY);

    if (!(flags & ADD_PLAYER_NO_MAP))
        set_first_map(p->ob);

    add_friendly_object(p->ob);

    /* In this case, the client is provide all the informatin for the
     * new character, so just return it.  Do not display any messages,
     * etc
     */
    if (flags & ADD_PLAYER_NO_STATS_ROLL)
        return p;

    if (flags & ADD_PLAYER_NEW) {
        roll_again(p->ob);
        player_set_state(p, ST_ROLL_STAT);
    } else {
        send_rules(p->ob);
        send_news(p->ob);
        display_motd(p->ob);
        get_name(p->ob);
    }
    return p;
}

/**
 * Get next player archetype from archetype list.
 * Not very efficient routine, but used only creating new players.
 * @note there MUST be at least one player archetype! Will exit() if none.
 *
 * @param at
 * archetype to search from.
 * @return
 * next player archetype available.
 */
static archetype *get_player_archetype(archetype *at) {
    archetype *start = at;

    for (;;) {
        if (at == NULL || at->next == NULL)
            at = first_archetype;
        else
            at = at->next;
        if (at->clone.type == PLAYER)
            return at;
        if (at == start) {
            LOG(llevError, "No Player archetypes\n");
            exit(-1);
        }
    }
}

/**
 * Finds the nearest visible player for some object.
 *
 * @param mon
 * what object is searching a player.
 * @return
 * player, or NULL if nothing suitable.
 */
object *get_nearest_player(object *mon) {
    object *op = NULL;
    player *pl = NULL;
    objectlink *ol;
    unsigned lastdist;
    rv_vector rv;

    for (ol = first_friendly_object, lastdist = 1000; ol != NULL; ol = ol->next) {
        /* We should not find free objects on this friendly list, but it
         * does periodically happen.  Given that, lets deal with it.
         * While unlikely, it is possible the next object on the friendly
         * list is also free, so encapsulate this in a while loop.
         */
        while (QUERY_FLAG(ol->ob, FLAG_FREED) || !QUERY_FLAG(ol->ob, FLAG_FRIENDLY)) {
            object *tmp = ol->ob;

            /* Can't do much more other than log the fact, because the object
             * itself will have been cleared.
             */
            LOG(llevDebug, "get_nearest_player: Found free/non friendly object on friendly list\n");
            ol = ol->next;
            remove_friendly_object(tmp);
            if (!ol)
                return op;
        }

        /* Remove special check for player from this.  First, it looks to cause
         * some crashes (ol->ob->contr not set properly?), but secondly, a more
         * complicated method of state checking would be needed in any case -
         * as it was, a clever player could type quit, and the function would
         * skip them over while waiting for confirmation.  Remove
         * on_same_map check, as monster_can_detect_enemy() also does this
         */
        if (!monster_can_detect_enemy(mon, ol->ob, &rv))
            continue;

        if (lastdist > rv.distance) {
            op = ol->ob;
            lastdist = rv.distance;
        }
    }
    for (pl = first_player; pl != NULL; pl = pl->next) {
        if (monster_can_detect_enemy(mon, pl->ob, &rv)) {
            if (lastdist > rv.distance) {
                op = pl->ob;
                lastdist = rv.distance;
            }
        }
    }
    return op;
}

/**
 * This value basically determines how large a detour a monster will take from
 * the direction path when looking for a path to the player.
 *
 * The values are in the amount of direction the deviation is.
 *
 * I believe this can safely go to 2, 3 is questionable, 4 will likely
 * result in a monster paths backtracking.
 */
#define DETOUR_AMOUNT 2

/**
 * This is used to prevent infinite loops.  Consider a case where the
 * player is in a chamber (with gate closed), and monsters are outside.
 * with DETOUR_AMOUNT==2, the function will turn each corner, trying to
 * find a path into the chamber.  This is a good thing, but since there
 * is no real path, it will just keep circling the chamber for
 * ever (this could be a nice effect for monsters, but not for the function
 * to get stuck in.  I think for the monsters, if max is reached and
 * we return the first direction the creature could move would result in the
 * circling behaviour.  Unfortunately, this function is also used to determined
 * if the creature should cast a spell, so returning a direction in that case
 * is probably not a good thing.
 */
#define MAX_SPACES 50

/**
 * Returns the direction to the player, if valid.  Returns 0 otherwise.
 *
 * Modified to verify there is a path to the player.  Does this by stepping towards
 * player and if path is blocked then see if blockage is close enough to player that
 * direction to player is changed (ie zig or zag).  Continue zig zag until either
 * reach player or path is blocked.  Thus, will only return true if there is a free
 * path to player.  Though path may not be a straight line. Note that it will find
 * player hiding along a corridor at right angles to the corridor with the monster.
 *
 * Modified by MSW 2001-08-06 to handle tiled maps. Various notes:
 * - With DETOUR_AMOUNT being 2, it should still go and find players hiding
 * down corridors.
 * - I think the old code was broken if the first direction the monster
 * should move was blocked - the code would store the first direction without
 * verifying that the player can actually move in that direction.  The new
 * code does not store anything in firstdir until we have verified that the
 * monster can in fact move one space in that direction.
 * - I'm not sure how good this code will be for moving multipart monsters,
 * since only simple checks to blocked are being called, which could mean the monster
 * is blocking itself.
 *
 * @param mon
 * source object.
 * @param pl
 * target.
 * @param mindiff
 * minimal distance mon and pl should have.
 * @return
 * direction from mon to pl, 0 if can't get there.
 */
int path_to_player(object *mon, object *pl, unsigned mindiff) {
    rv_vector rv;
    sint16  x, y;
    int lastx, lasty, dir, i, diff, firstdir = 0, lastdir, max = MAX_SPACES, mflags, blocked;
    mapstruct *m, *lastmap;

    if (!get_rangevector(mon, pl, &rv, 0))
        return 0;

    if (rv.distance < mindiff)
        return 0;

    x = mon->x;
    y = mon->y;
    m = mon->map;
    dir = rv.direction;
    lastdir = firstdir = rv.direction; /* perhaps we stand next to pl, init firstdir too */
    diff = MAX(FABS(rv.distance_x), FABS(rv.distance_y));
    /* If we can't solve it within the search distance, return now. */
    if (diff > max)
        return 0;
    while (diff > 1 && max > 0) {
        lastx = x;
        lasty = y;
        lastmap = m;
        x = lastx+freearr_x[dir];
        y = lasty+freearr_y[dir];

        mflags = get_map_flags(m, &m, x, y, &x, &y);
        blocked = (mflags&P_OUT_OF_MAP) ? MOVE_ALL : GET_MAP_MOVE_BLOCK(m, x, y);

        /* Space is blocked - try changing direction a little */
        if ((mflags&P_OUT_OF_MAP)
        || ((OB_TYPE_MOVE_BLOCK(mon, blocked) || (mflags&P_IS_ALIVE))
        && (m == mon->map && blocked_link(mon, m, x, y)))) {
            /* recalculate direction from last good location.  Possible
             * we were not traversing ideal location before.
             */
            if (get_rangevector_from_mapcoord(lastmap, lastx, lasty, pl, &rv, 0) && rv.direction != dir) {
                /* OK - says direction should be different - lets reset the
                 * the values so it will try again.
                 */
                x = lastx;
                y = lasty;
                m = lastmap;
                dir = firstdir = rv.direction;
            } else {
                /* direct path is blocked - try taking a side step to
                 * either the left or right.
                 * Note increase the values in the loop below to be
                 * more than -1/1 respectively will mean the monster takes
                 * bigger detour.  Have to be careful about these values getting
                 * too big (3 or maybe 4 or higher) as the monster may just try
                 * stepping back and forth
                 */
                for (i = -DETOUR_AMOUNT; i <= DETOUR_AMOUNT; i++) {
                    if (i == 0)
                        continue; /* already did this, so skip it */
                    /* Use lastdir here - otherwise,
                     * since the direction that the creature should move in
                     * may change, you could get infinite loops.
                     * ie, player is northwest, but monster can only
                     * move west, so it does that.  It goes some distance,
                     * gets blocked, finds that it should move north,
                     * can't do that, but now finds it can move east, and
                     * gets back to its original point.  lastdir contains
                     * the last direction the creature has successfully
                     * moved.
                     */

                    x = lastx+freearr_x[absdir(lastdir+i)];
                    y = lasty+freearr_y[absdir(lastdir+i)];
                    m = lastmap;
                    mflags = get_map_flags(m, &m, x, y, &x, &y);
                    if (mflags&P_OUT_OF_MAP)
                        continue;
                    blocked = GET_MAP_MOVE_BLOCK(m, x, y);
                    if (OB_TYPE_MOVE_BLOCK(mon, blocked))
                        continue;
                    if (mflags&P_IS_ALIVE)
                        continue;

                    if (m == mon->map && blocked_link(mon, m, x, y))
                        break;
                }
                /* go through entire loop without finding a valid
                 * sidestep to take - thus, no valid path.
                 */
                if (i == (DETOUR_AMOUNT+1))
                    return 0;
                diff--;
                lastdir = dir;
                max--;
                if (!firstdir)
                    firstdir = dir+i;
            } /* else check alternate directions */
        } /* if blocked */
        else {
            /* we moved towards creature, so diff is less */
            diff--;
            max--;
            lastdir = dir;
            if (!firstdir)
                firstdir = dir;
        }
        if (diff <= 1) {
            /* Recalculate diff (distance) because we may not have actually
             * headed toward player for entire distance.
             */
            if (!get_rangevector_from_mapcoord(m, x, y, pl, &rv, 0))
                return 0;
            diff = MAX(FABS(rv.distance_x), FABS(rv.distance_y));
        }
        if (diff > max)
            return 0;
    }
    /* If we reached the max, didn't find a direction in time */
    if (!max)
        return 0;

    return firstdir;
}

/**
 * Gives a new player her initial items.
 *
 * They will be god-given, and suitable for the player's race/restrictions.
 *
 * @param pl
 * player.
 * @param items
 * treasure list containing the items.
 */
void give_initial_items(object *pl, treasurelist *items) {
    if (pl->randomitems != NULL)
        create_treasure(items, pl, GT_STARTEQUIP|GT_ONLY_GOOD, 1, 0);

    FOR_INV_PREPARE(pl, op) {
        /* Forces get applied per default, unless they have the
         * flag "neutral" set. Sorry but I can't think of a better way
         */
        if (op->type == FORCE && !QUERY_FLAG(op, FLAG_NEUTRAL))
            SET_FLAG(op, FLAG_APPLIED);

        /* we never give weapons/armour if these cannot be used
         * by this player due to race restrictions
         */
        if (pl->type == PLAYER) {
            if ((!QUERY_FLAG(pl, FLAG_USE_ARMOUR) && IS_ARMOR(op))
            || (!QUERY_FLAG(pl, FLAG_USE_WEAPON) && IS_WEAPON(op))
            || (!QUERY_FLAG(pl, FLAG_USE_SHIELD) && IS_SHIELD(op))) {
                object_remove(op);
                object_free_drop_inventory(op);
                continue;
            }
        }

        /* This really needs to be better - we should really give
         * a substitute spellbook.  The problem is that we don't really
         * have a good idea what to replace it with (need something like
         * a first level treasurelist for each skill.)
         * remove duplicate skills also
         */
        if (op->type == SPELLBOOK || op->type == SKILL) {
            int found;

            found = 0;
            FOR_BELOW_PREPARE(op, tmp)
                if (tmp->type == op->type && tmp->name == op->name) {
                    found = 1;
                    break;
                }
            FOR_BELOW_FINISH();
            if (found) {
                LOG(llevError, "give_initial_items: Removing duplicate object %s\n", op->name);
                object_remove(op);
                object_free_drop_inventory(op);
                continue;
            }
            if (op->nrof > 1)
                op->nrof = 1;
        }

        if (op->type == SPELLBOOK && op->inv) {
            CLEAR_FLAG(op->inv, FLAG_STARTEQUIP);
        }

        /* Give starting characters identified, uncursed, and undamned
         * items.  Just don't identify gold or silver, or it won't be
         * merged properly.
         */
        if (need_identify(op)) {
            SET_FLAG(op, FLAG_IDENTIFIED);
            CLEAR_FLAG(op, FLAG_CURSED);
            CLEAR_FLAG(op, FLAG_DAMNED);
        }
        if (op->type == SKILL)  {
            SET_FLAG(op, FLAG_CAN_USE_SKILL);
            op->stats.exp = 0;
            op->level = 1;
        }
        /* lock all 'normal items by default */
        else
            SET_FLAG(op, FLAG_INV_LOCKED);
    } FOR_INV_FINISH(); /* for loop of objects in player inv */

    /* Need to set up the skill pointers */
    link_player_skills(pl);

    /**
     * Now we do a second loop, to apply weapons/armors/...
     * This is because weapons require the skill, which can be given after the first loop.
     */
    FOR_INV_PREPARE(pl, op)
        if ((IS_ARMOR(op) || IS_WEAPON(op) || IS_SHIELD(op)) && !QUERY_FLAG(op, FLAG_APPLIED))
            apply_manual(pl, op, AP_NOPRINT);
    FOR_INV_FINISH();
}

/**
 * Waiting for the player's name.
 *
 * @param op
 * player.
 */
void get_name(object *op) {
    op->contr->write_buf[0] = '\0';
    player_set_state(op->contr, ST_GET_NAME);
    send_query(&op->contr->socket, 0, "What is your name?\n:");
}

/**
 * Waiting for the player's password.
 *
 * @param op
 * player.
 */
void get_password(object *op) {
    op->contr->write_buf[0] = '\0';
    player_set_state(op->contr, ST_GET_PASSWORD);
    send_query(&op->contr->socket, CS_QUERY_HIDEINPUT, "What is your password?\n:");
}

/**
 * Ask the player whether to play again or disconnect.
 *
 * @param op
 * player.
 */
void play_again(object *op) {
    SockList sl;

    player_set_state(op->contr, ST_PLAY_AGAIN);
    op->chosen_skill = NULL;

    /*
     * For old clients, ask if they want to play again.
     * For clients with account support, just return to character seletion (see below).
     */
    if (op->contr->socket.login_method == 0) {
        send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "Do you want to play again (a/q)?");
    }
    /* a bit of a hack, but there are various places early in th
     * player creation process that a user can quit (eg, roll
     * stats) that isn't removing the player.  Taking a quick
     * look, there are many places that call play_again without
     * removing the player - it probably makes more sense
     * to leave it to play_again to remove the object in all
     * cases.
     */
    if (!QUERY_FLAG(op, FLAG_REMOVED))
        object_remove(op);
    /* Need to set this to null - otherwise, it could point to garbage,
     * and draw() doesn't check to see if the player is removed, only if
     * the map is null or not swapped out.
     */
    op->map = NULL;

    SockList_Init(&sl);
    SockList_AddString(&sl, "player ");
    SockList_AddInt(&sl, 0);
    SockList_AddInt(&sl, 0);
    SockList_AddInt(&sl, 0);
    SockList_AddChar(&sl, 0);

    Send_With_Handling(&op->contr->socket, &sl);
    SockList_Term(&sl);

    if (op->contr->socket.login_method > 0) {
        receive_play_again(op, 'a');
    }
}

/**
 * Player replied to play again / disconnect.
 *
 * @param op
 * player.
 * @param key
 * received choice.
 */
void receive_play_again(object *op, char key) {
    if (key == 'q' || key == 'Q') {
        remove_friendly_object(op);
        leave(op->contr, 0); /* ericserver will draw the message */
        return;
    } else if (key == 'a' || key == 'A') {
        player *pl = op->contr;
        const char *name = op->name;

        add_refcount(name);
        remove_friendly_object(op);
        object_free_drop_inventory(op);
        pl = get_player(pl);
        op = pl->ob;
        add_friendly_object(op);
        op->contr->password[0] = '~';
        FREE_AND_CLEAR_STR(op->name);
        FREE_AND_CLEAR_STR(op->name_pl);
        if (pl->socket.login_method >= 1 && pl->socket.account_name != NULL) {
            /* If we are using new login, we send the
             * list of characters to the client - this should
             * result in the client popping up this list so
             * the player can choose which one to play - better
             * than going to legacy login code.
             * If the account_name is NULL, it means the client
             * says it uses account but started playing without logging in.
             */
            send_account_players(&pl->socket);
            player_set_state(pl, ST_GET_NAME);
        } else {
            /* Lets put a space in here */
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                      "\n");
            get_name(op);
            set_first_map(op);
        }
        op->name = name;  /* Already added a refcount above */
        op->name_pl = add_string(name);
    } else {
        /* user pressed something else so just ask again... */
        play_again(op);
    }
}

/**
 * Ask the player to confirm her password during creation.
 *
 * @param op
 * player.
 */
void confirm_password(object *op) {
    op->contr->write_buf[0] = '\0';
    player_set_state(op->contr, ST_CONFIRM_PASSWORD);
    send_query(&op->contr->socket, CS_QUERY_HIDEINPUT, "Please type your password again.\n:");
}

/**
 * Ask the player for the password of the party she wants to join.
 *
 * @param op
 * player.
 * @param party
 * party op wishes to join.
 * @return
 * whether a party password has been requested from the client
 */
int get_party_password(object *op, partylist *party) {
    if (*party_get_password(party) == '\0') {
        return 0;
    }

    op->contr->write_buf[0] = '\0';
    player_set_state(op->contr, ST_GET_PARTY_PASSWORD);
    op->contr->party_to_join = party;
    send_query(&op->contr->socket, CS_QUERY_HIDEINPUT, "What is the password?\n:");
    return 1;
}

/**
 * This rolls four 1-6 rolls and sums the best 3 of the 4.
 *
 * @return
 * sum of rolls.
 */
int roll_stat(void) {
    int a[4], i, j, k;

    for (i = 0; i < 4; i++)
        a[i] = (int)RANDOM()%6+1;

    for (i = 0, j = 0, k = 7; i < 4; i++)
        if (a[i] < k)
            k = a[i],
            j = i;

    for (i = 0, k = 0; i < 4; i++) {
        if (i != j)
            k += a[i];
    }
    return k;
}

/**
 * Roll the initial player's statistics.
 *
 * @param op
 * player to roll for.
 */
void roll_stats(object *op) {
    int sum = 0;
    int i = 0, j = 0;
    int statsort[7];

    do {
        op->stats.Str = roll_stat();
        op->stats.Dex = roll_stat();
        op->stats.Int = roll_stat();
        op->stats.Con = roll_stat();
        op->stats.Wis = roll_stat();
        op->stats.Pow = roll_stat();
        op->stats.Cha = roll_stat();
        sum = op->stats.Str+op->stats.Dex+op->stats.Int+op->stats.Con+op->stats.Wis+op->stats.Pow+op->stats.Cha;
    } while (sum != settings.roll_stat_points);

    /* Sort the stats so that rerolling is easier... */
    statsort[0] = op->stats.Str;
    statsort[1] = op->stats.Dex;
    statsort[2] = op->stats.Int;
    statsort[3] = op->stats.Con;
    statsort[4] = op->stats.Wis;
    statsort[5] = op->stats.Pow;
    statsort[6] = op->stats.Cha;

    /* a quick and dirty bubblesort? */
    do {
        if (statsort[i] < statsort[i+1]) {
            j = statsort[i];
            statsort[i] = statsort[i+1];
            statsort[i+1] = j;
            i = 0;
        } else {
            i++;
        }
    } while (i < 6);

    op->stats.Str = statsort[0];
    op->stats.Dex = statsort[1];
    op->stats.Con = statsort[2];
    op->stats.Int = statsort[3];
    op->stats.Wis = statsort[4];
    op->stats.Pow = statsort[5];
    op->stats.Cha = statsort[6];

    op->contr->orig_stats.Str = op->stats.Str;
    op->contr->orig_stats.Dex = op->stats.Dex;
    op->contr->orig_stats.Int = op->stats.Int;
    op->contr->orig_stats.Con = op->stats.Con;
    op->contr->orig_stats.Wis = op->stats.Wis;
    op->contr->orig_stats.Pow = op->stats.Pow;
    op->contr->orig_stats.Cha = op->stats.Cha;

    op->level = 1;
    op->stats.exp = 0;
    op->stats.ac = 0;

    op->contr->levhp[1] = 9;
    op->contr->levsp[1] = 6;
    op->contr->levgrace[1] = 3;

    fix_object(op);
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = op->stats.maxgrace;
    op->contr->orig_stats = op->stats;
}

/**
 * Ask the player what to do with the statistics.
 *
 * @param op
 * player.
 */
void roll_again(object *op) {
    esrv_new_player(op->contr, 0);
    send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "<y> to roll new stats <n> to use stats\n<1-7> <1-7> to swap stats.\nRoll again (y/n/1-7)?  ");
}

/**
 * Player finishes selecting what stats to swap.
 *
 * @param op
 * player.
 * @param swap_second
 * second statistic to swap.
 * @todo why the reinit of exp/ac/...?
 */
static void swap_stat(object *op, int swap_second) {
    signed char tmp;

    if (op->contr->swap_first == -1) {
        LOG(llevError, "player.c:swap_stat() - swap_first is -1\n");
        return;
    }

    tmp = get_attr_value(&op->contr->orig_stats, op->contr->swap_first);

    set_attr_value(&op->contr->orig_stats, op->contr->swap_first, get_attr_value(&op->contr->orig_stats, swap_second));

    set_attr_value(&op->contr->orig_stats, swap_second, tmp);

    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_NEWPLAYER,
                         "%s done\n",
                         short_stat_name[swap_second]);

    op->stats.Str = op->contr->orig_stats.Str;
    op->stats.Dex = op->contr->orig_stats.Dex;
    op->stats.Con = op->contr->orig_stats.Con;
    op->stats.Int = op->contr->orig_stats.Int;
    op->stats.Wis = op->contr->orig_stats.Wis;
    op->stats.Pow = op->contr->orig_stats.Pow;
    op->stats.Cha = op->contr->orig_stats.Cha;
    op->stats.ac = 0;

    op->level = 1;
    op->stats.exp = 0;
    op->stats.ac = 0;

    op->contr->levhp[1] = 9;
    op->contr->levsp[1] = 6;
    op->contr->levgrace[1] = 3;

    fix_object(op);
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = op->stats.maxgrace;
    op->contr->orig_stats = op->stats;
    op->contr->swap_first = -1;
}

/**
 * Player is currently swapping stats.
 *
 * This code has been greatly reduced, because with set_attr_value
 * and get_attr_value, the stats can be accessed just numeric
 * ids.  stat_trans is a table that translate the number entered
 * into the actual stat.  It is needed because the order the stats
 * are displayed in the stat window is not the same as how
 * the number's access that stat.  The table does that translation.
 *
 * @param op
 * player.
 * @param key
 * received key.
 */
void key_roll_stat(object *op, char key) {
    int keynum = key-'0';
    static const sint8 stat_trans[] = {
        -1,
        STR,
        DEX,
        CON,
        INT,
        WIS,
        POW,
        CHA
    };

    if (keynum > 0 && keynum <= 7) {
        if (op->contr->swap_first == -1) {
            op->contr->swap_first = stat_trans[keynum];
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_NEWPLAYER,
                                 "%s ->",
                                 short_stat_name[stat_trans[keynum]]);
        } else
            swap_stat(op, stat_trans[keynum]);

        send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "");
        return;
    }
    switch (key) {
    case 'n':
    case 'N': {
        SET_FLAG(op, FLAG_WIZ);
        if (op->map == NULL) {
            LOG(llevError, "Map == NULL in state 2\n");
            break;
        }

        SET_ANIMATION(op, 2);     /* So player faces south */
        /* Enter exit adds a player otherwise */
        add_statbonus(op);
        send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "Now choose a character.\nPress any key to change outlook.\nPress `d' when you're pleased.\n");
        player_set_state(op->contr, ST_CHANGE_CLASS);
        if (op->msg)
            draw_ext_info(NDI_BLUE, 0, op,
                          MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_NEWPLAYER,
                          op->msg);
        return;
    }
    case 'y':
    case 'Y':
        roll_stats(op);
        send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "");
        return;

    case 'q':
    case 'Q':
        play_again(op);
        return;

    default:
        send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "Yes, No, Quit or 1-6.  Roll again?");
        return;
    }
    return;
}

/**
 * This function takes the key that is passed, and does the
 * appropriate action with it (change race, or other things).
 * The function name is for historical reasons - now we have
 * separate race and class; this actually changes the RACE,
 * not the class.
 *
 * @param op
 * player.
 * @param key
 * key to handle.
 */

void key_change_class(object *op, char key) {
    int tmp_loop;

    if (key == 'q' || key == 'Q') {
        object_remove(op);
        play_again(op);
        return;
    }
    if (key == 'd' || key == 'D') {
        char buf[MAX_BUF];

        /* this must before then initial items are given */
        esrv_new_player(op->contr, op->weight+op->carrying);
        create_treasure(find_treasurelist("starting_wealth"), op, 0, 0, 0);

        /* Lauwenmark : Here we handle the BORN global event */
        execute_global_event(EVENT_BORN, op);

        /* Lauwenmark : We then generate a LOGIN event */
        execute_global_event(EVENT_LOGIN, op->contr, op->contr->socket.host);
        player_set_state(op->contr, ST_PLAYING);

        object_set_msg(op, NULL);

        /* We create this now because some of the unique maps will need it
         * to save here.
         */
        snprintf(buf, sizeof(buf), "%s/%s/%s", settings.localdir, settings.playerdir, op->name);
        make_path_to_file(buf);

#ifdef AUTOSAVE
        op->contr->last_save_tick = pticks;
#endif
        start_info(op);
        CLEAR_FLAG(op, FLAG_WIZ);
        give_initial_items(op, op->randomitems);
        link_player_skills(op);
        esrv_send_inventory(op, op);
        fix_object(op);

        /* This moves the player to a different start map, if there
         * is one for this race
         */
        if (*first_map_ext_path) {
            object *tmp;
            char mapname[MAX_BUF];
            mapstruct *oldmap;

            oldmap = op->map;

            snprintf(mapname, MAX_BUF-1, "%s/%s", first_map_ext_path, op->arch->name);
            /*printf("%s\n", mapname);*/
            tmp = object_new();
            EXIT_PATH(tmp) = add_string(mapname);
            EXIT_X(tmp) = op->x;
            EXIT_Y(tmp) = op->y;
            enter_exit(op, tmp);

            if (oldmap != op->map) {
                /* map exists, update bed of reality location, in case player dies */
                op->contr->bed_x = op->x;
                op->contr->bed_y = op->y;
                snprintf(op->contr->savebed_map, sizeof(op->contr->savebed_map), "%s", mapname);
            }

            object_free_drop_inventory(tmp);
        } else {
            LOG(llevDebug, "first_map_ext_path not set\n");
        }
        return;
    }

    /* Following actually changes the race - this is the default command
     * if we don't match with one of the options above.
     */

    tmp_loop = 0;
    while (!tmp_loop) {
        const char *name = add_string(op->name);
        int x = op->x, y = op->y;

        remove_statbonus(op);
        object_remove(op);
        /* get_player_archetype() is really misnamed - it will
         * get the next archetype from the list.
         */
        op->arch = get_player_archetype(op->arch);
        object_copy(&op->arch->clone, op);
        op->stats = op->contr->orig_stats;
        free_string(op->name);
        op->name = name;
        free_string(op->name_pl);
        op->name_pl = add_string(name);
        SET_ANIMATION(op, 2);    /* So player faces south */
        object_insert_in_map_at(op, op->map, op, 0, x, y);
        strncpy(op->contr->title, op->arch->clone.name, sizeof(op->contr->title)-1);
        op->contr->title[sizeof(op->contr->title)-1] = '\0';
        add_statbonus(op);
        tmp_loop = allowed_class(op);
    }
    object_update(op, UP_OBJ_FACE);
    esrv_update_item(UPD_FACE, op, op);
    fix_object(op);
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = 0;
    if (op->msg)
        draw_ext_info(NDI_BLUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_NEWPLAYER,
                      op->msg);
    send_query(&op->contr->socket, CS_QUERY_SINGLECHAR, "Press any key for the next race.\nPress `d' to play this race.\n");
}

/**
 * This checks to see if the race and class are legal.
 * Legal in this contexts means that after apply the race/class stat
 * modifiers, the race is still in legal range.
 *
 * @param stats
 * The statistics to modify - this data is modified, so the caller
 * can use it on the next update.
 * @param race
 * The race to try and apply.  It can be NULL, in which case no race
 * will be applied.
 * @param opclass
 * The class to apply.  It can be NULL, in which case no class will
 * be applied.
 * @return
 * 0 on success, 1 on failure.  Note that no distinction is made on
 * whether the failure is from race or class, as it is the final
 * statistics which are important.  For example, if race is -2 Str,
 * and class is +2 Str, if the starting strength is 1, that is still
 * legal (as final result is 1)
 */
int check_race_and_class(living *stats, archetype *race, archetype *opclass)
{
    int i, stat, failure=0;

    for (i = 0; i < NUM_STATS; i++) {
        stat = get_attr_value(stats, i);
        if (race)
            stat += get_attr_value(&race->clone.stats, i);

        if (opclass)
            stat += get_attr_value(&opclass->clone.stats, i);

        set_attr_value(stats, i, stat);

        /* We process all stats, regardless if there is a failure
         * or not.
         */
        if (stat < MIN_STAT) failure=1;

        /* Maybe this should be an error?  Player is losing
         * some stats points here, but it is legal.
         */
        if (stat > settings.max_stat) stat = settings.max_stat;
    }
    return failure;

}

/**
 * This is somewhat like key_change_class() above, except we know
 * the race to change to, but we still basically need to do
 * the same work (apply bonuses, update archetype, etc.)
 *
 * @param op
 * Player object
 * @param race
 * race to use - caller should do sanity checking that this is
 * a valid race.
 * @param opclass
 * class to use - like race, caller should do sanity checking.
 * @param stats
 * If set, use these stats for the character, do not apply new ones.
 * Note: It is required that the caller only use valid stat values
 * (generated by check_race_and_class() for example), as this function
 * will not do checking on the stats.
 * @return
 * 0 on success, non zero on failure (may be extended with
 * unique error codes).  It is the responsibility of the
 * caller to notify the client of this failure.
 */
int apply_race_and_class(object *op, archetype *race, archetype *opclass, living *stats)
{
    const char *name = add_string(op->name);
    char buf[MAX_BUF];
    object *inv;

    /* Free any objects in character inventory - they
     * shouldn't have any, but there is the potential that
     * we give them objects below and then get a creation
     * failure (stat out of range), in which case
     * those objects would be in the inventory.
     */
    while (op->inv) {
        inv = op->inv;
        object_remove(inv);
        object_free2(inv, 0);
    }

    object_copy(&race->clone, op);
    free_string(op->name);
    op->name = name;
    free_string(op->name_pl);
    op->name_pl = add_string(name);
    SET_ANIMATION(op, 2);    /* So player faces south */
    snprintf(op->contr->title, sizeof(op->contr->title), "%s", op->arch->clone.name);

    if (stats) {
        /* Copy over the stats.  Use this instead a memcpy because
         * we only want to copy over a few specific stats, and
         * leave things like maxhp, maxsp, etc, unchanged.
         */
        int i, stat;
        for (i = 0; i < NUM_STATS; i++) {
            stat = get_attr_value(stats, i);
            set_attr_value(&op->stats, i, stat);
            set_attr_value(&op->contr->orig_stats, i, stat);
        }
    } else {
        /* Note that this will repeated increase the stat values
         * if the caller does not reset them.  Only do this
         * if stats is not provided - if stats is provided, those
         * are already adjusted.
         */
        add_statbonus(op);

        /* Checks that all stats are greater than 1.  Once again,
         * only do this if stats are not provided
         */
        if (!allowed_class(op)) return 1;
    }

    object_update(op, UP_OBJ_FACE);
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = 0;

    /* this must before then initial items are given */
    esrv_new_player(op->contr, op->weight+op->carrying);
    create_treasure(find_treasurelist("starting_wealth"), op, 0, 0, 0);

    /* This has to be done before class, otherwise the NOCLASSFACECHANGE
     * object is not in the inventory, and racial face will get overwritten.
     */
    give_initial_items(op, op->randomitems);

    if (stats) {
        /* Apply class information */
        apply_changes_to_player(op, &opclass->clone, AC_PLAYER_STAT_NO_CHANGE);
    } else {
        apply_changes_to_player(op, &opclass->clone, 0);

        /* Checks that all stats are greater than 1 */
        if (!allowed_class(op)) return 2;
    }

    /* Lauwenmark : Here we handle the BORN global event */
    execute_global_event(EVENT_BORN, op);

    /* Lauwenmark : We then generate a LOGIN event */
    execute_global_event(EVENT_LOGIN, op->contr, op->contr->socket.host);

    object_set_msg(op, NULL);

    /* We create this now because some of the unique maps will need it
     * to save here.
     */
    snprintf(buf, sizeof(buf), "%s/%s/%s", settings.localdir, settings.playerdir, op->name);
    make_path_to_file(buf);

#ifdef AUTOSAVE
    op->contr->last_save_tick = pticks;
#endif

    CLEAR_FLAG(op, FLAG_WIZ);
    link_player_skills(op);
    fix_object(op);

    esrv_send_inventory(op, op);
    esrv_update_item(UPD_FACE, op, op);

    return 0;

}

/**
 * We receive the reply to the 'quit confirmation' message.
 *
 * @param op
 * player.
 * @param key
 * reply.
 */
void key_confirm_quit(object *op, char key) {
    char buf[MAX_BUF];
    mapstruct *mp, *next;

    // this was tested when 'quit' command was issued, but better safe than sorry.
    if (QUERY_FLAG(op, FLAG_WAS_WIZ)) {
        player_set_state(op->contr, ST_PLAYING);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN, "Can't quit when in DM mode.");
        return;
    }

    if (key != 'y' && key != 'Y' && key != 'q' && key != 'Q') {
        player_set_state(op->contr, ST_PLAYING);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_LOGIN,
                      "OK, continuing to play.");
        return;
    }

    /* Lauwenmark : Here we handle the REMOVE global event */
    execute_global_event(EVENT_REMOVE, op);
    pets_terminate_all(op);
    object_remove(op);
    op->direction = 0;
    draw_ext_info_format(NDI_UNIQUE|NDI_ALL|NDI_DK_ORANGE, 5, NULL, MSG_TYPE_ADMIN, MSG_TYPE_ADMIN_PLAYER,
                         "%s quits the game.",
                         op->name);

    strcpy(op->contr->killer, "quit");
    hiscore_check(op, 0);
    party_leave(op);
    if (settings.set_title == TRUE)
        player_set_own_title(op->contr, "");


    /* We need to hunt for any per player unique maps in memory and
     * get rid of them.  The trailing slash in the path is intentional,
     * so that players named 'Ab' won't match against players 'Abe' pathname
     */
    snprintf(buf, sizeof(buf), "%s/%s/%s/", settings.localdir, settings.playerdir, op->name);
    for (mp = first_map; mp != NULL; mp = next) {
        next = mp->next;
        if (!strncmp(mp->path, buf, strlen(buf)))
            delete_map(mp);
    }

    delete_character(op->name);

    /* Remove player from account list and send back data if needed */
    if (op->contr->socket.account_chars != NULL) {
        op->contr->socket.account_chars = account_char_remove(op->contr->socket.account_chars, op->name);
        account_char_save(op->contr->socket.account_name, op->contr->socket.account_chars);
        /* char information is reloaded in send_account_players below */
        account_char_free(op->contr->socket.account_chars);
        op->contr->socket.account_chars = NULL;
        account_remove_player_from_account(op->contr->socket.account_name, op->name);
        send_account_players(&op->contr->socket);
    }

    play_again(op);
}

/**
 * The player is scared, and should flee. If she can't, then she isn't scared anymore.
 *
 * @param op
 * player.
 */
static void flee_player(object *op) {
    int dir, diff;
    rv_vector rv;

    if (op->stats.hp < 0) {
        LOG(llevDebug, "Fleeing player is dead.\n");
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }

    if (op->enemy == NULL) {
        LOG(llevDebug, "Fleeing player had no enemy.\n");
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }

    /* Seen some crashes here.  Since we don't store an
     * op->enemy_count, it is possible that something destroys the
     * actual enemy, and the object is recycled.
     */
    if (op->enemy->map == NULL) {
        CLEAR_FLAG(op, FLAG_SCARED);
        object_set_enemy(op, NULL);
        return;
    }

    if (!(random_roll(0, 4, op, PREFER_LOW)) && did_make_save(op, op->level, 0)) {
        object_set_enemy(op, NULL);
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }
    if (!get_rangevector(op, op->enemy, &rv, 0)) {
        object_set_enemy(op, NULL);
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }

    dir = absdir(4+rv.direction);
    for (diff = 0; diff < 3; diff++) {
        int m = 1-(RANDOM()&2);
        if (move_ob(op, absdir(dir+diff*m), op)
        || (diff == 0 && move_ob(op, absdir(dir-diff*m), op))) {
            return;
        }
    }
    /* Cornered, get rid of scared */
    CLEAR_FLAG(op, FLAG_SCARED);
    object_set_enemy(op, NULL);
}

/**
 * Sees if there is stuff to be picked up/picks up stuff, for players only.
 * @param op
 * player to check for.
 * @retval 1
 * player should keep on moving.
 * @retval 0
 * player should stop.
 */
int check_pick(object *op) {
    tag_t op_tag;
    int stop = 0;
    int j, k, wvratio, current_ratio;
    char putstring[128], tmpstr[16];

    /* if you're flying, you can't pick up anything */
    if (op->move_type&MOVE_FLYING)
        return 1;

    op_tag = op->count;

    FOR_BELOW_PREPARE(op, tmp) {
        if (object_was_destroyed(op, op_tag))
            return 0;

        if (!object_can_pick(op, tmp))
            continue;

        if (op->contr->search_str[0] != '\0' && settings.search_items == TRUE) {
            if (object_matches_string(op, tmp, op->contr->search_str))
                pick_up(op, tmp);
            continue;
        }

        /* high not bit set?  We're using the old autopickup model */
        if (!(op->contr->mode&PU_NEWMODE)) {
            switch (op->contr->mode) {
            case 0:
                return 1; /* don't pick up */

            case 1:
                pick_up(op, tmp);
                return 1;

            case 2:
                pick_up(op, tmp);
                return 0;

            case 3:
                return 0; /* stop before pickup */

            case 4:
                pick_up(op, tmp);
                break;

            case 5:
                pick_up(op, tmp);
                stop = 1;
                break;

            case 6:
                if (QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL)
                && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED))
                    pick_up(op, tmp);
                break;

            case 7:
                if (tmp->type == MONEY || tmp->type == GEM)
                    pick_up(op, tmp);
                break;

            default:
                /* use value density */
                if (!QUERY_FLAG(tmp, FLAG_UNPAID)
                && (query_cost(tmp, op, BS_TRUE)*100/(tmp->weight*MAX(tmp->nrof, 1))) >= op->contr->mode)
                    pick_up(op, tmp);
            }
        } else { /* old model */
            /* NEW pickup handling */
            if (op->contr->mode&PU_DEBUG) {
                /* some debugging code to figure out item information */
                draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_DEBUG,
                                     "item name: %s    item type: %d    weight/value: %d",
                                     tmp->name ? tmp->name : tmp->arch->name, tmp->type,
                                     (int)(query_cost(tmp, op, BS_TRUE)*100/(tmp->weight*MAX(tmp->nrof, 1))));


                snprintf(putstring, sizeof(putstring), "...flags: ");
                for (k = 0; k < 4; k++) {
                    for (j = 0; j < 32; j++) {
                        if ((tmp->flags[k]>>j)&0x01) {
                            snprintf(tmpstr, sizeof(tmpstr), "%d ", k*32+j);
                            strcat(putstring, tmpstr);
                        }
                    }
                }
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                              putstring);
            }
            /* philosophy:
             * It's easy to grab an item type from a pile, as long as it's
             * generic.  This takes no game-time.  For more detailed pickups
             * and selections, select-items should be used.  This is a
             * grab-as-you-run type mode that's really useful for arrows for
             * example.
             * The drawback: right now it has no frontend, so you need to
             * stick the bits you want into a calculator in hex mode and then
             * convert to decimal and then 'pickup <#>
             */

            /* the first two modes are exclusive: if NOTHING we return, if
             * STOP then we stop.  All the rest are applied sequentially,
             * meaning if any test passes, the item gets picked up. */

            /* if mode is set to pick nothing up, return */

            if (op->contr->mode == PU_NOTHING)
                return 1;

            /* if mode is set to stop when encountering objects, return.
             * Take STOP before INHIBIT since it doesn't actually pick
             * anything up */

            if (op->contr->mode&PU_STOP)
                return 0;

            /* useful for going into stores and not losing your settings... */
            /* and for battles where you don't want to get loaded down while
             * fighting */
            if (op->contr->mode&PU_INHIBIT)
                return 1;

            /* prevent us from turning into auto-thieves :) */
            if (QUERY_FLAG(tmp, FLAG_UNPAID))
                continue;

            /* ignore known cursed objects */
            if (QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) && op->contr->mode&PU_NOT_CURSED)
                continue;

            /* all food and drink if desired */
            /* question: don't pick up known-poisonous stuff? */
            if (op->contr->mode&PU_FOOD)
                if (tmp->type == FOOD) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_DRINK)
                if (tmp->type == DRINK || (tmp->type == POISON && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED))) {
                    pick_up(op, tmp);
                    continue;
                }
            /* we don't forget dragon food */
            if (op->contr->mode&PU_FLESH)
                if (tmp->type == FLESH) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_POTION)
                if (tmp->type == POTION) {
                    pick_up(op, tmp);
                    continue;
                }

            /* spellbooks, skillscrolls and normal books/scrolls */
            if (op->contr->mode&PU_SPELLBOOK)
                if (tmp->type == SPELLBOOK) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_SKILLSCROLL)
                if (tmp->type == SKILLSCROLL) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_READABLES)
                if (tmp->type == BOOK || tmp->type == SCROLL) {
                    pick_up(op, tmp);
                    continue;
                }

            /* wands/staves/rods/horns/skill tools */
            if (op->contr->mode&PU_MAGIC_DEVICE)
                if (tmp->type == WAND || tmp->type == ROD || tmp->type == WEAPON_IMPROVER || tmp->type == ARMOUR_IMPROVER || tmp->type == SKILL_TOOL) {
                    pick_up(op, tmp);
                    continue;
                }

            /* pick up all magical items */
            if (op->contr->mode&PU_MAGICAL)
                if (QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED)) {
                    pick_up(op, tmp);
                    continue;
                }

            if (op->contr->mode&PU_VALUABLES) {
                if (tmp->type == MONEY || tmp->type == GEM) {
                    pick_up(op, tmp);
                    continue;
                }
            }

            /* rings & amulets - talismans seems to be typed AMULET */
            if (op->contr->mode&PU_JEWELS)
                if (tmp->type == RING || tmp->type == AMULET) {
                    pick_up(op, tmp);
                    continue;
                }

            /* bows and arrows. Bows are good for selling! */
            if (op->contr->mode&PU_BOW)
                if (tmp->type == BOW) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_ARROW)
                if (tmp->type == ARROW) {
                    pick_up(op, tmp);
                    continue;
                }

            /* all kinds of armor etc. */
            if (op->contr->mode&PU_ARMOUR)
                if (tmp->type == ARMOUR) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_HELMET)
                if (tmp->type == HELMET) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_SHIELD)
                if (tmp->type == SHIELD) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_BOOTS)
                if (tmp->type == BOOTS) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_GLOVES)
                if (tmp->type == GLOVES) {
                    pick_up(op, tmp);
                    continue;
                }
            if (op->contr->mode&PU_CLOAK)
                if (tmp->type == CLOAK) {
                    pick_up(op, tmp);
                    continue;
                }

            /* hoping to catch throwing daggers here */
            if (op->contr->mode&PU_MISSILEWEAPON)
                if (tmp->type == WEAPON && QUERY_FLAG(tmp, FLAG_IS_THROWN)) {
                    pick_up(op, tmp);
                    continue;
                }

            /* careful: chairs and tables are weapons! */
            if (op->contr->mode&PU_MELEEWEAPON) {
                if (tmp->type == WEAPON && tmp->name != NULL) {
                    if (strstr(tmp->name, "table") == NULL
                    && strstr(tmp->arch->name, "table") == NULL
                    && strstr(tmp->name, "chair") == NULL
                    && strstr(tmp->arch->name, "chair") == NULL) {
                        pick_up(op, tmp);
                        continue;
                    }
                }
                if (tmp->type == WEAPON && tmp->name == NULL) {
                    if (strstr(tmp->arch->name, "table") == NULL
                    && strstr(tmp->arch->name, "chair") == NULL) {
                        pick_up(op, tmp);
                        continue;
                    }
                }
            }

            /* misc stuff that's useful */
            if (op->contr->mode&PU_KEY)
                if (tmp->type == KEY || tmp->type == SPECIAL_KEY) {
                    pick_up(op, tmp);
                    continue;
                }

            if (op->contr->mode&PU_CONTAINER && tmp->type == CONTAINER) {
                pick_up(op, tmp);
                continue;
            }

            /* any of the last 4 bits set means we use the ratio for value
             * pickups */
            if (op->contr->mode&PU_RATIO) {
                /* use value density to decide what else to grab.
                 * >=7 was >= op->contr->mode
                 * >=7 is the old standard setting.  Now we take the last 4 bits
                 * and multiply them by 5, giving 0..15*5== 5..75 */
                wvratio = (op->contr->mode&PU_RATIO)*5;
                current_ratio = query_cost(tmp, op, BS_TRUE)*100/(tmp->weight*MAX(tmp->nrof, 1));
                if (current_ratio >= wvratio) {
                    pick_up(op, tmp);
                    continue;
                }
            }
        } /* the new pickup model */
    } FOR_BELOW_FINISH();
    return !stop;
}

/**
 * Find an arrow in the inventory and after that
 *  in the right type container (quiver). Pointer to the
 *  found object is returned.
 *
 * @param op
 * object to find arrow for.
 * @param type
 * what arrow race to search for.
 * @return
 * suitable arrow, NULL if none found.
 */
static object *find_arrow(object *op, const char *type) {
    object *tmp = NULL;

    FOR_INV_PREPARE(op, inv)
        if (!tmp
        && inv->type == CONTAINER
        && inv->race == type
        && QUERY_FLAG(inv, FLAG_APPLIED))
            tmp = find_arrow(inv, type);
        else if (inv->type == ARROW && inv->race == type)
            return inv;
    FOR_INV_FINISH();
    return tmp;
}

/**
 * Similar to find_arrow(), but looks for (roughly) the best arrow to use
 * against the target.  A full test is not performed, simply a basic test
 * of resistances.  The archer is making a quick guess at what he sees down
 * the hall.  Failing that it does it's best to pick the highest plus arrow.
 *
 * @param op
 * who to search arrows for.
 * @param target
 * what op is aiming at.
 * @param type
 * arrow race to search for.
 * @param[out] better
 * will contain the arrow's value if not NULL.
 * @return
 * suitable arrow, NULL if none found.
 */
static object *find_better_arrow(object *op, object *target, const char *type, int *better) {
    object *tmp = NULL, *ntmp;
    int attacknum, attacktype, betterby = 0, i;

    if (!type)
        return NULL;

    FOR_INV_PREPARE(op, arrow) {
        if (arrow->type == CONTAINER
        && arrow->race == type
        && QUERY_FLAG(arrow, FLAG_APPLIED)) {
            i = 0;
            ntmp = find_better_arrow(arrow, target, type, &i);
            if (i > betterby) {
                tmp = ntmp;
                betterby = i;
            }
        } else if (arrow->type == ARROW && arrow->race == type) {
            /* always prefer assassination/slaying */
            if (target->race != NULL
            && arrow->slaying != NULL
            && strstr(arrow->slaying, target->race)) {
                if (arrow->attacktype&AT_DEATH) {
                    if (better)
                        *better = 100;
                    return arrow;
                } else {
                    tmp = arrow;
                    betterby = (arrow->magic+arrow->stats.dam)*2;
                }
            } else {
                for (attacknum = 0; attacknum < NROFATTACKS; attacknum++) {
                    attacktype = 1<<attacknum;
                    if ((arrow->attacktype&attacktype) && (target->arch->clone.resist[attacknum]) < 0)
                        if (((arrow->magic+arrow->stats.dam)*(100-target->arch->clone.resist[attacknum])/100) > betterby) {
                            tmp = arrow;
                            betterby = (arrow->magic+arrow->stats.dam)*(100-target->arch->clone.resist[attacknum])/100;
                        }
                }
                if ((2+arrow->magic+arrow->stats.dam) > betterby) {
                    tmp = arrow;
                    betterby = 2+arrow->magic+arrow->stats.dam;
                }
                if (arrow->title && (1+arrow->magic+arrow->stats.dam) > betterby) {
                    tmp = arrow;
                    betterby = 1+arrow->magic+arrow->stats.dam;
                }
            }
        }
    } FOR_INV_FINISH();
    if (tmp == NULL)
        return find_arrow(op, type);

    if (better)
        *better = betterby;
    return tmp;
}

/**
 * Looks in a given direction, finds the first valid target, and calls
 * find_better_arrow() to find a decent arrow to use.
 * @param op
 * shooter.
 * @param type
 * arrow's race to search for (the bow's usually).
 * @param dir
 * fire direction.
 * @return
 * suitable arrow, or NULL if none found.
 */
static object *pick_arrow_target(object *op, const char *type, int dir) {
    object *tmp = NULL;
    mapstruct *m;
    int i, mflags, found, number;
    sint16 x, y;

    if (op->map == NULL)
        return find_arrow(op, type);

    /* do a dex check */
    number = (die_roll(2, 40, op, PREFER_LOW)-2)/2;
    if (number > (op->stats.Dex+(op->chosen_skill ? op->chosen_skill->level : op->level)))
        return find_arrow(op, type);

    m = op->map;
    x = op->x;
    y = op->y;

    /* find the first target */
    for (i = 0, found = 0; i < 20; i++) {
        x += freearr_x[dir];
        y += freearr_y[dir];
        mflags = get_map_flags(m, &m, x, y, &x, &y);
        if (mflags&P_OUT_OF_MAP || mflags&P_BLOCKSVIEW) {
            tmp = NULL;
            break;
        } else if (GET_MAP_MOVE_BLOCK(m, x, y) == MOVE_FLY_LOW) {
            /* This block presumes arrows and the like are MOVE_FLY_SLOW -
             * perhaps a bad assumption.
             */
            tmp = NULL;
            break;
        }
        if (mflags&P_IS_ALIVE) {
            FOR_MAP_PREPARE(m, x, y, tmp2)
                if (QUERY_FLAG(tmp2, FLAG_ALIVE)) {
                    tmp = tmp2;
                    found++;
                    break;
                }
            FOR_MAP_FINISH();
            if (found)
                break;
        }
    }
    if (tmp == NULL)
        return find_arrow(op, type);

    return find_better_arrow(op, HEAD(tmp), type, NULL);
}

/**
 * Creature (monster or player) fires a bow.
 *
 * @param op
 * object firing the bow.
 * @param arrow
 * object to fire.
 * @param dir
 * direction of fire.
 * @param wc_mod
 * any special modifier to give (used in special player fire modes)
 * @param sx
 * @param sy
 * coordinates to fire arrow from - also used in some of the special player fire modes.
 * @return
 * 1 if bow was actually fired, 0 otherwise.
 * @todo describe player firing modes.
 */
int fire_bow(object *op, object *arrow, int dir, int wc_mod, sint16 sx, sint16 sy) {
    object *bow;
    tag_t tag;
    int bowspeed, mflags;
    mapstruct *m;

    if (!dir) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You can't shoot yourself!");
        return 0;
    }
    if (op->type == PLAYER)
        bow = op->contr->ranges[range_bow];
    else {
        /* Don't check for applied - monsters don't apply bows - in that way, they
         * don't need to switch back and forth between bows and weapons.
         */
        bow = object_find_by_type(op, BOW);
        if (!bow) {
            LOG(llevError, "Range: bow without activated bow (%s).\n", op->name);
            return 0;
        }
    }
    if (!bow->race || !bow->skill) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "Your %s is broken.",
                             bow->name);
        return 0;
    }

    bowspeed = bow->stats.sp+get_dex_bonus(op->stats.Dex);

    /* penalize ROF for bestarrow */
    if (op->type == PLAYER && op->contr->bowtype == bow_bestarrow)
        bowspeed -= get_dex_bonus(op->stats.Dex)+5;
    if (bowspeed < 1)
        bowspeed = 1;

    if (arrow == NULL) {
        arrow = find_arrow(op, bow->race);
        if (arrow == NULL) {
            if (op->type == PLAYER)
                draw_ext_info_format(NDI_UNIQUE, 0, op,
                                     MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                                     "You have no %s left.",
                                     bow->race);
            /* FLAG_READY_BOW will get reset if the monsters picks up some arrows */
            else
                CLEAR_FLAG(op, FLAG_READY_BOW);
            return 0;
        }
    }
    mflags = get_map_flags(op->map, &m, sx, sy, &sx, &sy);
    if (mflags&P_OUT_OF_MAP) {
        return 0;
    }
    if (GET_MAP_MOVE_BLOCK(m, sx, sy)&MOVE_FLY_LOW) {
        return 0;
    }

    /* this should not happen, but sometimes does */
    if (arrow->nrof == 0) {
        object_remove(arrow);
        object_free_drop_inventory(arrow);
        return 0;
    }

    arrow = object_split(arrow, 1, NULL, 0);
    if (arrow == NULL) {
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                             "You have no %s left.",
                             bow->race);
        return 0;
    }
    object_set_owner(arrow, op);
    if (arrow->skill)
        free_string(arrow->skill);
    arrow->skill = add_refcount(bow->skill);

    arrow->direction = dir;

    if (op->type == PLAYER) {
        op->speed_left = 0.01-(float)FABS(op->speed)*100/bowspeed;
        fix_object(op);
    }

    if (bow->anim_suffix != NULL)
        apply_anim_suffix(op, bow->anim_suffix);

/*    SET_ANIMATION(arrow, arrow->direction);*/
    object_update_turn_face(arrow);
    arrow->stats.sp = arrow->stats.wc; /* save original wc and dam */
    arrow->stats.hp = arrow->stats.dam;
    arrow->stats.grace = arrow->attacktype;
    if (arrow->slaying != NULL)
        arrow->spellarg = strdup_local(arrow->slaying);

    /* Note that this was different for monsters - they got their level
     * added to the damage.  I think the strength bonus is more proper.
     */

    arrow->stats.dam += (QUERY_FLAG(bow, FLAG_NO_STRENGTH) ? 0 : get_dam_bonus(op->stats.Str))
        +bow->stats.dam
        +bow->magic
        +arrow->magic;

    /* update the speed */
    arrow->speed = (float)((QUERY_FLAG(bow, FLAG_NO_STRENGTH) ? 0 : get_dam_bonus(op->stats.Str))+bow->magic+arrow->magic)/5.0
        +(float)bow->stats.dam/7.0;

    if (arrow->speed < 1.0)
        arrow->speed = 1.0;
    object_update_speed(arrow);
    arrow->speed_left = 0;

    if (op->type == PLAYER) {
        /* we don't want overflows of wc (sint), so cap the value - mod and pl should be subtracted */
        int mod = bow->magic
            +arrow->magic
            +get_dex_bonus(op->stats.Dex)
            +get_thaco_bonus(op->stats.Str)
            +arrow->stats.wc
            +bow->stats.wc
            -wc_mod;
        int plmod = (op->chosen_skill ? op->chosen_skill->level : op->level);
        if (plmod+mod > 140)
            plmod = 140-mod;
        else if (plmod+mod < -100)
            plmod = -100-mod;
        arrow->stats.wc = 20-(sint8)plmod-(sint8)mod;

        arrow->level = op->chosen_skill ? op->chosen_skill->level : op->level;
    } else {
        arrow->stats.wc = op->stats.wc
            -bow->magic
            -arrow->magic
            -arrow->stats.wc
            +wc_mod;

        arrow->level = op->level;
    }
    if (arrow->attacktype == AT_PHYSICAL)
        arrow->attacktype |= bow->attacktype;
    if (bow->slaying != NULL)
        arrow->slaying = add_string(bow->slaying);

    /* If move_type is ever changed, monster.c:monster_use_bow() needs to be changed too. */
    arrow->move_type = MOVE_FLY_LOW;
    arrow->move_on = MOVE_FLY_LOW|MOVE_WALK;

    tag = arrow->count;
    object_insert_in_map_at(arrow, m, op, 0, sx, sy);

    if (!object_was_destroyed(arrow, tag)) {
        play_sound_map(SOUND_TYPE_ITEM, arrow, arrow->direction, "fire");
        ob_process(arrow);
    }

    return 1;
}

/**
 * Is direction a similar to direction b? Find out in this exciting function
 * below.
 *
 * @param a
 * @param b
 * directions to compare.
 * @return
 * 1 if similar, 0 if not.
 */
static int similar_direction(int a, int b) {
    /* shortcut the obvious */
    if (a == b)
        return 1;

    switch (a) {
    case 1: if (b <= 2 || b == 8) return 1; break;
    case 2: if (b > 0 && b < 4) return 1; break;
    case 3: if (b > 1 && b < 5) return 1; break;
    case 4: if (b > 2 && b < 6) return 1; break;
    case 5: if (b > 3 && b < 7) return 1; break;
    case 6: if (b > 4 && b < 8) return 1; break;
    case 7: if (b > 5) return 1; break;
    case 8: if (b > 6 || b == 1) return 1; break;
    }
    return 0;
}

/**
 * Special fire code for players - this takes into
 * account the special fire modes players can have
 * but monsters can't.  Putting that code here
 * makes the fire_bow() code much cleaner.
 *
 * This function should only be called if 'op' is a player,
 * hence the function name.
 *
 * @param op
 * player.
 * @param dir
 * firing direction.
 * @return
 * 1 if arrow was fired, 0 else.
 */
static int player_fire_bow(object *op, int dir) {
    int ret = 0, wcmod = 0;

    if (op->contr->bowtype == bow_bestarrow) {
        ret = fire_bow(op, pick_arrow_target(op, op->contr->ranges[range_bow]->race, dir), dir, 0, op->x, op->y);
    } else if (op->contr->bowtype >= bow_n && op->contr->bowtype <= bow_nw) {
        if (!similar_direction(dir, op->contr->bowtype-bow_n+1))
            wcmod = -1;
        ret = fire_bow(op, NULL, op->contr->bowtype-bow_n+1, wcmod, op->x, op->y);
    } else if (op->contr->bowtype == bow_threewide) {
        ret = fire_bow(op, NULL, dir, 0, op->x, op->y);
        ret |= fire_bow(op, NULL, dir, -5, op->x+freearr_x[absdir(dir+2)], op->y+freearr_y[absdir(dir+2)]);
        ret |= fire_bow(op, NULL, dir, -5, op->x+freearr_x[absdir(dir-2)], op->y+freearr_y[absdir(dir-2)]);
    } else if (op->contr->bowtype == bow_spreadshot) {
        ret |= fire_bow(op, NULL, dir, 0, op->x, op->y);
        ret |= fire_bow(op, NULL, absdir(dir-1), -5, op->x, op->y);
        ret |= fire_bow(op, NULL, absdir(dir+1), -5, op->x, op->y);
    } else {
        /* Simple case */
        ret = fire_bow(op, NULL, dir, 0, op->x, op->y);
    }
    return ret;
}

/**
 * Fires a misc (wand/rod/horn) object in 'dir'.
 * Broken apart from 'fire' to keep it more readable.
 *
 * @param op
 * player firing.
 * @param dir
 * firing direction.
 *
 * @warning
 * op must be a player (contr != NULL).
 */
static void fire_misc_object(object *op, int dir) {
    object *item;
    char name[MAX_BUF];

    item = op->contr->ranges[range_misc];
    if (!item) {
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "You have no range item readied.");
        return;
    }
    if (!item->inv) {
        LOG(llevError, "Object %s lacks a spell\n", item->name);
        return;
    }
    if (item->type == WAND) {
        if (item->stats.food <= 0) {
            play_sound_player_only(op->contr, SOUND_TYPE_ITEM, item, 0, "poof");
            query_base_name(item, 0, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                                 "The %s goes poof.",
                                 name);
            return;
        }
    } else if (item->type == ROD) {
        if (item->stats.hp < SP_level_spellpoint_cost(item, item->inv, SPELL_HIGHEST)) {
            play_sound_player_only(op->contr, SOUND_TYPE_ITEM, item, 0, "poof");
            query_base_name(item, 0, name, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_FAILURE,
                                 "The %s whines for a while, but nothing happens.",
                                 name);
            return;
        }
    }

    if (cast_spell(op, item, dir, item->inv, NULL)) {
        SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
        if (item->type == WAND) {
            drain_wand_charge(item);
        } else if (item->type == ROD) {
            drain_rod_charge(item);
        }
    }
}

/**
 * Received a fire command for the player - go and do it.
 *
 * @param op
 * player.
 * @param dir
 * direction to fire into.
 */
void fire(object *op, int dir) {

    /* check for loss of invisiblity/hide */
    if (action_makes_visible(op))
        make_visible(op);

    switch (op->contr->shoottype) {
    case range_none:
        return;

    case range_bow:
        player_fire_bow(op, dir);
        return;

    case range_magic: /* Casting spells */
        cast_spell(op, op, dir, op->contr->ranges[range_magic], op->contr->spellparam[0] ? op->contr->spellparam : NULL);
        return;

    case range_misc:
        fire_misc_object(op, dir);
        return;

    case range_golem: /* Control summoned monsters from scrolls */
        if (op->contr->ranges[range_golem] == NULL
        || op->contr->golem_count != op->contr->ranges[range_golem]->count) {
            op->contr->ranges[range_golem] = NULL;
            op->contr->shoottype = range_none;
            op->contr->golem_count = 0;
        } else
            pets_control_golem(op->contr->ranges[range_golem], dir);
        return;

    case range_skill:
        if (!op->chosen_skill) {
            if (op->type == PLAYER)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                              "You have no applicable skill to use.");
            return;
        }
        (void)do_skill(op, op, op->chosen_skill, dir, NULL);
        return;

    case range_builder:
        apply_map_builder(op, dir);
        return;

    default:
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Illegal shoot type.");
        return;
    }
}

/**
 * We try to find a key for the door as passed.  If we find a key
 * and player can use it (based on the usekeys settings), we return the key, otherwise NULL.
 *
 * This function merges both normal and locked door, since the logic
 * for both is the same - just the specific key is different.
 *
 * This function can be called recursively to search containers.
 *
 * @param pl
 * player.
 * @param container
 * inventory to searched for keys.
 * @param door
 * door we are trying to match against.
 * @return
 * key to use, NULL if none found or usekeys mode doesn't let reach the key.
 * @todo document use key modes.
 */
object *find_key(object *pl, object *container, object *door) {
    object *tmp, *key;

    /* Should not happen, but sanity checking is never bad */
    if (container->inv == NULL)
        return NULL;

    /* First, lets try to find a key in the top level inventory */
    tmp = NULL;
    if (door->type == DOOR) {
        tmp = object_find_by_type(container, KEY);
    }
    /* For sanity, we should really check door type, but other stuff
     * (like containers) can be locked with special keys
     */
    if (!tmp && door->slaying != NULL) {
        tmp = object_find_by_type_and_slaying(container, SPECIAL_KEY, door->slaying);
    }
    /* No key found - lets search inventories now */
    /* If we find and use a key in an inventory, return at that time.
     * otherwise, if we search all the inventories and still don't find
     * a key, return
     */
    if (!tmp) {
        FOR_INV_PREPARE(container, tmp) {
            /* No reason to search empty containers */
            if (tmp->type == CONTAINER && tmp->inv) {
                key = find_key(pl, tmp, door);
                if (key != NULL)
                    return key;
            }
        } FOR_INV_FINISH();
        return NULL;
    }
    /* We get down here if we have found a key.  Now if its in a container,
     * see if we actually want to use it
     */
    if (pl != container) {
        /* Only let players use keys in containers */
        if (!pl->contr)
            return NULL;
        /* cases where this fails:
         * If we only search the player inventory, return now since we
         * are not in the players inventory.
         * If the container is not active, return now since only active
         * containers can be used.
         * If we only search keyrings and the container does not have
         * a race/isn't a keyring.
         * No checking for all containers - to fall through past here,
         * inv must have been an container and must have been active.
         *
         * Change the color so that the message doesn't disappear with
         * all the others.
         */
        if (pl->contr->usekeys == key_inventory
        || !QUERY_FLAG(container, FLAG_APPLIED)
        || (pl->contr->usekeys == keyrings && (!container->race || strcmp(container->race, "keys")))) {
            char name_tmp[MAX_BUF], name_cont[MAX_BUF];

            query_name(tmp, name_tmp, MAX_BUF);
            query_name(container, name_cont, MAX_BUF);
            draw_ext_info_format(NDI_UNIQUE|NDI_BROWN, 0, pl,
                                 MSG_TYPE_ITEM, MSG_TYPE_ITEM_INFO,
                                 "The %s in your %s vibrates as you approach the door",
                                 name_tmp, name_cont);
            return NULL;
        }
    }
    return tmp;
}

/**
 * Player is "attacking" a door. Will try to open it with a key, or warn if can't open it.
 *
 * Moved out of move_player_attack().
 *
 * @retval 1
 * player has opened the door with a key such that the caller should not do anything more.
 * @retval 0
 * nothing happened.
 */
static int player_attack_door(object *op, object *door) {
    /* If its a door, try to find a use a key.  If we do destroy the door,
     * might as well return immediately as there is nothing more to do -
     * otherwise, we fall through to the rest of the code.
     */
    object *key = find_key(op, op, door);

    assert(door->type == DOOR || door->type == LOCKED_DOOR);

    /* IF we found a key, do some extra work */
    if (key) {
        char name[HUGE_BUF];

        play_sound_map(SOUND_TYPE_GROUND, door, 0, "open");
        if (action_makes_visible(op))
            make_visible(op);
        if (door->inv && (door->inv->type == RUNE || door->inv->type == TRAP))
            spring_trap(door->inv, op);

        query_short_name(key, name, HUGE_BUF);
        draw_ext_info_format(NDI_UNIQUE, NDI_BROWN, op,
                             MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                             "You open the door with the %s",
                             name);

        if (door->type == DOOR)
            remove_door(door);
        else
            remove_locked_door(door); /* remove door without violence ;-) */

        /* Do this after we print the message */
        object_decrease_nrof_by_one(key); /* Use up one of the keys */

        return 1; /* Nothing more to do below */

    }

    if (door->type == LOCKED_DOOR) {
        /* Might as well return now - no other way to open this */
        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_NOKEY,
                      door->msg);
        return 1;
    }

    if (door->type == DOOR && op->contr && !op->contr->run_on) {
        /* Player so try to pick the door */
        object *lock = find_skill_by_name(op, "lockpicking");
        if (lock) {
            /* Even if the lockpicking failed, don't go on moving, player should explicitely attack or run
             * to bash the door. */
            do_skill(op, op, lock, op->facing, NULL);
            return 1;
        }
    }

    return 0;
}

/**
 * The player is also actually going to try and move (not fire weapons).
 *
 * This function is just part of a breakup from move_player().
 * It should keep the code cleaner.
 * When this is called, the players direction has been updated
 * (taking into account confusion).
 *
 * @param op
 * player moving.
 * @param dir
 * moving direction.
 */
void move_player_attack(object *op, int dir) {
    object *mon, *tpl, *mon_owner;
    sint16 nx, ny;
    int on_battleground;
    mapstruct *m;

    if (op->contr->transport)
        tpl = op->contr->transport;
    else
        tpl = op;
    nx = freearr_x[dir]+tpl->x;
    ny = freearr_y[dir]+tpl->y;

    on_battleground = op_on_battleground(tpl, NULL, NULL, NULL);

    /* If braced, or can't move to the square, and it is not out of the
     * map, attack it.  Note order of if statement is important - don't
     * want to be calling move_ob if braced, because move_ob will move the
     * player.  This is a pretty nasty hack, because if we could
     * move to some space, it then means that if we are braced, we should
     * do nothing at all.  As it is, if we are braced, we go through
     * quite a bit of processing.  However, it probably is less than what
     * move_ob uses.
     */
    if ((op->contr->braced || !move_ob(tpl, dir, tpl)) && !out_of_map(tpl->map, nx, ny)) {
        if (OUT_OF_REAL_MAP(tpl->map, nx, ny)) {
            m = get_map_from_coord(tpl->map, &nx, &ny);
            if (!m)
                return; /* Don't think this should happen */
        } else
            m = tpl->map;

        if (GET_MAP_OB(m, nx, ny) == NULL) {
            /* LOG(llevError, "player_move_attack: GET_MAP_OB returns NULL, but player can not move there.\n");*/
            return;
        }

        mon = NULL;
        /* Go through all the objects, and find ones of interest. Only stop if
         * we find a monster - that is something we know we want to attack.
         * if its a door or barrel (can roll) see if there may be monsters
         * on the space
         */
        FOR_MAP_PREPARE(m, nx, ny, tmp) {
            if (tmp == op) {
                continue;
            }
            if (QUERY_FLAG(tmp, FLAG_ALIVE)) {
                mon = tmp;
                /* Gros: Objects like (pass-through) doors are alive, but haven't
                 * their monster flag set - so this is a good way attack real
                 * monsters in priority.
                 */
                if (QUERY_FLAG(tmp, FLAG_MONSTER))
                    break;
            }
            if (tmp->type == LOCKED_DOOR || QUERY_FLAG(tmp, FLAG_CAN_ROLL))
                mon = tmp;
        } FOR_MAP_FINISH();

        if (mon == NULL)  /* This happens anytime the player tries to move */
            return;  /* into a wall */

        mon = HEAD(mon);
        if ((mon->type == DOOR && mon->stats.hp >= 0) || (mon->type == LOCKED_DOOR))
            if (player_attack_door(op, mon))
                return;

        /* The following deals with possibly attacking peaceful
         * or friendly creatures.  Basically, all players are considered
         * unaggressive.  If the moving player has peaceful set, then the
         * object should be pushed instead of attacked.  It is assumed that
         * if you are braced, you will not attack friends accidently,
         * and thus will not push them.
         */

        /* If the creature is a pet, push it even if the player is not
         * peaceful.  Our assumption is the creature is a pet if the
         * player owns it and it is either friendly or unaggressive.
         */
        mon_owner = object_get_owner(mon);
        if ((op->type == PLAYER)
        && (mon_owner == op || (mon_owner != NULL && mon_owner->type == PLAYER && mon_owner->contr->party != NULL && mon_owner->contr->party == op->contr->party))
        && (QUERY_FLAG(mon, FLAG_UNAGGRESSIVE) || QUERY_FLAG(mon, FLAG_FRIENDLY))) {
            /* If we're braced, we don't want to switch places with it */
            if (op->contr->braced)
                return;
            play_sound_map(SOUND_TYPE_LIVING, mon, dir, "push");
            (void)push_ob(mon, dir, op);
            if (op->contr->tmp_invis || op->hide)
                make_visible(op);
            return;
        }

        /* in certain circumstances, you shouldn't attack friendly
         * creatures.  Note that if you are braced, you can't push
         * someone, but put it inside this loop so that you won't
         * attack them either.
         */
        if ((mon->type == PLAYER || mon->enemy != op)
        && (mon->type == PLAYER || QUERY_FLAG(mon, FLAG_UNAGGRESSIVE) || QUERY_FLAG(mon, FLAG_FRIENDLY))
        && (op->contr->peaceful && !on_battleground)) {
            if (!op->contr->braced) {
                play_sound_map(SOUND_TYPE_LIVING, mon, dir, "push");
                (void)push_ob(mon, dir, op);
            } else {
                draw_ext_info(0, 0, op, MSG_TYPE_ATTACK, MSG_TYPE_ATTACK_NOATTACK,
                              "You withhold your attack");
            }
            if (op->contr->tmp_invis || op->hide)
                make_visible(op);
        }

        /* If the object is a boulder or other rollable object, then
         * roll it if not braced.  You can't roll it if you are braced.
         */
        else if (QUERY_FLAG(mon, FLAG_CAN_ROLL) && (!op->contr->braced)) {
            recursive_roll(mon, dir, op);
            if (action_makes_visible(op))
                make_visible(op);

        /* Any generic living creature.  Including things like doors.
         * Way it works is like this:  First, it must have some hit points
         * and be living.  Then, it must be one of the following:
         * 1) Not a player, 2) A player, but of a different party.  Note
         * that party_number -1 is no party, so attacks can still happen.
         */
        } else if ((mon->stats.hp >= 0)
        && QUERY_FLAG(mon, FLAG_ALIVE)
        && ((mon->type != PLAYER || op->contr->party == NULL || op->contr->party != mon->contr->party))) {
            /* If the player hasn't hit something this tick, and does
             * so, give them speed boost based on weapon speed.  Doing
             * it here is better than process_players2, which basically
             * incurred a 1 tick offset.
             */
            if (op->weapon_speed_left < 0) {
                op->speed_left = -0.01;
                return;
            }
            op->weapon_speed_left -= 1.0;

            skill_attack(mon, op, 0, NULL, NULL);

            /* If attacking another player, that player gets automatic
             * hitback, and doesn't loose luck either.
             * Disable hitback on the battleground or if the target is
             * the wiz.
             */
            if (mon->type == PLAYER
            && mon->stats.hp >= 0
            && !mon->contr->has_hit
            && !on_battleground
            && !QUERY_FLAG(mon, FLAG_WIZ)) {
                short luck = mon->stats.luck;
                mon->contr->has_hit = 1;
                skill_attack(op, mon, 0, NULL, NULL);
                mon->stats.luck = luck;
            }
            if (action_makes_visible(op))
                make_visible(op);
        }
    } /* if player should attack something */
}

/**
 * Update the move_type of a transport based on the direction. The transport MUST be square.
 * Depending on the direction, the right column of tiles or the bottom line of tiles will have a move_type of 0.
 * @param transport what to update.
 * @param dir direction to update flags for.
 */
static void update_transport_block(object *transport, int dir) {
    object *part;
    int sx, sy, x, y;

    object_get_multi_size(transport, &sx, &sy, NULL, NULL);
    assert(sx == sy);

    if (dir == 1 || dir == 5) {
        part = transport;
        for (y = 0; y <= sy; y++) {
            for (x = 0; x < sx; x++) {
                part->move_type = transport->move_type;
                part = part->more;
            }
            part->move_type = 0;
            part = part->more;
        }
    } else if (dir == 3 || dir == 7) {
        part = transport;
        for (y = 0; y < sy; y++) {
            for (x = 0; x <= sx; x++) {
                part->move_type = transport->move_type;
                part = part->more;
            }
        }
        while (part) {
            part->move_type = 0;
            part = part->more;
        }
    } else {
        for (part = transport; part; part = part->more) {
            part->move_type = transport->move_type;
        }
    }
}

/**
 * Turn a transport to an adjacent direction (+1 or -1), updating the move_type flags in the same process.
 * @param transport what to turn. Must be of type TRANSPORT.
 * @param captain who wants to turn the boat.
 * @param dir direction to turn to.
 * @return
 * - 1 if the transport turned (so can't move anymore this tick)
 * - 2 if the transport couldn't turn
 */
static int turn_one_transport(object *transport, object *captain, int dir) {
    int x, y, scroll_dir = 0;

    assert(transport->type == TRANSPORT);

    x = transport->x;
    y = transport->y;

    if (transport->direction == 1 && dir == 8) {
        x--;
    } else if (transport->direction == 2 && dir == 3) {
        y++;
    } else if (transport->direction == 3 && dir == 2) {
        y--;
    } else if (transport->direction == 5 && dir == 6) {
        x--;
    } else if (transport->direction == 6 && dir == 5) {
        x++;
    } else if (transport->direction == 7 && dir == 8) {
        y--;
    } else if (transport->direction == 8 && dir == 7) {
        y++;
    } else if (transport->direction == 8 && dir == 1) {
        x++;
    }

    update_transport_block(transport, dir);
    object_remove(transport);
    if (ob_blocked(transport, transport->map, x, y)) {
        update_transport_block(transport, transport->direction);
        object_insert_in_map_at(transport, transport->map, NULL, 0, x, y);
        return 2;
    }

    if (x != transport->x || y != transport->y) {
/*        assert(scroll_dir != 0);*/

        FOR_INV_PREPARE(transport, pl) {
            if (pl->type == PLAYER) {
                pl->contr->do_los = 1;
                pl->map = transport->map;
                pl->x = x;
                pl->y = y;
                esrv_map_scroll(&pl->contr->socket, freearr_x[scroll_dir], freearr_y[scroll_dir]);
                pl->contr->socket.update_look = 1;
                pl->contr->socket.look_position = 0;
            }
        } FOR_INV_FINISH();
    }

    object_insert_in_map_at(transport, transport->map, NULL, 0, x, y);
    transport->direction = dir;
    transport->facing = dir;
    animate_object(transport, dir);
    captain->direction = dir;
    return 1;
}

/**
 * Try to turn a transport in the desired direction.
 * This takes into account transports that turn and don't occupy the same space depending on the direction it is facing.
 * The transport MUST be a square for it to turn correctly when adjusting tile occupation.
 * @param transport what to turn. Must be of type TRANSPORT.
 * @param captain who wants to turn the boat.
 * @param dir direction to turn to.
 * @return
 * - 0 if transport is in the right direction
 * - 1 if the transport turned (so can't move anymore this tick)
 * - 2 if the transport couldn't turn
 */
static int turn_transport(object *transport, object *captain, int dir) {
    assert(transport->type == TRANSPORT);

    if (object_get_value(transport, "turnable_transport") == NULL) {
        transport->direction = dir;
        transport->facing = dir;
        if (QUERY_FLAG(transport, FLAG_ANIMATE)) {
            animate_object(transport, dir);
        }
        captain->direction = dir;
        return 0;
    }

    if (transport->direction == dir)
        return 0;

    if (absdir(transport->direction-dir) > 2)
        return turn_one_transport(transport, captain, absdir(transport->direction+1));
    else
        return turn_one_transport(transport, captain, absdir(transport->direction-1));
}

/**
 * Player gave us a direction, check whether to move or fire.
 *
 * @param op
 * player.
 * @param dir
 * direction to move/fire.
 * @return
 * 0.
 */
int move_player(object *op, int dir) {
    int pick;
    object *transport = op->contr->transport;

    if (!transport && (op->map == NULL || op->map->in_memory != MAP_IN_MEMORY))
        return 0;

    /* Sanity check: make sure dir is valid */
    if ((dir < 0) || (dir >= 9)) {
        LOG(llevError, "move_player: invalid direction %d\n", dir);
        return 0;
    }

    /* peterm:  added following line */
    if (QUERY_FLAG(op, FLAG_CONFUSED) && dir)
        dir = get_randomized_dir(dir);

    op->facing = dir;

    if (!transport && op->hide)
        do_hidden_move(op);

    if (transport) {
        int turn;

        /* transport->contr is set up for the person in charge of the boat.
         * if that isn't this person, he can't steer it, etc
         */
        if (transport->contr != op->contr)
            return 0;

        /* Transport can't move.  But update dir so it at least
         * will point in the same direction if player is running.
         */
        if (transport->speed_left < 0.0) {
            return 0;
        }
        /* Remove transport speed.  Give player just a little speed -
         * enough so that they will get an action again quickly.
         */
        transport->speed_left -= 1.0;
        if (op->speed_left < 0.0)
            op->speed_left = -0.01;

        turn = turn_transport(transport, op, dir);
        if (turn != 0)
            return 0;
    } else {
        /* it is important to change the animation now, as fire or move_player_attack can start a compound animation,
         * and leave us with state = 0, which we don't want to change again. */
        op->state++; /* player moved, so change animation. */
        animate_object(op, op->facing);
    }

    if (op->contr->fire_on) {
        fire(op, dir);
    } else
        move_player_attack(op, dir);

    pick = check_pick(op);


    /* Add special check for newcs players and fire on - this way, the
     * server can handle repeat firing.
     */
    if (op->contr->fire_on || (op->contr->run_on && pick != 0)) {
        op->direction = dir;
    } else {
        op->direction = 0;
    }
    return 0;
}

/**
 * Handles commands the player can send us, and various checks on
 * invisibility, golem and such.
 *
 * This is sort of special, in that the new client/server actually uses
 * the new speed values for commands.
 *
 * @param op
 * player to handle.
 * @return
 * true if there are more actions we can do.
 */
int handle_newcs_player(object *op) {
    if (op->contr->hidden) {
        op->invisible = 1000;
        /* the socket code flashes the player visible/invisible
         * depending on the value if invisible, so we need to
         * alternate it here for it to work correctly.
         */
        if (pticks&2)
            op->invisible--;
    } else if (op->invisible && !(QUERY_FLAG(op, FLAG_MAKE_INVIS))) {
        op->invisible--;
        if (!op->invisible) {
            make_visible(op);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SPELL, MSG_TYPE_SPELL_END,
                          "Your invisibility spell runs out.");
        }
    }

    if (QUERY_FLAG(op, FLAG_SCARED)) {
        flee_player(op);
        /* If player is still scared, that is his action for this tick */
        if (QUERY_FLAG(op, FLAG_SCARED)) {
            op->speed_left--;
            return 0;
        }
    }

    /* I've been seeing crashes where the golem has been destroyed, but
     * the player object still points to the defunct golem.  The code that
     * destroys the golem looks correct, and it doesn't always happen, so
     * put this in a a workaround to clean up the golem pointer.
     */
    if (op->contr->ranges[range_golem]
    && ((op->contr->golem_count != op->contr->ranges[range_golem]->count) || QUERY_FLAG(op->contr->ranges[range_golem], FLAG_REMOVED))) {
        op->contr->ranges[range_golem] = NULL;
        op->contr->golem_count = 0;
    }

    /* call this here - we also will call this in do_ericserver, but
     * the players time has been increased when doericserver has been
     * called, so we recheck it here.
     */
    handle_client(&op->contr->socket, op->contr);
    if (op->speed_left < 0)
        return 0;

    if (op->direction && (op->contr->run_on || op->contr->fire_on)) {
        /* All move commands take 1 tick, at least for now */
        op->speed_left--;

        /* Instead of all the stuff below, let move_player take care
         * of it.  Also, some of the skill stuff is only put in
         * there, as well as the confusion stuff.
         */
        move_player(op, op->direction);
        if (op->speed_left > 0)
            return 1;
        else
            return 0;
    }
    return 0;
}

/**
 * Can the player be saved by an item?
 *
 * @param op
 * player to try to save.
 * @retval 1
 * player had his life saved by an item, first item saving life is removed.
 * @retval 0
 * player had no life-saving item.
 */
static int save_life(object *op) {
    object *tmp;

    if (!QUERY_FLAG(op, FLAG_LIFESAVE))
        return 0;

    tmp = object_find_by_flag_applied(op, FLAG_LIFESAVE);
    if (tmp != NULL) {
        char name[MAX_BUF];

        query_name(tmp, name, MAX_BUF);
        play_sound_map(SOUND_TYPE_ITEM, tmp, 0, "evaporate");
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                             "Your %s vibrates violently, then evaporates.",
                             name);
        object_remove(tmp);
        object_free_drop_inventory(tmp);
        CLEAR_FLAG(op, FLAG_LIFESAVE);
        if (op->stats.hp < 0)
            op->stats.hp = op->stats.maxhp;
        if (op->stats.food < 0)
            op->stats.food = 999;
        fix_object(op);
        return 1;
    }
    LOG(llevError, "Error: LIFESAVE set without applied object.\n");
    CLEAR_FLAG(op, FLAG_LIFESAVE);
    enter_player_savebed(op); /* bring him home. */
    return 0;
}

/**
 * This goes throws the inventory and removes unpaid objects, and puts them
 * back in the map (location and map determined by values of env) or frees them.  This
 * function will descend into containers.
 *
 * @param op
 * object to start the search from.
 * @param env
 * top-level container, should be in a map if free_items is 0, unused if free_items is 1.
 * @param free_items
 * if set, unpaid items are freed, else they are inserted in the same map as env.
 */
void remove_unpaid_objects(object *op, object *env, int free_items) {
    FOR_OB_AND_BELOW_PREPARE(op) {
        if (QUERY_FLAG(op, FLAG_UNPAID)) {
            object_remove(op);
            if (free_items)
                object_free_drop_inventory(op);
            else
                object_insert_in_map_at(op, env->map, NULL, 0, env->x, env->y);
        } else if (op->inv)
            remove_unpaid_objects(op->inv, env, free_items);
    } FOR_OB_AND_BELOW_FINISH();
}

/**
 * Create a text for a player's gravestone.
 *
 * Moved from apply.c to player.c - player.c is what
 * actually uses this function.  player.c may not be quite the
 * best, a misc file for object actions is probably better,
 * but there isn't one in the server directory.
 *
 * @param op
 * player.
 * @param buf2
 * buffer to write the text to. Mustn't be NULL.
 * @param len
 * length of buf2.
 * @return
 * buf2, containing gravestone text.
 */
static const char *gravestone_text(object *op, char *buf2, int len) {
    char buf[MAX_BUF];
    time_t now = time(NULL);

    strncpy(buf2, "                 R.I.P.\n\n", len);
    if (op->type == PLAYER)
        snprintf(buf, sizeof(buf), "%s the %s\n", op->name, op->contr->title);
    else
        snprintf(buf, sizeof(buf), "%s\n", op->name);
    strncat(buf2, "                    ",  20-strlen(buf)/2);
    strncat(buf2, buf, len-strlen(buf2)-1);
    if (op->type == PLAYER)
        snprintf(buf, sizeof(buf), "who was in level %d when killed\n", op->level);
    else
        snprintf(buf, sizeof(buf), "who was in level %d when died.\n\n", op->level);
    strncat(buf2, "                    ", 20-strlen(buf)/2);
    strncat(buf2, buf, len-strlen(buf2)-1);
    if (op->type == PLAYER) {
        snprintf(buf, sizeof(buf), "by %s.\n\n", op->contr->killer);
        strncat(buf2, "                    ",  21-strlen(buf)/2);
        strncat(buf2, buf, len-strlen(buf2)-1);
    }
    strftime(buf, MAX_BUF, "%b %d %Y\n", localtime(&now));
    strncat(buf2, "                    ",  20-strlen(buf)/2);
    strncat(buf2, buf, len-strlen(buf2)-1);
    return buf2;
}

/**
 * Regenerate hp/sp/gr, decreases food. This only works for players.
 * Will grab food if needed, or kill player.
 *
 * @param op
 * player to regenerate for.
 */
void do_some_living(object *op) {
    int last_food = op->stats.food;
    int gen_hp, gen_sp, gen_grace;
    int rate_hp = 1200;
    int rate_sp = 2500;
    int rate_grace = 2000;

    if (op->contr->state == ST_PLAYING) {
        /* these next three if clauses make it possible to SLOW DOWN
           hp/grace/spellpoint regeneration. */
        if (op->contr->gen_hp >= 0)
            gen_hp = (op->contr->gen_hp+1)*op->stats.maxhp;
        else {
            gen_hp = op->stats.maxhp;
            rate_hp -= rate_hp/2*op->contr->gen_hp;
        }
        if (op->contr->gen_sp >= 0)
            gen_sp = (op->contr->gen_sp+1)*op->stats.maxsp;
        else {
            gen_sp = op->stats.maxsp;
            rate_sp -= rate_sp/2*op->contr->gen_sp;
        }
        if (op->contr->gen_grace >= 0)
            gen_grace = (op->contr->gen_grace+1)*op->stats.maxgrace;
        else {
            gen_grace = op->stats.maxgrace;
            rate_grace -= rate_grace/2*op->contr->gen_grace;
        }

        /* Regenerate Spell Points */
        if (op->contr->ranges[range_golem] == NULL && --op->last_sp < 0) {
            gen_sp = gen_sp*10/MAX(op->contr->gen_sp_armour, 10);
            if (op->stats.sp < op->stats.maxsp) {
                op->stats.sp++;
                /* dms do not consume food */
                if (!QUERY_FLAG(op, FLAG_WIZ)) {
                    op->stats.food--;
                    if (op->contr->digestion < 0)
                        op->stats.food += op->contr->digestion;
                    else if (op->contr->digestion > 0
                    && random_roll(0, op->contr->digestion, op, PREFER_HIGH))
                        op->stats.food = last_food;
                }
            }
            op->last_sp = rate_sp/(MAX(gen_sp, 20)+10);
        }

        /* Regenerate Grace */
        /* I altered this a little - maximum grace is only achieved through prayer -b.t.*/
        if (--op->last_grace < 0) {
            if (op->stats.grace < op->stats.maxgrace/2)
                op->stats.grace++; /* no penalty in food for regaining grace */
            op->last_grace = rate_grace/(MAX(gen_grace, 20)+10);
            /* wearing stuff doesn't detract from grace generation. */
        }

        /* Regenerate Hit Points (unless you are a wraith player) */
        if (--op->last_heal < 0 && !is_wraith_pl(op)) {
            if (op->stats.hp < op->stats.maxhp) {
                op->stats.hp++;
                /* dms do not consume food */
                if (!QUERY_FLAG(op, FLAG_WIZ)) {
                    op->stats.food--;
                    if (op->contr->digestion < 0)
                        op->stats.food += op->contr->digestion;
                    else if (op->contr->digestion > 0
                    && random_roll(0, op->contr->digestion, op, PREFER_HIGH))
                        op->stats.food = last_food;
                }
            }
            op->last_heal = rate_hp/(MAX(gen_hp, 20)+10);
        }

        /* Digestion */
        if (--op->last_eat < 0) {
            int bonus = MAX(op->contr->digestion, 0);
            int penalty = MAX(-op->contr->digestion, 0);
            if (op->contr->gen_hp > 0)
                op->last_eat = 25*(1+bonus)/(op->contr->gen_hp+penalty+1);
            else
                op->last_eat = 25*(1+bonus)/(penalty+1);
            /* dms do not consume food */
            if (!QUERY_FLAG(op, FLAG_WIZ))
                op->stats.food--;
        }
    }

    if (op->contr->state == ST_PLAYING && op->stats.food < 0 && op->stats.hp >= 0) {
        if (is_wraith_pl(op))
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE, "You feel a hunger for living flesh.");
        else {
            object *flesh = NULL;

            FOR_INV_PREPARE(op, tmp) {
                if (!QUERY_FLAG(tmp, FLAG_UNPAID)) {
                    if (tmp->type == FOOD || tmp->type == DRINK || tmp->type == POISON) {
                        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                                      "You blindly grab for a bite of food.");
                        apply_manual(op, tmp, 0);
                        if (op->stats.food >= 0 || op->stats.hp < 0)
                            break;
                    } else if (tmp->type == FLESH)
                        flesh = tmp;
                } /* End if paid for object */
            } FOR_INV_FINISH(); /* end of for loop */
            /* If player is still starving, it means they don't have any food, so
             * eat flesh instead.
             */
            if (op->stats.food < 0 && op->stats.hp >= 0 && flesh) {
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ITEM, MSG_TYPE_ITEM_REMOVE,
                              "You blindly grab for a bite of food.");
                apply_manual(op, flesh, 0);
            }
        } /* end not wraith */
    } /* end if player is starving */

    while (op->stats.food < 0 && op->stats.hp > 0)
        op->stats.food++,
        op->stats.hp--;

    if (!op->contr->state && !QUERY_FLAG(op, FLAG_WIZ) && (op->stats.hp < 0 || op->stats.food < 0))
        kill_player(op, NULL);
}

/**
 * Grab and destroy some treasure.
 *
 * @param op
 * object to loot.
 */
static void loot_object(object *op) {
    object *tmp2;

    if (op->container) { /* close open sack first */
        apply_container(op, op->container);
    }

    FOR_INV_PREPARE(op, tmp) {
        if (tmp->invisible)
            continue;
        object_remove(tmp);
        tmp->x = op->x,
        tmp->y = op->y;
        if (tmp->type == CONTAINER) { /* empty container to ground */
            loot_object(tmp);
        }
        if (!QUERY_FLAG(tmp, FLAG_UNIQUE)
        && (QUERY_FLAG(tmp, FLAG_STARTEQUIP) || QUERY_FLAG(tmp, FLAG_NO_DROP) || !(RANDOM()%3))) {
            if (tmp->nrof > 1) {
                tmp2 = object_split(tmp, 1+RANDOM()%(tmp->nrof-1), NULL, 0);
                object_free_drop_inventory(tmp2);
                object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
            } else
                object_free_drop_inventory(tmp);
        } else
            object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);
    } FOR_INV_FINISH();
}

/**
 * Handle a player's death.
 *
 * Also deals with lifesaving objects, arena deaths, cleaning disease/poison,
 * death penalties, and removing the player file in case of permadeath.
 *
 * @param op
 * Player to be killed.
 * @param killer
 * The object that's trying to kill op, which can be NULL.
 */
void kill_player(object *op, const object *killer) {
    char buf[MAX_BUF];
    int x, y;
    archetype *at;
    object *tmp;
    archetype *trophy = NULL;

    /* Don't die if the player's life can be saved. */
    if (save_life(op)) {
        return;
    }

    /* If player dies on BATTLEGROUND, no stat/exp loss! For Combat-Arenas
     * in cities ONLY!!! It is very important that this doesn't get abused.
     * Look at op_on_battleground() for more info       --AndreasV
     */
    if (op_on_battleground(op, &x, &y, &trophy)) {
        assert(trophy != NULL);
        draw_ext_info(NDI_UNIQUE|NDI_NAVY, 0, op, MSG_TYPE_VICTIM,  MSG_TYPE_VICTIM_DIED,
                      "You have been defeated in combat!\n"
                      "Local medics have saved your life...");

        /* restore player */
        at = find_archetype("poisoning");
        tmp = arch_present_in_ob(at, op);
        if (tmp) {
            object_remove(tmp);
            object_free_drop_inventory(tmp);
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
                          "Your body feels cleansed");
        }

        at = find_archetype("confusion");
        tmp = arch_present_in_ob(at, op);
        if (tmp) {
            object_remove(tmp);
            object_free_drop_inventory(tmp);
            draw_ext_info(NDI_UNIQUE, 0, tmp, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
                          "Your mind feels clearer");
        }

        cure_disease(op, NULL, NULL);  /* remove any disease */
        op->stats.hp = op->stats.maxhp;
        if (op->stats.food <= 0)
          op->stats.food = 999;

        /* create a bodypart-trophy to make the winner happy */
        tmp = arch_to_object(trophy);
        if (tmp != NULL) {
            snprintf(buf, sizeof(buf), "%s's %s", op->name, tmp->name);
            tmp->name = add_string(buf);

            snprintf(buf, sizeof(buf),
                    "This %s was %s %s the %s, who was defeated at level %d by %s.\n",
                    tmp->name, tmp->type == FLESH ? "cut off" : "taken from",
                    op->name, op->contr->title,
                    (int)(op->level), op->contr->killer);

            object_set_msg(tmp, buf);
            tmp->type = 0;
            tmp->value = 0;
            tmp->material = 0;
            tmp->materialname = NULL;
            object_insert_in_map_at(tmp, op->map, op, 0, op->x, op->y);
        }

        /* teleport defeated player to new destination*/
        transfer_ob(op, x, y, 0, NULL);
        op->contr->braced = 0;
        return;
    }

    /* Lauwenmark: Handle for plugin death event */
    if (execute_event(op, EVENT_DEATH, NULL, NULL, NULL, SCRIPT_FIX_ALL) != 0)
        return;

    /* Lauwenmark: Handle for the global death event */
    execute_global_event(EVENT_PLAYER_DEATH, op, killer);
    if (op->stats.food < 0) {
        snprintf(buf, sizeof(buf), "%s starved to death.", op->name);
        strcpy(op->contr->killer, "starvation");
    } else {
        snprintf(buf, sizeof(buf), "%s died.", op->name);
    }
    play_sound_player_only(op->contr, SOUND_TYPE_LIVING, op, 0, "death");

    if (settings.not_permadeth == TRUE) {
        kill_player_not_permadeath(op);
    } else {
        kill_player_permadeath(op);
    }
}

/**
 * Kills a player in non-permadeath mode. This basically brings the character
 * back to life if they are dead - it takes some exp and a random stat. See the
 * config.h file for a little more in depth detail about this.
 *
 * @param op
 * the player to kill.
 */
static void kill_player_not_permadeath(object *op) {
    int num_stats_lose;
    int will_kill_again;
    int lost_a_stat;
    int z;
    object *tmp;
    char buf[MAX_BUF];
    archetype *at;

    /* Basically two ways to go - remove a stat permanently, or just
     * make it depletion.  This bunch of code deals with that aspect
     * of death.
     */
    if (settings.balanced_stat_loss) {
        /* If stat loss is permanent, lose one stat only. */
        /* Lower level chars don't lose as many stats because they suffer
           more if they do. */
        /* Higher level characters can afford things such as potions of
           restoration, or better, stat potions. So we slug them that
           little bit harder. */
        /* GD */
        if (settings.stat_loss_on_death)
            num_stats_lose = 1;
        else
            num_stats_lose = 1+op->level/BALSL_NUMBER_LOSSES_RATIO;
    } else {
        num_stats_lose = 1;
    }
    lost_a_stat = 0;

    for (z = 0; z < num_stats_lose; z++) {
        if (settings.stat_loss_on_death) {
            int i;

            /* Pick a random stat and take a point off it.  Tell the player
             * what he lost.
             */
            i = RANDOM()%7;
            change_attr_value(&(op->stats), i, -1);
            check_stat_bounds(&(op->stats), MIN_STAT, settings.max_stat);
            change_attr_value(&(op->contr->orig_stats), i, -1);
            check_stat_bounds(&(op->contr->orig_stats), MIN_STAT, settings.max_stat);
            draw_ext_info(NDI_UNIQUE, 0, op,
                MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_STAT_LOSS,
                lose_msg[i]);
            lost_a_stat = 1;
        } else {
            /* deplete a stat */
            archetype *deparch = find_archetype(ARCH_DEPLETION);
            object *dep;
            int lose_this_stat;
            int i;

            i = RANDOM()%7;
            dep = arch_present_in_ob(deparch, op);
            if (!dep) {
                dep = arch_to_object(deparch);
                object_insert_in_ob(dep, op);
            }
            lose_this_stat = 1;
            if (settings.balanced_stat_loss) {
                int this_stat;

                /* GD */
                /* Get the stat that we're about to deplete. */
                this_stat = get_attr_value(&(dep->stats), i);
                if (this_stat < 0) {
                    int loss_chance = 1+op->level/BALSL_LOSS_CHANCE_RATIO;
                    int keep_chance = this_stat*this_stat;
                    /* Yes, I am paranoid. Sue me. */
                    if (keep_chance < 1)
                        keep_chance = 1;

                    /* There is a maximum depletion total per level. */
                    if (this_stat < -1-op->level/BALSL_MAX_LOSS_RATIO) {
                        lose_this_stat = 0;
                        /* Take loss chance vs keep chance to see if we
                           retain the stat. */
                    } else {
                        if (random_roll(0, loss_chance+keep_chance-1, op, PREFER_LOW) < keep_chance)
                            lose_this_stat = 0;
                        /* LOG(llevDebug, "Determining stat loss. Stat: %d Keep: %d Lose: %d Result: %s.\n", this_stat, keep_chance, loss_chance, lose_this_stat ? "LOSE" : "KEEP"); */
                    }
                }
            }

            if (lose_this_stat) {
                int this_stat;

                this_stat = get_attr_value(&(dep->stats), i);
                /* We could try to do something clever like find another
                 * stat to reduce if this fails.  But chances are, if
                 * stats have been depleted to -50, all are pretty low
                 * and should be roughly the same, so it shouldn't make a
                 * difference.
                 */
                if (this_stat >= -50) {
                    change_attr_value(&(dep->stats), i, -1);
                    SET_FLAG(dep, FLAG_APPLIED);
                    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_STAT_LOSS,
                        lose_msg[i]);
                    fix_object(op);
                        lost_a_stat = 1;
                }
            }
        }
    }
    /* If no stat lost, tell the player. */
    if (!lost_a_stat) {
        /* determine_god() seems to not work sometimes... why is this? Should I be using something else? GD */
        const char *god = determine_god(op);

        if (god && (strcmp(god, "none")))
            draw_ext_info_format(NDI_UNIQUE, 0, op,
                MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                "For a brief moment you feel the holy presence of %s protecting you",
                god);
        else
            draw_ext_info(NDI_UNIQUE, 0, op,
                MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_GOD,
                "For a brief moment you feel a holy presence protecting you.");
    }

    /* Put a gravestone up where the character 'almost' died.  List the
     * exp loss on the stone.
     */
    tmp = arch_to_object(find_archetype("gravestone"));
    snprintf(buf, sizeof(buf), "%s's gravestone", op->name);
    FREE_AND_COPY(tmp->name, buf);
    snprintf(buf, sizeof(buf), "%s's gravestones", op->name);
    FREE_AND_COPY(tmp->name_pl, buf);
    snprintf(buf, sizeof(buf), "RIP\nHere rests the hero %s the %s,\n"
        "who was killed\n"
        "by %s.\n",
        op->name, op->contr->title,
        op->contr->killer);
    object_set_msg(tmp, buf);
    object_insert_in_map_at(tmp, op->map, NULL, 0, op->x, op->y);

    /* restore player: remove any poisoning, disease and confusion the
     * character may be suffering.*/
    at = find_archetype("poisoning");
    tmp = arch_present_in_ob(at, op);
    if (tmp) {
        object_remove(tmp);
        object_free_drop_inventory(tmp);
        draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
            "Your body feels cleansed");
    }

    at = find_archetype("confusion");
    tmp = arch_present_in_ob(at, op);
    if (tmp) {
        object_remove(tmp);
        object_free_drop_inventory(tmp);
        draw_ext_info(NDI_UNIQUE, 0, tmp, MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_BAD_EFFECT_END,
            "Your mind feels clearer");
    }
    cure_disease(op, NULL, NULL);  /* remove any disease */

    /* Subtract the experience points, if we died cause of food, give
     * us food, and reset HP's...
     */
    apply_death_exp_penalty(op);
    if (op->stats.food < 100)
        op->stats.food = 900;
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = MAX(op->stats.sp, op->stats.maxsp);
    op->stats.grace = MAX(op->stats.grace, op->stats.maxgrace);

    /* Check to see if the player is in a shop. IF so, then check to see if
     * the player has any unpaid items.  If so, remove them and put them back
     * in the map.
     *
     * If they are not in a shop, just free the unpaid items instead of
     * putting them back on map.
     */
    if (is_in_shop(op))
        remove_unpaid_objects(op->inv, op, 0);
    else
        remove_unpaid_objects(op->inv, op, 1);

    /* Move player to his current respawn-position (usually last savebed) */
    enter_player_savebed(op);

    /* Save the player before inserting the force to reduce chance of abuse. */
    op->contr->braced = 0;
    save_player(op, 1);

    /* it is possible that the player has blown something up
     * at his savebed location, and that can have long lasting
     * spell effects.  So first see if there is a spell effect
     * on the space that might harm the player.
     */
    will_kill_again = 0;
    FOR_MAP_PREPARE(op->map, op->x, op->y, tmp)
        if (tmp->type == SPELL_EFFECT)
            will_kill_again |= tmp->attacktype;
    FOR_MAP_FINISH();
    if (will_kill_again) {
        object *force;
        int at;

        force = create_archetype(FORCE_NAME);
        /* 50 ticks should be enough time for the spell to abate */
        force->speed = 0.1;
        force->speed_left = -5.0;
        SET_FLAG(force, FLAG_APPLIED);
        for (at = 0; at < NROFATTACKS; at++) {
            if (will_kill_again&(1<<at))
                force->resist[at] = 100;
        }
        object_insert_in_ob(force, op);
        fix_object(op);
    }

    /* Tell the player they have died */
    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_DIED,
        "YOU HAVE DIED.");
}

/**
 * Kills a player in permadeath mode.
 *
 * @param op
 * the player to kill.
 */
static void kill_player_permadeath(object *op) {
    char buf[MAX_BUF];
    char ac_buf[MAX_BUF];
    int x, y;
    mapstruct *map;
    object *tmp;

    /*  save the map location for corpse, gravestone*/
    x = op->x;
    y = op->y;
    map = op->map;

    party_leave(op);
    if (settings.set_title == TRUE)
        player_set_own_title(op->contr, "");

    /* buf should be the kill message */
    draw_ext_info(NDI_UNIQUE|NDI_ALL, 0, NULL, MSG_TYPE_VICTIM, MSG_TYPE_VICTIM_DIED,
        buf);
    hiscore_check(op, 0);
    if (op->contr->ranges[range_golem] != NULL) {
        remove_friendly_object(op->contr->ranges[range_golem]);
        object_remove(op->contr->ranges[range_golem]);
        object_free_drop_inventory(op->contr->ranges[range_golem]);
        op->contr->ranges[range_golem] = NULL;
        op->contr->golem_count = 0;
    }
    loot_object(op); /* Remove some of the items for good */
    object_remove(op);
    op->direction = 0;

    if (!QUERY_FLAG(op, FLAG_WAS_WIZ) && op->stats.exp) {
        if (settings.resurrection == TRUE) {
            /* save playerfile sans equipment when player dies
             * -then save it as player.pl.dead so that future resurrection
             * -type spells will work on them nicely
             */
            op->stats.hp = op->stats.maxhp;
            op->stats.food = 999;

            /* set the location of where the person will reappear when  */
            /* maybe resurrection code should fix map also */
            strcpy(op->contr->maplevel, settings.emergency_mapname);
            if (op->map != NULL)
                op->map = NULL;
            op->x = settings.emergency_x;
            op->y = settings.emergency_y;
            save_player(op, 0);
            op->map = map;
            /* please see resurrection.c: peterm */
            dead_player(op);
        } else {
            delete_character(op->name);
        }
    }
    play_again(op);

    /*  peterm:  added to create a corpse at deathsite.  */
    tmp = arch_to_object(find_archetype("corpse_pl"));
    snprintf(buf, sizeof(buf), "%s", op->name);
    FREE_AND_COPY(tmp->name, buf);
    FREE_AND_COPY(tmp->name_pl, buf);
    tmp->level = op->level;
    object_set_msg(tmp, gravestone_text(op, buf, sizeof(buf)));
    SET_FLAG(tmp, FLAG_UNIQUE);
    /*
     * Put the account name under slaying.
     * Does not seem to cause weird effects, but more testing may ensure this.
     */
    snprintf(ac_buf, sizeof(ac_buf), "%s", op->contr->socket.account_name);
    FREE_AND_COPY(tmp->slaying, ac_buf);
    object_insert_in_map_at(tmp, map, NULL, 0, x, y);
}

/**
 * Check recursively the weight of all players, and fix
 * what needs to be fixed.  Refresh windows and fix speed if anything
 * was changed.
 *
 * @todo is this still useful?
 */
void fix_weight(void) {
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next) {
        int old = pl->ob->carrying, sum = object_sum_weight(pl->ob);

        if (old == sum)
            continue;
        fix_object(pl->ob);
        LOG(llevDebug, "Fixed inventory in %s (%d -> %d)\n", pl->ob->name, old, sum);
    }
}

/**
 * Fixes luck of players, slowly move it towards 0.
 */
void fix_luck(void) {
    player *pl;

    for (pl = first_player; pl != NULL; pl = pl->next)
        if (!pl->ob->contr->state)
            change_luck(pl->ob, 0);
}


/**
 * Handles op throwing objects of type 'DUST'.
 * This is much simpler in the new spell code - we basically
 * just treat this as any other spell casting object.
 *
 * @param op
 * object throwing.
 * @param throw_ob
 * what to throw.
 * @param dir
 * direction to throw into.
 */
void cast_dust(object *op, object *throw_ob, int dir) {
    object *skop, *spob;

    skop = find_skill_by_name(op, throw_ob->skill);

    /* casting POTION 'dusts' is really a use_magic_item skill */
    if (op->type == PLAYER && throw_ob->type == POTION && !skop) {
        LOG(llevError, "Player %s lacks critical skill use_magic_item!\n", op->name);
        return;
    }
    spob = throw_ob->inv;
    if (op->type == PLAYER && spob)
        draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_APPLY, MSG_TYPE_APPLY_SUCCESS,
                             "You cast %s.",
                             spob->name);

    cast_spell(op, throw_ob, dir, spob, NULL);

    if (!QUERY_FLAG(throw_ob, FLAG_REMOVED))
        object_remove(throw_ob);
    object_free_drop_inventory(throw_ob);
}

/**
 * Makes an object visible again.
 *
 * @param op
 * what to make visible.
 */
void make_visible(object *op) {
    op->hide = 0;
    op->invisible = 0;
    if (op->type == PLAYER) {
        op->contr->tmp_invis = 0;
        if (op->contr->invis_race)
            FREE_AND_CLEAR_STR(op->contr->invis_race);
    }
    object_update(op, UP_OBJ_FACE);
}

/**
 * Is the object a true undead?
 *
 * @param op
 * object to test.
 * @return
 * 1 if undead, 0 else.
 * @todo remove loop on type 44 (was EXPERIENCE)
 */
int is_true_undead(object *op) {
    if (QUERY_FLAG(&op->arch->clone, FLAG_UNDEAD))
        return 1;

    if (op->type == PLAYER)
        FOR_INV_PREPARE(op, tmp) {
            if (tmp->type == 44 && tmp->stats.Wis)
                if (QUERY_FLAG(tmp, FLAG_UNDEAD))
                    return 1;
        } FOR_INV_FINISH();
    return 0;
}

/**
 * Look at the surrounding terrain to determine
 * the hideability of this object. Positive levels
 * indicate greater hideability.
 *
 * @param ob
 * object that may want to hide.
 * @return
 * the higher the value, the easier to hide here.
 */
int hideability(object *ob) {
    int i, level = 0, mflag;
    sint16 x, y;

    if (!ob || !ob->map)
        return 0;

    /* so, on normal lighted maps, its hard to hide */
    level = ob->map->darkness-2;

    /* this also picks up whether the object is glowing.
     * If you carry a light on a non-dark map, its not
     * as bad as carrying a light on a pitch dark map
     */
    if (has_carried_lights(ob))
        level = -(10+(2*ob->map->darkness));

    /* scan through all nearby squares for terrain to hide in */
    for (i = 0, x = ob->x, y = ob->y; i < 9; i++, x = ob->x+freearr_x[i], y = ob->y+freearr_y[i]) {
        mflag = get_map_flags(ob->map, NULL, x, y, NULL, NULL);
        if (mflag&P_OUT_OF_MAP) {
            continue;
        }
        if (mflag&P_BLOCKSVIEW) /* something to hide near! */
            level += 2;
        else /* open terrain! */
            level -= 1;
    }

    return level;
}

/**
 * For hidden creatures - a chance of becoming 'unhidden'
 * every time they move - as we subtract off 'invisibility'
 * AND, for players, if they move into a ridiculously unhideable
 * spot (surrounded by clear terrain in broad daylight). -b.t.
 *
 * @param op
 * object moving.
 */
void do_hidden_move(object *op) {
    int hide = 0, num = random_roll(0, 19, op, PREFER_LOW);
    object *skop;

    if (!op || !op->map)
        return;

    skop = object_find_by_type_subtype(op, SKILL, SK_HIDING);

    /* its *extremely *hard to run and sneak/hide at the same time! */
    if (op->type == PLAYER && op->contr->run_on) {
        if (!skop || num >= skop->level) {
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                          "You ran too much! You are no longer hidden!");
            make_visible(op);
            return;
        } else
            num += 20;
    }
    num += op->map->difficulty;
    hide = hideability(op); /* modify by terrain hidden level */
    num -= hide;
    if ((op->type == PLAYER && hide < -10)
    || ((op->invisible -= num) <= 0)) {
        make_visible(op);
        if (op->type == PLAYER)
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE,
                          "You moved out of hiding! You are visible!");
    } else if (op->type == PLAYER && skop) {
        change_exp(op, calc_skill_exp(op, NULL, skop), skop->skill, 0);
    }
}

/**
 * Determine if who is standing near a hostile creature.
 *
 * @param who
 * object to check.
 * @return
 * 1 if near a monster, 0 else.
 */
int stand_near_hostile(object *who) {
    int i, friendly = 0, player = 0, mflags;
    mapstruct *m;
    sint16  x, y;

    if (!who)
        return 0;

    if (who->type == PLAYER)
        player = 1;
    else
        friendly = QUERY_FLAG(who, FLAG_FRIENDLY);

    /* search adjacent squares */
    for (i = 1; i < 9; i++) {
        x = who->x+freearr_x[i];
        y = who->y+freearr_y[i];
        m = who->map;
        mflags = get_map_flags(m, &m, x, y, &x, &y);
        /* space must be blocked if there is a monster.  If not
         * blocked, don't need to check this space.
         */
        if (mflags&P_OUT_OF_MAP)
            continue;
        if (OB_TYPE_MOVE_BLOCK(who, GET_MAP_MOVE_BLOCK(m, x, y)))
            continue;

        FOR_MAP_PREPARE(m, x, y, tmp) {
            if ((player || friendly)
            && QUERY_FLAG(tmp, FLAG_MONSTER)
            && !QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE))
                return 1;
            else if (tmp->type == PLAYER) {
                /*don't let a hidden DM prevent you from hiding*/
                if (!QUERY_FLAG(tmp, FLAG_WIZ) || tmp->contr->hidden == 0)
                    return 1;
            }
        } FOR_MAP_FINISH();
    }
    return 0;
}

/**
 * Check the player los field for viewability of the
 * object op. This function works fine for monsters,
 * but we dont worry if the object isnt the top one in
 * a pile (say a coin under a table would return "viewable"
 * by this routine). Another question, should we be
 * concerned with the direction the player is looking
 * in? Realistically, most of use cant see stuff behind
 * our backs...on the other hand, does the "facing" direction
 * imply the way your head, or body is facing? Its possible
 * for them to differ. Sigh, this fctn could get a bit more complex.
 * -b.t.
 *
 * This function is now map tiling safe.
 *
 * @param pl
 * player that may see op.
 * @param op
 * what may be seen by pl.
 * @retval -1
 * pl isn't a player
 * @retval 0
 * pl can't see op.
 * @retval 1
 * pl can see op.
 */
int player_can_view(object *pl, object *op) {
    rv_vector rv;
    int dx, dy;

    if (pl->type != PLAYER) {
        LOG(llevError, "player_can_view() called for non-player object\n");
        return -1;
    }
    if (!pl || !op)
        return 0;

    op = HEAD(op);
    if (!get_rangevector(pl, op, &rv, 0x1))
        return 0;

    /* starting with the 'head' part, lets loop
     * through the object and find if it has any
     * part that is in the los array but isnt on
     * a blocked los square.
     * we use the archetype to figure out offsets.
     */
    while (op) {
        dx = rv.distance_x+op->arch->clone.x;
        dy = rv.distance_y+op->arch->clone.y;

        /* only the viewable area the player sees is updated by LOS
         * code, so we need to restrict ourselves to that range of values
         * for any meaningful values.
         */
        if (FABS(dx) <= (pl->contr->socket.mapx/2)
        && FABS(dy) <= (pl->contr->socket.mapy/2)
        && !pl->contr->blocked_los[dx+(pl->contr->socket.mapx/2)][dy+(pl->contr->socket.mapy/2)])
            return 1;
        op = op->more;
    }
    return 0;
}

/**
 * We call this when there is a possibility for our action disturbing our hiding
 * place or invisibility spell. Artefact invisibility is not
 * effected by this. If we aren't invisible to begin with, we
 * return 0.
 *
 * This routine works for both players and monsters.
 *
 * @param op
 * object to check.
 * @return
 * 1 if op isn't invisible anymore, 0 else.
 */
static int action_makes_visible(object *op) {
    if (op->invisible && QUERY_FLAG(op, FLAG_ALIVE)) {
        if (QUERY_FLAG(op, FLAG_MAKE_INVIS))
            return 0;

        if (op->contr && op->contr->tmp_invis == 0)
            return 0;

        /* If monsters, they should become visible */
        if (op->hide || !op->contr || (op->contr && op->contr->tmp_invis)) {
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_MISC, MSG_SUBTYPE_NONE,
                                 "You become %s!",
                                 op->hide ? "unhidden" : "visible");
            return 1;
        }
    }
    return 0;
}

/**
 * Check if the given object (usually a player) is standing on a battleground
 * tile. This is used to handle deaths and special attacks in arenas.
 *
 * A battleground tile must have the following attributes set:
 *  - name "battleground"
 *  - type 58 (BATTLEGROUND)
 *  - is_floor 1 (must be the first tile beneath the player's feet)
 *  - no_pick 1
 *  - sp / hp > 0 (non-zero exit coordinates)
 *
 * If the tile has 'slaying', 'maxhp', and 'maxsp' set, and the player has a
 * matching marker, send the player to those coordinates instead.
 *
 * If the tile has 'other_arch' set, then create that archetype as the trophy
 * instead of the default ("finger").
 *
 * @param op
 * Object to check (usually a player).
 * @param[out] x
 * @param[out] y
 * If not NULL and standing on a battleground tile, store exit coordinates.
 * @param[out] trophy
 * If not NULL and standing on a battleground tile, store a pointer to the
 * archetype that can be collected by the winner.
 * @return
 * TRUE if op is on a battleground, FALSE if not.
 */
int op_on_battleground(object *op, int *x, int *y, archetype **trophy) {
    FOR_BELOW_PREPARE(op, tmp) {
        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR)) {
            if (QUERY_FLAG(tmp, FLAG_NO_PICK)
            && strcmp(tmp->name, "battleground") == 0
            && tmp->type == BATTLEGROUND
            && EXIT_X(tmp)
            && EXIT_Y(tmp)) {
                /*before we assign the exit, check if this is a teambattle*/
                if (EXIT_ALT_X(tmp) && EXIT_ALT_Y(tmp) && EXIT_PATH(tmp)) {
                    object *invtmp;

                    invtmp = object_find_by_type_and_slaying(op, FORCE, EXIT_PATH(tmp));
                    if (invtmp != NULL) {
                        if (x != NULL && y != NULL)
                            *x = EXIT_ALT_X(tmp),
                                *y = EXIT_ALT_Y(tmp);
                        return 1;
                    }
                }
                if (x != NULL && y != NULL)
                    *x = EXIT_X(tmp),
                    *y = EXIT_Y(tmp);

                /* If 'other_arch' is not specified, give a finger. */
                if (trophy != NULL) {
                    if (tmp->other_arch) {
                        *trophy = tmp->other_arch;
                    } else {
                        *trophy = find_archetype("finger");
                    }
                }
                return 1;
            }
        }
    } FOR_BELOW_FINISH();
    /* If we got here, did not find a battleground */
    return 0;
}

/**
 * When a dragon-player gains a new stage of evolution, he gets some treasure.
 *
 * @param who
 * the dragon player.
 * @param atnr
 * the attack-number of the ability focus.
 * @param level
 * ability level.
 */
void dragon_ability_gain(object *who, int atnr, int level) {
    treasurelist *trlist = NULL;    /* treasurelist */
    treasure *tr;      /* treasure */
    object *tmp, *skop;      /* tmp. object */
    object *item;      /* treasure object */
    char buf[MAX_BUF];      /* tmp. string buffer */
    int i = 0, j = 0;

    /* get the appropriate treasurelist */
    if (atnr == ATNR_FIRE)
        trlist = find_treasurelist("dragon_ability_fire");
    else if (atnr == ATNR_COLD)
        trlist = find_treasurelist("dragon_ability_cold");
    else if (atnr == ATNR_ELECTRICITY)
        trlist = find_treasurelist("dragon_ability_elec");
    else if (atnr == ATNR_POISON)
        trlist = find_treasurelist("dragon_ability_poison");

    if (trlist == NULL || who->type != PLAYER)
        return;

    for (i = 0, tr = trlist->items; tr != NULL && i < level-1; tr = tr->next, i++)
        ;
    if (tr == NULL || tr->item == NULL) {
        /* LOG(llevDebug, "-> no more treasure for %s\n", change_resist_msg[atnr]); */
        return;
    }

    /* everything seems okay - now bring on the gift: */
    item = &(tr->item->clone);

    if (item->type == SPELL) {
        if (check_spell_known(who, item->name))
            return;

        draw_ext_info_format(NDI_UNIQUE|NDI_BLUE, 0, who,
                             MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_RACE,
                             "You gained the ability of %s",
                             item->name);
        do_learn_spell(who, item, 0);
        return;
    }

    /* grant direct spell */
    if (item->type == SPELLBOOK) {
        if (!item->inv) {
            LOG(llevDebug, "dragon_ability_gain: Broken spellbook %s\n", item->name);
            return;
        }
        if (check_spell_known(who, item->inv->name))
            return;
        if (item->invisible) {
            draw_ext_info_format(NDI_UNIQUE|NDI_BLUE, 0, who,
                                 MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_RACE,
                                 "You gained the ability of %s",
                                 item->inv->name);
            do_learn_spell(who, item->inv, 0);
            return;
        }
    } else if (item->type == SKILL_TOOL && item->invisible) {
        if (item->subtype == SK_CLAWING && (skop = find_skill_by_name(who, item->skill)) != NULL) {
            /* should this perhaps be (skop->attackyp&item->attacktype) != item->attacktype ...
             * in this way, if the player is missing any of the attacktypes, he gets
             * them.  As it is now, if the player has any that match the granted skill,
             * but not all of them, he gets nothing.
             */
            if (!(skop->attacktype&item->attacktype)) {
                /* Give new attacktype */
                skop->attacktype |= item->attacktype;

                /* always add physical if there's none */
                skop->attacktype |= AT_PHYSICAL;

                if (item->msg != NULL)
                    draw_ext_info(NDI_UNIQUE|NDI_BLUE, 0, who,
                                  MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_RACE,
                                  item->msg);

                /* Give player new face */
                if (item->animation_id) {
                    who->face = skop->face;
                    who->animation_id = item->animation_id;
                    who->anim_speed = item->anim_speed;
                    who->last_anim = 0;
                    who->state = 0;
                    animate_object(who, who->direction);
                }
            }
        }
    } else if (item->type == FORCE) {
        /* forces in the treasurelist can alter the player's stats */
        object *skin;

        /* first get the dragon skin force */
        skin = object_find_by_arch_name(who, "dragon_skin_force");
        if (skin == NULL)
            return;

        /* adding new spellpath attunements */
        if (item->path_attuned > 0 && !(skin->path_attuned&item->path_attuned)) {
            skin->path_attuned |= item->path_attuned; /* add attunement to skin */

            /* print message */
            snprintf(buf, sizeof(buf), "You feel attuned to ");
            for (i = 0, j = 0; i < NRSPELLPATHS; i++) {
                if (item->path_attuned&(1<<i)) {
                    if (j)
                        strcat(buf, " and ");
                    else
                        j = 1;
                    strcat(buf, spellpathnames[i]);
                }
            }
            strcat(buf, ".");
            draw_ext_info(NDI_UNIQUE|NDI_BLUE, 0, who,
                          MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_RACE,
                          buf);
        }

        /* evtl. adding flags: */
        if (QUERY_FLAG(item, FLAG_XRAYS))
            SET_FLAG(skin, FLAG_XRAYS);
        if (QUERY_FLAG(item, FLAG_STEALTH))
            SET_FLAG(skin, FLAG_STEALTH);
        if (QUERY_FLAG(item, FLAG_SEE_IN_DARK))
            SET_FLAG(skin, FLAG_SEE_IN_DARK);

        /* print message if there is one */
        if (item->msg != NULL)
            draw_ext_info(NDI_UNIQUE|NDI_BLUE, 0, who,
                          MSG_TYPE_ATTRIBUTE, MSG_TYPE_ATTRIBUTE_RACE,
                          item->msg);
    } else {
        /* generate misc. treasure */
        char name[HUGE_BUF];

        tmp = arch_to_object(tr->item);
        query_short_name(tmp, name, HUGE_BUF);
        draw_ext_info_format(NDI_UNIQUE|NDI_BLUE, 0, who,
                             MSG_TYPE_ITEM, MSG_TYPE_ITEM_ADD,
                             "You gained %s",
                             name);
        tmp = object_insert_in_ob(tmp, who);
        if (who->type == PLAYER)
            esrv_send_item(who, tmp);
    }
}

/**
 * Unready an object for a player. This function does nothing if the object was
 * not readied.
 *
 * @param pl
 * player.
 * @param ob
 * object to unready.
 */
void player_unready_range_ob(player *pl, object *ob) {
    rangetype i;

    for (i = 0; i < range_size; i++) {
        if (pl->ranges[i] == ob) {
            pl->ranges[i] = NULL;
            if (pl->shoottype == i) {
                pl->shoottype = range_none;
            }
        }
    }
}

/**
 * Set the player's state to the specified one.
 * @param pl who to set state for.
 * @param state new state.
 */
void player_set_state(player *pl, uint8 state) {

    assert(pl);
    assert(state >= ST_PLAYING && state <= ST_CHANGE_PASSWORD_CONFIRM);
    pl->state = state;
}
