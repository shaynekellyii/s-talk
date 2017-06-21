/***************************************************************
 * Implementation of a custom list data structure              *
 * Author: Shayne Kelly II                                     *
 * Date: May 19, 2017                                          *
 ***************************************************************/

/***************************************************************
 * Imports                                                     *
 ***************************************************************/
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/***************************************************************
 * Defines                                                     *
 ***************************************************************/
#define NODE_POOL_SIZE 100
#define LIST_POOL_SIZE 10

#define NODE_POOL_FULL				(numNodesAvailable <= 0)
#define LIST_POOL_FULL				(numListsAvailable <= 0)
#define LIST_IS_EMPTY				(list->size == 0)
#define CURRENT_NODE_BEYOND_START	(list->currentIsBeyond == -1)
#define CURRENT_NODE_BEYOND_END		(list->currentIsBeyond == 1)
#define CURRENT_NODE_IS_HEAD		(list->current == list->head)
#define CURRENT_NODE_IS_TAIL		(list->current == list->tail)

/***************************************************************
 * Statics                                                     *
 ***************************************************************/
static NODE nodePool[NODE_POOL_SIZE];
static LIST listPool[LIST_POOL_SIZE];
static int availableNodeArr[NODE_POOL_SIZE];
static int availableListArr[LIST_POOL_SIZE];
static int numNodesAvailable = NODE_POOL_SIZE;
static int numListsAvailable = LIST_POOL_SIZE;
static int initialisationFlag = 0;

static void addItemToEmptyList(LIST *list, void *item);
static void addItemToListSizeOne(LIST *list, void *item, int afterHead);
static void addItemBetweenTwoOthers(LIST *list, void *item, NODE *pre, NODE *post);

/***************************************************************
 * Global Functions                                            *
 ***************************************************************/

/** 
 * Creates a new list and returns a pointer to it.
 */
LIST *ListCreate(void) {
	LIST list;

	/* Set all of the indices of the available nodes when the first list is created */
	if (!initialisationFlag) {
		for (int i = 0; i < NODE_POOL_SIZE; i++) {
			availableNodeArr[i] = i;
		}
		for (int i = 0; i < LIST_POOL_SIZE; i++) {
			availableListArr[i] = i;
		}
		initialisationFlag = 1;
	}

	/* Ensure there is space in the list pool */
	if (LIST_POOL_FULL) {
		return NULL;
	}

	/* Set parameters for a new list */
	list.current = NULL;
	list.head = NULL;
	list.tail = NULL;
	list.size = 0;
	list.currentIsBeyond = 0;

	/* Add local new list to the list pool and return it */
	int listIndex = availableListArr[numListsAvailable - 1];
	listPool[listIndex] = list;
	numListsAvailable--;
	return &listPool[listIndex];
}

/** 
 * Returns the number of items in the list.
 */
int ListCount(LIST *list) {
	if (list != NULL) {
		return list->size;
	} else {
		return 0;
	}
}

/** 
 * Returns a pointer to the first item in the list and makes it the current item.
 * Returns NULL if the list is empty.
 */
void *ListFirst(LIST *list) {
	if (list == NULL || LIST_IS_EMPTY) {
		return NULL;
	}

	list->current = list->head;
	list->currentIsBeyond = 0;
	return list->current->item;
}

/**
 * Returns the last item in the list and makes it the current item.
 * Returns NULL if the list is empty.
 */
void *ListLast(LIST *list) {
	if (list == NULL || LIST_IS_EMPTY) {
		return NULL;
	}

	list->current = list->tail;
	return list->current->item;
}

/**
 * Increments the current item.
 * Returns a pointer to the new current item.
 * Returns NULL if the current item advances beyond the end of the list.
 */
void *ListNext(LIST *list) {
	if (list == NULL) {
		return NULL;
	}

	if (CURRENT_NODE_BEYOND_END || CURRENT_NODE_IS_TAIL || LIST_IS_EMPTY) {
		list->current = NULL;
		list->currentIsBeyond = 1;
		return NULL;
	}

	if (CURRENT_NODE_BEYOND_START) {
		list->current = list->head;
		list->currentIsBeyond = 0;
		return list->current->item;
	}

	list->current = list->current->next;
	return list->current->item;
}

/**
 * Decrements the current item.
 * Returns a pointer to the new current item.
 * Returns NULL if the current item advances beyond the start of the list.
 */
void *ListPrev(LIST *list) {
	if (list == NULL) {
		return NULL;
	}

	if (CURRENT_NODE_BEYOND_START || CURRENT_NODE_IS_HEAD) {
		list->current = NULL;
		list->currentIsBeyond = -1;
		return NULL;
	}

	if (CURRENT_NODE_BEYOND_END) {
		list->current = list->tail;
		list->currentIsBeyond = 0;
		return list->current->item;
	}

	list->current = list->current->previous;
	return list->current->item;
}

/**
 * Returns a pointer to the current item in the list
 */
void *ListCurr(LIST *list) {
	if (list == NULL || LIST_IS_EMPTY || CURRENT_NODE_BEYOND_START || CURRENT_NODE_BEYOND_END) {
		return NULL;
	}	
	return list->current->item;
}

/**
 * Adds the new item to the list directly after the current item and makes it the current item.
 * If the current pointer is before the start of the list, the item is added to the start.
 * If the current pointer is after the end of the list, the item is added to the end.
 * Returns 0 if successful, -1 if failed.
 */
int ListAdd(LIST *list, void *item) {
	if (list == NULL || item == NULL || NODE_POOL_FULL) {
		return -1;
	}

	if (LIST_IS_EMPTY) {
		addItemToEmptyList(list, item);
	} else if (list->size == 1) {
		addItemToListSizeOne(list, item, CURRENT_NODE_BEYOND_START ? 0 : 1);
	} else if (CURRENT_NODE_BEYOND_START) {
		ListPrepend(list, item);
	} else if (CURRENT_NODE_BEYOND_END || CURRENT_NODE_IS_TAIL) {
		ListAppend(list, item);
	} else {
		addItemBetweenTwoOthers(list, item, list->current, list->current->next);
	}
	return 0;
}

/**
 * Adds the new item to the list directly before the current item and makes it the current item.
 * If the current pointer is before the start of the list, the item is added to the start.
 * If the current pointer is after the end of the list, the item is added to the end.
 * Returns 0 if successful, -1 if failed
 */
int ListInsert(LIST *list, void *item) {
	if (list == NULL || item == NULL || NODE_POOL_FULL) {
		return -1;
	}

	if (LIST_IS_EMPTY) {
		addItemToEmptyList(list, item);
	} else if (list->size == 1) {
		addItemToListSizeOne(list, item, CURRENT_NODE_BEYOND_END ? 1 : 0);
	} else if (CURRENT_NODE_BEYOND_START || CURRENT_NODE_IS_HEAD) {
		ListPrepend(list, item);
	} else if (CURRENT_NODE_BEYOND_END) {
		ListAppend(list, item);
	} else {
		addItemBetweenTwoOthers(list, item, list->current->previous, list->current);
	}
	return 0;
}

/**
 * Adds item to the end of the list and makes the new item the current one.
 * Returns 0 if successful, -1 if failed.
 */
int ListAppend(LIST *list, void *item) {
	NODE node;

	if (list == NULL || item == NULL || NODE_POOL_FULL) {
		return -1;
	}
	if (LIST_IS_EMPTY) {
		addItemToEmptyList(list, item);
		return 0;
	}

	node.item = item;
	node.previous = list->tail;
	node.next = NULL;
	
	int nodeIndex = availableNodeArr[numNodesAvailable - 1];
	nodePool[nodeIndex] = node;
	numNodesAvailable--;

	if (list->size == 1) {
		list->head->next = &nodePool[nodeIndex];
	}
	list->tail->next = &nodePool[nodeIndex];
	list->tail->next->previous = list->tail;
	list->tail = list->tail->next;
	list->current = list->tail;
	list->size++;
	list->currentIsBeyond = 0;

	return 0;
}

/**
 * Adds item to the front of list, and makes the new item the current one.
 * Returns 0 on success, -1 on failure.
 */
int ListPrepend(LIST *list, void *item) {
	NODE node;

	if (list == NULL || item == NULL || NODE_POOL_FULL) {
		return -1;
	}

	if (LIST_IS_EMPTY) {
		addItemToEmptyList(list, item);
		return 0;
	}

	node.item = item;
	node.previous = NULL;
	node.next = list->head;

	int nodeIndex = availableNodeArr[numNodesAvailable - 1];
	nodePool[nodeIndex] = node;
	numNodesAvailable--;

	list->head->previous = &nodePool[nodeIndex];
	list->head = &nodePool[nodeIndex];
	list->size++;
	list->current = list->head;
	list->currentIsBeyond = 0;

	return 0;
}

/**
 * Return current item and take it out of the list.
 * Make the next item the current one.
 */
void *ListRemove(LIST *list) {
	if (list == NULL || LIST_IS_EMPTY || CURRENT_NODE_BEYOND_START || CURRENT_NODE_BEYOND_END) {
		return NULL;
	}

	void *item = list->current->item;
	ptrdiff_t nodeIndex = list->current - nodePool;

	if (list->size == 1) {
		list->head = NULL;
		list->current = NULL;
		list->tail = NULL;
	} else if (CURRENT_NODE_IS_HEAD) {
		list->head = list->head->next;
		list->head->previous = NULL;
		list->current = list->head;
	} else if (CURRENT_NODE_IS_TAIL) {
		return ListTrim(list);
	} else {
		NODE *preRemovedNode = list->current->previous;
		NODE *postRemovedNode = list->current->next;
		preRemovedNode->next = postRemovedNode;
		postRemovedNode->previous = preRemovedNode;
		list->current = postRemovedNode;
	}

	list->currentIsBeyond = 0;
	list->size--;

	availableNodeArr[numNodesAvailable] = nodeIndex;
	numNodesAvailable++;
	return item;
}

/**
 * Adds list2 to the end of list1. 
 * The current pointer is set to the current pointer of list1. 
 * List2 no longer exists after the operation.
 */
void ListConcat(LIST *list1, LIST *list2) {
	if (list1 == NULL || list2 == NULL) {
		return;
	}

	if (list1->size == 1 && list2->size == 1) {
		list1->head->next = list2->head;
		list1->tail = list2->head;
		list1->tail->previous = list1->head;
	} else if (list1->size == 1 && list2->size > 1) {
		list1->head->next = list2->head;
		list1->tail = list2->tail;
		list2->head->previous = list1->tail;
	} else if (list1->size > 1 && list2->size >= 1) {
		list1->tail->next = list2->head;
		list2->head->previous = list1->tail;
		list1->tail = list2->tail;
	} else if (list1->size == 0 && list2->size > 0) {
		list1->head = list2->head;
		list1->tail = list2->tail;
		list1->current = list2->current;
		list1->currentIsBeyond = 0;
	}
	list1->size += list2->size;

	ptrdiff_t listIndex2 = list2 - listPool;
	availableListArr[numListsAvailable] = listIndex2;
	numListsAvailable++;
	list2 = NULL;
}

/** 
 * Delete list. 
 * ItemFree is a pointer to a routine that frees an item. 
 * It should be invoked (within ListFree) as: (* itemFree)(itemToBeFreed);
 */
void ListFree(LIST *list, void (*itemFree)(void *)) {
	if (list == NULL) {
		return;
	}

	NODE *nodeToDelete = list->head;
	while (nodeToDelete != NULL) {
		if (itemFree != NULL) {
			(* itemFree)(nodeToDelete->item);
		}

		ptrdiff_t nodeIndex = nodeToDelete - nodePool;
		availableNodeArr[numNodesAvailable] = nodeIndex;
		numNodesAvailable++;

		NODE *oldNode = nodeToDelete;
		nodeToDelete = nodeToDelete->next;
		oldNode->item = NULL;
		oldNode->previous = NULL;
		oldNode->next = NULL;
	}

	list->current = NULL;
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->currentIsBeyond = 0;

	ptrdiff_t listIndex = list - listPool;
	availableListArr[numListsAvailable] = listIndex;
	numListsAvailable++;
}

/**
 * Return last item and take it out of list. 
 * Make the new last item the current one.
 */
void *ListTrim(LIST *list) {
	if (list == NULL || LIST_IS_EMPTY) {
		return NULL;
	}

	void *item = list->tail->item;
	ptrdiff_t nodeIndex = list->tail - nodePool;
	availableNodeArr[numNodesAvailable] = nodeIndex;
	numNodesAvailable++;

	if (list->size > 1) {
		list->tail = list->tail->previous;
		list->tail->next = NULL;
		list->current = list->tail;
	} else {
		list->tail = NULL;
		list->head = NULL;
		list->current = NULL;
	}
	list->size--;
	list->currentIsBeyond = 0;

	return item;
}

/**
 * Searches list starting at the current item until the end is reached or a match is found. 
 * In this context, a match is determined by the comparator parameter. 
 * This parameter is a pointer to a routine that takes as its first argument an item pointer, and as its second argument comparisonArg. 
 * Comparator returns 0 if the item and comparisonArg don't match, or 1 if they do. 
 * Exactly what constitutes a match is up to the implementor of comparator. 
 * If a match is found, the current pointer is left at the matched item and the pointer to that item is returned. 
 * If no match is found, the current pointer is left beyond the end of the list and a NULL pointer is returned.
 */
void *ListSearch(LIST *list, int (*comparator)(void *, void *), void *comparisonArg) {
	if (list == NULL || comparator == NULL || LIST_IS_EMPTY) {
		return NULL;
	}

	NODE *searchNode = (list->current == NULL && list->currentIsBeyond == -1) ? 
		list->head : list->current;
	while (searchNode != NULL) {
		if ((* comparator)(searchNode->item, comparisonArg) == 1) {
			list->current = searchNode;
			list->currentIsBeyond = 0;
			return list->current->item;
		}
		searchNode = searchNode->next;
	}

	list->current = NULL;
	list->currentIsBeyond = 1;
	return NULL;
}


/***************************************************************
 * Static Functions                                            *
 ***************************************************************/

/**
 * Add item to empty list
 */
static void addItemToEmptyList(LIST *list, void *item) {
	NODE node;
	node.item = item;
	node.previous = NULL;
	node.next = NULL;

	int nodeIndex = availableNodeArr[numNodesAvailable - 1];
	nodePool[nodeIndex] = node;
	numNodesAvailable--;

	list->current = &nodePool[nodeIndex];
	list->head = &nodePool[nodeIndex];
	list->tail = &nodePool[nodeIndex];
	list->size++;
}

/**
 * Add item to list with only one item
 */
static void addItemToListSizeOne(LIST *list, void *item, int afterHead) {
	NODE node;
	node.item = item;
	if (afterHead) {
		node.previous = list->head;
		node.next = NULL;
	} else {
		node.previous = NULL;
		node.next = list->head;
	}

	int nodeIndex = availableNodeArr[numNodesAvailable - 1];
	nodePool[nodeIndex] = node;
	numNodesAvailable--;

	list->current = &nodePool[nodeIndex];
	if (afterHead) {
		list->head->next = &nodePool[nodeIndex];
		list->tail = &nodePool[nodeIndex];
	} else {
		list->head = &nodePool[nodeIndex];
		list->tail = list->head->next;
		list->tail->previous = list->head;
	}
	list->currentIsBeyond = 0;
	list->size++;
}

/**
 * Add item between two index items of a non-empty list
 */
static void addItemBetweenTwoOthers(LIST *list, void *item, NODE *pre, NODE *post) {
	NODE node;
	node.item = item;
	node.previous = pre;
	node.next = post;

	int nodeIndex = availableNodeArr[numNodesAvailable - 1];
	nodePool[nodeIndex] = node;
	numNodesAvailable--;
	
	pre->next = &nodePool[nodeIndex];
	post->previous = &nodePool[nodeIndex];

	list->current = &nodePool[nodeIndex];
	list->size++;
	list->currentIsBeyond = 0;
}