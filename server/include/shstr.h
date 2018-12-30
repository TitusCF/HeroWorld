/**
 * @file
 * Shared-strings defines.
 */

#ifndef SHSTR_H
#define SHSTR_H

/**
 * The size of the shared strings hashtable. This must be smaller than
 * 32767, but 947 ought to be plenty enough.
 */
#define TABLESIZE 4133

/**
 * This specifies how many characters the hashing routine should look at.
 * You may actually save CPU by increasing this number if the typical string
 * is large.
 */
#ifndef MAXSTRING
#define MAXSTRING 20
#endif

/**
 * In the unlikely occurence that 16383 references to a string are too
 * few, you can modify the below type to something bigger.
 * (The top bit of "refcount" is used to signify that "u.array" points
 * at the array entry.)
 */
#define REFCOUNT_TYPE int

/**
 * The offsetof macro is part of ANSI C, but many compilers lack it, for
 * example "gcc -ansi"
 */
#if !defined(offsetof)
#define offsetof(type, member) (int)&(((type *)0)->member)
#endif

/**
 * SS(string) will return the address of the shared_string struct which
 * contains "string".
 */
#define SS(x) ((shared_string *) ((x)-offsetof(shared_string, string)))

#define SS_DUMP_TABLE   1
#define SS_DUMP_TOTALS  2

#ifdef SS_STATISTICS
/** Used to collect statistics on string manipulation. */
static struct statistics {
    int calls;
    int hashed;
    int strcmps;
    int search;
    int linked;
} add_stats, add_ref_stats, free_stats, find_stats, hash_stats;
#define GATHER(n) (++n)
#else /* !SS_STATISTICS */
#define GATHER(n)
#endif /* SS_STATISTICS */

#define TOPBIT ((unsigned REFCOUNT_TYPE)1<<(sizeof(REFCOUNT_TYPE)*CHAR_BIT-1))

#define PADDING ((2*sizeof(long)-sizeof(REFCOUNT_TYPE))%sizeof(long))+1

/**
 * One actual shared string.
 */
typedef struct _shared_string {
    union {
        struct _shared_string **array;
        struct _shared_string *previous;
    } u;
    struct _shared_string *next;
    /* The top bit of "refcount" is used to signify that "u.array" points
     * at the array entry.
     */
    unsigned REFCOUNT_TYPE refcount;
    /* Padding will be unused memory, since we can't know how large
     * the padding when allocating memory. We assume here that
     * sizeof(long) is a good boundary.
     */
    char string[PADDING];
} shared_string;

#endif /* SHSTR_H */
