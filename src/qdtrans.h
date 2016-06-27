#ifndef QDTRANS_QDTRANS
#define QDTRANS_QDTRANS
#include <clang-c/Index.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"
#include "printer.h"

struct variable {
    char* name;
    char* typename;
    bool pointer;
    bool threadLocal;
  //struct treeNode* decl;
    struct variable* next;
};

struct criticalSection {
    bool needsRefactoring;
    bool needsWait;
    struct treeNode* lockvarNode;
  //struct treeNode* lockNode;
  //struct treeNode* unlockNode;
    struct variable* accessedvars;
  //struct treeListNode* returnstmts;
    struct treeListNode* nodelist;
    struct treeListNode* nodesafter;
    struct criticalSection* next;
};

bool first;

int moddednodes;
int tnodes;
int critCount;
int moddepth;

void modifyTree(struct treeNode* node, CXTranslationUnit cxtup);
void debugTree(struct treeNode* node, CXTranslationUnit cxtup);
void debugTree2(struct treeNode* node, CXTranslationUnit cxtup);
void debugNode(struct treeNode* node, CXTranslationUnit cxtup);
void debugCrits(struct criticalSection* crits, CXTranslationUnit cxtup);
void printCrit(struct criticalSection* crit, CXTranslationUnit cxtup);
void refactorCrits(struct treeNode* node, CXTranslationUnit cxtup);
void scanCrits(CXTranslationUnit cxtup);
void scanCrit(struct criticalSection* crit, CXTranslationUnit cxtup);
void scanCritRecursive(struct criticalSection* crit, struct treeNode* node, CXTranslationUnit cxtup, bool isreturn);
void findCrits(struct treeNode* node, CXTranslationUnit cxtup);
void scanTree(struct treeNode* node, CXTranslationUnit cxtup);
void freeCrits(struct criticalSection* crit);
void printQDUsage();

struct nodeTree* tree;

struct criticalSection* crits;

#endif /* QDTRANS_QDTRANS */
