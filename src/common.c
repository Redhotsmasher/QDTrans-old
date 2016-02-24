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
    int modified;
    char* newContent;
    struct treeNode* parent;
    struct treeListNode* modifiedNodes;
    int startline;
    int startcol;
    bool validcursor;
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

FILE* filefile;

const char* filename;

int nodes = 0;
int depth = -1;

int maxdepth;

struct treeNode* currentnode;

void addModified(struct treeNode* node, struct treeNode* modified) {
    struct treeListNode* currnode = node->modifiedNodes;
    //printf("Add %i to %i.\n", child, currnode);
    if(currnode != NULL) {
      //printf("currnode != NULL\n"); 
      //printf("%u\n", currnode->next);
        while(currnode->next != NULL) {
	    currnode = currnode->next;
	}
	struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	if(modified->modified == 0) {
	    printf("Assertion failed: modified node has modified == 0!");
	}
	newnode->node = modified;
	newnode->next = NULL;
	currnode->next = newnode;
    } else {
      //printf("currnode == NULL\n");
        struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	if(modified->modified == 0) {
	    printf("Assertion failed: modified node has modified == 0!");
	}
	newnode->node = modified;
	newnode->next = NULL;
	node->modifiedNodes = newnode;
    }
}

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
	child->parent = node;
    } else {
      //printf("currnode == NULL\n");
        struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	newnode->node = child;
	newnode->next = NULL;
	node->children = newnode;
	child->parent = node;
    }
}

/*
 * Only works for unmodified nodes and modified nodes, not nodes which have modified children!
 */
void addChildAfter(struct treeNode* node, struct treeNode* child, struct treeNode* after) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    while(currnode->next != NULL && (clang_equalCursors(currnode->node->cursor, after->cursor) == 0)) {
        currnode = currnode->next;
    }
    if(child->modified != 0) {
        node->modified++;
        struct treeNode* currnode2 = node;
        while(currnode2->parent != NULL) {
	    currnode2 = currnode2->parent;
	    currnode2->modified++;
	    addModified(currnode2, child);
	}
	currnode2->modified++;
	addModified(currnode2, child);
    }
    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
    newnode->node = child;
    newnode->next = currnode->next;
    currnode->next = newnode;
    child->parent = node;
    CXSourceRange range = clang_getCursorExtent(currnode->node->cursor);
    CXSourceLocation rstart = clang_getRangeEnd(range);
    clang_getFileLocation(rstart, NULL, &(child->startline), &(child->startcol), NULL);
}

void addChildBefore(struct treeNode* node, struct treeNode* child, struct treeNode* before) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    while(currnode->next != NULL && (clang_equalCursors(currnode->next->node->cursor, before->cursor) == 0)) {
        currnode = currnode->next;
    }
    if(child->modified != 0) {
        node->modified++;
        struct treeNode* currnode2 = node;
        while(currnode2->parent != NULL) {
	    currnode2 = currnode2->parent;
	    currnode2->modified++;
	    addModified(currnode2, child);
	}
	currnode2->modified++;
	addModified(currnode2, child);
    }
    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
    newnode->node = child;
    newnode->next = currnode->next->next;
    currnode->next->next = newnode;
    child->parent = node;
    CXSourceRange range = clang_getCursorExtent(currnode->node->cursor);
    CXSourceLocation rend = clang_getRangeEnd(range);
    clang_getFileLocation(rend, NULL, &(child->startline), &(child->startcol), NULL);
}

struct treeNode* getChild(struct treeNode* node, int childNum) {
    if(childNum <= node->childCount) {
        struct treeListNode* currnode = node->children;
	for(int i = 1; i < childNum; i++) {
	    currnode = currnode->next;
	}
	//printf("returning %i->node\n", currnode);
	return currnode->node;
    } else {
        return NULL;
    }
}

enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    nodes++;
    struct treeNode* newnode = malloc(sizeof(struct treeNode));
    newnode->cursor = cursor;
    newnode->children = NULL;
    newnode->childCount = 0;
    newnode->modified = 0;
    newnode->newContent = NULL;
    unsigned sline;
    unsigned scolumn;
    newnode->startline = -1;
    newnode->startcol = -1;
    newnode->validcursor = true;
    //printf("vN: %i\n", currentnode);
    //if((currentnode != NULL) && ())
    addChild(currentnode, newnode);
    return CXChildVisit_Continue;
}

enum CXChildVisitResult (*visitor)(CXCursor, CXCursor, CXClientData) = &visit;

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

void disposeTree(struct treeNode* node) {
    if(node->newContent != NULL) {
        free(node->newContent);
    }
    if(node->childCount == 0) {
        free(node);
    } else {
        struct treeListNode* current = node->children;
	struct treeListNode* next = current;
	while(next != NULL) {
	    current = next;
	    next = current->next;
	    disposeTree(current->node);
	    free(current);
	}
	if(node->modified != 0) {
	    current = node->modifiedNodes;
	    next = current;
	    while(next != NULL) {
	        current = next;
		next = current->next;
		free(current);
	    }
	}
	free(node);
    }
}
