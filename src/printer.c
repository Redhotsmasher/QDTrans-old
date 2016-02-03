#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

struct treeNode* d1nodelist;

char* lastPrintedToken;

int prevline = 0;
int prevcol = 0;

int nodenum = 0;

void printTreeRecursive(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    CXSourceRange range = clang_getCursorExtent(node->cursor);
    CXSourceLocation rstart = clang_getRangeStart(range);
    if(clang_Location_isFromMainFile(rstart) != 0) {
	enum CXCursorKind cursorkind = clang_getCursorKind(node->cursor);
        if(depth == 1 && cursorkind != CXCursor_MacroExpansion) {
	  nodenum++;
	  if(cursorkind == CXCursor_StructDecl) {
	        CXToken* currTokens;
		unsigned int numCurrTokens;
	        clang_tokenize(cxtup, range, &currTokens, &numCurrTokens);
		CXToken token = currTokens[numCurrTokens-1];
	        CXString tokenstring = clang_getTokenSpelling(cxtup, token);
		char* tokenstr = clang_getCString(tokenstring);
		clang_disposeTokens(cxtup, currTokens, numCurrTokens);
		//printf("\ntoken: %s\n", tokenstr);
		if(strcmp(tokenstr, ";")) {
		    clang_disposeString(tokenstring);
		    // Print nothing, do nothing.
		} else {
		    clang_disposeString(tokenstring);
		    goto PRINT;
		}
	    } else {
	    PRINT:
	        1+1;
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
		clang_disposeString(tokenstring);  
		clang_disposeTokens(cxtup, tokens, numTokens);
		free(startcol);
		free(startline);
		free(endcol);
		free(endline);
	    }
        }
    }
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    printTreeRecursive(childlist->node, cxtup);
	    childlist = childlist->next;
	}
    }
    depth--;
}

/*
void scanTree(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    CXSourceRange range = clang_getCursorExtent(node->cursor);
    CXSourceLocation rstart = clang_getRangeStart(range);
    if(clang_Location_isFromMainFile(rstart) != 0) {
        enum CXCursorKind cursorkind = clang_getCursorKind(node->cursor);
        if(depth == 1 && cursorkind != CXCursor_MacroExpansion) {
	    CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
	    char* str = clang_getCString(cdisplaystring);
	    //printf("Scanning: %s\n", str);
	    clang_disposeString(cdisplaystring);
	    struct treeNode* newnode = malloc(sizeof(struct treeNode));
	    newnode->childCount = 0;
	    newnode->children = NULL;
	    newnode->cursor = node->cursor;
	    addChild(d1nodelist, newnode);
        }
    }
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    scanTree(childlist->node, cxtup);
	    childlist = childlist->next;
	}
    }
    depth--;
}
*/

void printTree(struct treeNode* node, CXTranslationUnit cxtup) {
    lastPrintedToken = malloc(500*sizeof(char));
    strcpy(lastPrintedToken, " ");
    printTreeRecursive(node, cxtup);
    free(lastPrintedToken);
}
