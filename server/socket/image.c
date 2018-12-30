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

/** \file
 * Image related communication
 *
 *  \date 2006-03-18
 *
 * This file deals with the image related communication to the
 * client.  I've located all the functions in this file - this
 * localizes it more, and means that we don't need to declare
 * things like all the structures as globals.
 */

#include <global.h>
#include <sproto.h>

#include <shared/newclient.h>
#include <newserver.h>
#include <image.h>

/**
 * Client has requested pixmap that it somehow missed getting.
 * This will be called often if the client is
 * caching images.
 * @param buff data received from the client.
 * @param len length of buff, ignored.
 * @param ns socket to send data to.
 */
void send_face_cmd(char *buff, int len, socket_struct *ns) {
    long tmpnum = atoi(buff);
    uint16 facenum = tmpnum&0xffff;

    if (facenum != 0)
        esrv_send_face(ns, facenum, 1);
}

/**
 * Sends a face to a client if they are in pixmap mode,
 * nothing gets sent in bitmap mode.
 * If nocache is true (nonzero), ignore the cache setting from the client -
 * this is needed for the askface, in which we really do want to send the
 * face (and askface is the only place that should be setting it).  Otherwise,
 * we look at the facecache, and if set, send the image name.
 * @param ns socket to send the date to.
 * @param face_num face number to send.
 * @param nocache if 1 then send a 'image2', else depending on client cache setting.
 */
void esrv_send_face(socket_struct *ns, uint16 face_num, int nocache) {
    SockList sl;
    int fallback;

    if (face_num == 0 || face_num >= nrofpixmaps) {
        LOG(llevError, "esrv_send_face (%d) out of bounds??\n", face_num);
        return;
    }

    SockList_Init(&sl);
    fallback = get_face_fallback(ns->faceset, face_num);

    if (facesets[fallback].faces[face_num].data == NULL) {
        LOG(llevError, "esrv_send_face: faces[%d].data == NULL\n", face_num);
        return;
    }

    if (ns->facecache && !nocache) {
        SockList_AddString(&sl, "face2 ");
        SockList_AddShort(&sl, face_num);
        SockList_AddChar(&sl, fallback);
        SockList_AddInt(&sl, facesets[fallback].faces[face_num].checksum);
        SockList_AddString(&sl, new_faces[face_num].name);
        Send_With_Handling(ns, &sl);
    } else {
        SockList_AddString(&sl, "image2 ");
        SockList_AddInt(&sl, face_num);
        SockList_AddChar(&sl, fallback);
        SockList_AddInt(&sl, facesets[fallback].faces[face_num].datalen);
        SockList_AddData(&sl, facesets[fallback].faces[face_num].data, facesets[fallback].faces[face_num].datalen);
        Send_With_Handling(ns, &sl);
    }
    ns->faces_sent[face_num] |= NS_FACESENT_FACE;
    SockList_Term(&sl);
}

/**
 * Sends the number of images, checksum of the face file,
 * and the image_info file information.  See the doc/Developers/protocol
 * if you want further detail.
 * @param ns socket to send data to.
 */
void send_image_info(socket_struct *ns) {
    SockList sl;
    int i;

    SockList_Init(&sl);
    SockList_AddPrintf(&sl, "replyinfo image_info\n%d\n%d\n", nrofpixmaps-1, bmaps_checksum);
    for (i = 0; i < MAX_FACE_SETS; i++) {
        if (facesets[i].prefix) {
            SockList_AddPrintf(&sl, "%d:%s:%s:%d:%s:%s:%s",
                     i, facesets[i].prefix, facesets[i].fullname,
                     facesets[i].fallback, facesets[i].size,
                     facesets[i].extension, facesets[i].comment);
        }
    }
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}

/**
 * Sends requested face information.
 * \param ns socket to send to
 * \param params contains first and last index of face
 *
 * For each image in [start..stop] sends
 *  - checksum
 *  - name
 */
void send_image_sums(socket_struct *ns, char *params) {
    int start, stop;
    short i;
    char *cp;
    SockList sl;

    SockList_Init(&sl);

    start = atoi(params);
    for (cp = params; *cp != '\0'; cp++)
        if (*cp == ' ')
            break;

    stop = atoi(cp);
    if (stop < start
    || *cp == '\0'
    || (stop-start) > 1000
    || stop >= nrofpixmaps) {
        SockList_AddPrintf(&sl, "replyinfo image_sums %d %d", start, stop);
        Send_With_Handling(ns, &sl);
        SockList_Term(&sl);
        return;
    }
    SockList_AddPrintf(&sl, "replyinfo image_sums %d %d ", start, stop);

    for (i = start; i <= stop; i++) {
        int faceset;

        if (SockList_Avail(&sl) < 2+4+1+1+strlen(new_faces[i].name)+1) {
            LOG(llevError, "send_image_sums: buffer overflow, rejecting range %d..%d\n", start, stop);
            SockList_Reset(&sl);
            SockList_AddPrintf(&sl, "replyinfo image_sums %d %d", start, stop);
            Send_With_Handling(ns, &sl);
            SockList_Term(&sl);
            return;
        }

        SockList_AddShort(&sl, i);
        ns->faces_sent[i] |= NS_FACESENT_FACE;

        faceset = get_face_fallback(ns->faceset, i);
        SockList_AddInt(&sl, facesets[faceset].faces[i].checksum);
        SockList_AddChar(&sl, faceset);
        SockList_AddLen8Data(&sl, new_faces[i].name, strlen(new_faces[i].name)+1);
    }
    Send_With_Handling(ns, &sl);
    SockList_Term(&sl);
}
