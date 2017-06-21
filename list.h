#ifndef _LIST_H_
#define _LIST_H_

/**
 * Structs
 */
typedef struct NODE {
	void *item;
	struct NODE *previous;
	struct NODE *next;
} NODE;
typedef struct LIST {
	NODE *current;
	NODE *head;
	NODE *tail;
	int size;
	int currentIsBeyond; // 0 if current is not beyond the list boundaries, -1 if before, 1 if after
} LIST;

/**
 * Function prototypes
 */
LIST *ListCreate(void);
int ListCount(LIST *list);
void *ListFirst(LIST *list);
void *ListLast(LIST *list);
void *ListNext(LIST *list);
void *ListPrev(LIST *list);
void *ListCurr(LIST *list);
int ListAdd(LIST *list, void *item);
int ListInsert(LIST *list, void *item);
int ListAppend(LIST *list, void *item);
int ListPrepend(LIST *list, void *item);
void *ListRemove(LIST *list);
void ListConcat(LIST *list1, LIST *list2);
void ListFree(LIST *list, void (*itemFree)(void *));
void *ListTrim(LIST *list);
void *ListSearch(LIST *list, int (*comparator)(void *, void *), void *comparisonArg);

#endif /* _LIST_H_ */