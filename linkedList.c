#include <stdlib.h>
#include <stdio.h>
#include "linkedList.h"

void printError(char *err)
{
   perror(err);
   exit(EXIT_FAILURE);
}

ListNode* addHead(ListNode *list, void *value)
{
   ListNode *head = malloc(sizeof(ListNode));
   if (head == NULL)
   {
      printError("Failed to allocate memory for node");
   }
   head->value = value;
   head->next = list;
   return head;
}

ListNode* addNode(ListNode *list, void *value, int index)
{
   int current = 0;
   ListNode *node = malloc(sizeof(ListNode));
   if (node == NULL)
   {
      printError("Failed to allocate memory for node");
   }
   ListNode *temp = list;
   node->value = value;
   node->next = NULL;
   while (current < index - 1)
   {
      if (temp == NULL || temp->next == NULL)
      {
         printError("Index out of bounds");
      }
      temp = temp->next;
      current++;
   }
   if (temp == NULL)
   {
      return node;
   }
   node->next = temp->next;
   temp->next = node;
   return list;
}

ListNode* addTail(ListNode *list, void *value)
{
   ListNode *node = malloc(sizeof(ListNode));
   ListNode *temp = list;
   node->value = value;
   node->next = NULL;
   if (node == NULL)
   {
      printError("Failed to allocate memory for node");
   }
   if (temp == NULL)
   {
      return node;
   }
   while (temp->next != NULL) {temp = temp->next;}
   temp->next = node;
   return list;
}

void* removeNode(ListNode *list, int index)
{
   int idx = 0;
   ListNode *temp = list;
   ListNode *removed;
   void *rm_value;
   while (idx < index - 1)
   {
      if (temp == NULL || temp->next == NULL)
      {
         printError("Index out of bounds");
      }
      temp = temp->next;
      idx++;
   }
   if (temp == NULL)
   {
      printError("Index out of bounds");
   }
   else if (temp->next == NULL && index == 0)
   {
      free(list);
      list = NULL;
      return list;
   }
   removed = temp->next;
   temp->next = removed->next;
   rm_value = removed->value;
   free(removed);
   return rm_value;
}

void* getValue(ListNode *list, int index)
{
   int idx = 0;
   ListNode *temp = list;
   while (idx < index)
   {
      if (temp == NULL || temp->next == NULL)
      {
         printError("Index out of bounds");
      }
      temp = temp->next;
      idx++;
   }
   if (temp == NULL)
   {
      printError("Index out of bounds");
   }
   return temp->value;
}


















