#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"
#include "printer.h"

struct treeNode* thetree;

int moddednodes = 0;

int tnodes = 0;

void modifyTree(struct treeNode* node, CXTranslationUnit cxtup);

void debugTree(struct treeNode* node, CXTranslationUnit cxtup);

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
    thetree->modified = 0;
    thetree->validcursor = true;
    thetree->parent = NULL;
    thetree->modifiedNodes = NULL;
    thetree->startline = -1;
    thetree->startcol = -1;
    currentnode = thetree;
    clang_visitChildren(cursor, visitor, NULL);
    visitRecursive(thetree->children);
    modifyTree(thetree, cxtup);
    printTree(thetree, cxtup);
    disposeTree(thetree);
    clang_disposeTranslationUnit(cxtup);
    clang_disposeIndex(cxi);
    printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", error, nodes, maxdepth);
END:
    return 0;
}

void modifyTree(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    if(node->modified == 0) {
        CXSourceRange range = clang_getCursorExtent(node->cursor);
        CXSourceLocation rstart = clang_getRangeStart(range);
        if(clang_Location_isFromMainFile(rstart) != 0) {
	    enum CXCursorKind cursorkind = clang_getCursorKind(node->cursor);
	    if(cursorkind == CXCursor_CallExpr) {
	      //printf("Node is a callexpr!\n");
	        CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
		char* str = clang_getCString(cdisplaystring);
	      //printf("Node's CursorDisplayName is \"%s\"\n", str);
		bool start = (strcmp(str, &("pthread_mutex_lock")) == 0);
		bool end = (strcmp(str, &("pthread_mutex_unlock")) == 0);
	      //printf("%i, %i\n", start, end);
		if(start || end) {
		    struct treeNode* newnode = malloc(sizeof(struct treeNode));
		    newnode->validcursor = false;
		    newnode->childCount = 0;
		    newnode->children = NULL;
		    newnode->modified = 1;
		    CXSourceLocation endloc = clang_getRangeEnd(range);
		    unsigned eline;
		    unsigned ecolumn;
		    clang_getFileLocation(endloc, NULL, &eline, &ecolumn, NULL);
		    //printf("node: %i, eline: %i, ecolumn: %i\n", newnode, eline, ecolumn);
		    newnode->startline = eline;
		    newnode->startcol = ecolumn;
		    if(start) {
		        newnode->newContent = malloc(40*sizeof(char));
			strcpy(newnode->newContent, &"\n    printf(\"Critical section start!\")");
		    } else {
		        newnode->newContent = malloc(38*sizeof(char));
			strcpy(newnode->newContent, &"\n    printf(\"Critical section end!\")");
		    }
		    //newnode->parent = node->parent;
		    addChildAfter(node->parent, newnode, node);
		}
		clang_disposeString(cdisplaystring);
	    }
	}
    } 
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    modifyTree(childlist->node, cxtup);
	    childlist = childlist->next;
	}
    }
    depth--;
}

char* space = "  ";

void debugTree(struct treeNode* node, CXTranslationUnit cxtup) {
    tnodes++;
    depth++;
    if(node->validcursor == true) {
        CXType type = clang_getCursorType(node->cursor);
	CXString typestring = clang_getTypeSpelling(type);
	CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
	CXString cspellstring = clang_getCursorSpelling(node->cursor);
	unsigned* curlines = malloc(sizeof(unsigned));
	unsigned* curlinee = malloc(sizeof(unsigned));
	unsigned* curcols = malloc(sizeof(unsigned));
	unsigned* curcole = malloc(sizeof(unsigned));
	CXSourceRange range = clang_getCursorExtent(node->cursor);
	CXSourceLocation rstart = clang_getRangeStart(range);
	CXSourceLocation rend = clang_getRangeEnd(range);
	clang_getFileLocation (rstart, NULL, curlines, curcols, NULL);
	clang_getFileLocation (rend, NULL, curlinee, curcole, NULL);
	enum CXCursorKind cursorkind = clang_getCursorKind(node->cursor);
	char* str2 = clang_getCString(typestring);
	char* str3 = clang_getCString(cdisplaystring);
	char* str4 = clang_getCString(cspellstring);
	if(clang_Location_isFromMainFile(rstart) != 0) {
	    for(int i = 0; i < depth; i++) {
	        printf("%s", space);
		//printf("%i", depth);
	    }
            printf("%i, [%i]:%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing ", node->modified, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
	    CXToken* tokens;
	    unsigned int numTokens;
	    clang_tokenize(cxtup, range, &tokens, &numTokens);
	    printf("%u tokens (", numTokens);
	    for(int i = 0; i<numTokens; i++) {
	        CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
	        printf("%s ", clang_getCString(tokenstring));
	    }
	    clang_disposeTokens(cxtup, tokens, numTokens);
	    printf(")");
	    if(node->children != NULL) {
	        if(clang_Location_isFromMainFile(rstart) != 0) {
	            printf("->\n");
		}
	        struct treeListNode* childlist = node->children;
	        while(childlist != NULL) {
	            debugTree(childlist->node, cxtup);
		    childlist = childlist->next;
		}
	    } else {
	        if(clang_Location_isFromMainFile(rstart) != 0) {
	            printf("\n");
		}
	    }
	}
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
    } else {
        tnodes++;
        moddednodes++;
        for(int i = 0; i < depth; i++) {
	    printf("%s", space);
	    //printf("%i", depth);
	}
	printf("%i, %s", node->modified, node->newContent);
	if(node->children != NULL) {
	    printf("->\n");
	    struct treeListNode* childlist = node->children;
	    while(childlist != NULL) {
	        debugTree(childlist->node, cxtup);
		childlist = childlist->next;
	    }
	} else {
	    printf("\n");
	}
    }
    depth--;
}

int printQDUsage() {
    printf("USAGE:\n");
    printf("\n");
    printf("\tqdtrans [FILENAME]\n");
}
