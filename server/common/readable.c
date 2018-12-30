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
 * @file readable.c
 * This file contains code relevant to the BOOKS hack -- designed
 * to allow randomly occuring messages in non-magical texts.
 *
 * @todo
 * replace message type with defines/enums & such things.
 */

/* laid down initial file - dec 1995. -b.t. thomas@astro.psu.edu */

#include <stdio.h>
#include <global.h>
#include <book.h>
#include <living.h>
#include <spells.h>
#include <assert.h>

/* Define this if you want to archive book titles by contents.
 * This option should enforce UNIQUE combinations of titles,authors and
 * msg contents during and *between *game sessions.
 * Note: a slight degeneracy exists since books are archived based on an integer
 * index value calculated from the message text (similar to alchemy formulae).
 * Sometimes two widely different messages have the same index value (rare). In
 * this case,  it is possible to occasionally generate 2 books with same title and
 * different message content. Not really a bug, but rather a feature. This action
 * should  keeps player on their toes ;).
 * Also, note that there is *finite *space available for archiving message and titles.
 * Once this space is used, books will stop being archived. Not a serious problem
 * under the current regime, since there are generally fewer possible (random)
 * messages than space available on the titlelists.
 * One exception (for sure) are the monster messages. But no worries, you should
 * see all of the monster info in some order (but not all possble combinations)
 * before the monster titlelist space is run out. You can increase titlelist
 * space by increasing the array sizes for the monster book_authours and book_names
 * (see  max_titles[] array and include/read.h). Since the unique_book algorthm is
 * kinda stupid, this program *may *slow down program execution if defined (but I don't
 * think its a significant problem, at least, I have no problems running this option
 * on a Sparc 10! Also, once archive title lists are filled and/or all possible msg
 * combinations have been generated, unique_book isnt called anymore. It takes 5-10
 * sessions for this to happen).
 * Final note: the game remembers book/title/msg combinations from reading the
 * file lib/bookarch. If you REMOVE this file, you will lose your archive. So
 * be sure to copy it over to the new lib directory when you change versions.
 * -b.t.
 */

/* This flag is useful to see what kind of output messages are created */
/* #define BOOK_MSG_DEBUG */

/* This flag is useful for debugging archiving action */
/* #define ARCHIVE_DEBUG */

/** How many times to try to generate a unique name for a book. */
#define MAX_TITLE_CHECK 20

/** Message from the lib/messages file. */
#define MSGTYPE_LIB 0
/** Monster-related information. */
#define MSGTYPE_MONSTER 1
/** Artifact-related information. */
#define MSGTYPE_ARTIFACT 2
/** Spellpath-related information. */
#define MSGTYPE_SPELLPATH 3
/** Alchemy-related information. */
#define MSGTYPE_ALCHEMY 4
/** God-related information. */
#define MSGTYPE_GODS 5
/** Message from the lib/messages file. */
#define MSGTYPE_MSGFILE 6

/**
 * Returns the element size of an array.
 * @param arrayname the array's name
 * @return the number of elements
 */
#define arraysize(arrayname) (sizeof(arrayname)/sizeof(*(arrayname)))

/* Moved these structures from struct.h to this file in 0.94.3 - they
 * are not needed anyplace else, so why have them globally declared?
 */

/**
 * Information on one title.
 * 'title' and 'titlelist' are used by the readable code
 */
typedef struct titlestruct {
    const char *name;      /**< the name of the book */
    const char *authour;   /**< the name of the book authour */
    const char *archname;  /**< the archetype name of the book */
    int level;             /**< level of difficulty of this message */
    size_t size;           /**< size of the book message */
    int msg_index;         /**< an index value derived from book message */
    struct titlestruct *next;   /**< next item in the list */
} title;

/**
 * Titles for one message type.
 */
typedef struct titleliststruct {
    int number;       /**< number of items in the list */
    struct titlestruct *first_book;     /**< pointer to first book in this list */
    struct titleliststruct *next;  /**< pointer to next book list */
} titlelist;

/** special structure, used only by art_name_array[] */
typedef struct namebytype {
    const char *name;  /**< generic name to call artifacts of this type */
    int type;          /**< matching type */
} arttypename;

/**
 * One general message, from the lib/messages file.
 */
struct GeneralMessage {
    int chance;             /**< Relative chance of the message appearing
                              randomly. If 0 will never appear. */
    sstring identifier;     /**< Message identifier, can be NULL. */
    sstring title;          /**< The message's title, only used for knowledge. */
    sstring message;        /**< The message's body. */
    sstring quest_code;     /**< Optional quest code and state this message will start. */
    GeneralMessage *next;   /**< Next message in the list. */
};

static void add_book(title *book, int type, const char *fname, int lineno);

/**
 * Buffer of books read in from the bookarch file. It's element
 * size does not exceed arraysize(max_titles).
 */
static titlelist *booklist = NULL;

/** Information on monsters. */
static objectlink *first_mon_info = NULL;

static int nrofmon = 0, /**< Number of monsters in the ::first_mon_info list. */
    need_to_write_bookarchive = 0; /**< If set then we have information to save. */

/**
 * First message from data read from the messages file.
 * Note that this points to the last message in the file,
 * as messages are added to the start of the list.
 */
static GeneralMessage *first_msg = NULL;

/**
 * Total chance of messages (GeneralMessage), to randomly select one.
 */
static int msg_total_chance = 0;

/**
 * Spellpath information.
 */
static const uint32 spellpathdef[NRSPELLPATHS] = {
    PATH_PROT,
    PATH_FIRE,
    PATH_FROST,
    PATH_ELEC,
    PATH_MISSILE,
    PATH_SELF,
    PATH_SUMMON,
    PATH_ABJURE,
    PATH_RESTORE,
    PATH_DETONATE,
    PATH_MIND,
    PATH_CREATE,
    PATH_TELE,
    PATH_INFO,
    PATH_TRANSMUTE,
    PATH_TRANSFER,
    PATH_TURNING,
    PATH_WOUNDING,
    PATH_DEATH,
    PATH_LIGHT
};

/** Book names for path information. */
static const char *const path_book_name[] = {
    "codex",
    "compendium",
    "exposition",
    "tables",
    "treatise"
};

/** Used by spellpath texts. */
static const char *const path_author[] = {
    "aether",
    "astral byways",
    "connections",
    "the Grey Council",
    "deep pathways",
    "knowledge",
    "magic",
    "mystic ways",
    "pathways",
    "power",
    "spells",
    "transforms",
    "the mystic veil",
    "unknown spells"
};

/**
 * Artiface/item information.
 *
 * if it isnt listed here, then art_attr_msg() will never generate
 * a message for this type of artifact. -b.t.
 */
static const arttypename art_name_array[] = {
    { "Helmet", HELMET },
    { "Amulet", AMULET },
    { "Shield", SHIELD },
    { "Bracers", BRACERS },
    { "Boots", BOOTS },
    { "Cloak", CLOAK },
    { "Gloves", GLOVES },
    { "Gridle", GIRDLE },
    { "Ring", RING },
    { "Horn", ROD },
    { "Missile Weapon", BOW },
    { "Missile", ARROW },
    { "Hand Weapon", WEAPON },
    { "Artifact", SKILL },
    { "Food", FOOD },
    { "Body Armour", ARMOUR }
};

/** Book titles for artifact information. */
static const char *const art_book_name[] = {
    "collection",
    "file",
    "files",
    "guide",
    "handbook",
    "index",
    "inventory",
    "list",
    "listing",
    "record",
    "record book"
};

/** Used by artifact texts */
static const char *const art_author[] = {
    "ancient things",
    "artifacts",
    "Havlor",   /* ancient warrior scribe :) */
    "items",
    "lost artifacts",
    "the ancients",
    "useful things"
};

/**
 * Monster book information.
 */
static const char *const mon_book_name[] = {
    "beastuary",
    "catalog",
    "compilation",
    "collection",
    "encyclopedia",
    "guide",
    "handbook",
    "list",
    "manual",
    "notes",
    "record",
    "register",
    "volume"
};

/** Used by monster beastuary texts. */
static const char *const mon_author[] = {
    "beasts",
    "creatures",
    "dezidens",
    "dwellers",
    "evil nature",
    "life",
    "monsters",
    "nature",
    "new life",
    "residents",
    "the spawn",
    "the living",
    "things"
};

/**
 * God book information.
 */
static const char *const gods_book_name[] = {
    "devotional",
    "devout notes",
    "divine text",
    "divine work",
    "holy book",
    "holy record",
    "moral text",
    "sacred guide",
    "testament",
    "transcript"
};

/** Used by gods texts. */
static const char *const gods_author[] = {
    "cults",
    "joy",
    "lasting curse",
    "madness",
    "religions",
    "the dead",
    "the gods",
    "the heirophant",
    "the poor priest",
    "the priestess",
    "pain",
    "white"
};

/**
 * Alchemy (formula) information.
 */
static const char *const formula_book_name[] = {
    "cookbook",
    "formulary",
    "lab book",
    "lab notes",
    "recipe book",
    "experiment record",
    "work plan",
    "design notes"
};

/** This isn't used except for empty books. */
static const char *const formula_author[] = {
    "Albertus Magnus",
    "alchemy",
    "balms",
    "creation",
    "dusts",
    "magical manufacture",
    "making",
    "philosophical items",
    "potions",
    "powders",
    "the cauldron",
    "the lamp black",
    "transmutation",
    "waters"
};

/**
 * Generic book information
 */

/** Used by msg file and 'generic' books. */
static const char *const light_book_name[] = {
    "calendar",
    "datebook",
    "diary",
    "guidebook",
    "handbook",
    "ledger",
    "notes",
    "notebook",
    "octavo",
    "pamphlet",
    "practicum",
    "script",
    "transcript"
};

/** Name for big books. */
static const char *const heavy_book_name[] = {
    "catalog",
    "compendium",
    "guide",
    "manual",
    "opus",
    "tome",
    "treatise",
    "volume",
    "work"
};

/** Used by 'generic' books. */
static const char *const book_author[] = {
    "Abdulah",
    "Al'hezred",
    "Alywn",
    "Arundel",
    "Arvind",
    "Aerlingas",
    "Bacon",
    "Baliqendii",
    "Bosworth",
    "Beathis",
    "Bertil",
    "Cauchy",
    "Chakrabarti",
    "der Waalis",
    "Dirk",
    "Djwimii",
    "Eisenstaadt",
    "Fendris",
    "Frank",
    "Habbi",
    "Harlod",
    "Ichibod",
    "Janus",
    "June",
    "Magnuson",
    "Nandii",
    "Nitfeder",
    "Norris",
    "Parael",
    "Penhew",
    "Sophia",
    "Skilly",
    "Tahir",
    "Thockmorton",
    "Thomas",
    "van Helsing",
    "van Pelt",
    "Voormis",
    "Xavier",
    "Xeno",
    "Zardoz",
    "Zagy"
};

/** Book descriptions. */
static const char *const book_descrpt[] = {
    "ancient",
    "cryptic",
    "cryptical",
    "dusty",
    "hiearchical",
    "grizzled",
    "gold-guilt",
    "great",
    "lost",
    "magnificent",
    "musty",
    "mythical",
    "mystical",
    "rustic",
    "stained",
    "silvered",
    "transcendental",
    "weathered"
};

/**
 * Each line of this array is a readable subtype.
 * Be careful to keep the order. If you add readable subtype, add them
 * at the bottom of the list. Never delete a subtype because index is used as
 * subtype parameter in arch files!
 */
static const readable_message_type readable_message_types[] = {
    /*subtype 0  */ { 0, 0 },
                    /* book messages subtypes */
    /*subtype 1  */ { MSG_TYPE_BOOK, MSG_TYPE_BOOK_CLASP_1 },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_CLASP_2 },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_ELEGANT_1 },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_ELEGANT_2 },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_QUARTO_1 },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_QUARTO_2 },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_SPELL_EVOKER },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_SPELL_PRAYER },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_SPELL_PYRO },
    /*subtype 10 */ { MSG_TYPE_BOOK, MSG_TYPE_BOOK_SPELL_SORCERER },
                    { MSG_TYPE_BOOK, MSG_TYPE_BOOK_SPELL_SUMMONER },
                    /* card messages subtypes*/
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_SIMPLE_1 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_SIMPLE_2 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_SIMPLE_3 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_ELEGANT_1 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_ELEGANT_2 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_ELEGANT_3 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_STRANGE_1 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_STRANGE_2 },
    /*subtype 20 */ { MSG_TYPE_CARD, MSG_TYPE_CARD_STRANGE_3 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_MONEY_1 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_MONEY_2 },
                    { MSG_TYPE_CARD, MSG_TYPE_CARD_MONEY_3 },
                    /* Paper messages subtypes */
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_NOTE_1 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_NOTE_2 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_NOTE_3 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_LETTER_OLD_1 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_LETTER_OLD_2 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_LETTER_NEW_1 },
    /*subtype 30 */ { MSG_TYPE_PAPER, MSG_TYPE_PAPER_LETTER_NEW_2 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_ENVELOPE_1 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_ENVELOPE_2 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_SCROLL_OLD_1 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_SCROLL_OLD_2 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_SCROLL_NEW_1 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_SCROLL_NEW_2 },
                    { MSG_TYPE_PAPER, MSG_TYPE_PAPER_SCROLL_MAGIC },
                    /* road signs messages subtypes */
                    { MSG_TYPE_SIGN, MSG_TYPE_SIGN_BASIC },
                    { MSG_TYPE_SIGN, MSG_TYPE_SIGN_DIR_LEFT },
    /*subtype 40 */ { MSG_TYPE_SIGN, MSG_TYPE_SIGN_DIR_RIGHT },
                    { MSG_TYPE_SIGN, MSG_TYPE_SIGN_DIR_BOTH },
                    /* stones and monument messages */
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_STONE_1 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_STONE_2 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_STONE_3 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_STATUE_1 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_STATUE_2 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_STATUE_3 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_GRAVESTONE_1 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_GRAVESTONE_2 },
    /*subtype 50 */ { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_GRAVESTONE_3 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_WALL_1 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_WALL_2 },
                    { MSG_TYPE_MONUMENT, MSG_TYPE_MONUMENT_WALL_3 },
                    { MSG_TYPE_SIGN, MSG_TYPE_SIGN_MAGIC_MOUTH }
};

/** Number of elements in ::readable_message_types. */
static const int last_readable_subtype = arraysize(readable_message_types);

/** Number of titles for different name lists. */
static const int max_titles[6] = {
    (arraysize(light_book_name)+arraysize(heavy_book_name))*arraysize(book_author), /* MSGTYPE_LIB */
    arraysize(mon_book_name)*arraysize(mon_author), /* MSGTYPE_MONSTER */
    arraysize(art_book_name)*arraysize(art_author), /* MSGTYPE_ARTIFACT */
    arraysize(path_book_name)*arraysize(path_author), /* MSGTYPE_SPELLPATH */
    arraysize(formula_book_name)*arraysize(formula_author), /* MSGTYPE_ALCHEMY */
    arraysize(gods_book_name)*arraysize(gods_author), /* MSGTYPE_GODS */
};

/******************************************************************************
 *
 * Start of misc. readable functions used by others functions in this file
 *
 *****************************************************************************/

/**
 * Creates a titlelist.
 *
 * @return
 * new titlelist.
 *
 * @note
 * if memory allocation failes, calls fatal().
 */
static titlelist *get_empty_booklist(void) {
    titlelist *bl = (titlelist *)malloc(sizeof(titlelist));

    if (bl == NULL)
        fatal(OUT_OF_MEMORY);
    bl->number = 0;
    bl->first_book = NULL;
    bl->next = NULL;
    return bl;
}

/**
 * Creates a title.
 *
 * @return
 * new title.
 *
 * @note
 * if memory allocation failes, calls fatal().
 */
static title *get_empty_book(void) {
    title *t = (title *)malloc(sizeof(title));

    if (t == NULL)
        fatal(OUT_OF_MEMORY);
    t->name = NULL;
    t->archname = NULL;
    t->authour = NULL;
    t->level = 0;
    t->size = 0;
    t->msg_index = 0;
    t->next = NULL;
    return t;
}

/**
 * Gets the ith titlelist.
 *
 * Will create items if they don't exist.
 *
 * @param i
 * index to get.
 * @return
 * pointer to the title list referenced by i. Will never be NULL.
 */
static titlelist *get_titlelist(int i) {
    titlelist *tl;
    int number;

    if (i < 0 || i >= (int)arraysize(max_titles)) {
        LOG(llevInfo, "Warning: invalid book index %d, using 0 instead\n", i);
        return booklist;
    }

    for (tl = booklist, number = i; tl && number; tl = tl->next, number--) {
        if (!tl->next)
            tl->next = get_empty_booklist();
    }

    return tl;
}

/* HANDMADE STRING FUNCTIONS.., perhaps these belong in another file
 * (shstr.c ?), but the quantity BOOK_BUF will need to be defined. */

/**
 * Simple routine to return the number of list
 * items in buf1 as separated by the value of buf2
 *
 * @param buf1
 * items we want to split.
 * @param buf2
 * what to split by.
 * @return
 * number of elements.
 */
int nstrtok(const char *buf1, const char *buf2) {
    char *tbuf, buf[MAX_BUF];
    int number = 0;

    if (!buf1 || !buf2)
        return 0;

    snprintf(buf, sizeof(buf), "%s", buf1);
    for (tbuf = strtok(buf, buf2); tbuf; tbuf = strtok(NULL, buf2)) {
        number++;
    }
    return number;
}

/**
 * Takes a string in buf1 and separates it into
 * a list of strings delimited by buf2. Then returns a comma
 * separated string w/ decent punctuation.
 *
 * @param buf1
 * buffer to split.
 * @param buf2
 * what to split buf1 by.
 * @param retbuf
 * where to write the resulting string.
 * @param size
 * length of retbuf.
 * @return
 * retbuf.
 */
char *strtoktolin(const char *buf1, const char *buf2, char *retbuf, size_t size) {
    int maxi, i = nstrtok(buf1, buf2);
    char *tbuf, buf[MAX_BUF];

    maxi = i;
    snprintf(buf, sizeof(buf), "%s", buf1);
    snprintf(retbuf, size, " ");
    for (tbuf = strtok(buf, buf2); tbuf && i > 0; tbuf = strtok(NULL, buf2)) {
        snprintf(retbuf+strlen(retbuf), size-strlen(retbuf), "%s", tbuf);
        i--;
        if (i == 1 && maxi > 1)
            snprintf(retbuf+strlen(retbuf), size-strlen(retbuf), " and ");
        else if (i > 0 && maxi > 1)
            snprintf(retbuf+strlen(retbuf), size-strlen(retbuf), ", ");
        else
            snprintf(retbuf+strlen(retbuf), size-strlen(retbuf), ".");
    }
    return retbuf;
}

/**
 * Checks if buf1 and buf2 can be combined.
 * @param buf1
 * @param buf2
 * buffer we plan on combining.
 * @param booksize
 * maximum book size.
 * @return
 * 0 if buffers can be combined, 1 else.
 */
int book_overflow(const char *buf1, const char *buf2, size_t booksize) {
    if (buf_overflow(buf1, buf2, BOOK_BUF-2)   /* 2 less so always room for trailing \n */
    || buf_overflow(buf1, buf2, booksize))
        return 1;
    return 0;
}

/*****************************************************************************
 *
 * Start of initialization related functions.
 *
 ****************************************************************************/

/**
 * If not called before, initialize the info list.
 *
 * Reads the messages file into the list pointed to by first_msg
 */
static void init_msgfile(void) {
    FILE *fp;
    char buf[MAX_BUF], msgbuf[HUGE_BUF], fname[MAX_BUF], *cp;
    int text = 0, nrofmsg = 0;
    static int did_init_msgfile = 0;

    if (did_init_msgfile)
        return;
    did_init_msgfile = 1;

    snprintf(fname, sizeof(fname), "%s/messages", settings.datadir);
    LOG(llevDebug, "Reading messages from %s...\n", fname);

    fp = fopen(fname, "r");
    if (fp != NULL) {
        GeneralMessage *tmp = NULL;
        int lineno;
        int error_lineno;

        error_lineno = 0;
        for (lineno = 1; fgets(buf, MAX_BUF, fp) != NULL; lineno++) {
            if (*buf == '#')
                continue;
            cp = strchr(buf, '\n');
            if (cp != NULL) {
                while (cp > buf && (cp[-1] == ' ' || cp[-1] == '\t'))
                    cp--;
                *cp = '\0';
            }
            if (tmp != NULL) {
                if (text && strcmp(buf, "ENDMSG") == 0) {
                    if (strlen(msgbuf) > BOOK_BUF) {
                        LOG(llevDebug, "Warning: this string exceeded max book buf size:\n");
                        LOG(llevDebug, "  %s\n", msgbuf);
                    }
                    tmp->message = add_string(msgbuf);
                    tmp->next = first_msg;
                    first_msg = tmp;
                    nrofmsg++;
                    if (tmp->identifier != NULL && tmp->title == NULL) {
                        LOG(llevError, "Error: message can't have identifier without title, on line %d\n", error_lineno);
                        fatal(SEE_LAST_ERROR);
                    }
                    tmp = NULL;
                    text = 0;
                } else if (text) {
                    if (!buf_overflow(msgbuf, buf, HUGE_BUF-1)) {
                        strcat(msgbuf, buf);
                        strcat(msgbuf, "\n");
                    } else if (error_lineno != 0) {
                        LOG(llevInfo, "Warning: truncating book at %s, line %d\n", fname, error_lineno);
                    }
                } else if (strcmp(buf, "TEXT") == 0) {
                    text = 1;
                } else if (strncmp(buf, "CHANCE ", 7) == 0) {
                    tmp->chance = atoi(buf + 7);
                    msg_total_chance += tmp->chance;
                } else if (strncmp(buf, "TITLE ", 6) == 0) {
                    tmp->title = add_string(buf + 6);
                } else if (strncmp(buf, "QUEST ", 6) == 0) {
                    tmp->quest_code = add_string(buf + 6);
                } else if (error_lineno != 0) {
                    LOG(llevInfo, "Warning: unknown line %s, line %d\n", buf, error_lineno);
                }
            } else if (strncmp(buf, "MSG", 3) == 0) {
                error_lineno = lineno;
                tmp = (GeneralMessage *)calloc(1, sizeof(GeneralMessage));
                strcpy(msgbuf, " ");  /* reset msgbuf for new message */
                if (buf[3] == ' ') {
                    int i = 4;
                    while (buf[i] == ' ' && buf[i] != '\0')
                        i++;
                    if (buf[i] != '\0') {
                        tmp->identifier = add_string(buf + i);
                        if (get_message_from_identifier(buf + i)) {
                            LOG(llevError, "Duplicated message identifier %s at line %d\n", buf + i, error_lineno);
                            fatal(SEE_LAST_ERROR);
                        }
                    }
                }
            } else {
                LOG(llevInfo, "Warning: syntax error at %s, line %d\n", fname, lineno);
            }
        }
        fclose(fp);

        if (tmp != NULL) {
            LOG(llevError, "Invalid file %s", fname);
            fatal(SEE_LAST_ERROR);
        }
    }

    LOG(llevDebug, "done messages, found %d for total chance %d.\n", nrofmsg, msg_total_chance);
}

/**
 * If not called before, initialize the info list.
 *
 * This reads in the bookarch file into memory. bookarch is the file
 * created and updated across multiple runs of the program.
 */
static void init_book_archive(void) {
    FILE *fp;
    int nroftitle = 0;
    char buf[MAX_BUF], fname[MAX_BUF], *cp;
    static int did_init_barch = 0;

    if (did_init_barch)
        return;
    did_init_barch = 1;

    if (!booklist)
        booklist = get_empty_booklist();

    snprintf(fname, sizeof(fname), "%s/bookarch", settings.localdir);
    LOG(llevDebug, " Reading bookarch from %s...\n", fname);

    fp = fopen(fname, "r");
    if (fp != NULL) {
        int type;
        size_t i;
        titlelist *bl;
        int lineno;
        title *book;
        int skipping;

        skipping = 0;
        book = NULL;
        type = -1;
        for (lineno = 1; fgets(buf, MAX_BUF, fp) != NULL; lineno++) {
            int len;
            int value;

            if (*buf == '#')
                continue;
            cp = strchr(buf, '\n');
            if (cp != NULL) {
                while (cp > buf && (cp[-1] == ' ' || cp[-1] == '\t'))
                    cp--;
                *cp = '\0';
            }
            cp = buf;
            if (strncmp(buf, "title ", 6) == 0) {
                skipping = 0;
                cp = buf+6;
                while (*cp == ' ' || *cp == '\t')
                    cp++;
                if (*cp == '\0') {
                    LOG(llevInfo, "Warning: missing book title at %s, line %d\n", fname, lineno);
                    book = NULL;
                } else {
                    book = get_empty_book();   /* init new book entry */
                    book->name = add_string(cp);
                    type = -1;
                    nroftitle++;
                }
            } else if (book == NULL) {
                if (!skipping) {
                    skipping = 1;
                    LOG(llevInfo, "Warning: expecting 'title' at %s, line %d\n", fname, lineno);
                }
            } else if (strncmp(buf, "authour ", 8) == 0) {
                cp = buf+8;
                while (*cp == ' ' || *cp == '\t')
                    cp++;
                if (*cp == '\0') {
                    LOG(llevInfo, "Warning: missing book authour at %s, line %d\n", fname, lineno);
                } else {
                    book->authour = add_string(cp);
                }
            } else if (strncmp(buf, "arch ", 5) == 0) {
                cp = buf+5;
                while (*cp == ' ' || *cp == '\t')
                    cp++;
                if (*cp == '\0') {
                    LOG(llevInfo, "Warning: missing book arch at %s, line %d\n", fname, lineno);
                } else {
                    book->archname = add_string(cp);
                }
            } else if (sscanf(buf, "level %d%n", &value, &len) == 1 && len == (int)strlen(buf)) {
                book->level = value;
            } else if (sscanf(buf, "type %d%n", &value, &len) == 1 && len == (int)strlen(buf)) {
                type = value;
            } else if (sscanf(buf, "size %d%n", &value, &len) == 1 && len == (int)strlen(buf)) {
                book->size = value;
            } else if (sscanf(buf, "index %d%n", &value, &len) == 1 && len == (int)strlen(buf)) {
                book->msg_index = value;
            } else if (strcmp(buf, "end") == 0) { /* link it */
                add_book(book, type, fname, lineno);
                book = NULL;
                type = -1;
            } else {
                LOG(llevInfo, "Warning: syntax error at %s, line %d\n", fname, lineno);
            }
        }
        if (book != NULL) {
            LOG(llevInfo, "Warning: missing 'end' at %s, line %d\n", fname, lineno);
            add_book(book, type, fname, lineno);
        }
        LOG(llevDebug, " book archives(used/avail):\n");
        for (bl = booklist, i = 0; bl != NULL && i < arraysize(max_titles); bl = bl->next, i++) {
            LOG(llevDebug, "(%d/%d)\n", bl->number, max_titles[i]);
        }
        fclose(fp);
    }

#ifdef BOOK_MSG_DEBUG
    LOG(llevDebug, "\n init_book_archive() got %d titles.\n", nroftitle);
#endif
    LOG(llevDebug, " done.\n");
}

/**
 * Appends a book to the booklist.
 * @param book the book to add
 * @param type the book type
 * @param fname the file name; for error messages
 * @param lineno the line number; for error messages
 */
static void add_book(title *book, int type, const char *fname, int lineno) {
    titlelist *bl;

    if (type == -1) {
        LOG(llevInfo, "Warning: book with no type at %s, line %d; using type 0\n", fname, lineno);
        type = 0;
    }

    bl = get_titlelist(type);
    book->next = bl->first_book;
    bl->first_book = book;
    bl->number++;
}

/**
 * Creates the linked list of pointers to
 * monster archetype objects if not called previously.
 */
static void init_mon_info(void) {
    archetype *at;
    static int did_init_mon_info = 0;

    if (did_init_mon_info)
        return;
    did_init_mon_info = 1;

    for (at = first_archetype; at != NULL; at = at->next) {
        if (QUERY_FLAG(&at->clone, FLAG_MONSTER)
        && (!QUERY_FLAG(&at->clone, FLAG_CHANGING) || QUERY_FLAG(&at->clone, FLAG_UNAGGRESSIVE))) {
            objectlink *mon = (objectlink *)malloc(sizeof(objectlink));
            if (!mon) {
                LOG(llevError, "init_mon_info: malloc failed!\n");
                abort();
            }
            mon->ob = &at->clone;
            mon->id = nrofmon;
            mon->next = first_mon_info;
            first_mon_info = mon;
            nrofmon++;
        }
    }
    LOG(llevDebug, "init_mon_info() got %d monsters\n", nrofmon);
}

/**
 * Initialize linked lists utilized by message functions in tailor_readable_ob()
 *
 * This is the function called by the main routine to initialize
 * all the readable information.
 */
void init_readable(void) {
    static int did_this = 0;

    if (did_this)
        return;
    did_this = 1;

    LOG(llevDebug, "Initializing reading data...\n");
    init_msgfile();
    init_book_archive();
    init_mon_info();
    LOG(llevDebug, " done reading data\n");
}

/*****************************************************************************
 *
 * This is the start of the administrative functions when creating
 * new books (ie, updating title and the like)
 *
 *****************************************************************************/

/**
 * Search the titlelist (based on msgtype) to see if
 * book matches something already there.  IF so, return that title.
 *
 * @param book
 * book we're searching.
 * @param msgtype
 * message type.
 * @return
 * title if found, NULL if no match.
 */
static title *find_title(const object *book, int msgtype) {
    title *t;
    titlelist *tl;
    size_t length;
    int index;

    if (msgtype < 0)
        return (title *)NULL;

    tl = get_titlelist(msgtype);
    if (!tl)
        return (title *)NULL;

    length = strlen(book->msg);
    index = strtoint(book->msg);
    for (t = tl->first_book; t; t = t->next)
        if (t->size == length && t->msg_index == index) {
#ifdef ARCHIVE_DEBUG
            LOG(llevDebug, "Found title match (list %d): %s %s (%d)\n", msgtype, t->name, t->authour, t->msg_index);
#endif
            return t;
        }

    return (title *)NULL;
}

/**
 * Only for objects of type BOOK. SPELLBOOK stuff is
 * handled directly in change_book_name(). Names are based on text
 * msgtype
 *
 * This sets book book->name based on msgtype given.  What name
 * is given is based on various criteria
 *
 * @param book
 * book we want to alter.
 * @param msgtype
 * what information we want in the book.
 */
static void new_text_name(object *book, int msgtype) {
    const char *name;

    if (book->type != BOOK)
        return;

    switch (msgtype) {
    case MSGTYPE_MONSTER:
        name = mon_book_name[RANDOM()%arraysize(mon_book_name)];
        break;

    case MSGTYPE_ARTIFACT:
        name = art_book_name[RANDOM()%arraysize(art_book_name)];
        break;

    case MSGTYPE_SPELLPATH:
        name = path_book_name[RANDOM()%arraysize(path_book_name)];
        break;

    case MSGTYPE_ALCHEMY:
        name = formula_book_name[RANDOM()%arraysize(formula_book_name)];
        break;

    case MSGTYPE_GODS:
        name = gods_book_name[RANDOM()%arraysize(gods_book_name)];
        break;

    case MSGTYPE_MSGFILE:
    default:
        if (book->weight > 2000) {  /* based on weight */
            name = heavy_book_name[RANDOM()%arraysize(heavy_book_name)];
        } else {
            name = light_book_name[RANDOM()%arraysize(light_book_name)];
        }
        break;
    }
    free_string(book->name);
    book->name = add_string(name);
}

/**
 * A lot like new_text_name() above, but instead chooses an author
 * and sets op->title to that value
 *
 * @param op
 * book to alter.
 * @param msgtype
 * information we want.
 */
static void add_author(object *op, int msgtype) {
    char title[MAX_BUF];
    const char *name;

    if (msgtype < 0 || strlen(op->msg) < 5)
        return;

    switch (msgtype) {
    case MSGTYPE_MONSTER:
        name = mon_author[RANDOM()%arraysize(mon_author)];
        break;

    case MSGTYPE_ARTIFACT:
        name = art_author[RANDOM()%arraysize(art_author)];
        break;

    case MSGTYPE_SPELLPATH:
        name = path_author[RANDOM()%arraysize(path_author)];
        break;

    case MSGTYPE_ALCHEMY:
        name = formula_author[RANDOM()%arraysize(formula_author)];
        break;

    case MSGTYPE_GODS:
        name = gods_author[RANDOM()%arraysize(gods_author)];
        break;

    case MSGTYPE_MSGFILE:
    default:
        name = book_author[RANDOM()%arraysize(book_author)];
    }

    snprintf(title, sizeof(title), "of %s", name);
    op->title = add_string(title);
}

/**
 * Check to see if the book title/msg is unique. We
 * go through the entire list of possibilities each time. If we find
 * a match, then unique_book returns true (because inst unique).
 *
 * @param book
 * book we're searching.
 * @param msgtype
 * type of information contained.
 * @return
 */
static int unique_book(const object *book, int msgtype) {
    title *test;

    if (!booklist)
        return 1;  /* No archival entries! Must be unique! */

    /* Go through the booklist.  If the author and name match, not unique so
     * return 0.
     */
    for (test = get_titlelist(msgtype)->first_book; test; test = test->next) {
        if (!strcmp(test->name, book->name) && !strcmp(book->title, test->authour))
            return 0;
    }
    return 1;
}

/**
 * Adds a book to the list of existing books.
 *
 * @param book
 * book to add.
 * @param msgtype
 * what information the book contains.
 */
static void add_book_to_list(const object *book, int msgtype) {
    titlelist *tl = get_titlelist(msgtype);
    title *t;

    if (!tl) {
        LOG(llevError, "add_book_to_list can't get booklist!\n");
        return;
    }

    t = get_empty_book();
    t->name = add_string(book->name);
    t->authour = add_string(book->title);
    t->size = strlen(book->msg);
    t->msg_index = strtoint(book->msg);
    t->archname = add_string(book->arch->name);
    t->level = book->level;

    t->next = tl->first_book;
    tl->first_book = t;
    tl->number++;

    /* We have stuff we need to write now */
    need_to_write_bookarchive = 1;

#ifdef ARCHIVE_DEBUG
    LOG(llevDebug, "Archiving new title: %s %s (%d)\n", book->name, book->title, msgtype);
#endif
}

/**
 * Give a new, fancier name to generated
 * objects of type BOOK and SPELLBOOK.
 * Aug 96 I changed this so we will attempt to create consistent
 * authour/title and message content for BOOKs. Also, we will
 * alter books  that match archive entries to the archival
 * levels and architypes. -b.t.
 *
 * @param book
 * book to alter. Should be of type BOOK.
 * @param msgtype
 * what information the book contains.
 */
static void change_book(object *book, int msgtype) {
    titlelist *tl;
    title *t;
    int tries;

    if (book->type != BOOK) {
        LOG(llevError, "change_book_name() called w/ illegal obj type.\n");
        return;
    }

    tl = get_titlelist(msgtype);
    t = NULL;
    tries = 0;

    /* look to see if our msg already been archived. If so, alter
     * the book to match the archival text. If we fail to match,
     * then we archive the new title/name/msg combo if there is
     * room on the titlelist.
     */

    if (strlen(book->msg) > 5 && (t = find_title(book, msgtype))) {
        object *tmpbook;
        sstring marker = object_get_value(book, "knowledge_marker");

        /* alter book properties */
        tmpbook = create_archetype(t->archname);
        if (marker != NULL)
            /* need to copy the knowledge_marker */
            object_set_value(tmpbook, "knowledge_marker", marker, 1);
        object_set_msg(tmpbook, book->msg);
        object_copy(tmpbook, book);
        object_free_drop_inventory(tmpbook);

        book->title = add_string(t->authour);
        free_string(book->name);
        book->name = add_string(t->name);
        book->level = t->level;
    } else { /* Don't have any default title, so lets make up a new one */
        int numb, maxnames = max_titles[msgtype];
        const char *old_title;
        const char *old_name;

        old_title = book->title ? add_string(book->title) : NULL;
        old_name = add_string(book->name);

        /* some pre-generated books have title already set (from
         * maps), also don't bother looking for unique title if
         * we already used up all the available names! */

        if (!tl) {
            LOG(llevError, "change_book_name(): can't find title list\n");
            numb = 0;
        } else
            numb = tl->number;

        if (numb == maxnames) {
#ifdef ARCHIVE_DEBUG
            LOG(llevDebug, "titles for list %d full (%d possible).\n", msgtype, maxnames);
#endif
            if (old_title != NULL)
                free_string(old_title);
            free_string(old_name);
            return;
        }
        /* shouldnt change map-maker books */
        if (!book->title)
            do {
                /* random book name */
                new_text_name(book, msgtype);
                add_author(book, msgtype);  /* random author */
                tries++;
            } while (!unique_book(book, msgtype) && tries < MAX_TITLE_CHECK);

        /* Now deal with 2 cases.
         * 1) If no space for a new title exists lets just restore
         * the old book properties. Remember, if the book had
         * matchd an older entry on the titlelist, we shouldnt
         * have called this routine in the first place!
         * 2) If we got a unique title, we need to add it to
         * the list.
         */

        if (tries == MAX_TITLE_CHECK) {
#ifdef ARCHIVE_DEBUG
            LOG(llevDebug, "Failed to obtain unique title for %s %s (names:%d/%d)\n", book->name, book->title, numb, maxnames);
#endif
            /* restore old book properties here */
            free_string(book->name);
            free_string(book->title);
            book->title = old_title != NULL ? add_string(old_title) : NULL;

            if (RANDOM()%4) {
                /* Lets give the book a description to individualize it some */
                char new_name[MAX_BUF];

                snprintf(new_name, MAX_BUF, "%s %s", book_descrpt[RANDOM()%arraysize(book_descrpt)], old_name);
                book->name = add_string(new_name);
            } else {
                book->name = add_string(old_name);
            }
        } else if (book->title && strlen(book->msg) > 5) { /* archive if long msg texts */
            add_book_to_list(book, msgtype);
        }

        if (old_title != NULL)
            free_string(old_title);
        free_string(old_name);
    }
}

/*****************************************************************************
 *
 * This is the start of the area that generates the actual contents
 * of the book.
 *
 *****************************************************************************/

/*****************************************************************************
 * Monster msg generation code.
 ****************************************************************************/

/**
 * Returns a random monster selected from linked
 * list of all monsters in the current game.
 * Changed 971225 to be greater than equal to level passed.  Also
 * made choosing by level more random.
 *
 * @param level
 * if non-zero, then only monsters greater than that level will be returned.
 * @return
 * random monster, or NULL if failure.
 */
object *get_random_mon(int level) {
    objectlink *mon;
    int i, monnr;

    /* safety check.  Problem w/ init_mon_info list? */
    if (!nrofmon || !first_mon_info)
        return (object *)NULL;

    if (!level) {
        /* lets get a random monster from the mon_info linked list */
        monnr = RANDOM()%nrofmon;

        for (mon = first_mon_info, i = 0; mon; mon = mon->next, i++)
            if (i == monnr)
                break;

        if (!mon) {
            LOG(llevError, "get_random_mon: Didn't find a monster when we should have\n");
            return NULL;
        }
        return mon->ob;
    }

    /* Case where we are searching by level.  Redone 971225 to be clearer
     * and more random.  Before, it looks like it took a random monster from
     * the list, and then returned the first monster after that which was
     * appropriate level.  This wasn't very random because if you had a
     * bunch of low level monsters and then a high level one, if the random
     * determine took one of the low level ones, it would just forward to the
     * high level one and return that.  Thus, monsters that immediately followed
     * a bunch of low level monsters would be more heavily returned.  It also
     * means some of the dragons would be poorly represented, since they
     * are a group of high level monsters all around each other.
     */

    /* First count number of monsters meeting level criteria */
    for (mon = first_mon_info, i = 0; mon; mon = mon->next)
        if (mon->ob->level >= level)
            i++;

    if (i == 0) {
        LOG(llevError, "get_random_mon() couldn't return monster for level %d\n", level);
        return NULL;
    }

    monnr = RANDOM()%i;
    for (mon = first_mon_info; mon; mon = mon->next)
        if (mon->ob->level >= level && monnr-- == 0)
            return mon->ob;

    LOG(llevError, "get_random_mon(): didn't find a monster when we should have\n");
    return NULL;
}

/**
 * Returns a description of the monster.  This really needs to be
 * redone, as describe_item really gives a pretty internal description.
 *
 * @param mon
 * monster to describe.
 * @return
 * new StringBuffer containing the description.
 */
static StringBuffer *mon_desc(const object *mon) {
    StringBuffer *desc = stringbuffer_new();
    stringbuffer_append_printf(desc, "\n---\n *** %s ***\n", mon->name);
    describe_item(mon, NULL, desc);
    return desc;
}

/**
 * This function returns the next monster after 'tmp' in the monster list.
 *
 * @param tmp
 * monster.
 * @return
 * next monster, or if no match is found, it returns NULL.
 *
 * @note
 * list is considered circular, asking for the next of the last element will return the first one.
 */
static object *get_next_mon(const object *tmp) {
    objectlink *mon;

    for (mon = first_mon_info; mon; mon = mon->next)
        if (mon->ob == tmp)
            break;

    /* didn't find a match */
    if (!mon)
        return NULL;
    if (mon->next)
        return mon->next->ob;
    else
        return first_mon_info->ob;
}

/**
 * Generate a message detailing the properties of randomly monster(s), and add relevant knowledge markers.
 *
 * @param level
 * book level.
 * @param booksize
 * size (in characters) of the book we want.
 * @param book
 * book in which we will put the information.
 * @return
 * StringBuffer containing the information.
 */
static StringBuffer *mon_info_msg(int level, size_t booksize, object *book) {
    object *tmp;
    StringBuffer *marker = stringbuffer_new(), *desc = stringbuffer_new(), *mon = NULL;
    int added = 0;
    sstring final;

    /*preamble */
    stringbuffer_append_string(desc, "This beastiary contains:");
    stringbuffer_append_string(marker, "monster");

    /* lets print info on as many monsters as will fit in our
     * document.
     * 8-96 Had to change this a bit, otherwise there would
     * have been an impossibly large number of combinations
     * of text! (and flood out the available number of titles
     * in the archive in a snap!) -b.t.
     */
    for (tmp = get_random_mon(level*3); tmp; tmp = get_next_mon(tmp)) {
        /* monster description */
        mon = mon_desc(tmp);

        if (stringbuffer_length(desc) + stringbuffer_length(mon) >= booksize)
            break;
        added++;
        stringbuffer_append_printf(marker, ":%s", tmp->arch->name);
        stringbuffer_append_stringbuffer(desc, mon);
        stringbuffer_delete(mon);
    }

    if (mon != NULL)
        stringbuffer_delete(mon);

    final = stringbuffer_finish_shared(marker);
    if (added)
        object_set_value(book, "knowledge_marker", final, 1);
    free_string(final);

    return desc;
}

/*****************************************************************************
 * Artifact msg generation code.
 ****************************************************************************/

/**
 * Describe an artifact.
 * @param art artifact to describe.
 * @param al list art is part of.
 * @param message if non zero, and the artifact has a suitable message, add it to the description.
 * @param art_name index in ::art_name_array the artifact is in.
 * @param separator if non zero, a separator is inserted at the start of the description.
 * @return newly allocated StringBuffer* containing the description.
 */
static StringBuffer *artifact_describe(const artifact *art, const artifactlist *al, int message, int art_name, int separator) {
    object *tmp;
    int chance;
    StringBuffer *desc = stringbuffer_new(), *sbuf;

    if (separator)
        stringbuffer_append_string(desc, "---\n");

    /* Name */
    if (art->allowed != NULL) {
        archetype *arch;
        linked_char *temp = art->allowed;
        int inv = 0, w;

        assert(art->allowed_size > 0);
        if (art->allowed_size > 1)
            w = 1 + RANDOM() % art->allowed_size;
        else
            w = 1;

        while (w > 1) {
            assert(temp);
            temp = temp->next;
            w--;
        }

        if (temp->name[0] == '!')
            inv = 1;

        /** @todo check archetype when loading archetypes, not here */
        arch = try_find_archetype(temp->name + inv);
        if (!arch)
            arch = find_archetype_by_object_name(temp->name + inv);

        if (!arch)
            LOG(llevError, "artifact_msg: missing archetype %s for artifact %s (type %d)\n", temp->name + inv, art->item->name, art->item->type);
        else {
            if (inv)
                stringbuffer_append_printf(desc, " A %s (excepted %s) of %s", art_name_array[art_name].name, arch->clone.name_pl, art->item->name);
            else
                stringbuffer_append_printf(desc, " A %s of %s", arch->clone.name, art->item->name);
        }
    } else {  /* default name is used */
        /* use the base 'generic' name for our artifact */
        stringbuffer_append_printf(desc, " The %s of %s", art_name_array[art_name].name, art->item->name);
    }

    /* chance of finding */
    stringbuffer_append_string(desc, " is ");
    chance = 100*((float)art->chance/al->total_chance);
    if (chance >= 20)
        stringbuffer_append_string(desc, "an uncommon");
    else if (chance >= 10)
        stringbuffer_append_string(desc, "an unusual");
    else if (chance >= 5)
        stringbuffer_append_string(desc, "a rare");
    else
        stringbuffer_append_string(desc, "a very rare");

    /* value of artifact */
    stringbuffer_append_printf(desc, " item with a value that is %d times normal.\n", art->item->value);

    /* include the message about the artifact, if exists, and book
    * level is kinda high */
    if (message && !(strlen(art->item->msg) > BOOK_BUF))
        stringbuffer_append_string(desc, art->item->msg);

    /* properties of the artifact */
    tmp = object_new();
    add_abilities(tmp, art->item);
    tmp->type = al->type;
    SET_FLAG(tmp, FLAG_IDENTIFIED);
    sbuf = describe_item(tmp, NULL, NULL);
    if (stringbuffer_length(sbuf) > 1) {
        stringbuffer_append_string(desc, " Properties of this artifact include:\n ");
        stringbuffer_append_stringbuffer(desc, sbuf);
        stringbuffer_append_string(desc, "\n");
    }
    free(stringbuffer_finish(sbuf));
    object_free_drop_inventory(tmp);

    return desc;
}

/**
 * Generate a message detailing the properties
 * of 1-6 artifacts drawn sequentially from the artifact list.
 *
 * @param level
 * level of the book.
 * @param booksize
 * maximum length of the book.
 * @return
 * new StringBuffer containing the dsecription.
 */
static StringBuffer *artifact_msg(int level, size_t booksize) {
    const artifactlist *al;
    const artifact *art;
    int i, type, index;
    int book_entries = level > 5 ? RANDOM()%3+RANDOM()%3+2 : RANDOM()%level+1;
    StringBuffer *desc, *message = stringbuffer_new();

    /* values greater than 5 create msg buffers that are too big! */
    if (book_entries > 5)
        book_entries = 5;

    /* lets determine what kind of artifact type randomly.
     * Right now legal artifacts only come from those listed
     * in art_name_array. Also, we check to be sure an artifactlist
     * for that type exists!
     */
    i = 0;
    do {
        index = RANDOM()%arraysize(art_name_array);
        type = art_name_array[index].type;
        al = find_artifactlist(type);
        i++;
    } while (al == NULL && i < 10);

    if (i == 10) { /* Unable to find a message */
        stringbuffer_append_string(message, "None");
        return message;
    }

    /* There is no reason to start on the artifact list at the beginning. Lets
     * take our starting position randomly... */
    art = al->items;
    for (i = RANDOM()%level+RANDOM()%2+1; i > 0; i--) {
        if (art == NULL)
            art = al->items; /* hmm, out of stuff, loop back around */
        art = art->next;
    }

    /* Ok, lets print out the contents */
    stringbuffer_append_printf(message, "Herein %s detailed %s...\n", book_entries > 1 ? "are" : "is", book_entries > 1 ? "some artifacts" : "an artifact");

    i = 0;
    /* artifact msg attributes loop. Lets keep adding entries to the 'book'
     * as long as we have space up to the allowed max # (book_entires)
     */
    while (book_entries > 0) {
        int with_message;
        if (art == NULL)
            art = al->items;
        with_message = (art->item->msg && RANDOM()%4+1 < level) ? 1 : 0;

        desc = artifact_describe(art, al, with_message, index, i++);

        if (stringbuffer_length(message) + stringbuffer_length(desc) >= booksize) {
            stringbuffer_delete(desc);
            break;
        }

        stringbuffer_append_stringbuffer(message, desc);
        stringbuffer_delete(desc);

        art = art->next;
        book_entries--;
    }

    return message;
}

/*****************************************************************************
 * Spellpath message generation
 *****************************************************************************/

/**
 * Generate a message detailing the member incantations/prayers (and some of their
 * properties) belonging to a random spellpath.
 *
 * @param level
 * level of the book.
 * @param booksize
 * maximumlength of the book.
 * @param buf
 * where to store the message. If not NULL, it is supposed to contain the message header.
 * @return
 * buf, newly allocated StringBuffer if buf is NULL.
 */
static StringBuffer *spellpath_msg(int level, size_t booksize, StringBuffer *buf) {
    int path = RANDOM()%NRSPELLPATHS, prayers = RANDOM()%2;
    int did_first_sp = 0;
    uint32 pnum = spellpathdef[path];
    archetype *at;

    if (buf == NULL) {
        buf = stringbuffer_new();
        /* Preamble */
        stringbuffer_append_printf(buf, "Herein are detailed the names of %s", prayers ? "prayers" : "incantations");
        stringbuffer_append_printf(buf, " belonging to the path of %s:\n ", spellpathnames[path]);
    }

    for (at = first_archetype; at != NULL; at = at->next) {
        /* Determine if this is an appropriate spell.  Must
         * be of matching path, must be of appropriate type (prayer
         * or not), and must be within the valid level range.
         */
        if (at->clone.type == SPELL
        && at->clone.path_attuned&pnum
        && ((at->clone.stats.grace && prayers) || (at->clone.stats.sp && !prayers))
        && at->clone.level < level*8) {
            if (strlen(at->clone.name) + stringbuffer_length(buf) >= booksize)
                break;

            if (did_first_sp)
                stringbuffer_append_string(buf, ",\n ");
            did_first_sp = 1;
            stringbuffer_append_string(buf,at->clone.name);
        }
    }

    /* Geez, no spells were generated. */
    if (!did_first_sp) {
        if (RANDOM()%4) {  /* usually, lets make a recursive call... */
            return spellpath_msg(level, booksize, buf);
        }
        /* give up, cause knowing no spells exist for path is info too. need the header too. */
        stringbuffer_append_string(buf, "- no known spells exist -\n");
    }
    return buf;
}

/**
 * Generate a message detailing the properties of a randomly selected alchemical formula.
 *
 * @param book
 * book we write to.
 * @param level
 * level for formulaes and such.
 */
static void make_formula_book(object *book, int level) {
    recipelist *fl;
    recipe *formula;
    int chance, count = 0;
    const char *op_name;
    archetype *at;
    StringBuffer *text, *title;
    char *final, km[MAX_BUF];

    /* the higher the book level, the more complex (ie number of
     * ingredients) the formula can be.
     */
    fl = get_formulalist((RANDOM()%level)/3+1);
    if (!fl)
        fl = get_formulalist(1);  /* safety */

    if (fl->total_chance == 0) {
        object_set_msg(book, " <indecipherable text>\n");
        new_text_name(book, MSGTYPE_ALCHEMY);
        add_author(book, MSGTYPE_ALCHEMY);
        return;
    }

    /* get a random formula, weighted by its bookchance */
    chance = RANDOM()%fl->total_chance;
    for (formula = fl->items; formula != NULL; formula = formula->next) {
        chance -= formula->chance;
        if (chance <= 0 && formula->chance != 0 && !formula->is_combination)
            break;
    }

    if (!formula || formula->arch_names <= 0) {
        object_set_msg(book, " <indecipherable text>\n");
        new_text_name(book, MSGTYPE_ALCHEMY);
        add_author(book, MSGTYPE_ALCHEMY);
        return;
    }

    /* looks like a formula was found. Base the amount
     * of information on the booklevel and the spellevel
     * of the formula. */

    op_name = formula->arch_name[RANDOM()%formula->arch_names];
    at = find_archetype(op_name);
    if (at == (archetype *)NULL) {
        LOG(llevError, "formula_msg() can't find arch %s for formula.\n", op_name);
        object_set_msg(book, " <indecipherable text>\n");
        new_text_name(book, MSGTYPE_ALCHEMY);
        add_author(book, MSGTYPE_ALCHEMY);
        return;
    }
    op_name = at->clone.name;

    text = stringbuffer_new();
    title = stringbuffer_new();

    /* preamble */
    stringbuffer_append_printf(text, "Herein is described a project using %s:\n", formula->skill ? formula->skill : "an unknown skill");

    /* item name */
    if (strcmp(formula->title, "NONE")) {
        stringbuffer_append_printf(text, "The %s of %s", op_name, formula->title);
        /* This results in things like pile of philo. sulfur.
        * while philo. sulfur may look better, without this,
        * you get things like 'the wise' because its missing the
        * water of section.
        */
        stringbuffer_append_printf(title, "%s: %s of %s", formula_book_name[RANDOM()%arraysize(formula_book_name)], op_name, formula->title);
    } else {
        stringbuffer_append_printf(text, "The %s", op_name);
        stringbuffer_append_printf(title, formula_book_name[RANDOM()%arraysize(formula_book_name)], op_name);
        if (at->clone.title) {
            stringbuffer_append_printf(text, " %s", at->clone.title);
            stringbuffer_append_printf(title, " %s", at->clone.title);
        }
    }
    /* Lets name the book something meaningful ! */
    if (book->name)
        free_string(book->name);
    book->name = stringbuffer_finish_shared(title);
    if (book->title) {
        free_string(book->title);
        book->title = NULL;
    }

    /* ingredients to make it */
    if (formula->ingred != NULL) {
        linked_char *next;
        archetype *at;
        char name[MAX_BUF];

        at = find_archetype(formula->cauldron);
        if (at)
            query_name(&at->clone, name, MAX_BUF);
        else
            snprintf(name, sizeof(name), "an unknown place");

        stringbuffer_append_printf(text, " may be made at %s using the following ingredients:\n", name);

        for (next = formula->ingred; next != NULL; next = next->next) {
            count++;
            stringbuffer_append_printf(text, "%s\n", next->name);
        }
    } else {
        LOG(llevError, "formula_msg() no ingredient list for object %s of %s\n", op_name, formula->title);
        stringbuffer_append_string(text, "\n");
    }

    final = stringbuffer_finish(text);
    object_set_msg(book, final);
    free(final);

    /** knowledge marker */
    /** @todo this would be better in knowledge.c, except this file is in server, not common... */
    snprintf(km, sizeof(km), "alchemy:%d:%d:%s", count, formula->index, formula->title);
    object_set_value(book, "knowledge_marker", km, 1);
}

/**
 * Generate a message drawn randomly from lib/messages.
 *
 * @param book
 * book to fill.
 * @param booksize
 * length of the book we want.
 * @return
 * message to put into book, newly allocated StringBuffer the caller should free.
 */
static StringBuffer *msgfile_msg(object *book, size_t booksize) {
    int weight;
    GeneralMessage *msg = NULL;
    StringBuffer *ret = stringbuffer_new();

    /* get a random message for the 'book' from linked list */
    if (msg_total_chance > 0) {
        assert(first_msg != NULL);
        msg = first_msg;
        weight = (RANDOM() % msg_total_chance);
        while (msg) {
            weight -= msg->chance;
            if (weight < 0)
                break;
            msg = msg->next;
        }
        /* if msg is NULL, then something is really wrong in the computation! */
        assert(msg != NULL);
    }

    if (msg && strlen(msg->message) <= booksize) {
        stringbuffer_append_string(ret, msg->message);
        if (msg->identifier != NULL) {
            char km[HUGE_BUF];
            /** knowledge marker */
            /** @todo this would be better in knowledge.c, except this file is in server, not common... */
            snprintf(km, sizeof(km), "message:%s", msg->identifier);
            object_set_value(book, "knowledge_marker", km, 1);
        }
        if (msg->quest_code) {
            /* add a 'apply' hook to launch the quest */
            object *event = object_create_arch(find_archetype("quest_advance_apply"));
            FREE_AND_COPY(event->name, msg->quest_code);
            object_insert_in_ob(event, book);
        }
    } else
        stringbuffer_append_string(ret, "\n <undecipherable text>");

    return ret;
}

/**
 * Generate a message detailing the properties
 * of a random god. Used by the book hack. b.t.
 *
 * @param level
 * number of elements to give.
 * @param booksize
 * desired length of the book.
 * @param book
 * book we're writing the information to, for knowledge management.
 * @return
 * StringBuffer with the information that the caller is responsible for cleaning,
 * NULL if information overflows the booksize.
 */
static StringBuffer *god_info_msg(int level, size_t booksize, object *book) {
    int what = 0;
    const object *god = pntr_to_god_obj(get_rand_god());
    StringBuffer *desc = NULL;

    if (!god)
        return NULL; /* oops, problems... */

    if (booksize > BOOK_BUF) {
        LOG(llevError, "common/readable.c:god_info_msg() - passed in booksize (%lu) is larger than book buffer (%d)\n", (unsigned long)booksize, BOOK_BUF);
        booksize = BOOK_BUF;
    }

    if (level >= 2 && RANDOM()%2) {
        what |= GOD_ENEMY;
    }
    if (level >= 3 && RANDOM()%2) {
        what |= GOD_HOLYWORD;
    }
    if (level >= 4 && RANDOM()%2) {
        what |= GOD_RESISTANCES;
    }
    if (level >= 5 && RANDOM()%2) {
        what |= GOD_SACRED;
    }
    if (level >= 6 && RANDOM()%2) {
        what |= GOD_BLESSED;
    }
    if (level >= 8 && RANDOM()%2) {
        what |= GOD_IMMUNITIES;
    }
    if (level >= 12 && RANDOM()%2) {
        what |= GOD_PATHS;
    }

    desc = stringbuffer_new();
    what = describe_god(god, what, desc, booksize);

    /* check to be sure new buffer size dont exceed either
    * the maximum buffer size, or the 'natural' size of the
    * book... */
    if (stringbuffer_length(desc) > 1 && stringbuffer_length(desc) <= booksize) {
        char buf[BOOK_BUF];
        snprintf(buf, sizeof(buf), "god:%s:%d", god->name, what);
        object_set_value(book, "knowledge_marker", buf, 1);
        return desc;
    }

    stringbuffer_delete(desc);
    return NULL;
}

/**
 * The main routine. This chooses a random
 * message to put in given readable object (type==BOOK) which will
 * be referred hereafter as a 'book'. We use the book level to de-
 * termine the value of the information we will insert. Higher
 * values mean the book will (generally) have better/more info.
 * See individual cases as to how this will be utilized.
 * "Book" name/content length are based on the weight of the
 * document. If the value of msg_type is negative, we will randomly
 * choose the kind of message to generate.
 * -b.t. thomas@astro.psu.edu
 *
 * @param book
 * the object we are creating into. Must be a book, can have a level.
 * @param msg_type
 * if it is a positive value, we use that to determine the
 * message type - otherwise a random value is used.
 */
void tailor_readable_ob(object *book, int msg_type) {
    int level = book->level ? RANDOM()%book->level+1 : 1;
    size_t book_buf_size;
    StringBuffer *message = NULL;

    /* safety */
    if (book->type != BOOK)
        return;

    if (level <= 0)
        return;   /* if no level no point in doing any more... */

    /* Max text length this book can have. */
    book_buf_size = BOOKSIZE(book);
    book_buf_size -= strlen("\n"); /* Keep enough for final \n. */
    assert(book_buf_size < BOOK_BUF);

    /* &&& The message switch &&& */
    /* Below all of the possible types of messages in the "book"s.
     */
    /*
     * IF you add a new type of book msg, you will have to do several things.
     * 1) make sure there is an entry in the msg switch below!
     * 2) make sure there is an entry in max_titles[] array.
     * 3) make sure there are entries for your case in new_text_title()
     *    and add_authour().
     * 4) you may want separate authour/book name arrays in read.h
     */

    if (msg_type >= (int)arraysize(max_titles))
        msg_type = 0;

    msg_type = msg_type > 0 ? msg_type : RANDOM()%6;
    switch (msg_type) {
    case MSGTYPE_MONSTER:
        message = mon_info_msg(level, book_buf_size, book);
        break;

    case MSGTYPE_ARTIFACT:
        message = artifact_msg(level, book_buf_size);
        break;

    case MSGTYPE_SPELLPATH: /* grouping incantations/prayers by path */
        message = spellpath_msg(level, book_buf_size, NULL);
        break;

    case MSGTYPE_ALCHEMY: /* describe an alchemy formula */
        make_formula_book(book, level);
        /* make_formula_book already gives title */
        return;
        break;

    case MSGTYPE_GODS: /* bits of information about a god */
        message = god_info_msg(level, book_buf_size, book);
        break;

    case MSGTYPE_LIB: /* use info list in lib/ */
    default:
        message = msgfile_msg(book, book_buf_size);
        break;
    }

    if (message != NULL) {
        char *final;
        stringbuffer_append_string(message, "\n");
        final = stringbuffer_finish(message);
        object_set_msg(book, final);
        free(final);
        /* lets give the "book" a new name, which may be a compound word */
        change_book(book, msg_type);
    }
}

/*****************************************************************************
 *
 * Cleanup routine for readable stuff.
 *
 *****************************************************************************/

/**
 * Free all readable-related information.
 */
void free_all_readable(void) {
    titlelist *tlist, *tnext;
    title *title1, *titlenext;
    GeneralMessage *lmsg, *nextmsg;
    objectlink *monlink, *nextmon;

    LOG(llevDebug, "freeing all book information\n");

    for (tlist = booklist; tlist != NULL; tlist = tnext) {
        tnext = tlist->next;
        for (title1 = tlist->first_book; title1; title1 = titlenext) {
            titlenext = title1->next;
            if (title1->name)
                free_string(title1->name);
            if (title1->authour)
                free_string(title1->authour);
            if (title1->archname)
                free_string(title1->archname);
            free(title1);
        }
        free(tlist);
    }
    for (lmsg = first_msg; lmsg; lmsg = nextmsg) {
        nextmsg = lmsg->next;
        if (lmsg->identifier)
            free_string(lmsg->identifier);
        if (lmsg->title)
            free_string(lmsg->title);
        if (lmsg->message)
            free_string(lmsg->message);
        if (lmsg->quest_code)
            free_string(lmsg->quest_code);
        free(lmsg);
    }
    for (monlink = first_mon_info; monlink; monlink = nextmon) {
        nextmon = monlink->next;
        free(monlink);
    }
}

/*****************************************************************************
 *
 * Writeback routine for updating the bookarchive.
 *
 ****************************************************************************/

/**
 * Write out the updated book archive to bookarch file.
 */
void write_book_archive(void) {
    FILE *fp;
    int index;
    char fname[MAX_BUF];
    title *book;
    titlelist *bl;

    /* If nothing changed, don't write anything */
    if (!need_to_write_bookarchive)
        return;

    snprintf(fname, sizeof(fname), "%s/bookarch", settings.localdir);
    LOG(llevDebug, "Updating book archive: %s...\n", fname);

    fp = fopen(fname, "w");
    if (fp == NULL) {
        LOG(llevDebug, "Can't open book archive file %s\n", fname);
        return;
    }

    for (bl = get_titlelist(0), index = 0; bl; bl = bl->next, index++) {
        for (book = bl->first_book; book; book = book->next)
            if (book && book->authour) {
                fprintf(fp, "title %s\n", book->name);
                fprintf(fp, "authour %s\n", book->authour);
                fprintf(fp, "arch %s\n", book->archname);
                fprintf(fp, "level %d\n", book->level);
                fprintf(fp, "type %d\n", index);
                /* C89 doesn't have %zu... */
                fprintf(fp, "size %lu\n", (unsigned long)book->size);
                fprintf(fp, "index %d\n", book->msg_index);
                fprintf(fp, "end\n");
            }
    }
    if (ferror(fp)) {
        LOG(llevError, "Error during book archive save.\n");
        fclose(fp);
        return;
    }
    if (fclose(fp) != 0) {
        LOG(llevError, "Error during book archive save.\n");
        return;
    }
    chmod(fname, SAVE_MODE);
    need_to_write_bookarchive = 0;
}

/**
 * Get the readable type for an object (hopefully book).
 * @param readable
 * object for which we want the readable type.
 * @return
 * type of the book. Will never be NULL.
 */
const readable_message_type *get_readable_message_type(object *readable) {
    uint8 subtype = readable->subtype;

    if (subtype > last_readable_subtype)
        return &readable_message_types[0];
    return &readable_message_types[subtype];
}

/**
 * Find the message from its identifier.
 * @param identifier message's identifier.
 * @return corresponding message, NULL if no such message.
 */
const GeneralMessage *get_message_from_identifier(const char *identifier) {
    GeneralMessage *msg = first_msg;
    while (msg && ((msg->identifier == 0) || (strcmp(msg->identifier, identifier) != 0)))
        msg = msg->next;

    return msg;
}

/**
 * Get a message's title.
 * @param message message, must not be NULL.
 * @return title.
 */
sstring get_message_title(const GeneralMessage *message) {
    return message->title;
}

/**
 * Get a message's body.
 * @param message message, must not be NULL.
 * @return body.
 */
sstring get_message_body(const GeneralMessage *message) {
    return message->message;
}
