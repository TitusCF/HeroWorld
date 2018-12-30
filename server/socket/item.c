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
 * \file
 * Client/server logic.
 *
 * \date 2003-12-02
 *
 * This containes item logic for client/server.  It doesn't contain
 * the actual commands that send the data, but does contain
 * the logic for what items should be sent.
 */

#include <global.h>
#include <object.h> /* LOOK_OBJ */
#include <shared/newclient.h>
#include <newserver.h>
#include <sproto.h>

/** This is the maximum number of bytes we expect any one item to take up */
#define MAXITEMLEN  300

/*************************************************************************
 *
 * Functions related to sending object data to the client.
 *
 *************************************************************************
 */

/**
 *  This is a similar to query_name, but returns flags
 *  to be sended to client.
 */
static unsigned int query_flags(const object *op) {
    unsigned int flags = 0;

    if (QUERY_FLAG(op, FLAG_APPLIED)) {
        switch (op->type) {
        case BOW:
        case WAND:
        case ROD:
            flags = a_readied;
            break;

        case WEAPON:
            flags = a_wielded;
            break;

        case SKILL:
        case ARMOUR:
        case HELMET:
        case SHIELD:
        case RING:
        case BOOTS:
        case GLOVES:
        case AMULET:
        case GIRDLE:
        case BRACERS:
        case CLOAK:
            flags = a_worn;
            break;

        case CONTAINER:
            flags = a_active;
            break;

        default:
            flags = a_applied;
            break;
        }
    }
    if (op->type == CONTAINER
    && ((op->env && op->env->container == op) || (!op->env && QUERY_FLAG(op, FLAG_APPLIED))))
        flags |= F_OPEN;

    if (!QUERY_FLAG(op, FLAG_IDENTIFIED) && need_identify(op))
        flags |= F_UNIDENTIFIED;

    if (QUERY_FLAG(op, FLAG_KNOWN_CURSED)) {
        if (QUERY_FLAG(op, FLAG_DAMNED))
            flags |= F_DAMNED;
        else if (QUERY_FLAG(op, FLAG_CURSED))
            flags |= F_CURSED;
    }
    if (QUERY_FLAG(op, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(op, FLAG_IDENTIFIED))
        flags |= F_MAGIC;
    if (QUERY_FLAG(op, FLAG_UNPAID))
        flags |= F_UNPAID;
    if (QUERY_FLAG(op, FLAG_INV_LOCKED))
        flags |= F_LOCKED;
    if (QUERY_FLAG(op, FLAG_KNOWN_BLESSED) && QUERY_FLAG(op, FLAG_BLESSED))
        flags |= F_BLESSED;

    return flags;
}

/**
 * Used in the send_look to put object head into SockList
 * sl for socket ns.  Need socket to know if we need to send
 * animation of face to the client.
 */
static void add_object_to_socklist(socket_struct *ns, SockList *sl, object *head) {
    int flags, len, anim_speed;
    char item_n[MAX_BUF], item_p[MAX_BUF];

    flags = query_flags(head);
    if (QUERY_FLAG(head, FLAG_NO_PICK))
        flags |= F_NOPICK;

    if (!(ns->faces_sent[head->face->number]&NS_FACESENT_FACE))
        esrv_send_face(ns, head->face->number, 0);

    if (QUERY_FLAG(head, FLAG_ANIMATE)) {
        if (head->animation_id == 0) {
            LOG(llevError, "Item %s in %s (%d,%d) has FLAG_ANIMATE but animation_id 0\n", head->name, (head->env ? head->env->name : (head->map ? head->map->path : "???")), head->x, head->y);
            CLEAR_FLAG(head, FLAG_ANIMATE);
        } else if (!ns->anims_sent[head->animation_id])
            esrv_send_animation(ns, head->animation_id);
    }

    SockList_AddInt(sl, head->count);
    SockList_AddInt(sl, flags);
    SockList_AddInt(sl, QUERY_FLAG(head, FLAG_NO_PICK) ? -1 : WEIGHT(head));
    SockList_AddInt(sl, head->face->number);

    if (!head->custom_name) {
        query_base_name(head, 0, item_n, 126);
        item_n[127] = 0;
        len = strlen(item_n);
        query_base_name(head, 1, item_p, MAX_BUF);
    } else {
        strncpy(item_n, head->custom_name, 127);
        item_n[127] = 0;
        len = strlen(item_n);
        strncpy(item_p, head->custom_name, MAX_BUF);
    }
    strncpy(item_n+len+1, item_p, 127);
    /* This is needed because strncpy may not add a ending \0 if the string is long enough. */
    item_n[len+1+127] = 0;
    len += strlen(item_n+1+len)+1;
    SockList_AddLen8Data(sl, item_n, len);

    SockList_AddShort(sl, head->animation_id);
    anim_speed = 0;
    if (QUERY_FLAG(head, FLAG_ANIMATE)) {
        if (head->anim_speed)
            anim_speed = head->anim_speed;
        else {
            if (FABS(head->speed) < 0.001)
                anim_speed = 255;
            else if (FABS(head->speed) >= 1.0)
                anim_speed = 1;
            else
                anim_speed = (int)(1.0/FABS(head->speed));
        }
        if (anim_speed > 255)
            anim_speed = 255;
    }
    SockList_AddChar(sl, (char)anim_speed);
    SockList_AddInt(sl, head->nrof);

    SockList_AddShort(sl, head->client_type);

    SET_FLAG(head, FLAG_CLIENT_SENT);
}

/**
 * Send the look window.  Don't need to do animations here
 * This sends all the faces to the client, not just updates.  This is
 * because object ordering would otherwise be inconsistent
 */
void esrv_draw_look(object *pl) {
    object *tmp, *last;
    int got_one = 0, start_look = 0, end_look = 0, objects_sent = 0;
    SockList sl;
    char buf[MAX_BUF];

    if (!pl->contr->socket.update_look) {
        LOG(llevDebug, "esrv_draw_look called when update_look was not set\n");
        return;
    } else {
        pl->contr->socket.update_look = 0;
    }

    if (QUERY_FLAG(pl, FLAG_REMOVED)
    || pl->map == NULL
    || pl->map->in_memory != MAP_IN_MEMORY
    || out_of_map(pl->map, pl->x, pl->y))
        return;

    if (pl->contr->transport)
        for (tmp = pl->contr->transport->inv; tmp && tmp->above; tmp = tmp->above)
            ;
    else
        for (tmp = GET_MAP_OB(pl->map, pl->x, pl->y); tmp && tmp->above; tmp = tmp->above)
            ;

    SockList_Init(&sl);
    SockList_AddString(&sl, "delinv 0");
    Send_With_Handling(&pl->contr->socket, &sl);

    SockList_Reset(&sl);
    SockList_AddPrintf(&sl, "item2 ");
    SockList_AddInt(&sl, 0);

    if (!(pl->contr->socket.faces_sent[empty_face->number]&NS_FACESENT_FACE))
        esrv_send_face(&pl->contr->socket, empty_face->number, 0);

    if (pl->contr->socket.look_position) {
        int overhead = 1+(pl->contr->transport != NULL);
        int prev_len = pl->contr->socket.num_look_objects-overhead-(pl->contr->socket.look_position > pl->contr->socket.num_look_objects-overhead);
        SockList_AddInt(&sl, 0x80000000|MAX(0, pl->contr->socket.look_position-prev_len));
        SockList_AddInt(&sl, 0);
        SockList_AddInt(&sl, -1);
        SockList_AddInt(&sl, empty_face->number);
        snprintf(buf, sizeof(buf), "Click here to see previous group of items");
        SockList_AddLen8Data(&sl, buf, MIN(strlen(buf), 255));
        SockList_AddShort(&sl, 0);
        SockList_AddChar(&sl, 0);
        SockList_AddInt(&sl, 0);
        SockList_AddShort(&sl, 0);
        objects_sent++;
        got_one++;
    }

    if (pl->contr->transport) {
        add_object_to_socklist(&pl->contr->socket, &sl, pl->contr->transport);
        objects_sent++;
        got_one++;
    }

    last = NULL;
    FOR_OB_AND_BELOW_PREPARE(tmp) {
        object *head;

        if (tmp == last) {
            break;
        }

        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR) && !last) {
            last = tmp->below;  /* assumes double floor mode */
            if (last && QUERY_FLAG(last, FLAG_IS_FLOOR))
                last = last->below;
        }
        if (LOOK_OBJ(tmp)) {
            if (start_look++ < pl->contr->socket.look_position)
                continue;
            end_look++;
            objects_sent++;
            if (objects_sent >= pl->contr->socket.num_look_objects) {
                /* What we basically do is make a 'fake' object -
                 * when the user applies it, we notice the special
                 * tag the object has, and act accordingly.
                 */
                SockList_AddInt(&sl, 0x80000000|(pl->contr->socket.look_position+end_look-1));
                SockList_AddInt(&sl, 0);
                SockList_AddInt(&sl, -1);
                SockList_AddInt(&sl, empty_face->number);
                snprintf(buf, sizeof(buf), "Click here to see next group of items");
                SockList_AddLen8Data(&sl, buf, MIN(strlen(buf), 255));
                SockList_AddShort(&sl, 0);
                SockList_AddChar(&sl, 0);
                SockList_AddInt(&sl, 0);
                SockList_AddShort(&sl, 0);
                break;
            }
            head = HEAD(tmp);
            add_object_to_socklist(&pl->contr->socket, &sl, head);
            got_one++;

            if (SockList_Avail(&sl) < MAXITEMLEN) {
                Send_With_Handling(&pl->contr->socket, &sl);
                SockList_Reset(&sl);
                SockList_AddPrintf(&sl, "item2 ");
                SockList_AddInt(&sl, 0);
                got_one = 0;
            }
        } /* If LOOK_OBJ() */
    } FOR_OB_AND_BELOW_FINISH();
    if (got_one)
        Send_With_Handling(&pl->contr->socket, &sl);

    SockList_Term(&sl);
}

/**
 * Sends inventory of a container. If pl == op, then the whole contents are sent,
 * else the contents is sent with "next group" and "prev group" arrows if required.
 * @param pl who to send to.
 * @param op what item's contents to send.
 */
void esrv_send_inventory(object *pl, object *op) {
    int got_one = 0, start_look = 0, end_look = 0, objects_sent = 0;
    SockList sl;
    char buf[MAX_BUF];
    int prev_len = pl->contr->socket.num_look_objects - 2 - (((pl->contr->socket.container_position > pl->contr->socket.num_look_objects - 1)) ? 1 : 0);

    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "delinv %u", op->count);
    Send_With_Handling(&pl->contr->socket, &sl);

    SockList_Reset(&sl);
    SockList_AddString(&sl, "item2 ");
    SockList_AddInt(&sl, op->count);
    objects_sent++;

    if (pl != op && pl->contr->socket.container_position) {
        SockList_AddInt(&sl, 0x80000000|MAX(0, pl->contr->socket.container_position-prev_len));
        SockList_AddInt(&sl, 0);
        SockList_AddInt(&sl, -1);
        SockList_AddInt(&sl, empty_face->number);
        snprintf(buf, sizeof(buf), "Click here to see previous group of items");
        SockList_AddLen8Data(&sl, buf, MIN(strlen(buf), 255));
        SockList_AddShort(&sl, 0);
        SockList_AddChar(&sl, 0);
        SockList_AddInt(&sl, 0);
        SockList_AddShort(&sl, 0);
        objects_sent++;
        got_one++;
    }

    FOR_INV_PREPARE(op, tmp) {
        object *head;

        head = HEAD(tmp);
        if (LOOK_OBJ(head)) {
            if (start_look++ < pl->contr->socket.container_position && pl != op)
                continue;
            end_look++;
            objects_sent++;
            if (pl != op && objects_sent >= pl->contr->socket.num_look_objects) {
                /* What we basically do is make a 'fake' object -
                 * when the user applies it, we notice the special
                 * tag the object has, and act accordingly.
                 */
                SockList_AddInt(&sl, 0x80000000|(pl->contr->socket.container_position + end_look - 1));
                SockList_AddInt(&sl, 0);
                SockList_AddInt(&sl, -1);
                SockList_AddInt(&sl, empty_face->number);
                snprintf(buf, sizeof(buf), "Click here to see next group of items");
                SockList_AddLen8Data(&sl, buf, MIN(strlen(buf), 255));
                SockList_AddShort(&sl, 0);
                SockList_AddChar(&sl, 0);
                SockList_AddInt(&sl, 0);
                SockList_AddShort(&sl, 0);
                break;
            }

            add_object_to_socklist(&pl->contr->socket, &sl, head);

            got_one++;

            /* It is possible for players to accumulate a huge amount of
             * items (especially with some of the bags out there) to
             * overflow the buffer.  IF so, send multiple item commands.
             */
            if (SockList_Avail(&sl) < MAXITEMLEN) {
                Send_With_Handling(&pl->contr->socket, &sl);
                SockList_Reset(&sl);
                SockList_AddString(&sl, "item2 ");
                SockList_AddInt(&sl, op->count);
                got_one = 0;
            }
        } /* If LOOK_OBJ() */
    } FOR_INV_FINISH();
    if (got_one) {
        /* special case: only one item, the "prev group" arrow */
        if (pl != op && pl->contr->socket.container_position) {
            if (got_one > 1)
                Send_With_Handling(&pl->contr->socket, &sl);
            else {
                /* view shifted, get to previous page and resend */
                pl->contr->socket.container_position = MAX(0, pl->contr->socket.container_position - prev_len);
                esrv_send_inventory(pl, op);
            }
        } else
            Send_With_Handling(&pl->contr->socket, &sl);
    }
    SockList_Term(&sl);
}

/**
 * Updates object *op for player *pl.
 *
 * flags is a list of values to update
 * to the client (as defined in newclient.h - might as well use the
 * same value both places.
 */

void esrv_update_item(int flags, object *pl, object *op) {
    SockList sl;

    if (!pl->contr)
        return;

    /* If we have a request to send the player item, skip a few checks. */
    if (op != pl) {
        if (!LOOK_OBJ(op))
            return;
        /* we remove the check for op->env, because in theory, the object
         * is hopefully in the same place, so the client should preserve
         * order.
         */
    }
    if (!QUERY_FLAG(op, FLAG_CLIENT_SENT)) {
        /* FLAG_CLIENT_SENT is debug only.  We are using it to see where
         * this is happening - we can set a breakpoint here in the debugger
         * and track back the call.
         */
        LOG(llevDebug, "We have not sent item %s (%d)\n", op->name, op->count);
    }

    SockList_Init(&sl);
    SockList_AddString(&sl, "upditem ");
    SockList_AddChar(&sl, (char)flags);

    op = HEAD(op);
    SockList_AddInt(&sl, op->count);

    if (flags&UPD_LOCATION)
        SockList_AddInt(&sl, op->env ? op->env->count : 0);

    if (flags&UPD_FLAGS)
        SockList_AddInt(&sl, query_flags(op));

    if (flags&UPD_WEIGHT) {
        sint32 weight = WEIGHT(op);

        /* TRANSPORTS are odd - they sort of look like containers,
         * yet can't be picked up.  So we don't to send the weight,
         * as it is odd that you see weight sometimes and not other
         * (the draw_look won't send it for example.
         */
        SockList_AddInt(&sl, QUERY_FLAG(op, FLAG_NO_PICK) ? -1 : weight);
        if (pl == op) {
            op->contr->last_weight = weight;
        }
    }

    if (flags&UPD_FACE) {
        if (!(pl->contr->socket.faces_sent[op->face->number]&NS_FACESENT_FACE))
            esrv_send_face(&pl->contr->socket, op->face->number, 0);
        SockList_AddInt(&sl, op->face->number);
    }
    if (flags&UPD_NAME) {
        int len;
        char item_p[MAX_BUF];
        char item_n[MAX_BUF];

        if (!op->custom_name) {
            query_base_name(op, 0, item_n, sizeof(item_n)-1);
            query_base_name(op, 1, item_p, sizeof(item_p));
        } else {
            snprintf(item_n, sizeof(item_n)-1, "%s", op->custom_name);
            snprintf(item_p, sizeof(item_p), "%s", op->custom_name);
        }

        len = strlen(item_n)+1;
        snprintf(item_n+len, sizeof(item_n)-len, "%s", item_p);
        len += strlen(item_n+len);
        SockList_AddLen8Data(&sl, item_n, len);
    }
    if (flags&UPD_ANIM)
        SockList_AddShort(&sl, op->animation_id);

    if (flags&UPD_ANIMSPEED) {
        int anim_speed = 0;

        if (QUERY_FLAG(op, FLAG_ANIMATE)) {
            if (op->anim_speed)
                anim_speed = op->anim_speed;
            else {
                if (FABS(op->speed) < 0.001)
                    anim_speed = 255;
                else if (FABS(op->speed) >= 1.0)
                    anim_speed = 1;
                else
                    anim_speed = (int)(1.0/FABS(op->speed));
            }
            if (anim_speed > 255)
                anim_speed = 255;
        }
        SockList_AddChar(&sl, (char)anim_speed);
    }
    if (flags&UPD_NROF)
        SockList_AddInt(&sl, op->nrof);

    Send_With_Handling(&pl->contr->socket, &sl);
    SockList_Term(&sl);
}

/**
 * Sends item's info to player.
 */
void esrv_send_item(object *pl, object*op) {
    SockList sl;

    /* If this is not the player object, do some more checks */
    if (op != pl) {
        /* We only send 'visibile' objects to the client */
        if (!LOOK_OBJ(op))
            return;
        /* if the item is on the ground, mark that the look needs to
         * be updated.
         */
        if (!op->env) {
            pl->contr->socket.update_look = 1;
            return;
        }
    }

    SockList_Init(&sl);
    SockList_AddString(&sl, "item2 ");

    op = HEAD(op);
    SockList_AddInt(&sl, op->env ? op->env->count : 0);

    add_object_to_socklist(&pl->contr->socket, &sl, op);

    Send_With_Handling(&pl->contr->socket, &sl);
    SET_FLAG(op, FLAG_CLIENT_SENT);
    SockList_Term(&sl);

    /* if the object is in an opened container, then it may shift the contents,
     * so resend everything */
    if (pl->contr != NULL && pl->container != NULL && op->env == pl->container)
        pl->contr->socket.update_inventory = 1;
}

/**
 * Tells the client to delete an item.  Uses the item
 * command with a -1 location.
 */

void esrv_del_item(player *pl, object *ob) {
    SockList sl;

    SockList_Init(&sl);
    SockList_AddString(&sl, "delitem ");
    SockList_AddInt(&sl, ob->count);
    Send_With_Handling(&pl->socket, &sl);
    SockList_Term(&sl);
    /* if the object is in an opened container, then it may shift the contents,
     * so resend everything */
    if (pl->ob->container != NULL && ob->env == pl->ob->container)
        pl->socket.update_inventory = 1;
}

/**************************************************************************
 *
 * Client has requested us to do something with an object.
 *
 **************************************************************************
 */

/**
 * Takes a player and object count (tag) and returns the actual object
 * pointer, or null if it can't be found.
 */

static object *esrv_get_ob_from_count(object *pl, tag_t count) {
    if (pl->count == count)
        return pl;

    FOR_INV_PREPARE(pl, op)
        if (op->count == count)
            return op;
        else if (op->type == CONTAINER && pl->container == op) {
            FOR_INV_PREPARE(op, tmp)
                if (tmp->count == count)
                    return tmp;
            FOR_INV_FINISH();
        }
    FOR_INV_FINISH();

    FOR_MAP_PREPARE(pl->map, pl->x, pl->y, op)
        if (HEAD(op)->count == count)
            return op;
        else if (op->type == CONTAINER && pl->container == op) {
            FOR_INV_PREPARE(op, tmp)
                if (tmp->count == count)
                    return tmp;
            FOR_INV_FINISH();
        }
    FOR_MAP_FINISH();

    if (pl->contr->transport) {
        FOR_INV_PREPARE(pl->contr->transport, tmp)
            if (tmp->count == count)
                return tmp;
        FOR_INV_FINISH();
    }
    return NULL;
}

/** Client wants to examine some object.  So lets do so. */
void examine_cmd(char *buf, int len, player *pl) {
    long tag;
    object *op;

    if (len <= 0 || !buf) {
        LOG(llevDebug, "Player '%s' sent bogus examine_cmd information\n", pl->ob->name);
        return;
    }

    tag = atoi(buf);
    op = esrv_get_ob_from_count(pl->ob, tag);
    if (!op) {
        LOG(llevDebug, "Player '%s' tried to examine the unknown object (%ld)\n", pl->ob->name, tag);
        return;
    }
    examine(pl->ob, op);
}

/** Client wants to apply some object.  Lets do so. */
void apply_cmd(char *buf, int len, player *pl) {
    uint32 tag;
    object *op;

    if (!buf || len <= 0) {
        LOG(llevDebug, "Player '%s' sent bogus apply_cmd information\n", pl->ob->name);
        return;
    }

    tag = atoi(buf);
    op = esrv_get_ob_from_count(pl->ob, tag);

    /* sort of a hack, but if the player saves and the player then
     * manually applies a savebed (or otherwise tries to do stuff),
     * we run into trouble.
     */
    if (QUERY_FLAG(pl->ob, FLAG_REMOVED))
        return;

    /* If the high bit is set, player applied a pseudo object. */
    if (tag&0x80000000) {
        if (pl->ob->container != NULL) {
            pl->socket.container_position = tag&0x7fffffff;
            esrv_send_inventory(pl->ob, pl->ob->container);
            pl->socket.update_inventory = 0;
        } else {
            pl->socket.look_position = tag&0x7fffffff;
            pl->socket.update_look = 1;
        }
        return;
    }

    if (!op) {
        LOG(llevDebug, "Player '%s' tried to apply the unknown object (%d)\n", pl->ob->name, tag);
        return;
    }
    apply_by_living(pl->ob, op, 0, 0);
}

/** Client wants to apply some object.  Lets do so. */
void lock_item_cmd(uint8 *data, int len, player *pl) {
    int flag, tag;
    object *op;
    object *tmp;

    if (len != 5) {
        LOG(llevDebug, "Player '%s' sent bogus lock_item_cmd information\n", pl->ob->name);
        return;
    }
    flag = data[0];
    tag = GetInt_String(data+1);
    op = esrv_get_ob_from_count(pl->ob, tag);

    if (!op) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Could not find object to lock/unlock");
        return;
    }

    if (op->map) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
            "Can't lock/unlock an item on the ground");
        return;
    }
    if (op->env != pl->ob) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
            "Can't lock/unlock an item not directly in your inventory");
        return;
    }

    if (!flag)
        CLEAR_FLAG(op, FLAG_INV_LOCKED);
    else
        SET_FLAG(op, FLAG_INV_LOCKED);

    tmp = object_merge(op, NULL);
    if (tmp == NULL) {
        /* object was not merged - if it was, object_merge() sent updates for us. */
        esrv_update_item(UPD_FLAGS, pl->ob, op);
    }
}

/**
 * Client wants to mark some object.  Lets do so.
 *
 * @param data
 * object tag (4 chars).
 * @param len
 * data size.
 * @param pl
 * player.
 */
void mark_item_cmd(uint8 *data, int len, player *pl) {
    int tag;
    object *op;
    char name[MAX_BUF];

    if (len != 4) {
        LOG(llevDebug, "Player '%s' sent bogus mark_item_cmd information\n", pl->ob->name);
        return;
    }

    tag = GetInt_String(data);
    op = esrv_get_ob_from_count(pl->ob, tag);
    if (!op) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_ERROR,
                      "Could not find object to mark");
        return;
    }
    pl->mark = op;
    pl->mark_count = op->count;
    query_name(op, name, MAX_BUF);
    draw_ext_info_format(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                         "Marked item %s",
                         name);
}

/**
 * Prints items on the specified square.
 *
 * [ removed EARTHWALL check and added check for containers inventory.
 *   Tero.Haatanen@lut.fi ]
 */
void look_at(object *op, int dx, int dy) {
    object *tmp;
    int flag = 0;
    sint16 x, y;
    mapstruct *m;
    char name[MAX_BUF];

    if (out_of_map(op->map, op->x+dx, op->y+dy))
        return;

    x = op->x+dx;
    y = op->y+dy;

    m = get_map_from_coord(op->map, &x, &y);
    if (!m)
        return;

    for (tmp = GET_MAP_OB(m, x, y); tmp != NULL && tmp->above != NULL; tmp = tmp->above)
        ;

    FOR_OB_AND_BELOW_PREPARE(tmp) {
        if (tmp->invisible && !QUERY_FLAG(op, FLAG_WIZ))
            continue;

        if (!flag) {
            if (dx || dy)
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                              "There you see:");
            else {
                draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                              "You see:");
            }
            flag = 1;
        }

        query_name(tmp, name, MAX_BUF);
        if (QUERY_FLAG(op, FLAG_WIZ))
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_EXAMINE,
                                 "- %s (%d).",
                                 name, tmp->count);
        else
            draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_EXAMINE,
                                 "- %s.",
                                 name);

        if ((HEAD(tmp)->inv != NULL && (tmp->type != CONTAINER && tmp->type != FLESH))
        || QUERY_FLAG(op, FLAG_WIZ))
            inventory(op, HEAD(tmp));

        /* don't continue under the floor */
        if (QUERY_FLAG(tmp, FLAG_IS_FLOOR) && !QUERY_FLAG(op, FLAG_WIZ))
            break;
    } FOR_OB_AND_BELOW_FINISH();

    if (!flag) {
        if (dx || dy)
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                          "You see nothing there.");
        else
            draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_COMMAND, MSG_TYPE_COMMAND_SUCCESS,
                          "You see nothing.");
    }
}

/** Client wants to look at some object.  Lets do so. */
void look_at_cmd(char *buf, int len, player *pl) {
    int dx, dy;
    char *cp;

    dx = atoi(buf);
    if (!(cp = strchr(buf, ' '))) {
        return;
    }
    dy = atoi(cp);

    if (FABS(dx) > MAP_CLIENT_X/2 || FABS(dy) > MAP_CLIENT_Y/2)
        return;

    if (pl->blocked_los[dx+(pl->socket.mapx/2)][dy+(pl->socket.mapy/2)])
        return;
    look_at(pl->ob, dx, dy);
}

/** Move an object to a new location */
void esrv_move_object(object *pl, tag_t to, tag_t tag, long nrof) {
    object *op, *env;

    op = esrv_get_ob_from_count(pl, tag);
    if (!op) {
        LOG(llevDebug, "Player '%s' tried to move an unknown object (%lu)\n", pl->name, (unsigned long)tag);
        return;
    }

    /* If on a transport, you don't drop to the ground - you drop to the
     * transport.
     */
    if (!to && !pl->contr->transport) { /* drop it to the ground */
        /*  LOG(llevDebug, "Drop it on the ground.\n");*/

        if (op->map && !op->env) {
/*          LOG(llevDebug, "Dropping object to ground that is already on ground\n");*/
            return;
        }
        /* If it is an active container, then we should drop all objects
         * in the container and not the container itself.
         */
        if (op->inv && QUERY_FLAG(op, FLAG_APPLIED)) {
            FOR_INV_PREPARE(op, current)
                drop_object(pl, current, 0);
            FOR_INV_FINISH();
            esrv_update_item(UPD_WEIGHT, pl, op);
        } else {
            drop_object(pl, op, nrof);
        }
        return;
    } else if (to == pl->count) {     /* pick it up to the inventory */
        /* return if player has already picked it up */
        if (op->env == pl)
            return;

        pl->contr->count = nrof;
        pick_up(pl, op);
        return;
    }
    /* If not dropped or picked up, we are putting it into a sack */
    if (pl->contr->transport) {
        if (object_can_pick(pl, op)
        && transport_can_hold(pl->contr->transport, op, nrof)) {
            put_object_in_sack(pl, pl->contr->transport, op, nrof);
        }
    } else {
        env = esrv_get_ob_from_count(pl, to);
        if (!env) {
            LOG(llevDebug, "Player '%s' tried to move object to the unknown location (%d)\n", pl->name, to);
            return;
        }
        /* put_object_in_sack presumes that necessary sanity checking
         * has already been done (eg, it can be picked up and fits in
         * in a sack, so check for those things.  We should also check
         * an make sure env is in fact a container for that matter.
         */
        if (env->type == CONTAINER
        && object_can_pick(pl, op)
        && sack_can_hold(pl, env, op, nrof)) {
            put_object_in_sack(pl, env, op, nrof);
        }
    }
}

void inscribe_scroll_cmd(char *buf, int len, player *pl) {
    object *scroll, *spell, *marked, *inscription, *currentspell;
    tag_t tscroll, tspell, tmarked;
    char type;

    if (len < 1) {
        LOG(llevDebug, "Player %s sent an invalid inscribe command.\n", pl->ob->name);
        return;
    }

    type = buf[0];

    inscription = find_skill_by_name(pl->ob, "inscription");
    if (!inscription) {
        draw_ext_info(NDI_UNIQUE, 0, pl->ob, MSG_TYPE_SKILL, MSG_TYPE_SKILL_FAILURE, "You don't know how to write!");
        return;
    }

    if (type == 0) {
        if (len != 9) {
            LOG(llevDebug, "Player %s sent an invalid inscribe command.\n", pl->ob->name);
            return;
        }
        tscroll = GetInt_String((uint8 *)buf+1);
        tspell = GetInt_String((uint8 *)buf+5);

        scroll = esrv_get_ob_from_count(pl->ob, tscroll);
        if (!scroll) {
            LOG(llevDebug, "Player %s sent an invalid scroll for inscribe command.\n", pl->ob->name);
            return;
        }

        spell = esrv_get_ob_from_count(pl->ob, tspell);
        if (!spell) {
            LOG(llevDebug, "Player %s sent an invalid spell for inscribe command.\n", pl->ob->name);
            return;
        }

        tmarked = pl->mark_count;
        marked = pl->mark;
        currentspell = pl->ranges[range_magic];

        pl->mark_count = tscroll;
        pl->mark = scroll;
        pl->ranges[range_magic] = spell;

        write_on_item(pl->ob, "", inscription);

        pl->mark_count = tmarked;
        pl->mark = marked;
        pl->ranges[range_magic] = currentspell;
    } else {
    }
}
