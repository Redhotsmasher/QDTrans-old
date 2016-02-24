#ifndef QDTRANS_COMMON
#define QDTRANS_COMMON
#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct treeListNode;

struct treeNode;

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

int nodes;
int depth;

int maxdepth;

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

struct treeNode* currentnode;

void addChild(struct treeNode* node, struct treeNode* child);

void addChildAfter(struct treeNode* node, struct treeNode* child, struct treeNode* after);

struct treeNode* getChild(struct treeNode* node, int childNum);
  
enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData client_data);

void visitRecursive(struct treeListNode* node);

enum CXChildVisitResult (*visitor)(CXCursor, CXCursor, CXClientData);

void disposeTree(struct treeNode* node);

#endif /* QDTRANS_COMMON */
