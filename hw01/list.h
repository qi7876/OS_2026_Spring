/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Simplified standalone variant of Linux kernel list.h for user-space demos.
 * It keeps the core intrusive circular doubly linked list primitives and
 * removes kernel-only dependencies such as RCU, hardening hooks, and hlist.
 */
#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#include <stddef.h>
#include <stdint.h>

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define LIST_POISON1 ((struct list_head *)(uintptr_t)0x00100100)
#define LIST_POISON2 ((struct list_head *)(uintptr_t)0x00200200)

#define container_of(ptr, type, member) __extension__ ({			\
	const __typeof__(((type *)0)->member) *__mptr = (ptr);		\
	(type *)((char *)__mptr - offsetof(type, member));			\
})

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define list_entry_safe(ptr, type, member) __extension__ ({			\
	__typeof__(ptr) ____ptr = (ptr);					\
	____ptr ? list_entry(____ptr, type, member) : NULL;			\
})

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline int list_is_head(const struct list_head *list,
			       const struct list_head *head)
{
	return list == head;
}

static inline int list_is_first(const struct list_head *list,
				const struct list_head *head)
{
	return list->prev == head;
}

static inline int list_is_last(const struct list_head *list,
			       const struct list_head *head)
{
	return list->next == head;
}

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, __typeof__(*(pos)), member)

#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, __typeof__(*(pos)), member)

#define list_entry_is_head(pos, head, member) \
	(&(pos)->member == (head))

#define list_for_each(pos, head) \
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next;				\
	     !list_is_head(pos, (head));					\
	     pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, __typeof__(*pos), member);	\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, __typeof__(*pos), member),	\
	     n = list_next_entry(pos, member);				\
	     !list_entry_is_head(pos, head, member);			\
	     pos = n, n = list_next_entry(n, member))

static inline size_t list_count_nodes(const struct list_head *head)
{
	struct list_head *pos;
	size_t count = 0;

	list_for_each(pos, head)
		count++;

	return count;
}

#endif
