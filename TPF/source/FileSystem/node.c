/***************************************************************************//**
  @file		node.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		14 ene. 2023
 ******************************************************************************/

#include "node.h"

Node*	nodeNew(void* name, void* path)
{
	Node *n;

	n = (Node*)malloc(sizeof(Node));
	if (! n)
		return n;
	n->name = name;
	n->path = path;
	n->next = NULL;
	n->prev = NULL;
	n->parent = NULL;
	n->children = NULL;
	return n;
}

Node*	nodeInsert(Node *parent, int position, Node *node)
{
	if (! (parent && node))
		return NULL;
	if (position < 0)
		return nodeAppend(parent, node);
	else if (position == 0)
		return nodePrepend(parent, node);
	else
		return nodeInsertBefore(parent, nodeNthChild(parent, position), node);
}

/* Inserts a Node beneath the parent after the given sibling */
Node*	nodeInsertAfter(Node *parent, Node *sibling, Node *node)
{
	if (! (parent && node) || (sibling && sibling->parent == parent))
		return NULL;
	node->parent = parent;
	if (sibling)
	{
		if (sibling->next)
			sibling->next->prev = node;
		node->next = sibling->next;
		node->prev = sibling;
		sibling->next = node;
	}
	else
	{
		if (parent->children)
		{
			node->next = parent->children;
			parent->children->prev = node;
		}
		parent->children = node;
	}
	return node;
}

Node*	nodeInsertBefore(Node *parent, Node *sibling, Node *node)
{
	if (! (parent && node) || (sibling && sibling->parent == parent))
		return NULL;
	node->parent = parent;
	if (sibling)
	{
		if (sibling->prev)
		{
			node->prev = sibling->prev;
			node->prev->next = node;
		}
		else
			node->parent->children = node;
		node->next = sibling;
		sibling->prev = node;
	}
	else
	{
		if (parent->children)
		{
			sibling = parent->children;
			while (sibling->next)
				sibling = sibling->next;
			node->prev = sibling;
			sibling->next = node;
		}
		else
			node->parent->children = node;
	}
	return node;
}

Node*	nodeRoot(Node *node)
{
	if (! node)
		return NULL;
	if (node->parent)
		return nodeRoot(node->parent);
	return node;
}

Node*	nodeFind(Node *node, void *path, int (*compare)(void *a, void *b))
{
	if (! node)
		return node;
	if (! compare(path, node->path))
		return node;
	if (node->next)
		return nodeFind(node->next, path, compare);
	if (node->children)
		return nodeFind(node->children, path, compare);
	return NULL;
}

Node*	nodeNthChild(Node *node, int n)
{
	if (! node)
		return NULL;
	node = node->children;
	while (node && (n-- > 0))
		node = node->next;
	return node;
}

int	nodeTotal(Node	*root)
{
	if (! root)
		return 0;
	int t;

	t = 1;
	if (root->children)
		t += nodeTotal(root->children);
	if (root->next)
		t += nodeTotal(root->next);
	return t;
}

void	nodeUnlink(Node *node)
{
	if (! node)
		return;
	if (node->prev)
		node->prev->next = node->next;
	else if (node->parent)
		node->parent->children = node->next;
	if (node->next)
	{
		node->next->prev = node->prev;
		node->next = NULL;
	}
	node->prev = NULL;
	node->parent = NULL;
}

void	nodeFree(Node *node)
{
	if (! node)
		return;
	if (node->children)
		nodeFree(node->children);
	if (node->next)
		nodeFree(node->next);
	free(node);
}

void	nodeDestroy(Node *root)
{
	if (! root)
		return;
	if (! nodeIsRoot(root))
		nodeUnlink(root);
	nodeFree(root);
}
