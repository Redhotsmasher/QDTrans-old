#include "clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "common.h"

//struct treeNode* d1nodelist;

int prevline;
int prevcol;

bool prevmodded = false;

int nodenum = 0;

char* printNode(struct treeNode* node, CXTranslationUnit cxtup) {
    //printf("%i\n", sizeof(CXCursor));
    //printf("Printing node 0x%012lX, has cursor 0x%064lX\n", node, node->cursor);
    //printf("Still printing node 0x%012lX, has cursor 0x%064lX\n", node, node->cursor);
    unsigned sline;
    unsigned scol;
    unsigned eline;
    unsigned ecol;
    CXSourceRange range = clang_getCursorExtent(node->cursor);
    CXSourceLocation cstart = clang_getRangeEnd(range);
    clang_getFileLocation(cstart, NULL, &sline, &scol, NULL);
    CXSourceLocation cend = clang_getRangeEnd(range);
    clang_getFileLocation(cend, NULL, &eline, &ecol, NULL);
    //printf("Still printing node 0x%012lX, spans L%u-%u, C%u-%u\n", node, sline, eline, scol, ecol);
    prevline = 0;
    prevcol = 0;
    CXToken* tokens;
    unsigned int numTokens;
    clang_tokenize(cxtup, range, &tokens, &numTokens);
    int size = 5;
    unsigned startline;
    unsigned startcol;
    unsigned endline;
    unsigned endcol;
    CXSourceRange tokenrange;
    CXString tokenstring;
    for(int i = 0; i<numTokens; i++) {
        tokenrange = clang_getTokenExtent(cxtup, tokens[i]);
	tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
	CXSourceLocation currend = clang_getRangeEnd(tokenrange);
	clang_getFileLocation(currend, NULL, &endline, &endcol, NULL);
	if(prevcol != 0) {
	    CXSourceLocation currstart = clang_getRangeStart(tokenrange);
	    clang_getFileLocation(currstart, NULL, &startline, &startcol, NULL);
	    //printf("L%u-%u, C%u-%u", startline, prevline, startcol, prevcol);
	    int startl = startline;
	    int startc = startcol;
	    for(int i = 0; i < startl-prevline; i++) {
	        size++;
		//printf("*startline-prevline = %u, i = %u, prevline = %u\n", start-prevline, i, prevline);
	    }
	    if(startc-prevcol >= 0) {
	        for(int i = 0; i < startc-prevcol; i++) {
		    size++;
		}
	    } else {
	        for(int i = 1; i < startc; i++) {
		    size++;
		}
	    }
	}
	char* tstr = clang_getCString(tokenstring);
        size = size + strlen(tstr);
	prevline = endline;
	prevcol = endcol;
	clang_disposeString(tokenstring);
    }
    prevline = 0;
    prevcol = 0;
    printf("%i\n", size);
    char* currprint = malloc((size + 7));
    strcpy(currprint, &"    ");
    printf("%i tokens.\n", numTokens);
    for(int i = 0; i<numTokens; i++) {
        tokenrange = clang_getTokenExtent(cxtup, tokens[i]);
	tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
	CXSourceLocation currend = clang_getRangeEnd(tokenrange);
	clang_getFileLocation(currend, NULL, &endline, &endcol, NULL);
	if(prevcol != 0) {
	    CXSourceLocation currstart = clang_getRangeStart(tokenrange);
	    clang_getFileLocation(currstart, NULL, &startline, &startcol, NULL);
	    //printf("L%u-%u, C%u-%u", *startline, prevline, *startcol, prevcol);
	    int startl = startline;
	    int startc = startcol;
	    for(int i = 0; i < startl-prevline; i++) {
	        strcat(currprint, "\n");
		//printf("*startline-prevline = %u, i = %u, prevline = %u\n", start-prevline, i, prevline);
	    }
	    if(startc-prevcol >= 0) {
	        for(int i = 0; i < startc-prevcol; i++) {
		    strcat(currprint, " ");
		}
	    } else {
	        for(int i = 1; i < startc; i++) {
		    strcat(currprint, " ");
		}
	    }
	}
	char* tstr = clang_getCString(tokenstring);
	//printf("String %i: %s\n", i, tstr);
	if(i == 0 && endline == prevline && endcol == prevcol) {	    
	    // Do nothing, print nothing.
	} else {
	    //printf("%u -> %i, %u -> %i\n", *endline, prevline, *endcol, prevcol);
	    strcat(currprint, tstr);
	}
	//printf("Output string currenly has strlen %i.\n", strlen(currprint));
	//printf("Output string currently contains \"%s\".\n", currprint);
	prevline = endline;
	prevcol = endcol;
	clang_disposeString(tokenstring);
    }
    clang_disposeTokens(cxtup, tokens, numTokens);
    //return strcat(currprint, "\n");
    //printf("Output string finally contains \"%s\".\n", currprint);
    return currprint;
}

int generatePrintableListRecursive(struct treeNode* node, CXTranslationUnit cxtup, struct treeNode** list, int count) {
    depth++;
    if(node->modified == 0) {
        CXSourceRange range = clang_getCursorExtent(node->cursor);
        CXSourceLocation rstart = clang_getRangeStart(range);
        if(clang_Location_isFromMainFile(rstart) != 0) {
	    enum CXCursorKind cursorkind = clang_getCursorKind(node->cursor);
	    if(depth == 2 && cursorkind != CXCursor_MacroExpansion) {
	        nodenum++;
		/*if(cursorkind == CXCursor_StructDecl) {
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
			// Add printable
		    }
                    } else {*/
                if(list != NULL) {
                    list[count] = node;
                    /*printf("-%i-\n", count);
                    printf("%12lx\n", node);
                    printf("%12lx\n", list[count]);*/
                    count++;
                } else {
                    count++;
                }
                //}
            }
	}
    } else if(depth == 2) {
        if(list != NULL) {
            list[count] = node;
            /*printf("-%i-\n", count);
            printf("%12lx\n", node);
            printf("%12lx\n", list[count]);*/
            count++;
        } else {
            count++;
        }
    }
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
        while(childlist != NULL) {
            if(list != NULL) {
                count = generatePrintableListRecursive(childlist->node, cxtup, list, count);
            } else {
                count += generatePrintableListRecursive(childlist->node, cxtup, list, 0);
            }
            childlist = childlist->next;
        }
    }
    depth--;
    return count;
}

void printTreeIterative(CXTranslationUnit cxtup, struct treeNode** nodelist, int count) {
    struct treeNode* node;
    for(int n = 0; n < count; n++) {
        node = nodelist[n];
        if(node->modified == 0) {
            CXSourceRange range = clang_getCursorExtent(node->cursor);
            CXSourceLocation rstart = clang_getRangeStart(range);
            if(clang_Location_isFromMainFile(rstart) != 0) {
                enum CXCursorKind cursorkind = clang_getCursorKind(node->cursor);
                //printf("[%i] About to print: %i\n", depth, cursorkind);
                if(cursorkind != CXCursor_MacroExpansion) {
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
                                if(prevmodded == true) {
                                    //prevmodded = false;
                                } else {
                                    for(int i = 0; i < startl-prevline; i++) {

                                        printf("\n");
                                        //printf("*startline-prevline = %u, i = %u, prevline = %u\n", start-prevline, i, prevline);
                                    }
                                    if(startc-prevcol > 0) {
                                        for(int i = 0; i < startc-prevcol; i++) {
                                            printf(" ");
                                        }
                                    } else if (startc-prevcol < 0) {
                                        for(int i = 1; i < startc; i++) {
                                            printf(" ");
                                        }
                                    }
                                }
                            }
                            char* tstr = clang_getCString(tokenstring);
                            
                            if(i == numTokens-1) {
                                int m = n+1;
                                while(nodelist[m]->validcursor == false && m < count) {
                                    m++;
                                }
                                if(nodelist[m]->validcursor == true) {
                                    CXSourceRange nrange = clang_getCursorExtent(nodelist[m]->cursor);
                                    CXToken* ntokens;
                                    unsigned int nnumTokens;
                                    clang_tokenize(cxtup, range, &ntokens, &nnumTokens);
                                    CXSourceRange ntokenrange = clang_getTokenExtent(cxtup, ntokens[0]);
                                    CXSourceLocation nrend = clang_getRangeEnd(ntokenrange);
                                    unsigned* nendline = malloc(sizeof(unsigned));
                                    unsigned* nendcol = malloc(sizeof(unsigned));
                                    clang_getFileLocation(nrend, NULL, nendline, nendcol, NULL);
                                    CXString ntstring = clang_getTokenSpelling(cxtup, ntokens[0]);
                                    printf("%u == %u (%u), %u == %u, (%u)\n", *nendline, *endline, *startline, *nendcol, *endcol, *startcol);
                                    char* ntstr = clang_getCString(tokenstring);
                                    if(*nendline == *endline && *nendcol == *endcol) {
                                        prevmodded = true;
                                        // Prevent later deduplication, print nothing.
                                    } else {
                                        prevmodded = false;
                                        printf("%s", tstr);
                                    }
                                    free(nendcol);
                                    free(nendline);
                                    clang_disposeString(ntstring);
                                    clang_disposeTokens(cxtup, ntokens, nnumTokens);
                                }
                            } else if(i == 0 && *endline == prevline && *endcol == prevcol && prevmodded == false) {	    
                                // Do nothing, print nothing.
                            } else {
                                prevmodded = false;
                                //printf("[%i] %u -> %i, %u -> %i, prevmodded: %i\n", i, *endline, prevline, *endcol, prevcol, prevmodded);
                                printf("%s", tstr);
                            }
                            prevline = *endline;
                            prevcol = *endcol;
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
            //printf("prevmodded: %i", prevmodded);
        } else {
            //printf("Modified node detected.");
            //debugNode2(node, cxtup);
            if(node->validcursor == false) {
                //Print modified
                //debugNode(node, cxtup);
                prevmodded = true;
                printf("%s\n", node->newContent);
            } else {
                struct treeNode** nodes = malloc((node->modified) * sizeof(struct treeNode*));
                //printf("%i\n", sizeof(struct treeNode));
                //printf("%i\n", sizeof(nodes));
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
                int count = 0;
                while(nodes[(node->modified)-1] == NULL) {
                    while(currnode != NULL) {
                        //printf("currnode->node->startline = %i\n", currnode->node->startline);
                        //debugNode2(currnode->node, cxtup);
                        if(currnode->node->startline == -1) {
                            srange = clang_getCursorExtent(currnode->node->cursor);
                            sloc = clang_getRangeStart(srange);
                            clang_getFileLocation(sloc, NULL, &(currnode->node->startline), &(currnode->node->startcol), NULL);
                            //printf("Calculated node %i: startline: %i, startcol: %i\n", currnode->node, currnode->node->startline, currnode->node->startcol);
                            //debugNode2(currnode->node, cxtup);
                        }
                        //printf("\ncurrnode->node: %i, nextnode: %i\nnstartline: %i, smallestline: %i ,csline: %i\nnstartcol: %i, smallestcol: %i, cscol: %i\n", currnode->node, nextnode, currnode->node->startline, smallestline, csline, currnode->node->startcol, smallestcol, cscol);
                        if(currnode->node->startline < smallestline) {
                            //printf("\n1\, %i < %i", currnode->node->startline, smallestline);
                            if((currnode->node->startline > csline) /*|| (first == true)*/) {
                                //printf("\n2, %i > %i", currnode->node->startline, csline);
                                smallestline = currnode->node->startline;
                                smallestcol = currnode->node->startcol;
                                csline = currnode->node->startline;
                                cscol = currnode->node->startcol;
                                next = currnode->node;
                            }
                        } else if(currnode->node->startline == smallestline) {
                            //printf("\n3");
                            if(currnode->node->startcol < smallestcol) {
                                //printf("\n4, %i < %i", currnode->node->startcol, smallestcol);
                                if((currnode->node->startcol > cscol) /*|| (first == true)*/) {
                                    //printf("\n5, %i > %i", currnode->node->startcol, cscol);
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
                        //debugNode2(currnode->node, cxtup);
                        currnode = currnode->next;
                    }
                    //printf("nextnode: %i\n", nextnode);
                    //first = false;
                    nodes[nextnode] = next;
                    //printf("\nnnsl: %i ", nodes[nextnode]->startline);
                    //printf("nsl: %i", next->startline);
                    //printf("nn: %i\n", nextnode);
                    nextnode++;
                    smallestcol = INT_MAX;
                    smallestline = INT_MAX;
                    currnode = node->modifiedNodes;
                    //printf("nodes[node->modified-1]: %lX\n", nodes[node->modified-1]);
                }
                /*printf("Sorted nodes are:\n");
                  for(int i = 0; i <= (node->modified)-1; i++) {
                  printf("%i\n", node->modified);
                  printf("nodenum: %i, nstartline: %i, nstartcol: %i\n", i, nodes[i]->startline, nodes[i]->startcol);
                  printf("content%i: %s\n", i, nodes[i]->newContent);
                  debugNode2(nodes[i], cxtup);
                  }*/
                CXSourceRange range = clang_getCursorExtent(node->cursor);
                CXToken* tokens;
                unsigned int numTokens;
                int numNodes = nextnode;
                nextnode = 0;
                clang_tokenize(cxtup, range, &tokens, &numTokens);
                for(int i = 0; i<numTokens; i++) {
                    CXSourceRange tokenrange = clang_getTokenExtent(cxtup, tokens[i]);
                    CXSourceLocation tokenend = clang_getRangeEnd(tokenrange);
                    CXSourceRange noderange;
                    if(nextnode < numNodes) {
                        noderange = clang_getCursorExtent(nodes[nextnode]->cursor);
                    } else {
                        noderange = tokenrange; // noderange won't actually be used in this case (all modified nodes already printed) but needs to have a value to avoid attempting to read beyond the end of nodes[].
                    }
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
                        if(prevmodded == true) {
                            //prevmodded = false;
                        } else {
                            for(int i = 0; i < startl-prevline; i++) {
                                printf("\n");
                                //printf("*startline-prevline = %u, i = %u, prevline = %u\n", start-prevline, i, prevline);
                            }
                            if(startc-prevcol > 0) {
                                for(int i = 0; i < startc-prevcol; i++) {
                                    printf(" ");
                                }
                            } else if (startc-prevcol < 0) {
                                for(int i = 0; i < startc; i++) {
                                    printf(" ");
                                }
                            }
                        }
                    }
                    char* tstr = clang_getCString(tokenstring);
                    //printf("%i == %i, %i == %i\n", endline, prevline, endcol, prevcol);
                    if(/*i == 0 &&*/ endline == prevline && endcol == prevcol && prevmodded == false) {

                        //printf("i[%i] %u -> %i, %u -> %i, prevmodded: %i\n", i, endline, prevline, endcol, prevcol, prevmodded);
                        /*printf("(endline == prevline) == %i\n", (endline == prevline));
                          printf("(endcol == prevcol) == %i\n", (endcol == prevcol));
                          printf("(prevmodded == false) == %i\n", (prevmodded == false));
                          printf("%i, %i, %i\n", prevmodded, false, true == true);*/
                        // Do nothing, print nothing.
                    } else {
                        //printf("e[%i] %u -> %i, %u -> %i, prevmodded: %i\n", i, endline, prevline, endcol, prevcol, prevmodded);
                        //printf("nextstartline: %i, startline: %i\n", nodes[nextnode]->startline, startline);
                        if(nextnode < numNodes) {
                            //printf("\n%i < %i && (%i == %i) && (%i < %i)\n", nextnode, numNodes, nodes[nextnode]->startline, startline, nodes[nextnode]->startcol, startcol);
                        } else {
                            //printf("\n%i < %i && (%i == %i) && (%i < %i)\n", nextnode, numNodes, nodes[nextnode-1]->startline, startline, nodes[nextnode-1]->startcol, startcol);
                        }
                        if((nextnode < numNodes) && (nodes[nextnode]->startline == startline) && (nodes[nextnode]->startcol <= startcol)) {
                            prevmodded = true;
                            int* nodenext = &(nodes[nextnode]->cursor);
                            if(nodes[nextnode]->validcursor == false) {
                                printf("%s\n", nodes[nextnode]->newContent);
                                nextnode++;
                                i--;
                                prevmodded = true;
                                //printf("XYZZY");
                            } else {
                                //printf("%i\n", i);
                                char* nodenewcontent = nodes[nextnode]->newContent;
                                if(strcmp(nodenewcontent, "")) {
                                    printf("%s\n    ", nodenewcontent); // Not perfect but good enough.
                                }
                                prevline = endline;
                                prevcol = endcol;
                                if(nodes[nextnode]->validcursor == true) {
                                    CXSourceRange modrange = clang_getCursorExtent(nodes[nextnode]->cursor);
                                    CXSourceLocation modend = clang_getRangeEnd(modrange);
                                    int lastline;
                                    int lastcol;
                                    clang_getFileLocation(modend, NULL, &lastline, &lastcol, NULL);
                                    clang_getFileLocation(tokenend, NULL, &prevline, &prevcol, NULL);
                                    while((prevcol < lastcol || prevline < lastline) && (i < numTokens)) {
                                        //printf("%i < %i || %i < %i\n", prevcol, lastcol, prevline, lastline);
                                        tokenrange = clang_getTokenExtent(cxtup, tokens[i]);
                                        tokenend = clang_getRangeEnd(tokenrange);
                                        clang_getFileLocation(tokenend, NULL, &prevline, &prevcol, NULL);
                                        i++;
                                        // prevcol--; // Fix for duplication issue.
                                    }
                                }
                                //printf("%i < %i || %i < %i\n", prevcol, lastcol, prevline, lastline);
                                nextnode++;
                                //printf("%i\n", i);
                                prevmodded = true;
                            }
                        } else {
                            prevmodded = false;
                            printf("%s", tstr);
                            prevline = endline;
                            prevcol = endcol;
                        }
                    }
                    //printf("%i\n", sizeof(nodes));
                    clang_disposeString(tokenstring);  
                }
                clang_disposeTokens(cxtup, tokens, numTokens);
                free(nodes);
            }
            //printf("prevmodded: %i", prevmodded);
        }
    }
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
    prevline = 0;
    prevcol = 0;
    struct treeNode* node = inTree->root;
    CXTranslationUnit cxtup = inTree->cxtup;
    int count = generatePrintableListRecursive(node, cxtup, NULL, 0);
    //struct treeListNode list[count];
    struct treeNode** list = malloc(count * sizeof(struct treeNode*));
    generatePrintableListRecursive(node, cxtup, list, 0);
    /*for(int i = 0; i < count; i++) {
        printf("-%i-\n", i);
        debugNode2(list[i], cxtup);
        printf("\n%i\n", list[i]->modified);
        }*/
    printTreeIterative(cxtup, list, count);
}
