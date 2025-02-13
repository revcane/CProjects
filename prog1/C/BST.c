#include "BST.h"

struct Node* makeNode(const char *value){
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (!newNode) {
        return NULL;
    }
    strncpy(newNode->value, value, sizeof(newNode->value));
    newNode->value[sizeof(newNode->value) - 1] = '\0';
    
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}
int insert(struct Node **node, const char *value) {
    if (*node == NULL) {
        *node = makeNode(value);
        return (*node != NULL); 
    }
    if (strcmp(value, (*node)->value) < 0) {
        return insert(&(*node)->left, value);
    }
    if (strcmp(value, (*node)->value) > 0) {
        return insert(&(*node)->right, value);
    }
    return 0;
}
int search(struct Node *node, const char *value){
if (node == NULL) return 0;
if (strcmp(value, node->value) < 0) 
    return search(node->left,value);
if (strcmp(value, node->value) > 0) 
    return search(node->right,value);
return 1;
}

int removeNode(struct Node **node, const char *value){
    return 1;
}

void copyNode(struct Node *node, struct Node **bstCp){
    if (node == NULL) return;
    insert(bstCp, node->value);
    copyNode(node->left, bstCp);
    copyNode(node->right, bstCp);
}

void dealloc(struct Node **node) {
    if (*node == NULL) return;
    dealloc(&(*node)->left);
    dealloc(&(*node)->right);
    free(*node); 
    *node = NULL;
}


void createBST(struct BST *tree) {
    tree->root = NULL;
}
void destructor(struct BST *tree) {
    dealloc(&tree->root);
}
void copyBST(struct BST *src, struct BST *dest){
    if (src == NULL || dest == NULL) return;
    copyNode(src->root, &dest->root);
}

void postOrder(struct Node *node){
    if (node == NULL) return;
    postOrder(node->left);
    postOrder(node->right);
    printf("%s ", node->value);
}

