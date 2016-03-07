#include <clang-c/Index.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "common.h"

struct treeNode* d1nodelist;

char* lastPrintedToken;

int prevline = 0;
int prevcol = 0;

int nodenum = 0;

void printTreeRecursive(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    if(node->modified == 0) {
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
			/*if(i == 0 && !strcmp(tstr, lastPrintedToken)) {	    
			    // Do nothing, print nothing.
			    } else {*/
			    printf("%s", tstr);
			    //}
			prevline = *endline;
			prevcol = *endcol;
			char* str = clang_getCString(tokenstring);
			strcpy(lastPrintedToken, str);
			clang_disposeString(tokenstring);
		    }
		    //printf("%i\n", lastPrintedToken);
		    //printf("\n");
		    clang_disposeTokens(cxtup, tokens, numTokens);
		    free(startcol);
		    free(startline);
		    free(endcol);
		    free(endline);
		}
	    }
	}
    } else if(depth == 1) {
      //printf("Modified node detected.");
        if(node->validcursor == false) {
	    //Print modified
	    //printf("%s\n", node->newContent);
        } else {
	    struct treeNode** nodes = malloc((node->modified) * sizeof(struct treeNode));
	    //printf("%i\n", sizeof(nodes))
	    nodes[node->modified-1] = NULL;
	    //printf("nodes[node->modified-1]: %i\n", nodes[node->modified-1]);
	    struct treeNode* next = NULL;
	    int nextnode = 0;
	    int smallestcol = INT_MAX;
	    int smallestline = INT_MAX;
	    int cscol = INT_MIN;
	    int csline = INT_MIN;
	    struct treeListNode* currnode = node->modifiedNodes;
	    CXSourceRange srange;
	    CXSourceLocation sloc;
	    bool first = true;
	    while(nodes[(node->modified)-1] == NULL) {
	        while(currnode != NULL) {
		    if(currnode->node->startline == -1) {
		        srange = clang_getCursorExtent(currnode->node->cursor);
			sloc = clang_getRangeStart(srange);
			clang_getFileLocation(sloc, NULL, &(currnode->node->startline), &(currnode->node->startcol), NULL);
			//printf("Calculated node %i: startline: %i, startcol: %i\n", currnode->node, currnode->node->startline, currnode->node->startcol);
		    }
		    //printf("\ncurrnode->node: %i, nextnode: %i\nnstartline: %i, smallestline: %i ,csline: %i\nnstartcol: %i, smallestcol: %i, cscol: %i\n", currnode->node, nextnode, currnode->node->startline, smallestline, csline, currnode->node->startcol, smallestcol, cscol);
		    if(currnode->node->startline < smallestline) {
		      //printf("\n1\, %i < %i", currnode->node->startline, smallestline);
		        if((currnode->node->startline > csline) || (first == true)) {
			  //printf("\n2, %i > %i", currnode->node->startline);
			    smallestline = currnode->node->startline;
			    smallestcol = currnode->node->startcol;
			    csline = currnode->node->startline;
			    cscol = currnode->node->startcol;
			    next = currnode->node;
			}
		    } else if(currnode->node->startline == smallestline) {
		      //printf("3\n");
		        if(currnode->node->startcol < smallestcol) {
			  //printf("4\n");
			    if((currnode->node->startcol > cscol) || (first == true)) {
			      //printf("5\n");
			        smallestline = currnode->node->startline;
				smallestcol = currnode->node->startcol;
				csline = currnode->node->startline;
				cscol = currnode->node->startcol;
				next = currnode->node;
			    }
			}
		    }
		    //printf("\n6\n");
		    //printf("currnode->node: %i, nextnode: %i\nnstartline: %i, smallestline: %i ,csline: %i\nnstartcol: %i, smallestcol: %i, cscol: %i\n", currnode->node, nextnode, currnode->node->startline, smallestline, csline, currnode->node->startcol, smallestcol, cscol);
		    currnode = currnode->next;
		}
		first = false;
		//printf("%i ", nodes[nextnode]);
		//printf("%i\n", next);
		nodes[nextnode] = next;
		nextnode++;
		smallestcol = INT_MAX;
		smallestline = INT_MAX;
		currnode = node->modifiedNodes;
	    }
	    for(int i = 0; i <= 1; i++) {
	      //printf("nodenum: %i, nstartline: %i, nstartcol: %i\n", i, nodes[i]->startline, nodes[i]->startcol);
	    }
	    CXSourceRange range = clang_getCursorExtent(node->cursor);
	    CXToken* tokens;
	    unsigned int numTokens;
	    int numNodes = nextnode;
	    nextnode = 0;
	    clang_tokenize(cxtup, range, &tokens, &numTokens);
	    for(int i = 0; i<numTokens; i++) {
	        CXSourceRange tokenrange = clang_getTokenExtent(cxtup, tokens[i]);
	        CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
		unsigned startline;
		unsigned startcol;
		unsigned endline;
		unsigned endcol;
	        CXSourceLocation currend = clang_getRangeEnd(tokenrange);
	        clang_getFileLocation(currend, NULL, &endline, &endcol, NULL);
	        if(prevcol != 0) {
		    CXSourceLocation currstart = clang_getRangeStart(tokenrange);
		    clang_getFileLocation(currstart, NULL, &startline, &startcol, NULL);
		    //printf("L%u-%u, C%u-%u", *startline, prevline, *startcol, prevcol);
		    int startl = startline;
		    int startc = startcol;
		    for(int i = 0; i < startl-prevline; i++) {
		        printf("\n");
		        //printf("*startline-prevline = %u, i = %u, prevline = %u\n", start-prevline, i, prevline);
		    }
		    if(startc-prevcol >= 0) {
		        for(int i = 0; i < startc-prevcol; i++) {
			    printf(" ");
		        }
		    } else {
		        for(int i = 0; i < startc; i++) {
			printf(" ");
			}
		    }
		}
		char* tstr = clang_getCString(tokenstring);
		if(i == 0 && !strcmp(tstr, lastPrintedToken)) {	    
		    // Do nothing, print nothing.
		} else {
		  //printf("nextstartline: %i, startline: %i\n", nodes[nextnode]->startline, startline);
		  if((nextnode < numNodes) && (nodes[nextnode]->startline == startline) && (nodes[nextnode]->startcol == startcol)) {
		        int* nodenext = &(nodes[nextnode]->cursor);
		        if(nodenext == NULL) {
			    printf("%s\n", nodes[nextnode]->newContent);
			    nextnode++;
			} else {
			    printf("%s", nodes[nextnode]->newContent);
			    prevline = endline;
			    prevcol = endcol;
			    nextnode++;
			}
		    } else {
		        printf("%s", tstr);
			prevline = endline;
			prevcol = endcol;
		    }
		}
		//printf("%i\n", sizeof(nodes));
		char* str = clang_getCString(tokenstring);
		strcpy(lastPrintedToken, str);
		clang_disposeString(tokenstring);  
	    }
	    clang_disposeTokens(cxtup, tokens, numTokens);
	    free(nodes);
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

void printTree(struct nodeTree* inTree) {
    struct treeNode* node = inTree->root;
    CXTranslationUnit cxtup = inTree->cxtup;
    lastPrintedToken = malloc(500*sizeof(char));
    strcpy(lastPrintedToken, " ");
    printTreeRecursive(node, cxtup);
    free(lastPrintedToken);
}
