#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct treeListNode;

struct treeNode {
    CXCursor cursor;
    int childCount; // The number of children this node has.
    struct treeListNode* children;
    bool modified;
    char* newContent;
};

struct treeListNode {
    struct treeNode* node;
    struct treeListNode* next;
};

CXFile* file;
unsigned* curline;
unsigned* parline;
unsigned* curline2;
unsigned* parline2;
unsigned* curcol;
unsigned* parcol;
unsigned* curcol2;
unsigned* parcol2;

char* space = "  ";

FILE* filefile;

const char* filename;

int nodes = 0;
int depth = -1;

int maxdepth;

struct treeNode* currentnode;

void addChild(struct treeNode* node, struct treeNode* child) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    //printf("Add %i to %i.\n", child, currnode);
    if(currnode != NULL) {
      //printf("currnode != NULL\n"); 
      //printf("%u\n", currnode->next);
        while(currnode->next != NULL) {
	    currnode = currnode->next;
	}
	struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	newnode->node = child;
	newnode->next = NULL;
	currnode->next = newnode;
    } else {
      //printf("currnode == NULL\n");
        struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	newnode->node = child;
	newnode->next = NULL;
	node->children = newnode;
    }
}

void addChildAfter(struct treeNode* node, struct treeNode* child, struct treeNode* after) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    while(currnode->next != NULL && (clang_equalCursors(currnode->node->cursor, after->cursor) == 0)) {
        currnode = currnode->next;
    }
    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
    newnode->node = child;
    newnode->next = currnode->next;
    currnode->next = newnode;
} 

enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    nodes++;
    struct treeNode* newnode = malloc(sizeof(struct treeNode));
    newnode->cursor = cursor;
    newnode->children = NULL;
    newnode->childCount = 0;
    //printf("vN: %i\n", currentnode);
    //if((currentnode != NULL) && ())
    addChild(currentnode, newnode);
    return CXChildVisit_Continue;
}

void visitRecursive(struct treeListNode* node) {
    if(node != NULL) {
        depth++;
	if(depth > maxdepth) {
	    maxdepth = depth;
	}
	struct treeListNode* nodeToVisit = node;
        while(nodeToVisit != NULL) {
	    currentnode = nodeToVisit->node;
	    //printf("vRN: %i\n", currentnode);
	    clang_visitChildren(nodeToVisit->node->cursor, visit, NULL);
	    visitRecursive(nodeToVisit->node->children);
	    nodeToVisit = nodeToVisit->next;
	}
	depth--;
    }
}

int printUsage() {
    printf("USAGE:\n");
    printf("\n");
    printf("\tdumptree [FILENAME]\n");
}
