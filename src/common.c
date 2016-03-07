#include <clang-c/Index.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int visitcounter = 0;

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

struct nodeTree {
    struct treeNode* root;
    CXIndex cxi;
    CXTranslationUnit cxtup;
    CXFile file;
    FILE* filefile;
    enum CXErrorCode error;
    int nodes;
    int unmodifiedDepth;
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

int nodes = 0;
int depth = -1;

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
    newnode->modifiedNodes = NULL;
    unsigned sline;
    unsigned scolumn;
    newnode->startline = -1;
    newnode->startcol = -1;
    newnode->validcursor = true;
    //printf("vN: %i\n", currentnode);
    //if((currentnode != NULL) && ())
    //visitcounter++;
    addChild(currentnode, newnode);
    return CXChildVisit_Continue;
}

enum CXChildVisitResult (*visitor)(CXCursor, CXCursor, CXClientData) = &visit;

void visitRecursive(struct treeListNode* node, struct nodeTree* tree) {
    if(node != NULL) {
        depth++;
	if(depth > tree->unmodifiedDepth) {
	    tree->unmodifiedDepth = depth;
	}
	struct treeListNode* nodeToVisit = node;
        while(nodeToVisit != NULL) {
	    currentnode = nodeToVisit->node;
	    //printf("vRN: %i\n", currentnode);
	    tree->nodes = tree->nodes + currentnode->childCount;
	    clang_visitChildren(nodeToVisit->node->cursor, visit, NULL);
	    visitRecursive(nodeToVisit->node->children, tree);
	    nodeToVisit = nodeToVisit->next;
	}
	depth--;
    }
}

struct nodeTree* generateTree(char* filename) {
    printf("%s\n", filename);
    filefile = fopen(filename, "r+");
    struct nodeTree* ntree = malloc(sizeof(struct nodeTree));
    struct treeNode* thetree = malloc(sizeof(struct treeNode));
    ntree->filefile = filefile;
    ntree->root = thetree;
    CXIndex cxix = clang_createIndex(1, 0);
    ntree->cxi = cxix;
    printf("cxix: %lx, cxi: %lx\n", cxix, ntree->cxi);
    unsigned flags = (CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_Incomplete);
    CXTranslationUnit cxtu = clang_createTranslationUnit(cxix, filename);
    ntree->cxtup = cxtu;
    ntree->cxi = cxix;
    printf("qdtranscxtup: %lx\n", ntree->cxtup);
    printf("cxix: %lx, cxi: %lx, cxtu: %lx, cxtup: %lx\n", cxix, ntree->cxi, cxtu, ntree->cxtup);
    printf("qdtranscxtup: %lx\n", ntree->cxtup);
    ntree->error = clang_parseTranslationUnit2(cxix, filename, NULL, 0, NULL, 0, flags, &cxtu);
    ntree->cxi = cxix;
    ntree->cxtup = cxtu;
    printf("cxix: %lx, cxi: %lx, cxtu: %lx, cxtup: %lx\n", cxix, ntree->cxi, cxtu, ntree->cxtup);
    printf("qdtranscxtup: %lx\n", ntree->cxtup);
    file = clang_getFile (cxtu, filename);
    ntree->file = file;
    CXCursor cursor = clang_getTranslationUnitCursor(cxtu);
    thetree->cursor = cursor;
    thetree->modified = 0;
    thetree->validcursor = true;
    thetree->parent = NULL;
    thetree->modifiedNodes = NULL;
    thetree->startline = -1;
    thetree->startcol = -1;
    thetree->children = NULL;
    thetree->newContent = NULL;
    currentnode = thetree;
    clang_visitChildren(cursor, visitor, NULL);
    visitRecursive(thetree->children, ntree);
    printf("Visits: %dx", visitcounter);
    return(ntree);
}

void disposeTree2(struct treeNode* node);

void disposeTree(struct nodeTree* tree) {
    disposeTree2(tree->root);
    clang_disposeTranslationUnit(tree->cxtup);
    clang_disposeIndex(tree->cxi);
    fclose(tree->filefile);
    free(tree);
}

void disposeTree2(struct treeNode* node) {
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
	    disposeTree2(current->node);
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
