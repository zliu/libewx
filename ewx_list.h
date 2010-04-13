#ifndef __EWX_LIST_H__
#define __EWX_LIST_H__

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
#define LIST_POISON1  ((void *) 0x00811208)
#define LIST_POISON2  ((void *) 0x00821222)

/// ewx 链表数据结构
typedef struct ewx_list_head {
	struct ewx_list_head *next, *prev;
} ewx_list_head_t;

#define EWX_LIST_HEAD_PRE_INIT(name) { &(name), &(name) }

#define EWX_LIST_HEAD(name) \
	struct ewx_list_head name = EWX_LIST_HEAD_PRE_INIT(name)

static inline void EWX_LIST_HEAD_INIT(struct ewx_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct ewx_list_head *new,
			      struct ewx_list_head *prev,
			      struct ewx_list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}


/**
 * add a new entry
 * @param new: new entry to be added
 * @param head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void ewx_list_add(struct ewx_list_head *new, struct ewx_list_head *head)
{
	__list_add(new, head, head->next);
}

/**
 * @brief add a new entry
 * @param new: new entry to be added
 * @param head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void ewx_list_add_tail(struct ewx_list_head *new, struct ewx_list_head *head)
{
	__list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct ewx_list_head * prev, struct ewx_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * deletes entry from list.
 * @param entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void ewx_list_del(struct ewx_list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}


/**
 * replace old entry by new one
 * @param old : the element to be replaced
 * @param new : the new element to insert
 *
 * If old was empty, it will be overwritten.
 */
static inline void list_replace(struct ewx_list_head *old,
				struct ewx_list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void list_replace_init(struct ewx_list_head *old,
					struct ewx_list_head *new)
{
	list_replace(old, new);
	EWX_LIST_HEAD_INIT(old);
}

/**
 * deletes entry from list and reinitialize it.
 * @param entry: the element to delete from the list.
 */
static inline void list_del_init(struct ewx_list_head *entry)
{
	__list_del(entry->prev, entry->next);
	EWX_LIST_HEAD_INIT(entry);
}

/**
 * delete from one list and add as another's head
 * @param list: the entry to move
 * @param head: the head that will precede our entry
 */
static inline void ewx_list_move(struct ewx_list_head *list, struct ewx_list_head *head)
{
        __list_del(list->prev, list->next);
        ewx_list_add(list, head);
}

/**
 * delete from one list and add as another's tail
 * @param list: the entry to move
 * @param head: the head that will follow our entry
 */
static inline void ewx_list_move_tail(struct ewx_list_head *list,
				  struct ewx_list_head *head)
{
        __list_del(list->prev, list->next);
        ewx_list_add_tail(list, head);
}

/**
 * tests whether list is the last entry in list head
 * @param list the entry to test
 * @param head the head of the list
 */
static inline int ewx_list_is_last(const struct ewx_list_head *list,
				const struct ewx_list_head *head)
{
	return list->next == head;
}

/**
 * tests whether a list is empty
 * @param head the list to test.
 */
static inline int ewx_list_empty(const struct ewx_list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(struct ewx_list_head *list,
				 struct ewx_list_head *head)
{
	struct ewx_list_head *first = list->next;
	struct ewx_list_head *last = list->prev;
	struct ewx_list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * join two lists
 * @param list: the new list to add.
 * @param head: the place to add it in the first list.
 */
static inline void ewx_list_splice(struct ewx_list_head *list, struct ewx_list_head *head)
{
	if (!ewx_list_empty(list))
		__list_splice(list, head);
}

/**
 * join two lists and reinitialise the emptied list.
 * @param list: the new list to add.
 * @param head: the place to add it in the first list.
 *
 * The list at list is reinitialised
 */
static inline void ewx_list_splice_init(struct ewx_list_head *list,
				    struct ewx_list_head *head)
{
	if (!ewx_list_empty(list)) {
		__list_splice(list, head);
		EWX_LIST_HEAD_INIT(list);
	}
}

/**
 * get the struct for this entry
 * @param ptr:	the &struct ewx_list_head pointer.
 * @param type:	the type of the struct this is embedded in.
 * @param member:	the name of the list_struct within the struct.
 */
#define ewx_list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * iterate over a list
 * @param pos:	the &struct ewx_list_head to use as a loop cursor.
 * @param head:	the head for your list.
 */
#define ewx_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)

/**
 * iterate over a list
 * @param pos:	the &struct ewx_list_head to use as a loop cursor.
 * @param head:	the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * iterate over a list safe against removal of list entry
 * @param pos:	the &struct ewx_list_head to use as a loop cursor.
 * @param n:		another &struct ewx_list_head to use as temporary storage
 * @param head:	the head for your list.
 */
#define ewx_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * prepare a pos entry for use in list_for_each_entry_continue()
 * @param pos:	the type * to use as a start point
 * @param head:	the head of the list
 * @param member:	the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define ewx_list_prepare_entry(pos, head, member) \
	((pos) ? : ewx_list_entry(head, typeof(*pos), member))

#endif
