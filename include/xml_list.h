#ifndef __XML_LIST_H_
#define __XML_LIST_H_

#include <stddef.h>
#include <stdbool.h>
#include <linux/kernel.h>

typedef struct xss_list_head {
    struct xss_list_head *prev;
    struct xss_list_head *next;
}  xss_list_head_t;

#define _offsetof(type, member) (size_t)&(((type*)0)->member)

#define _container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - _offsetof(type,member) );})

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define XSS_LIST_HEAD_INIT(name) { &(name), &(name) }

#define XSS_LIST_HEAD(name) \
	struct xss_list_head name = XSS_LIST_HEAD_INIT(name)

static inline void XSS_INIT_LIST_HEAD(struct xss_list_head *list)
{
	list->next = list;
	list->prev = list;
}

#ifdef CONFIG_DEBUG_LIST
extern bool __list_add_valid(struct xss_list_head *new,
			      struct xss_list_head *prev,
			      struct xss_list_head *next);
extern bool __list_del_entry_valid(struct xss_list_head *entry);
#else
static inline bool __list_add_valid(struct xss_list_head *new,
				struct xss_list_head *prev,
				struct xss_list_head *next)
{
	return true;
}
static inline bool __list_del_entry_valid(struct xss_list_head *entry)
{
	return true;
}
#endif

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct xss_list_head *new,
			      struct xss_list_head *prev,
			      struct xss_list_head *next)
{
	if (!__list_add_valid(new, prev, next))
		return;

	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * xss_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void xss_list_add(struct xss_list_head *new, struct xss_list_head *head)
{
	__list_add(new, head, head->next);
}


/**
 * xss_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void xss_list_add_tail(struct xss_list_head *new, struct xss_list_head *head)
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
static inline void __list_del(struct xss_list_head * prev, struct xss_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __list_del_entry(struct xss_list_head *entry)
{
    if (!__list_del_entry_valid(entry))
        return;

    __list_del(entry->prev, entry->next);
}

static inline void xss_list_del(struct xss_list_head *entry)
{
    __list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void xss_list_replace(struct xss_list_head *old,
				struct xss_list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

/**
 * list_swap - replace entry1 with entry2 and re-add entry1 at entry2's position
 * @entry1: the location to place entry2
 * @entry2: the location to place entry1
 */
static inline void xss_list_swap(struct xss_list_head *entry1,
			     struct xss_list_head *entry2)
{
	struct xss_list_head *pos = entry2->prev;

	xss_list_del(entry2);
	xss_list_replace(entry1, entry2);
	if (pos == entry1)
		pos = entry2;
	xss_list_add(entry1, pos);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void xss_list_move(struct xss_list_head *list, struct xss_list_head *head)
{
	__list_del_entry(list);
	xss_list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void xss_list_move_tail(struct xss_list_head *list,
				  struct xss_list_head *head)
{
	__list_del_entry(list);
	xss_list_add_tail(list, head);
}

/**
 * list_bulk_move_tail - move a subsection of a list to its tail
 * @head: the head that will follow our entry
 * @first: first entry to move
 * @last: last entry to move, can be the same as first
 *
 * Move all entries between @first and including @last before @head.
 * All three entries must belong to the same linked list.
 */
static inline void xss_list_bulk_move_tail(struct xss_list_head *head,
				       struct xss_list_head *first,
				       struct xss_list_head *last)
{
	first->prev->next = last->next;
	last->next->prev = first->prev;

	head->prev->next = first;
	first->prev = head->prev;

	last->next = head;
	head->prev = last;
}

/**
 * list_is_first -- tests whether @list is the first entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int xss_list_is_first(const struct xss_list_head *list,
					const struct xss_list_head *head)
{
	return list->prev == head;
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int xss_list_is_last(const struct xss_list_head *list,
				const struct xss_list_head *head)
{
	return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int xss_list_empty(const struct xss_list_head *head)
{
	return head->next == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int xss_list_empty_careful(const struct xss_list_head *head)
{
	struct xss_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct xss_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define xss_list_entry(ptr, type, member) \
        _container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define xss_list_first_entry(ptr, type, member) \
	xss_list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define xss_list_last_entry(ptr, type, member) \
	xss_list_entry((ptr)->prev, type, member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define xss_list_first_entry_or_null(ptr, type, member) ({ \
	struct xss_list_head *head__ = (ptr); \
	struct xss_list_head *pos__ = head__->next; \
	pos__ != head__ ? xss_list_entry(pos__, type, member) : NULL; \
})

/**
 * list_next_entry - get the next element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define xss_list_next_entry(pos, member) \
	xss_list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_prev_entry - get the prev element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define xss_list_prev_entry(pos, member) \
	xss_list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct xss_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define xss_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct xss_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define xss_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct xss_list_head to use as a loop cursor.
 * @n:		another &struct xss_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define xss_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct xss_list_head to use as a loop cursor.
 * @n:		another &struct xss_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define xss_list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     pos != (head); \
	     pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define xss_list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define xss_list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_last_entry(head, typeof(*pos), member);		\
	     &pos->member != (head); 					\
	     pos = list_prev_entry(pos, member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define xss_list_prepare_entry(pos, head, member) \
	((pos) ? : xss_list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define xss_list_for_each_entry_continue(pos, head, member) 		\
	for (pos = xss_list_next_entry(pos, member);			\
	     &pos->member != (head);					\
	     pos = xss_list_next_entry(pos, member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define xss_list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = xss_list_prev_entry(pos, member);			\
	     &pos->member != (head);					\
	     pos = xss_list_prev_entry(pos, member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define xss_list_for_each_entry_from(pos, head, member) 			\
	for (; &pos->member != (head);					\
	     pos = xss_list_next_entry(pos, member))

/**
 * list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define xss_list_for_each_entry_from_reverse(pos, head, member)		\
	for (; &pos->member != (head);					\
	     pos = xss_list_prev_entry(pos, member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define xss_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = xss_list_next_entry(pos, member);			\
	     &pos->member != (head); 					\
	     pos = n, n = xss_list_next_entry(n, member))

/**
 * list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define xss_list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = xss_list_next_entry(pos, member), 				\
		n = xss_list_next_entry(pos, member);				\
	     &pos->member != (head);						\
	     pos = n, n = xss_list_next_entry(n, member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define xss_list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = xss_list_next_entry(pos, member);					\
	     &pos->member != (head);						\
	     pos = n, n = xss_list_next_entry(n, member))

#endif
