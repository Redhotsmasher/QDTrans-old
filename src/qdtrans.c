#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "printer.h"

struct treeNode* thetree;

int main(int argc, char *argv[]) {
  
    if(argc == 2) {
        filename = argv[1];
    } else {
      //filename = "Test1.c";
      printQDUsage();
      goto END;
    }
  
    filefile = fopen(filename, "r+");    
    
    CXIndex cxi = clang_createIndex(1, 0);
    unsigned flags = (CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_Incomplete);
    CXTranslationUnit cxtup = clang_createTranslationUnit(cxi, filename);
    enum CXErrorCode error = clang_parseTranslationUnit2(cxi, filename, NULL, 0, NULL, 0, flags, &cxtup);
    file = clang_getFile (cxtup, filename);
    CXCursor cursor = clang_getTranslationUnitCursor(cxtup);
    thetree = malloc(sizeof(struct treeNode));
    thetree->cursor = cursor;
    currentnode = thetree;
    clang_visitChildren(cursor, visitor, NULL);
    visitRecursive(thetree->children);
    printTree(thetree, cxtup);
    disposeTree(thetree);
    clang_disposeTranslationUnit(cxtup);
    clang_disposeIndex(cxi);
    printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", error, nodes, maxdepth);
END:
    return 0;
}

int printQDUsage() {
    printf("USAGE:\n");
    printf("\n");
    printf("\tqdtrans [FILENAME]\n");
}
