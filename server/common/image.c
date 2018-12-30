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
 * Handles face-related stuff, including the actual face data.
 */

#include <assert.h>
#include <global.h>
#include <stdio.h>

#include "image.h"

/**
 * Contains face information, with names, numbers, magicmap color and such.
 * It is sorted by alphabetical order.
 */
New_Face *new_faces;

/**
 * Following can just as easily be pointers, but
 * it is easier to keep them like this.
 */
New_Face *blank_face, *empty_face, *smooth_face;


/**
 * Number of bitmaps loaded from the "bmaps" file.
 */
unsigned int nrofpixmaps = 0;

face_sets facesets[MAX_FACE_SETS];    /**< All facesets */

/**
 * The only thing this table is used for now is to
 * translate the colorname in the magicmap field of the
 * face into a numeric index that is then sent to the
 * client for magic map commands.  The order of this table
 * must match that of the NDI colors in include/newclient.h.
 */
static const char *const colorname[] = {
    "black",        /* 0  */
    "white",        /* 1  */
    "blue",         /* 2  */
    "red",          /* 3  */
    "orange",       /* 4  */
    "light_blue",   /* 5  */
    "dark_orange",  /* 6  */
    "green",        /* 7  */
    "light_green",  /* 8  */
    "grey",         /* 9  */
    "brown",        /* 10 */
    "yellow",       /* 11 */
    "khaki"         /* 12 */
};

/**
 * Used for bsearch searching for faces by name.
 * The face "bug.111" is always put at first.
 * @todo "bug.111" should be a regular face, alas many places consider face 0
 * to be that face. This should be fixed at some point.
 * @param a first item to compare.
 * @param b second item to compare.
 * @retval -1 if a < b
 * @retval 0 if a == b
 * @retval 1 if a > b
 */
static int compare_face(const New_Face *a, const New_Face *b) {
    if (strcmp(a->name, "bug.111") == 0) {
        if (strcmp(b->name, "bug.111") == 0)
            return 0;
        return -1;
    } else if (strcmp(b->name, "bug.111") == 0)
        return 1;
    return strcmp(a->name, b->name);
}

/**
 * Finds a color by name.
 *
 * @param name
 * color name, case-sensitive.
 * @return
 * the matching color in the coloralias if found,
 * 0 otherwise.
 *
 * @note
 * 0 will actually be black, so there is no
 * way the calling function can tell if an error occurred or not
 */
static uint8 find_color(const char *name) {
    uint8 i;

    for (i = 0; i < sizeof(colorname)/sizeof(*colorname); i++)
        if (!strcmp(name, colorname[i]))
            return i;

    LOG(llevError, "Unknown color: %s\n", name);
    return 0;
}

/**
 * This reads the lib/faces file, getting color and visibility information.
 * It is called by read_bmap_names().
 *
 * @note
 * will call exit() if file doesn't exist.
 */
static void read_face_data(void) {
    char buf[MAX_BUF], *cp;
    New_Face *on_face = NULL;
    FILE *fp;

    snprintf(buf, sizeof(buf), "%s/faces", settings.datadir);
    LOG(llevDebug, "Reading faces from %s...\n", buf);
    if ((fp = fopen(buf, "r")) == NULL) {
        LOG(llevError, "Cannot open faces file: %s\n", strerror_local(errno, buf, sizeof(buf)));
        exit(-1);
    }

    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;
        if (!strncmp(buf, "end", 3)) {
            on_face = NULL;
        } else if (!strncmp(buf, "face", 4)) {
            unsigned tmp;

            cp = buf+5;
            cp[strlen(cp)-1] = '\0'; /* remove newline */

            if ((tmp = find_face(cp, (unsigned)-1)) == (unsigned)-1) {
                LOG(llevError, "Could not find face %s\n", cp);
                on_face = NULL;
                continue;
            }
            on_face = &new_faces[tmp];
            on_face->visibility = 0;
        } else if (on_face == NULL) {
            LOG(llevError, "Got line with no face set: %s\n", buf);
        } else if (!strncmp(buf, "visibility", 10)) {
            on_face->visibility = atoi(buf+11);
        } else if (!strncmp(buf, "magicmap", 8)) {
            cp = buf+9;
            cp[strlen(cp)-1] = '\0';
            on_face->magicmap = find_color(cp);
        } else if (!strncmp(buf, "is_floor", 8)) {
            int value = atoi(buf+9);
            if (value)
                on_face->magicmap |= FACE_FLOOR;
        } else
            LOG(llevDebug, "Got unknown line in faces file: %s\n", buf);
    }
    LOG(llevDebug, "done\n");
    fclose(fp);
}

/**
 * This reads the bmaps file to get all the bitmap names and
 * stuff.  It only needs to be done once, because it is player
 * independent (ie, what display the person is on will not make a
 * difference.)
 *
 * @note
 * will call exit() if file doesn't exist, and abort() in case of memory error.
 */
void read_bmap_names(void) {
    char buf[MAX_BUF], *p;
    FILE *fp;
    unsigned int i;
    size_t l;

    bmaps_checksum = 0;
    snprintf(buf, sizeof(buf), "%s/bmaps.paths", settings.datadir);
    LOG(llevDebug, "Reading bmaps from %s...\n", buf);
    if ((fp = fopen(buf, "r")) == NULL) {
        LOG(llevError, "Cannot open bmaps.paths file: %s\n", strerror_local(errno, buf, sizeof(buf)));
        exit(-1);
    }

    nrofpixmaps = 0;

    /* First count how many bitmaps we have, so we can allocate correctly */
    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (buf[0] != '#' && buf[0] != '\n') {
            nrofpixmaps++;
        }
    }

    rewind(fp);
    assert(nrofpixmaps > 0);
    new_faces = (New_Face *)malloc(sizeof(New_Face)*nrofpixmaps);

    if (new_faces == NULL) {
        LOG(llevError, "read_bmap_names: new_faces memory allocation failure.\n");
        abort();
    }

    for (i = 0; i < nrofpixmaps; i++) {
        new_faces[i].name = NULL;
        new_faces[i].visibility = 0;
        new_faces[i].magicmap = 255;
        new_faces[i].smoothface = (uint16)-1;
    }

    i = 0;
    while (i < nrofpixmaps && fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;

        p = strrchr(buf, '/');
        if ((p == NULL) || (strtok(p, " \t\n") == NULL)) {
            LOG(llevError, "read_bmap_names: syntax error: %s\n", buf);
            fatal(SEE_LAST_ERROR);
        }
        /* strtok converted the final newline or tab to NULL so all is ok */
        new_faces[i].name = strdup_local(p + 1);

        /* We need to calculate the checksum of the bmaps file
         * name->number mapping to send to the client.  This does not
         * need to match what sum or other utility may come up with -
         * as long as we get the same results on the same real file
         * data, it does the job as it lets the client know if
         * the file has the same data or not.
         */
        ROTATE_RIGHT(bmaps_checksum);
        bmaps_checksum += i&0xff;
        bmaps_checksum &= 0xffffffff;

        ROTATE_RIGHT(bmaps_checksum);
        bmaps_checksum += (i>>8)&0xff;
        bmaps_checksum &= 0xffffffff;
        for (l = 0; l < strlen(p); l++) {
            ROTATE_RIGHT(bmaps_checksum);
            bmaps_checksum += p[l];
            bmaps_checksum &= 0xffffffff;
        }

        i++;
    }
    fclose(fp);

    if (i != nrofpixmaps) {
        LOG(llevError, "read_bmap_names: first read gave %d faces but only loaded %d??\n", nrofpixmaps, i);
        fatal(SEE_LAST_ERROR);
    }

    LOG(llevDebug, "done (got %d faces)\n", nrofpixmaps);

    qsort(new_faces, nrofpixmaps, sizeof(New_Face), (int (*)(const void *, const void *))compare_face);

    for (i = 0; i < nrofpixmaps; i++) {
        new_faces[i].number = i;
    }

    read_face_data();

    for (i = 0; i < nrofpixmaps; i++) {
        if (new_faces[i].magicmap == 255) {
            new_faces[i].magicmap = 0;
        }
    }
    /* Actually forcefully setting the colors here probably should not
     * be done - it could easily create confusion.
     */
    blank_face = &new_faces[find_face(BLANK_FACE_NAME, 0)];
    blank_face->magicmap = find_color("khaki")|FACE_FLOOR;

    empty_face = &new_faces[find_face(EMPTY_FACE_NAME, 0)];

    smooth_face = &new_faces[find_face(SMOOTH_FACE_NAME, 0)];
}

/**
 * This returns an the face number of face 'name'.  Number is constant
 * during an invocation, but not necessarily between versions (this
 * is because the faces are arranged in alphabetical order, so
 * if a face is removed or added, all faces after that will now
 * have a different number.
 *
 * @param name
 * face to search for
 * @param error
 * value to return if face was not found.
 *
 * @note
 * If a face is not found, then error is returned.  This can be useful if
 * you want some default face used, or can be set to negative
 * so that it will be known that the face could not be found
 * (needed in client, so that it will know to request that image
 * from the server)
 */
unsigned find_face(const char *name, unsigned error) {
    New_Face *bp, tmp;

    tmp.name = name;
    bp = (New_Face *)bsearch(&tmp, new_faces, nrofpixmaps, sizeof(New_Face), (int (*)(const void *, const void *))compare_face);

    return bp ? bp->number : error;
}

/**
 * Reads the smooth file to know how to smooth datas.
 * the smooth file if made of 2 elements lines.
 * lines starting with # are comment
 * the first element of line is face to smooth
 * the next element is the 16x2 faces picture
 * used for smoothing
 *
 * @note
 * will call exit() if file can't be opened.
 */
int read_smooth(void) {
    char buf[MAX_BUF], *p, *q;
    FILE *fp;
    unsigned regular, smoothed;
    int nrofsmooth = 0;

    snprintf(buf, sizeof(buf), "%s/smooth", settings.datadir);
    LOG(llevDebug, "Reading smooth from %s...\n", buf);
    if ((fp = fopen(buf, "r")) == NULL) {
        LOG(llevError, "Cannot open smooth file: %s\n", strerror_local(errno, buf, sizeof(buf)));
        exit(-1);
    }

    while (fgets(buf, MAX_BUF, fp) != NULL) {
        if (*buf == '#')
            continue;

        if ((p = strchr(buf, '\n')))
            *p = '\0';

        p = strchr(buf, ' ');
        if (!p)
            continue;

        *p = '\0';
        q = buf;
        regular = find_face(q, (unsigned)-1);
        if (regular == (unsigned)-1) {
            LOG(llevError, "invalid regular face: %s\n", q);
            continue;
        }
        q = p+1;
        smoothed = find_face(q, (unsigned)-1);
        if (smoothed == (unsigned)-1) {
            LOG(llevError, "invalid smoothed face: %s\n", q);
            continue;
        }

        new_faces[regular].smoothface = smoothed;

        nrofsmooth++;
    }
    fclose(fp);

    LOG(llevDebug, "done (got %d smooth entries)\n", nrofsmooth);
    return nrofsmooth;
}

/**
 * Find the smooth face for a given face.
 *
 * @param face the face to find the smoothing face for
 *
 * @param smoothed return value: set to smooth face
 *
 * @return 1=smooth face found, 0=no smooth face found
 */
int find_smooth(uint16 face, uint16 *smoothed) {
    (*smoothed) = 0;

    if (face < nrofpixmaps) {
        if (new_faces[face].smoothface == ((uint16)-1))
            return 0;

        (*smoothed) = new_faces[face].smoothface;
        return 1;
    }

    return 0;
}

/**
 * Deallocates memory allocated by read_bmap_names() and read_smooth().
 */
void free_all_images(void) {
    unsigned int i;

    for (i = 0; i < nrofpixmaps; i++)
        free((char*)(new_faces[i].name));
    free(new_faces);
}

/**
 * Checks fallback are correctly defined.
 * This is a simple recursive function that makes sure the fallbacks
 * are all proper (eg, the fall back to defined sets, and also
 * eventually fall back to 0).  At the top level, togo is set to
 * MAX_FACE_SETS.  If togo gets to zero, it means we have a loop.
 * This is only run when we first load the facesets.
 */
static void check_faceset_fallback(int faceset, int togo) {
    int fallback = facesets[faceset].fallback;

    /* proper case - falls back to base set */
    if (fallback == 0)
        return;

    if (!facesets[fallback].prefix) {
        LOG(llevError, "Face set %d falls to non set faceset %d\n", faceset, fallback);
        abort();
    }
    togo--;
    if (togo == 0) {
        LOG(llevError, "Infinite loop found in facesets. aborting.\n");
        abort();
    }
    check_faceset_fallback(fallback, togo);
}

/**
 * Loads all the image types into memory.
 *
 * This  way, we can easily send them to the client.  We should really
 * do something better than abort on any errors - on the other hand,
 * these are all fatal to the server (can't work around them), but the
 * abort just seems a bit messy (exit would probably be better.)
 *
 * Couple of notes:  We assume that the faces are in a continous block.
 * This works fine for now, but this could perhaps change in the future
 *
 * Function largely rewritten May 2000 to be more general purpose.
 * The server itself does not care what the image data is - to the server,
 * it is just data it needs to allocate.  As such, the code is written
 * to do such.
 */

void read_client_images(void) {
    char filename[400];
    char buf[HUGE_BUF];
    char *cp, *cps[7+1], *slash;
    FILE *infile;
    int len, fileno, i;
    unsigned int num;

    memset(facesets, 0, sizeof(facesets));
    snprintf(filename, sizeof(filename), "%s/image_info", settings.datadir);
    if ((infile = fopen(filename, "r")) == NULL) {
        LOG(llevError, "Unable to open %s\n", filename);
        abort();
    }
    while (fgets(buf, HUGE_BUF-1, infile) != NULL) {
        if (buf[0] == '#')
            continue;
        if (split_string(buf, cps, sizeof(cps)/sizeof(*cps), ':') != 7)
            LOG(llevError, "Bad line in image_info file, ignoring line:\n  %s", buf);
        else {
            len = atoi(cps[0]);
            if (len >= MAX_FACE_SETS) {
                LOG(llevError, "To high a setnum in image_info file: %d > %d\n", len, MAX_FACE_SETS);
                abort();
            }
            facesets[len].prefix = strdup_local(cps[1]);
            facesets[len].fullname = strdup_local(cps[2]);
            facesets[len].fallback = atoi(cps[3]);
            facesets[len].size = strdup_local(cps[4]);
            facesets[len].extension = strdup_local(cps[5]);
            facesets[len].comment = strdup_local(cps[6]);
        }
    }
    fclose(infile);
    for (i = 0; i < MAX_FACE_SETS; i++) {
        if (facesets[i].prefix)
            check_faceset_fallback(i, MAX_FACE_SETS);
    }
    /* Loaded the faceset information - now need to load up the
     * actual faces.
     */

    for (fileno = 0; fileno < MAX_FACE_SETS; fileno++) {
        /* if prefix is not set, this is not used */
        if (!facesets[fileno].prefix)
            continue;
        facesets[fileno].faces = calloc(nrofpixmaps, sizeof(face_info));

        snprintf(filename, sizeof(filename), "%s/crossfire.%d", settings.datadir, fileno);
        LOG(llevDebug, "Loading image file %s\n", filename);

        if ((infile = fopen(filename, "rb")) == NULL) {
            LOG(llevError, "Unable to open %s\n", filename);
            abort();
        }
        while (fgets(buf, HUGE_BUF-1, infile) != NULL) {
            if (strncmp(buf, "IMAGE ", 6) != 0) {
                LOG(llevError, "read_client_images:Bad image line - not IMAGE, instead\n%s", buf);
                abort();
            }
            cp = buf + 6;
            len = atoi(cp);
            if (len == 0 || len > MAX_IMAGE_SIZE) {
                LOG(llevError, "read_client_images: length not valid: %d > %d \n%s", len, MAX_IMAGE_SIZE, buf);
                abort();
            }

            for ( ; *cp != ' ' && *cp != '\n' && *cp != '\0'; cp++)
                ;
            if (*cp != ' ') {
                LOG(llevError, "read_client_images: couldn't find name start for %d\n", num);
                abort();
            }
            cp++;
            /* cp points to the start of the full name */
            slash = strrchr(cp, '/');
            if (slash != NULL)
                cp = slash + 1;
            if (cp[strlen(cp) - 1] == '\n')
                cp[strlen(cp) - 1] = '\0';

            /* cp points to the start of the picture name itself */
            num = find_face(cp, (unsigned)-1);
            if (num == (unsigned)-1) {
                LOG(llevError, "read_client_images: couldn't find picture %s\n", cp);
                abort();
            }
            if (num >= nrofpixmaps) {
                LOG(llevError, "read_client_images: invalid picture number %d for %s\n", num, cp);
                abort();
            }

            facesets[fileno].faces[num].datalen = len;
            facesets[fileno].faces[num].data = malloc(len);
            if ((i = fread(facesets[fileno].faces[num].data, len, 1, infile)) != 1) {
                LOG(llevError, "read_client_images: Did not read desired amount of data, wanted %d, got %d\n%s", len, i, buf);
                abort();
            }
            facesets[fileno].faces[num].checksum = 0;
            for (i = 0; i < len; i++) {
                ROTATE_RIGHT(facesets[fileno].faces[num].checksum);
                facesets[fileno].faces[num].checksum += facesets[fileno].faces[num].data[i];
                facesets[fileno].faces[num].checksum &= 0xffffffff;
            }
        }
        fclose(infile);
    } /* For fileno < MAX_FACE_SETS */
}

/**
 * Checks specified faceset is valid
 * \param fsn faceset number
 */
int is_valid_faceset(int fsn) {
    if (fsn >= 0 && fsn < MAX_FACE_SETS && facesets[fsn].prefix)
        return TRUE;
    return FALSE;
}

/**
 * Frees all faceset information
 */
void free_socket_images(void) {
    int num;
    unsigned int q;

    for (num = 0; num < MAX_FACE_SETS; num++) {
        if (facesets[num].prefix) {
            for (q = 0; q < nrofpixmaps; q++)
                free(facesets[num].faces[q].data);
            free(facesets[num].prefix);
            free(facesets[num].fullname);
            free(facesets[num].size);
            free(facesets[num].extension);
            free(facesets[num].comment);
            free(facesets[num].faces);
        }
    }
}

/**
 * This returns the set we will actually use when sending
 * a face.  This is used because the image files may be sparse.
 * This function is recursive.  imageno is the face number we are
 * trying to send
 *
 * If face is not found in specified faceset, tries with 'fallback' faceset.
 *
 * \param faceset faceset to check
 * \param imageno image number
 *
 */
int get_face_fallback(int faceset, int imageno) {
    /* faceset 0 is supposed to have every image, so just return.  Doing
     * so also prevents infinite loops in the case if it not having
     * the face, but in that case, we are likely to crash when we try
     * to access the data, but that is probably preferable to an infinite
     * loop.
     */
    if (faceset == 0)
        return 0;

    if (!facesets[faceset].prefix) {
        LOG(llevError, "get_face_fallback called with unused set (%d)?\n", faceset);
        return 0;   /* use default set */
    }
    if (facesets[faceset].faces[imageno].data)
        return faceset;
    return get_face_fallback(facesets[faceset].fallback, imageno);
}
