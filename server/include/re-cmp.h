/**
 * @file
 * Datastructures for representing a subset of regular expressions.
 *
 * @author
 * Kjetil T. Homme \<kjetilho@ifi.uio.no\> May 1993
 */

#ifndef RE_CMP_H
#define RE_CMP_H

/*   C o n f i g u r a t i o n
 */

#define SAFE_CHECKS     /**< Regexp's with syntax errors will core dump if
                         * this is undefined.
                         */

#define RE_TOKEN_MAX 64 /**< Max amount of tokens in a regexp.
                         * Each token uses ~264 bytes. They are allocated
                         * as needed, but never de-allocated.
                         * E.g. [A-Za-z0-9_] counts as one token, so 64
                         * should be plenty for most purposes.
                         */

/*   D o   n o t   c h a n g e    b e l o w
 */

#ifdef uchar
#    undef uchar
#endif
#ifdef Boolean
#    undef Boolean
#endif
#ifdef True
#    undef True
#endif
#ifdef False
#    undef False
#endif

#define uchar   unsigned char
#define Boolean uchar
#define True    1       /**< Changing this value will break the code */
#define False   0

typedef enum {
    sel_any,            /**< corresponds to eg . */
    sel_end,            /**< corresponds to eg \$ */
    sel_single,         /**< corresponds to eg q */
    sel_range,          /**< corresponds to eg [A-F] */
    sel_array,          /**< corresponds to eg [AF-RqO-T] */
    sel_not_single,     /**< corresponds to eg [^f] */
    sel_not_range       /**< corresponds to eg [^A-F] */
} selection_type;

typedef enum {
    rep_once,           /**< corresponds to no meta-char */
    rep_once_or_more,   /**< corresponds to + */
    rep_null_or_once,   /**< corresponds to eg ? */
    rep_null_or_more    /**< corresponds to eg * */
} repetetion_type;

typedef struct {
    selection_type type;
    union {
        uchar single;
        struct {
            uchar low, high;
        } range;
        Boolean array[UCHAR_MAX];
    } u;
    repetetion_type repeat;
} selection;

#endif /* RE_CMP_H */
