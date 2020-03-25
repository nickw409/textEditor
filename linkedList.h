#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct node
{
   //void pointer used so any type can be stored
   void *value;
   struct node *next;
} ListNode;
//Returns list
ListNode* addHead(ListNode *list, void *value);
//Returns list
ListNode* addNode(ListNode *list, void *value, int index);
//Returns list
ListNode* addTail(ListNode *list, void *value);
//Returns value from removed node
void* removeNode(ListNode *list, int index);
//Returns value from node at index
void* getValue(ListNode *list, int index);

#endif
