#ifndef QDTRANS_PRINTTREE
#define QDTRANS_PRINTTREE
#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

void printTree(struct treeNode* node, CXTranslationUnit cxtup);

#endif /* QDTRANS_PRINTTREE */
