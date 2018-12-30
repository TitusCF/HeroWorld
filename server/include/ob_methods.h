/**
 * @file
 * Object type functions and variables.
 */

#ifndef OB_METHODS_H
#define OB_METHODS_H

/**
 * Define some standard return values for callbacks which don't need to return
 * any other results. Later this might be expanded into a more complex return
 * value system if necessary
 */
typedef char method_ret;
#define METHOD_OK 0
#define METHOD_UNHANDLED 1
#define METHOD_ERROR 2
#define METHOD_SILENT_ERROR 3   /**< Player was warned she can't use the item for now. */

/**
 * Typedefs for ob_methods. Also used in ob_types.c for storing the pointers
 * temporarily. As a convention, callbacks parameters should begin with
 * "ob_methods *context, object *ob", and return method_ret unless it needs to
 * return something else.
 */
/* Example:
 * typedef method_ret (apply_func)(ob_methods *context, object *ob);
 */

/**
 * @struct ob_methods
 * This struct stores function pointers for actions that can be done to objects.
 * It is currently just used for type-specific object code. Add new function
 * pointers here when moving type specific code into the server/types/ *.c area.
 * When adding function pointers here, be sure to add to init_ob_methods() in
 * ob_types.c as necessary.
 */
typedef struct ob_methods ob_methods;
typedef method_ret (*apply_func)(ob_methods *, object *, object *, int);
typedef method_ret (*process_func)(ob_methods *, object *);
typedef void (*describe_func)(const ob_methods *, const object *, const object *, char *buf, size_t size);
typedef method_ret (*move_on_func)(ob_methods *, object *, object *, object *);
typedef method_ret (*trigger_func)(ob_methods *, object *, object *, int);

struct ob_methods {
    apply_func      apply;          /**< The apply method */
    process_func    process;        /**< The process method */
    describe_func   describe;       /**< The describe method */
    move_on_func    move_on;        /**< The move_on method */
    trigger_func    trigger;        /**< When something is triggered via a button. */
    struct ob_methods *fallback;    /**< ob_method structure to fallback to */
    /* Example:
     * apply_func *apply;
     */
};

#endif /* OB_METHODS_H */
