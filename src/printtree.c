#include <clang-c/Index.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "printer.h"

struct treeNode* tree;

int main(int argc, char *argv[]) {
  
    if(argc == 2) {
        filename = argv[1];
    } else {
      //filename = "Test1.c";
      printUsage();
      goto END;
    }
  
    /*filefile = fopen(filename, "r+");    
    
    CXIndex cxi = clang_createIndex(1, 0);
    unsigned flags = (CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_Incomplete);
    //const char* filename = "/home/redhotsmasher/QDTrans/src/Test1.c";
    //CXTranslationUnit cxtu = clang_createTranslationUnitFromSourceFile (cxi, filename, 0, NULL, 0, NULL);
    CXTranslationUnit cxtup = clang_createTranslationUnit(cxi, filename);
    enum CXErrorCode error = clang_parseTranslationUnit2(cxi, filename, NULL, 0, NULL, 0, flags, &cxtup);
    file = clang_getFile (cxtup, filename);
    CXCursor cursor = clang_getTranslationUnitCursor(cxtup);
    tree = malloc(sizeof(struct treeNode));
    tree->cursor = cursor;
    currentnode = tree;
    clang_visitChildren(cursor, visitor, NULL);
    visitRecursive(tree->children);
  /*d1nodelist = malloc(sizeof(struct treeNode));
    d1nodelist->children = NULL;
    d1nodelist->childCount = 0;*/
  /*scanTree(tree, cxtup);
    //printf("d1nodelist size: %i\n", d1nodelist->childCount);*/
    struct nodeTree* tree = generateTree(filename);
    printTree(tree);
    //printTree(d1nodelist, cxtup, 0);
    //disposeTree(d1nodelist);
    disposeTree(tree);
    printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", tree->error, tree->nodes, tree->unmodifiedDepth);
END:
    return 0;
}

int printUsage() {
    printf("USAGE:\n");
    printf("\n");
    printf("\tprinttree [FILENAME]\n");
}
