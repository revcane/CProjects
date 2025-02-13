#ifndef BST_H
#define BST_H

#include <stdio.h>
#include <string.h>
#include <malloc.h>

struct Node{
    char value[50];
    struct Node *left;
    struct Node *right;
};

struct BST{
     struct Node *root;
};

//node functions
struct Node* makeNode(const char *value);
int insert(struct Node **node, const char *value);
int search(struct Node *node, const char *value);
int removeNode(struct Node **node, const char *value);
void copyNode(struct Node *node, struct Node **bstCp);
void dealloc(struct Node **node);

//bst functions
void createBST(struct BST *tree);
void destructor(struct BST *tree);
void copyBST(struct BST *src, struct BST *dest);
void postOrder(struct Node *node);

#endif 