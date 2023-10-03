#ifndef MELOX_LIST_H
#define MELOX_LIST_H
#include<common.h>
struct llist_header
{
    struct llist_header* prev;
    struct llist_header* next;
};

static inline void __llist_add(struct llist_header* elem,
            struct llist_header* prev,
            struct llist_header* next)
{
    next->prev = elem;
    elem->next = next;
    elem->prev = prev;
    prev->next = elem;
}

static inline void llist_init_head(struct llist_header* head) {
    head->next = head;
    head->prev = head;
}

static inline void llist_append(struct llist_header* head, struct llist_header* elem)
{
    __llist_add(elem, head, head->next);
}

static inline int llist_empty(struct llist_header* elem) {
    return elem->next == elem;
}

static inline void
llist_delete(struct llist_header* elem) {
    elem->prev->next = elem->next;
    elem->next->prev = elem->next;
    
    // make elem orphaned
    elem->prev = elem;
    elem->next = elem;
}
/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define llist_for_each(pos, n, head, member)				    \
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					            \
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))
#endif