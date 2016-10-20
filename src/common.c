#include "clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

struct treeListNode;

//CXTranslationUnit thecxtup;

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

struct nodeTree {
    struct treeNode* root;
    CXIndex cxi;
    CXTranslationUnit cxtup;
    CXFile file;
    FILE* filefile;
    enum CXErrorCode error;
    int nodes;
    int unmodifiedDepth;
};

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

int nodes = 0;
int depth = -1;

struct treeNode* currentnode;

void addModified(struct treeNode* node, struct treeNode* modified) {
    struct treeListNode* currnode = node->modifiedNodes;
    //printf("Add %i to %i.\n", child, currnode);
    if(currnode != NULL) {
      //printf("currnode != NULL\n"); 
      //printf("%u\n", currnode->next);
        while(currnode->next != NULL) {
	    currnode = currnode->next;
	}
	struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	if(modified->modified == 0) {
	    printf("Assertion failed: modified node has modified == 0!\n");
	}
	newnode->node = modified;
	newnode->next = NULL;
	currnode->next = newnode;
    } else {
      //printf("currnode == NULL\n");
        struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	if(modified->modified == 0) {
	    printf("Assertion failed: modified node has modified == 0!\n");
	}
	newnode->node = modified;
	newnode->next = NULL;
	node->modifiedNodes = newnode;
    }
}

void addChild(struct treeNode* node, struct treeNode* child) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    //printf("Add %i to %i.\n", child, currnode);
    if(currnode != NULL) {
      //printf("currnode != NULL\n"); 
      //printf("%u\n", currnode->next);
        while(currnode->next != NULL) {
	    currnode = currnode->next;
	}
	struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	newnode->node = child;
	newnode->next = NULL;
	currnode->next = newnode;
	child->parent = node;
    } else {
      //printf("currnode == NULL\n");
        struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	newnode->node = child;
	newnode->next = NULL;
	node->children = newnode;
	child->parent = node;
    }
}

/*
 * Only works for unmodified nodes and modified nodes, not nodes which have modified children!
 */
void addChildAfter(struct treeNode* node, struct treeNode* child, struct treeNode* after) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    while(currnode->next != NULL) {
        //printf("%li == %li, which implies %i == %i\n", currnode->node->cursor, after->cursor, clang_getCursorKind(currnode->node->cursor), clang_getCursorKind(after->cursor));
        if(after->validcursor == true) {
	    if(currnode->node->validcursor == true) {
	        if(clang_equalCursors(currnode->node->cursor, after->cursor)) {
		    break;
		} else {
		    currnode = currnode->next;
		}
	    } else {
	        currnode = currnode->next;
	    }
	} else {
	    if(currnode->node->validcursor == false) {
	        if(strcmp(currnode->node->newContent, after->newContent) == 0) {
		    break;
		} else {
		    currnode = currnode->next;
		}
	    } else {
	        currnode = currnode->next;
	    }
        }
    }
    if(child->modified != 0) {
        child->modified++;
        node->modified++;
        struct treeNode* currnode2 = node;
	addModified(currnode2, child);
        while(currnode2->parent != NULL) {
	    currnode2 = currnode2->parent;
	    currnode2->modified++;
	    addModified(currnode2, child);
	}
    }
    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
    newnode->node = child;
    newnode->next = currnode->next;
    currnode->next = newnode;
    child->parent = node;
    if(currnode->next->next != NULL) {
        if(currnode->next->next->node->validcursor == true) {
	    CXSourceRange range = clang_getCursorExtent(currnode->next->next->node->cursor);
	    CXSourceLocation rstart = clang_getRangeStart(range);
	    clang_getFileLocation(rstart, NULL, &(child->startline), &(child->startcol), NULL);
	} else {
	    if (after->validcursor == true) {
	        CXSourceRange range = clang_getCursorExtent(after->cursor);
		CXSourceLocation rend = clang_getRangeEnd(range);
		clang_getFileLocation(rend, NULL, &(child->startline), &(child->startcol), NULL);
	    } else {
	        child->startline = after->startline;
		child->startcol = after->startcol;
		/*printf("child->startcol = %i\n\n", child->startcol);
                  printf("[III]\n\n");*/
                child->startcol += 100000;
	    }
	}
    } else {
        if (after->validcursor == true) {
	    CXSourceRange range = clang_getCursorExtent(after->cursor);
	    CXSourceLocation rend = clang_getRangeEnd(range);
	    clang_getFileLocation(rend, NULL, &(child->startline), &(child->startcol), NULL);
	} else {
	    child->startline = after->startline;
	    child->startcol = after->startcol;
	    /*printf("child->startcol = %i\n\n", child->startcol);
              printf("[V]\n\n");*/
            child->startcol += 100000;
	}
    }
    printf("child->startline = %i\n", child->startline);
    printf("child->startcol = %i\n", child->startcol);
}

/*
 * Only works for unmodified nodes and modified nodes, not nodes which have modified children!
 */
void addChildBefore(struct treeNode* node, struct treeNode* child, struct treeNode* before, CXTranslationUnit cxtup) {
    node->childCount++;
    struct treeListNode* currnode = node->children;
    //printf("childCount: %i\n", node->childCount);
    /*enum CXCursorKind ccursorkind = clang_getCursorKind(currnode->node->cursor);
    enum CXCursorKind bcursorkind = clang_getCursorKind(before->cursor);
    enum CXCursorKind ncursorkind = clang_getCursorKind(currnode->next->node->cursor);
    printf("%i\n", ccursorkind);
    debugNode2(currnode->node, cxtup);
    printf("%i\n", bcursorkind);
    debugNode2(before, cxtup);
    printf("%i\n", ncursorkind);
    debugNode2(currnode->next->node, cxtup);*/
    if(clang_equalCursors(currnode->node->cursor, before->cursor) == 1) {
        printf("addingToFirst\n");
	if(child->modified != 0) {
	    child->modified++;
	    node->modified++;
	    struct treeNode* currnode2 = node;
	    addModified(currnode2, child);
	    while(currnode2->parent != NULL) {
	        currnode2 = currnode2->parent;
		//debugNode2(currnode2, cxtup);
		currnode2->modified++;
		addModified(currnode2, child);
	    }
	}
	struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
	newnode->node = child;
	newnode->next = currnode;
	node->children = newnode;
	child->parent = node;
	CXSourceRange range = clang_getCursorExtent(newnode->next->node->cursor);
	//debugNode2(newnode->next->node);
	CXSourceLocation rend = clang_getRangeStart(range);
	clang_getFileLocation(rend, NULL, &(child->startline), &(child->startcol), NULL);
	child->startcol--;
	//unsigned sline;
	//unsigned scol;
	//clang_getFileLocation(rend, NULL, &sline, &scol, NULL);
	printf("child->startline = %i, child->startcol = %i\n", child->startline, child->startcol);
	//printf("sline = %i, scol = %i\n", sline, scol);
    } else {
        while(currnode->next != NULL) {
	    if(before->validcursor == true) {
	        if(currnode->next->node->validcursor == true) {
		    if(clang_equalCursors(currnode->next->node->cursor, before->cursor)) {
		        break;
		    } else {
		        currnode = currnode->next;
		    }
		} else {
		    currnode = currnode->next;
		}
	    } else {
	        if(currnode->node->validcursor == false) {
		    if(strcmp(currnode->next->node->newContent, before->newContent) == 0) {
		        break;
		    } else {
		        currnode = currnode->next;
		    }
		} else {
		    currnode = currnode->next;
		}
	    }
        }
      //printf("\n%li == %li, which implies %i == %i\n", currnode->next->node->cursor, before->cursor, currnode->next->node->validcursor, before->validcursor);
      /*printf("\n-O-\n");
	debugNode2(child, cxtup);
	printf("\n-\n");
        debugNode2(currnode->node, cxtup);
	printf("\n-\n");
	debugNode2(currnode->next->node, cxtup);
	printf("\n-O-\n");*/
        
        printf("addChildAfter(");
	debugNode2(node, cxtup);
	printf(", ");
	debugNode2(child, cxtup);
	printf(", ");
	debugNode2(currnode->node, cxtup);
	printf(")\n");
	addChildAfter(node, child, currnode->node);
    }
}

struct treeNode* getChild(struct treeNode* node, int childNum) {
    if(childNum <= node->childCount) {
        struct treeListNode* currnode = node->children;
	for(int i = 1; i < childNum; i++) {
	    currnode = currnode->next;
	}
	//printf("returning %i->node\n", currnode);
	return currnode->node;
    } else {
        return NULL;
    }
}

enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    nodes++;
    struct treeNode* newnode = malloc(sizeof(struct treeNode));
    newnode->cursor = cursor;
    newnode->children = NULL;
    newnode->childCount = 0;
    newnode->modified = 0;
    newnode->newContent = NULL;
    newnode->modifiedNodes = NULL;
    unsigned sline;
    unsigned scolumn;
    newnode->startline = -1;
    newnode->startcol = -1;
    newnode->validcursor = true;
    //printf("vN: %i\n", currentnode);
    //if((currentnode != NULL) && ())
    //visitcounter++;
    addChild(currentnode, newnode);
    return CXChildVisit_Continue;
}

enum CXChildVisitResult (*visitor)(CXCursor, CXCursor, CXClientData) = &visit;

void visitRecursive(struct treeListNode* node, struct nodeTree* tree) {
    if(node != NULL) {
        tree->nodes = tree->nodes + node->node->parent->childCount;
        depth++;
	if(depth > tree->unmodifiedDepth) {
	    tree->unmodifiedDepth = depth;
	}
	struct treeListNode* nodeToVisit = node;
        while(nodeToVisit != NULL) {
	    currentnode = nodeToVisit->node;
	    //printf("vRN: %i\n", currentnode);
	    clang_visitChildren(nodeToVisit->node->cursor, visit, NULL);
	    visitRecursive(nodeToVisit->node->children, tree);
	    nodeToVisit = nodeToVisit->next;
	}
	depth--;
    }
}

struct nodeTree* generateTree(char* filename) {
    //printf("%s\n", filename);
    filefile = fopen(filename, "r+");
    struct nodeTree* ntree = malloc(sizeof(struct nodeTree));
    struct treeNode* thetree = malloc(sizeof(struct treeNode));
    ntree->filefile = filefile;
    ntree->root = thetree;
    CXIndex cxix = clang_createIndex(1, 0);
    ntree->cxi = cxix;
    //printf("cxix: %lx, cxi: %lx\n", cxix, ntree->cxi);
    unsigned flags = (CXTranslationUnit_DetailedPreprocessingRecord /*| CXTranslationUnit_Incomplete*/);
    CXTranslationUnit cxtu = clang_createTranslationUnit(cxix, filename);
    ntree->cxtup = cxtu;
    ntree->cxi = cxix;
    //printf("qdtranscxtup: %lx\n", ntree->cxtup);
    //printf("cxix: %lx, cxi: %lx, cxtu: %lx, cxtup: %lx\n", cxix, ntree->cxi, cxtu, ntree->cxtup);
    //printf("qdtranscxtup: %lx\n", ntree->cxtup);
    char* c11 = "-std=c11";
    ntree->error = clang_parseTranslationUnit2(cxix, filename, &c11, 1, NULL, 0, flags, &cxtu);
    ntree->cxi = cxix;
    ntree->cxtup = cxtu;
    //printf("cxix: %lx, cxi: %lx, cxtu: %lx, cxtup: %lx\n", cxix, ntree->cxi, cxtu, ntree->cxtup);
    //printf("qdtranscxtup: %lx\n", ntree->cxtup);
    file = clang_getFile (cxtu, filename);
    ntree->file = file;
    CXCursor cursor = clang_getTranslationUnitCursor(cxtu);
    thetree->cursor = cursor;
    thetree->modified = 0;
    thetree->validcursor = true;
    thetree->parent = NULL;
    thetree->modifiedNodes = NULL;
    thetree->startline = -1;
    thetree->startcol = -1;
    thetree->children = NULL;
    thetree->newContent = NULL;
    currentnode = thetree;
    clang_visitChildren(cursor, visitor, NULL);
    ntree->nodes = 0;
    ntree->unmodifiedDepth = INT_MIN;
    visitRecursive(thetree->children, ntree);
    //printf("Visits: %dx", visitcounter);
    //thecxtup = ntree->cxtup;
    return(ntree);
}

void disposeTree2(struct treeNode* node);

void disposeTree(struct nodeTree* tree) {
    disposeTree2(tree->root);
    clang_disposeTranslationUnit(tree->cxtup);
    clang_disposeIndex(tree->cxi);
    fclose(tree->filefile);
    free(tree);
}

void disposeTree2(struct treeNode* node) {
    if(node->newContent != NULL) {
      //printf("\nAbout to free string \"%s\"\n", node->newContent);
        free(node->newContent);
    }
    if(node->childCount == 0) {
        free(node);
    } else {
        struct treeListNode* current = node->children;
	struct treeListNode* next = current;
	while(next != NULL) {
	    current = next;
	    next = current->next;
	    disposeTree2(current->node);
	    free(current);
	}
	if(node->modified != 0) {
	    current = node->modifiedNodes;
	    next = current;
	    while(next != NULL) {
	        current = next;
		next = current->next;
		free(current);
	    }
	}
	free(node);
    }
}

int debugNode2(struct treeNode* node, CXTranslationUnit cxtup) {
    if(node->validcursor == true) {
        if(node->modified > 0) {
	    printf("\n%i[\n", node->modified);
	    struct treeListNode* modlist = node->modifiedNodes;
	    while(modlist != NULL) {
	        printf("    %s\n", modlist->node->newContent);
		modlist = modlist->next;
	    }
	    //printf("    %s\n", modlist->node->newContent);
	    printf("]\n");
        }
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
	//if(clang_Location_isFromMainFile(rstart) != 0) {
            printf("%i, [%i]:%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing \n", node->modified, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
	    CXToken* tokens;
	    unsigned int numTokens;
	    clang_tokenize(cxtup, range, &tokens, &numTokens);
	    printf("%u tokens (", numTokens);
	    for(int i = 0; i<numTokens; i++) {
	        CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
	        printf("%s ", clang_getCString(tokenstring));
		clang_disposeString(tokenstring);
	    }
	    clang_disposeTokens(cxtup, tokens, numTokens);
	    printf(")");
            printf("\n");
	    //}*/
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
 	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
    } else {
        printf("Modded: ");
	printf("%i, %s", node->modified, node->newContent);
	printf("\n");
    }
}
