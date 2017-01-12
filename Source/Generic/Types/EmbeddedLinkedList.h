// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_EMBEDDEDLINKEDLIST_
#define _GENERIC_EMBEDDEDLINKEDLIST_

// These defines provide supported for embedded linked lists.
#define LIST_START(postfix, list) \
	(list).m_##postfix##_next	

#define LIST_NEXT(postfix, list) \
	(list)->m_##postfix##_next	

#define LIST_IS_END(postfix, list, head) \
	((list) == &(head))	

#define LIST_ADD(postfix, list, x) \
	{ \
	(x)->m_##postfix##_prev = (list).m_##postfix##_prev; \
	(x)->m_##postfix##_next = &(list); \
	(list).m_##postfix##_prev->m_##postfix##_next = (x); \
	(list).m_##postfix##_prev = (x); \
	} 

#define LIST_REMOVE(postfix, x) \
	{ \
	(x)->m_##postfix##_prev->m_##postfix##_next = (x)->m_##postfix##_next; \
	(x)->m_##postfix##_next->m_##postfix##_prev = (x)->m_##postfix##_prev; \
	}

#define LIST_IS_EMPTY(postfix, list) \
	((list).m_##postfix##_next == &(list))

#define LIST_CLEAR(postfix, list) \
	((list).m_##postfix##_next = (list).m_##postfix##_prev = &(list))

#endif