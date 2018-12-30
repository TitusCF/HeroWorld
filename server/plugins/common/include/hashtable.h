/* Usually we will store only about 10 or so elements, however we may get
 * cases of a fair bit more and we should handle those cases efficently.
 * hashptr() assumes this value will fit in int.
 */
#define PTR_ASSOC_TABLESIZE 251

typedef struct _ptr_assoc {
    struct _ptr_assoc **array;
    struct _ptr_assoc *previous;
    struct _ptr_assoc *next;
    void *key;
    void *value;
} ptr_assoc;

typedef ptr_assoc *ptr_assoc_table[PTR_ASSOC_TABLESIZE];

extern void init_ptr_assoc_table(ptr_assoc **hash_table);
extern void add_ptr_assoc(ptr_assoc **hash_table, void *key, void *value);
extern void *find_assoc_value(ptr_assoc **hash_table, void *key);
extern void free_ptr_assoc(ptr_assoc **hash_table, void *key);
