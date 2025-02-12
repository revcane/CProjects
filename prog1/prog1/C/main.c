#include "BST.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    char word[50];
    struct BST store;
    createBST(&store);

    // if (store == NULL) {
    //     perror("Failed to allocate the BST\n");
    //     exit(EXIT_FAILURE);
    // }

    printf("Please enter a series of words to store in a BST\n");
    printf("Type 'STOP' to end the series and output the BST\n");
    printf("In post order\n");
    
    while(1)
    {
        printf("Word = ");
        scanf("%49s", word);
        if (strcmp(word, "STOP") == 0) 
            break;

        if (!insert(&store.root, word)) {
            printf("Failed to insert '%s'. It might already exist.\n", word);
        }
    }
    printf("The BST ouput in postorder is ");
    postOrder(store.root);
    printf("\n");

  destructor(&store);
  return 0;
}