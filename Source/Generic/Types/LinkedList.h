// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_LINKEDLIST_
#define _GENERIC_LINKEDLIST_

#include "Generic/Types/CachedMemoryPool.h"

template <typename T>
class LinkedList
{
	MEMORY_ALLOCATOR(LinkedList<T>, "Data Types");

public:
	struct Node
	{
		MEMORY_ALLOCATOR(Node, "Data Types");

	public:
		T		Value;
		Node*	Next;
		Node*	Prev;
	};

	struct Iterator
	{
		MEMORY_ALLOCATOR(Iterator, "Data Types");

	protected:
		friend class LinkedList<T>;

		const Node* m_node;

	public:
		Iterator(Node* node)
		{
			m_node = node;
		}

		void operator++() 
		{ 
			m_node = m_node->Next; 
		}

		void operator++(int value) 
		{ 
			m_node = m_node->Next; 
		}
		
		bool operator!=(Iterator iter) const 
		{ 
			return m_node != iter.m_node; 
		}

		T operator*() const
		{
			return m_node->Value;
		}

		const Node* Get_Node() const
		{
			return m_node;
		}
	};

	typedef int (*Sort_Comparison_Function)(T, T, void* extra);

private:
	Node m_head;
	int  m_size;
	
	CachedMemoryPool<Node> m_memory_pool;

public:
	LinkedList()
	{
		m_head.Next = &m_head;
		m_head.Prev = &m_head;
		m_size = 0;
	}

	~LinkedList()
	{
		Clear();
	}

	int Size()
	{
		return m_size;
	}

	void Sort(Sort_Comparison_Function comparer, bool ascending = false, void* extra = NULL)
	{
		int ccsgn = -1;
        if (ascending == true)
            ccsgn = 1;

        int insize = 1;
        while (true)
        {
            int   merges  = 0;
            Node* tail    = &m_head;
            Node* p       = m_head.Next;

            while (p != &m_head)
            {
                merges += 1;

                Node*	q		= p->Next;
                int		qsize   = insize;
                int		psize   = 1;

                while (psize < insize && q != &m_head)
                {
                    psize += 1;
                    q = q->Next;
                }

                while (true)
                {
                    Node* t = NULL;
                    if (psize != 0 && qsize != 0 && q != &m_head)
                    {
                        int cc = comparer(p->Value, q->Value, extra) * ccsgn;
                        if (cc <= 0)
                        {
                            t = p;
                            p = p->Next;
                            psize -= 1;
                        }
                        else
                        {
                            t = q;
                            q = q->Next;
                            qsize -= 1;
                        }
                    }
                    else if (psize != 0)
                    {
                        t = p;
                        p = p->Next;
                        psize -= 1;
                    }
                    else if (qsize != 0 && q != &m_head)
                    {
                        t = q;
                        q = q->Next;
                        qsize -= 1;
                    }
                    else
                    {
                        break;
                    }

                    t->Prev = tail;
                    tail->Next = t;
                    tail = t;
                }

                p = q;
            }

            tail->Next = &m_head;
            m_head.Prev = tail;

            if (merges <= 1)
                return;

            insize *= 2;
        }
	}

	void Clear()
	{
		Node* node = m_head.Next;

		while (node != &m_head)
		{
			Node* next = node->Next;
			//delete node;
			m_memory_pool.Release(node);
			node = next;
		}

		m_head.Next = &m_head;
		m_head.Prev = &m_head;

		m_size = 0;
	}

	Iterator Begin()
	{
		return Iterator(m_head.Next);
	}

	Iterator End()
	{
		return Iterator(&m_head);
	}
	
	T First()
	{
		DBG_ASSERT(m_size > 0);

		Node* node = m_head.Next;
		return node->Value;
	}

	T Pop_First()
	{
		DBG_ASSERT(m_size > 0);

		Node* node = m_head.Next;
		T node_value = node->Value;

		node->Prev->Next = node->Next;
		node->Next->Prev = node->Prev;

		m_memory_pool.Release(node);
		//delete value;

		m_size--;

		return node_value;
	}

	const Node* Add(T value)
	{
		return Add_After(value, m_head.Prev);
	}

	const Node* Add_After(T value, const Node* other)
	{
		Node* other_node	= const_cast<Node*>(other);

		Node* node			= new(m_memory_pool.Allocate()) Node;
		node->Value			= value;
		node->Prev			= other_node;
		node->Next			= other_node->Next;
		node->Next->Prev	= node;
		other_node->Next	= node;

		m_size++;

		return node;
	}
	
	const Node* Add_Before(T value, const Node* other)
	{
		Node* other_node	= const_cast<Node*>(other);

		Node* node			= new(m_memory_pool.Allocate()) Node();
		node->Value			= value;
		node->Next			= other_node;
		node->Prev			= other_node->Prev;
		node->Prev->Next	= node;
		other_node->Prev	= node;

		m_size++;

		return node;
	}
	
	void Remove(const Node* value)
	{
		Node* node = const_cast<Node*>(value);

		value->Prev->Next = value->Next;
		value->Next->Prev = value->Prev;

		m_memory_pool.Release(node);
		//delete value;

		m_size--;
	}

	Iterator Remove(Iterator iter)
	{
		Node* node = const_cast<Node*>(iter.m_node);
		Node* next = node->Next;
		Remove(node);
		return Iterator(next);
	}

	const Node* Find(T value)
	{
		Node* node = m_head.Next;

		while (node != &m_head)
		{
			if (value == node->Value)
			{
				return node;
			}
			node = node->Next;
		}

		return NULL;
	}

	T At_Index(int lookup_index)
	{
		Node* node = m_head.Next;

		int index = 0;
		while (index < m_size)
		{
			if (lookup_index == index)
			{
				return node->Value;
			}
			node = node->Next;
			index++;
		}

		return NULL;
	}

	int Index_Of(T value)
	{
		Node* node = m_head.Next;
		int index = 0;

		while (node != &m_head)
		{
			if (value == node->Value)
			{
				return index;
			}
			index++;
			node = node->Next;
		}

		return -1;
	}

	bool Contains(T value)
	{
		return Find(value) != NULL;
	}

};

#endif