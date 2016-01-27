#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

char* kinds[701];

struct treeNode* tree;

struct treeListNode* d1nodelist;

char* lastPrintedToken;

int prevline = 0;
int prevcol = 0;

void printTree(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    CXSourceRange range = clang_getCursorExtent(node->cursor);
    CXSourceLocation rstart = clang_getRangeStart(range);
    if(clang_Location_isFromMainFile(rstart) != 0) {
        if(depth == 1) {
	  /*if(node->childCount != 0) {
	        struct treeListNode* currnode = node->children;
		while(currnode->next != NULL) {
		    currnode = currnode->next;
		}
		CXSourceLocation tokenizeStart = clang_getRangeStart(range);
		CXSourceRange temp = clang_getCursorExtent(currnode->node->cursor);
		CXSourceLocation tokenizeEnd = clang_getRangeEnd(temp);
		range = clang_getRange (tokenizeStart, tokenizeEnd);
	    }*/
	    CXToken* tokens;
	    unsigned int numTokens;
	    clang_tokenize(cxtup, range, &tokens, &numTokens);
	    CXString tokenstring;
	    CXSourceRange tokenrange;
	    unsigned* startcol = malloc(sizeof(unsigned));
	    unsigned* startline = malloc(sizeof(unsigned));
	    unsigned* endcol = malloc(sizeof(unsigned));
	    unsigned* endline = malloc(sizeof(unsigned));
	    for(int i = 0; i<numTokens; i++) {
	        tokenrange = clang_getTokenExtent(cxtup, tokens[i]);
	        tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
		CXSourceLocation currend = clang_getRangeEnd(tokenrange);
		clang_getFileLocation(currend, NULL, endline, endcol, NULL);
		if(prevcol != 0) {
		    CXSourceLocation currstart = clang_getRangeStart(tokenrange);
		    clang_getFileLocation(currstart, NULL, startline, startcol, NULL);
		    //printf("L%u-%u, C%u-%u", *startline, prevline, *startcol, prevcol);
                    int startl = *startline;
		    int startc = *startcol;
		    for(int i = 0; i < startl-prevline; i++) {
		      printf("\n");
		      //printf("*startline-prevline = %u, i = %u, prevline = %u\n", start-prevline, i, prevline);
		    }
		    if(startc-prevcol >= 0) {
		        for(int i = 0; i < startc-prevcol; i++) {
			    printf(" ");
			}
		    } else {
      		        for(int i = 1; i < startc; i++) {
			    printf(" ");
			}
		    }
		}
		char* tstr = clang_getCString(tokenstring);
		if(i == 0 && !strcmp(tstr, lastPrintedToken)) {	    
		    // Do nothing, print nothing.
		} else {
		    printf("%s", tstr);
		}
		prevline = *endline;
		prevcol = *endcol;
		}
	    //printf("%i\n", lastPrintedToken);
	    //printf("\n");
   	    char* str = clang_getCString(tokenstring);
      	    strcpy(lastPrintedToken, str);
	    clang_disposeTokens(cxtup, tokens, numTokens);
    	    free(startcol);
	    free(startline);
	    free(endcol);
	    free(endline);
        }
    }
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    printTree(childlist->node, cxtup);
	    childlist = childlist->next;
	}
    }
    depth--;
}


int main(int argc, char *argv[]) {
  
    if(argc == 2) {
        filename = argv[1];
    } else {
        filename = "Test1.c";
      //printUsage();
      //goto END;
    }
  
    filefile = fopen(filename, "r+");    
    
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
    lastPrintedToken = malloc(500*sizeof(char));
    strcpy(lastPrintedToken, " ");
    printTree(tree, cxtup);
    free(lastPrintedToken);
    clang_disposeTranslationUnit(cxtup);
    clang_disposeIndex(cxi);
    printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", error, nodes, maxdepth);
END:
    return 0;
}
