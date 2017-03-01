#include "clang-c/Index.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"
#include "printer.h"

enum locality {UNDEF, GLOBAL, CRITLOCAL, FUNLOCAL, ELSELOCAL}; // GLOBAL = global, CRITLOCAL = declared inside critical section, FUNLOCAL = declared inside the function which contains the critical section, ELSELOCAL = declared inside another function.

struct variable {
    char* name;
    char* typename;
    enum locality locality;
    bool threadLocal;
    bool pointer;
    bool needsreturn;
  //struct treeNode* decl;
    struct variable* next;
};

/*struct strct {
    char* name;
    struct treeNode* decl;
    struct variable* vars;
    struct strct* next;
};*/

struct criticalSection {
    bool needsRefactoring;
    bool needsWait;
  //struct treeNode* lockvarNode;
  //struct treeNode* lockNode;
  //struct treeNode* unlockNode;
    struct variable* accessedvars;
  //struct treeListNode* returnstmts;
    struct treeListNode* nodelist;
    struct treeListNode* nodesafter;
    struct criticalSection* next;
};

struct lockListNode {
    char* lockname;
    struct treeNode* declnode;
    struct lockListNode* next;
};

struct lockListNode* locklist;

//struct treeNode* dNode;

struct criticalSection* crits = NULL;

struct nodeTree* tree;

int moddednodes = 0;
int tnodes = 0;
int critCount = 0;
int moddepth = 0;

bool first;

//void modifyTree(struct treeNode* node, CXTranslationUnit cxtup);

void debugTree(struct treeNode* node, CXTranslationUnit cxtup);

void debugTree2(struct treeNode* node, CXTranslationUnit cxtup);

void debugTree3(struct treeNode* node, CXTranslationUnit cxtup);

void debugNode(struct treeNode* node, CXTranslationUnit cxtup);

void debugUnmodifiedNode(struct treeNode* node, CXTranslationUnit cxtup);

void debugCrits(struct criticalSection* crits, CXTranslationUnit cxtup);

void printCrit(struct criticalSection* crit, CXTranslationUnit cxtup);

void modifyLocks(struct treeNode* node, CXTranslationUnit cxtup);

void modifyLocksRecursive(struct treeNode* node, CXTranslationUnit cxtup);

void refactorCrits(struct treeNode* node, CXTranslationUnit cxtup);

struct treeNode* findNode(struct treeNode* node, CXCursor cursor, CXTranslationUnit cxtup);

void scanCrits(CXTranslationUnit cxtup);

void scanCrit(struct criticalSection* crit, CXTranslationUnit cxtup);

void scanCritRecursive(struct criticalSection* crit, struct treeNode* node, CXTranslationUnit cxtup, bool isreturn);

void findCrits(struct treeNode* node, CXTranslationUnit cxtup);

void scanTree(struct treeNode* node, CXTranslationUnit cxtup);

void freeCrits(struct criticalSection* crit);

void printQDUsage();

void clearNode(struct treeListNode* node) {
    struct treeListNode* currnode = node;
    //char* newchar = calloc(1, 1);
    char* newchar = malloc(1);
    strcpy(newchar, "");
    currnode->node->newContent = newchar;
    struct treeNode* currnode2 = currnode->node;
    while(currnode2 != NULL) {
        currnode2->modified++;
        addModified(currnode2, currnode->node);
        currnode2 = currnode2->parent;
    }
    if(currnode->node->children != NULL) {
        struct treeListNode* childlist = currnode->node->children;
	while(childlist != NULL) {
	    clearNode(childlist);
	    childlist = childlist->next;
	}
    }
}

void modifyLocks(struct treeNode* node, CXTranslationUnit cxtup) {
    modifyLocksRecursive(node, cxtup);
    struct lockListNode* currlock = locklist;
    while(currlock != NULL) {
        if(currlock->declnode->modified == 0) {
            char* newstring = malloc(15 + strlen(currlock->lockname));
            sprintf(newstring, "    QDLock * %s;", currlock->lockname);
            struct treeNode* currnode2 = currlock->declnode;
            while(currnode2->parent != NULL) {
                addModified(currnode2->parent, currlock->declnode);
                currnode2->parent->modified++;
                currnode2 = currnode2->parent;
            }
            currlock->declnode->modified++;
            currlock->declnode->newContent = newstring;
        }
        currlock = currlock->next;
    }
}

void modifyLocksRecursive(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    if(node->validcursor == true) {
        CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
	char* str = clang_getCString(cdisplaystring);
        bool init = (strcmp(str, &("pthread_mutex_init")) == 0);
        bool destroy = (strcmp(str, &("pthread_mutex_destroy")) == 0);
        bool lock = (strcmp(str, &("pthread_mutex_lock")) == 0);
        bool unlock = (strcmp(str, &("pthread_mutex_unlock")) == 0);
        if(strstr(str, &("pthread_mutex")) != NULL) {
            printf("Found %s.\n", str);
        }
        if(init == true || destroy == true || lock == true || unlock == true) {
            printf("LOCK!\n");
            debugNode(node, cxtup);
            struct treeNode* currnode = node->children->next->node;
            enum CXCursorKind cxck = clang_getCursorKind(currnode->cursor);
            while((cxck != CXCursor_ParenExpr) && (currnode->childCount != 0)) {
                //debugNode(currnode, cxtup);
                currnode = currnode->children->node;
                cxck = clang_getCursorKind(currnode->cursor);
            }
            //debugNode(currnode, cxtup);
            currnode = currnode->children->node;
            CXString cdisplaystring2 = clang_getCursorDisplayName(currnode->cursor);
            char* str2 = clang_getCString(cdisplaystring2);
            struct lockListNode* currlock = locklist;
            bool match = false;
            while(currlock != NULL) {
                printf("strcmp(%s, %s) (->%lX)\n", str2, currlock->lockname, currlock->next);
                if(strcmp(str2, currlock->lockname) == 0) {
                    match = true;
                }
                clang_disposeString(cdisplaystring2);
                cdisplaystring2 = clang_getCursorDisplayName(currnode->cursor);
                str2 = clang_getCString(cdisplaystring2);
                currlock = currlock->next;
            }
            clang_disposeString(cdisplaystring2);
            if(match == true) {
                printf("MATCH!\n");
                CXToken* tokens;
                unsigned int numTokens;
                unsigned int numt;
                CXSourceRange range = clang_getCursorExtent(currnode->cursor);
                clang_tokenize(cxtup, range, &tokens, &numTokens);
                if(numTokens % 2 == 0) { // Deduplication bug workaround
                    numt = numTokens-1;
                } else {
                    numt = numTokens;
                }
                unsigned long strlength = 0;
                for(unsigned int i = 0; i < numt; i++) {
                    CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
                    char* tstr = clang_getCString(tokenstring);
                    strlength += strlen(tstr);
                    clang_disposeString(tokenstring);
                }
                char* lockstring = malloc(strlength+1);
                printf("strlength: %i, numTokens: %i\n", strlength, numTokens);
                for(unsigned int i = 0; i < numt; i++) {
                    CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
                    char* tstr = clang_getCString(tokenstring);
                    printf("tstr: %s\n", tstr);
                    strcat(lockstring, tstr);
                    clang_disposeString(tokenstring);
                }
                printf("lockstring: %s\n", lockstring);
                char* newstring;
                if(init == true) {
                    newstring = malloc(33 + strlength);
                    sprintf(newstring, "    %s = LL_create(PLAIN_QD_LOCK);", lockstring);
                } else if(destroy == true) {
                    newstring = malloc(15 + strlength);
                    sprintf(newstring, "    LL_free(%s);", lockstring);
                } else if(lock == true) {
                    newstring = malloc(15 + strlength);
                    sprintf(newstring, "    LL_lock(%s);", lockstring);
                } else /*if(unlock == true)*/ {
                    newstring = malloc(17 + strlength);
                    sprintf(newstring, "    LL_unlock(%s);", lockstring);
                }
                addModified(node->parent, node);
                struct treeNode* currnode2 = node;
                while(currnode2->parent != NULL) {
                    addModified(currnode2->parent, node);
                    currnode2->parent->modified++;
                    currnode2 = currnode2->parent;
                }
                node->modified++;
                node->newContent = newstring;
                free(lockstring);
                clang_disposeTokens(cxtup, tokens, numTokens);
                clang_disposeString(cdisplaystring);
                printf("RETURN!\n");
                return;
            }
        }
        clang_disposeString(cdisplaystring);
    }  
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
        while(childlist != NULL) {
            modifyLocksRecursive(childlist->node, cxtup);
            childlist = childlist->next;
        }
    } 
    depth--;
}

void refactorCrits(struct treeNode* node, CXTranslationUnit cxtup) {
    tree->root->modified = 0;
	    /*while(tree->root->modifiedNodes != NULL) {
	        struct treeListNode* nodetodelete = tree->root->modifiedNodes;
		tree->root->modifiedNodes == tree->root->modifiedNodes->next;
		free(nodetodelete);
		}*/
    tree->root->modifiedNodes = NULL;
    struct criticalSection* currcrit = crits;
    while(currcrit != NULL) {
        if((currcrit->needsRefactoring == false) && (currcrit->nodesafter == NULL)) {
	    //Critical section is already in a function of its own, no need to refactor.
        } else {
	    //Critical section is not in a function of its own and therefore needs to be factored out.
	    struct treeListNode* currnode = currcrit->nodelist;
	    int nodeCount = 0;
	    while(currnode != NULL) {
	        nodeCount++;
		currnode = currnode->next;
	    }
            struct variable* currvar = currcrit->accessedvars;
            int varCount = 0;
            while(currvar != NULL) {
                varCount++;
                currvar = currvar->next;
            }
	    nodeCount++; // For unlock node, technically part of "nodesafter".
	    char* nodestrings[nodeCount];
            char* varstrings[varCount];
	    currvar = currcrit->accessedvars;
            for(int i = 0; i < varCount; i++) {
                varstrings[i] = malloc(strlen(currvar->typename) + strlen(currvar->name) + 7);
                if(currvar->locality != GLOBAL) {
                    sprintf(varstrings[i], "    %s %s;", currvar->typename, currvar->name);
                } else {
                    sprintf(varstrings[i], "");
                }
                currvar = currvar->next;
            }
            currnode = currcrit->nodelist;
	    for(int i = 0; i < (nodeCount-1); i++) {
	      /*char* asdf = "asdf\n";
		nodestrings[i] = asdf;
		printf("sadf\n");
		printf("String %lx (%lx): \"%s\" (\"%s\")\n", nodestrings[i], &asdf, nodestrings[i], asdf);
		printf("asdf\n");*/
                if(currnode->node->modified == 0) {
                    nodestrings[i] = printNode(currnode->node, cxtup);
                } else {
                    nodestrings[i] = malloc(strlen(currnode->node->newContent)*sizeof(char)+1);
                    strcpy(nodestrings[i], currnode->node->newContent);
                }
		/*printf("S%i|%s\n", i, printNode(currnode->node, cxtup));
		printf("Node %i: %s\n", i, nodestrings[i]);
		printf("ÄÅÖPOLIKUJYHGTRFEDWSQ\n");
		char* kuken = printNode(currnode->node, cxtup);
		printf("%s\n", kuken);
		char* currstring = nodestrings[i];
		//char currstringchar = currstring*;
		/*for(int i = 0; i <= 1000; i++) {
		    printf("%i|%c\n", i, *currstring+i);
		    }*/
		//printf("S%i|%s\n", i, nodestrings[i]);
	        currnode = currnode->next;
	    }
	    nodestrings[nodeCount-1] = printNode(currcrit->nodesafter->node, cxtup);
	    currvar = currcrit->accessedvars;
	    int size = 3;
	    while(currvar != NULL) {
	        size = size + strlen(currvar->name) + strlen(currvar->typename) + 7;
		currvar = currvar->next;
	    }
	    int size2 = size;
	    char* newfunname = malloc(12);
	    sprintf(newfunname, "critSec%i", critCount);
	    critCount++;
	    char* fheaderstring = malloc(3*(varCount-1) + size + 23 + (5*varCount));
            char* structnamestring = malloc(20);
            sprintf(structnamestring, "} %s_msg;", newfunname);
	    *fheaderstring = NULL;
	    currvar = currcrit->accessedvars;
	    strcat(fheaderstring, "\n\nvoid ");
	    strcat(fheaderstring, newfunname);
	    strcat(fheaderstring, "(");
	    strcat(fheaderstring, currvar->typename);
	    strcat(fheaderstring, " ");
	    strcat(fheaderstring, currvar->name);
	    currvar = currvar->next;
	    while(currvar != NULL) {
                if(currvar->locality != GLOBAL) {
                    strcat(fheaderstring, ", ");
                    strcat(fheaderstring, currvar->typename);
                    if(/*currvar->threadLocal == true*/(currvar->needsreturn == true && strcmp(currvar->typename, "pthread_mutex_t") != 0)) {
                        strcat(fheaderstring, " * ");
                        strcat(fheaderstring, "__");
                        strcat(fheaderstring, currvar->name);
                        strcat(fheaderstring, "__");
                    } else {
                        strcat(fheaderstring, " ");
                        strcat(fheaderstring, currvar->name);
                    }
                }
		currvar = currvar->next;
	    }
	    strcat(fheaderstring, ") {");
	    currvar = currcrit->accessedvars;
	    size = 0;
	    currcrit->needsWait = false;
	    while(currvar != NULL) {
	        size = size + 2*strlen(currvar->name) + strlen(currvar->typename) + 16*varCount;
		if(currvar->pointer == true || currvar->threadLocal == true) {
		    currcrit->needsWait = true;
		}
		currvar = currvar->next;
	    }
	    char* varstringbefore = malloc(size);
	    char* varstringafter = malloc(size);
	    *varstringbefore = NULL;
	    *varstringafter = NULL;
	    char* varstringbefore2 = varstringbefore;
	    char* varstringafter2 = varstringafter;
	    currvar = currcrit->accessedvars;
	    int pos = 0;
	    while(currvar != NULL) {
                //printf("currvar->locality == %i\n", currvar->locality);
	        if((currvar->needsreturn == true && currvar->locality != GLOBAL && strcmp(currvar->typename, "pthread_mutex_t") != 0) /*|| (currvar->threadLocal == true && currvar->pointer == true)*/) {
		  //printf("%012lX (\"%s\")\n---\n", currvar, currvar->name);
		    sprintf(varstringbefore2, "    %s %s = *__%s__;\n", currvar->typename, currvar->name, currvar->name);
		    varstringbefore2 += (15 + strlen(currvar->typename) + 2*strlen(currvar->name));
		    sprintf(varstringafter2, "    *__%s__ = %s;\n", currvar->name, currvar->name);
		    varstringafter2 += (14 + 2*strlen(currvar->name));
	        }
		currvar = currvar->next;
	    }
	    printf("\n---\n%s\n---\n%s\n---\n", varstringbefore, varstringafter);
	    currvar = currcrit->accessedvars;
	    char* fcallstring = malloc(size2 + 12 + 5);
	    char* fcallstring2 = fcallstring;
	    *fcallstring = NULL;
	    if((currvar->needsreturn == true && currvar->locality != GLOBAL && strcmp(currvar->typename, "pthread_mutex_t") != 0)) {
	        sprintf(fcallstring2, "%s(&%s", newfunname, currvar->name);
		fcallstring2 += (2+strlen(newfunname)+strlen(currvar->name));
	    } else {
	        sprintf(fcallstring2, "%s(%s", newfunname, currvar->name);
		fcallstring2 += (1+strlen(newfunname)+strlen(currvar->name));
	    }
	    currvar = currvar->next;
	    while(currvar != NULL) {
                if(currvar->locality != GLOBAL) {
                    if((currvar->needsreturn == true)) {
                        sprintf(fcallstring2, ", &%s", currvar->name);
                        fcallstring2 += (3+strlen(currvar->name));
                    } else {
                        sprintf(fcallstring2, ", %s", currvar->name);
                        fcallstring2 += (2+strlen(currvar->name));
                    }
                }
		currvar = currvar->next;
	    }
	    sprintf(fcallstring2, ");");
            printf("\n\n---\n\n");
            printf("\n\ntypedef struct {\n");
            for(int i = 0; i < varCount; i++) {
                printf("%s\n", varstrings[i]);
            }
            printf("\n%s\n", structnamestring);
	    printf("\n\n---\n\n");
	    printf("%s\n", fcallstring);
	    printf("\n\n---\n\n");
	    printf("%s\n", fheaderstring);
	    printf("%s\n", varstringbefore);
	    for (int i = 0; i < nodeCount; i++) {
	        printf("%s\n", nodestrings[i]);
	    }
	    printf("%s\n", varstringafter);
	    printf("\n}\n"); //"funend"
            printf("\n\n---\n\n");
	    struct treeNode* currnode2 = currcrit->nodelist->node;
	    struct treeNode* locknode = currnode2;
            clearNode(currcrit->nodelist);
	    locknode->newContent = fcallstring;
	    printf("locknode->newContent: %s\n", locknode->newContent);
	    currnode2->modified++;
	    //addModified(currnode2, locknode);
	    printf("locknode->newContent: %s\n", locknode->newContent);
	    while(currnode2->parent != NULL) {
	        currnode2 = currnode2->parent;
		currnode2->modified++;
		addModified(currnode2, locknode);
	    }
	    printf("locknode->newContent: %s\n", locknode->newContent);
	    printf("locknode is:");
	    debugNode(currcrit->nodelist->node, cxtup);
	    currnode = currcrit->nodelist->next;
	    //debugNode(currnode->node, cxtup);
	    while(currnode != NULL) {
	        clearNode(currnode);
		currnode = currnode->next;
	    }
	    clearNode(currcrit->nodesafter);
	    currnode = tree->root->children;
	    CXSourceRange drange = clang_getCursorExtent(node->cursor);
            CXSourceLocation drstart = clang_getRangeStart(drange);
	    while(currnode->node->validcursor == false || clang_getCursorKind(currnode->node->cursor) != CXCursor_FunctionDecl || clang_Location_isFromMainFile(drstart) == 0) {
	        currnode = currnode->next;
                if(currnode->node->validcursor == true) {
                    drange = clang_getCursorExtent(currnode->node->cursor);
                    drstart = clang_getRangeStart(drange);
                }
	    }
	    printf("\nÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\n");
	    debugNode(currnode->node, tree->cxtup);
	    /*printf("\nQEWADSZX\n\n");
	    debugNode(currnode->node->parent, tree->cxtup);*/
            char* structheaderstring = malloc(19);
            sprintf(structheaderstring, "\n\ntypedef struct {");
	    struct treeNode* newNode;
            newNode = malloc(sizeof(struct treeNode));
            newNode->childCount = 0;
	    newNode->children = NULL;
	    newNode->modified = 1;
            newNode->newContent = structheaderstring;
            newNode->modifiedNodes = NULL;
	    newNode->validcursor = false;
	    addChildBefore(currnode->node->parent, newNode, currnode->node);
            for(int i = 0; i < varCount; i++) {
	        newNode = malloc(sizeof(struct treeNode));
		newNode->childCount = 0;
		newNode->children = NULL;
		newNode->modified = 1;
		newNode->newContent = varstrings[i];
		newNode->modifiedNodes = NULL;
		newNode->validcursor = false;
		addChildBefore(currnode->node->parent, newNode, currnode->node);
	    }
            newNode = malloc(sizeof(struct treeNode));
	    newNode->childCount = 0;
	    newNode->children = NULL;
	    newNode->modified = 1;
	    newNode->newContent = structnamestring;
	    newNode->modifiedNodes = NULL;
	    newNode->validcursor = false;
	    addChildBefore(currnode->node->parent, newNode, currnode->node);
            newNode = malloc(sizeof(struct treeNode));
	    newNode->childCount = 0;
	    newNode->children = NULL;
	    newNode->modified = 1;
	    newNode->newContent = fheaderstring;
	    newNode->modifiedNodes = NULL;
	    newNode->validcursor = false;
	    addChildBefore(currnode->node->parent, newNode, currnode->node);
	    newNode = malloc(sizeof(struct treeNode));
	    newNode->childCount = 0;
	    newNode->children = NULL;
	    newNode->modified = 1;
	    newNode->newContent = varstringbefore;
	    newNode->modifiedNodes = NULL;
	    newNode->validcursor = false;
	    addChildBefore(currnode->node->parent, newNode, currnode->node);
	    for(int i = 0; i < nodeCount; i++) {
	        newNode = malloc(sizeof(struct treeNode));
		newNode->childCount = 0;
		newNode->children = NULL;
		newNode->modified = 1;
		newNode->newContent = nodestrings[i];
		newNode->modifiedNodes = NULL;
		newNode->validcursor = false;
		addChildBefore(currnode->node->parent, newNode, currnode->node);
	    }
	    strcat(varstringafter, "\n}\n");
	    newNode = malloc(sizeof(struct treeNode));
	    newNode->childCount = 0;
	    newNode->children = NULL;
	    newNode->modified = 1;
	    newNode->newContent = varstringafter;
	    newNode->modifiedNodes = NULL;
	    newNode->validcursor = false;
	    addChildBefore(currnode->node->parent, newNode, currnode->node);
	    /*char* funend = malloc(3);
	    strcpy(funend, "\n}\n");
	    newNode = malloc(sizeof(struct treeNode));
	    newNode->childCount = 0;
	    newNode->children = NULL;
	    newNode->modified = 1;
	    newNode->newContent = funend;
	    newNode->modifiedNodes = NULL;
	    newNode->validcursor = false;
	    addChildBefore(currnode->node->parent, newNode, currnode->node);*/
	    free(newfunname);	    
	}
        depth = 0;
        debugTree2(tree->root, tree->cxtup);
        currcrit = currcrit->next;
        depth = 0;
    }
}

/*void QDifyCrits(struct treeNode* node, CXTranslationUnit cxtup) {
    struct criticalSection* currcrit = crits;
    while(currcrit != NULL) {
        if(crit->returnstmts != NULL) {
	    if(crit->returnstmts->next != NULL) {
	        int returns = 0;
		struct treeListNode* currret = crit->returnstmts;
		while(currret != NULL) {
		    returns++;
		    currret = currret->next;
		}
		char* rets[returns];
		currret = crit->returnstmts;
		for(i = 0; i < returns; i++) {
		    rets[i] = printNode(currret, cxtup);
		    currret = currret->next;
		}
		boolean returnsEqual = true;
		for(i = 0; i < returns; i++) {
		    if(strcmp(rets[0], rets[i])) {
		        //Do nothing;
		    } else {
		        returnsEqual = false;
			break;
		    }
		}
		if(returnsEqual == true) {
		    // Modify return statements to write
		}
	    }
	}
    }
    currcrit = currcrit->next;
}*/

void scanCrits(CXTranslationUnit cxtup) {
    struct criticalSection* crit = crits;
    while(crit != NULL) {
        first = true;
        scanCrit(crit, cxtup);
        crit = crit->next;
    }
}

void scanCrit(struct criticalSection* crit, CXTranslationUnit cxtup) {
    struct treeListNode* currnode = crit->nodelist;
    while(currnode != NULL) {
      //printf("nextCrit\n");
        scanCritRecursive(crit, currnode->node, cxtup, false);
	currnode = currnode->next;
    }
}

enum accesstype {NONE, READ, WRITE};

enum accesstype findNodeAccess(struct treeNode* node, char* name, CXTranslationUnit cxtup);

void scanCritRecursive(struct criticalSection* crit, struct treeNode* node, CXTranslationUnit cxtup, bool isreturn) {
  //printf("Scan!\n");
    depth++;
    if(node->validcursor == true) {
        bool isret = isreturn;
	enum CXCursorKind cxck = clang_getCursorKind(node->cursor);
	enum CXCursorKind cxck2;
	if(node->parent != NULL && node->parent->parent != NULL) {
	    cxck2 = clang_getCursorKind(node->parent->parent->cursor);
	} else {
	    cxck2 = CXCursor_TranslationUnit; // Dummy value != CXCursor_CallExpr
	}
	//printf("cxck = %i, cxck2 = %i\n", cxck, cxck2);
	/*if(cxck == CXCursor_ReturnStmt) {
	    isret = true;
	    }*/ if(cxck == CXCursor_DeclRefExpr) {
	  //printf("1\n");
	    if(cxck2 != CXCursor_CallExpr) {
	      //printf("2\n");
	        //if(first == false) {
	      //printf("3\n");
      /*if(isret == true) {
	    struct treeListNode* newstmt = malloc(sizeof(struct treeListNode));
	    newstmt->next = NULL;
	    newstmt->node = node;
	    if(crit->returnstmts == NULL) {
	        crit->returnstmts = newstmt;
	    } else {
	        struct treeListNode* currstmt = crit->returnstmts;
		while(currstmt->next != NULL) {
		    currstmt = currstmt->next;
		}
		currstmt->next = newstmt;
	    }
	    } else {*/
		    CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
		    char* namestring = clang_getCString(cdisplaystring);
		    struct variable* currvar = crit->accessedvars;
		    bool isDuplicate = false;
		    while(currvar != NULL) {
		        isDuplicate = (isDuplicate || (strcmp(namestring, currvar->name) == NULL));
			currvar = currvar->next;
		    }
		    if(isDuplicate == false) {
		        struct variable* newvar = malloc(sizeof(struct variable));
			newvar->name = malloc(strlen(namestring) + 1);
			strcpy(newvar->name, namestring);
			CXType type = clang_getCursorType(node->cursor);
                        CXString typestring2 = clang_getTypeSpelling(type);
                        CXType type2 = clang_getCanonicalType(type);
			CXString typestring = clang_getTypeSpelling(type2);
			char* tstring = clang_getCString(typestring);
			//printf("Type might be %s?\n", tstring);
			newvar->typename = malloc(strlen(tstring) + 1);
			strcpy(newvar->typename, tstring);
			char* isptr = (strchr(tstring, '*'));
			if(isptr != NULL) {
			    newvar->pointer = true;
			} else {
			    newvar->pointer = false;
			}
			//newvar->threadLocal = false;
                        newvar->locality = UNDEF;
			CXCursor refcursor = clang_getCursorReferenced(node->cursor);
			struct treeNode* refnode = findNode(tree->root, refcursor, cxtup);
			//debugNode(refnode, cxtup);
                        char* tstring2 = clang_getCString(typestring2);
                        if(strcmp(tstring2, "pthread_mutex_t") == 0) {
                            struct lockListNode* newlock = malloc(sizeof(struct lockListNode));
                            newlock->lockname = malloc(strlen(namestring) + 1);
                            strcpy(newlock->lockname, namestring);
                            newlock->declnode = refnode;
                            newlock->next = NULL;
                            struct lockListNode* currlock = locklist;
                            if(currlock != NULL) {
                                while(currlock->next != NULL) {
                                    currlock = currlock->next;
                                }
                                currlock->next = newlock;
                            } else {
                                locklist = newlock;
                            }
                        }
                        clang_disposeString(typestring2);
                        if(clang_getCursorKind(refnode->parent->cursor) == CXCursor_TranslationUnit) {
			    newvar->locality = GLOBAL;
                            newvar->needsreturn = false;
			} 
			CXSourceRange range = clang_getCursorExtent(refcursor);
                        if(newvar->locality == UNDEF) {
                            CXSourceRange rlock = clang_getCursorExtent(crit->nodelist->node->cursor);
                            CXSourceRange runlock = clang_getCursorExtent(crit->nodesafter->node->cursor);
                            CXSourceLocation lvar = clang_getRangeStart(range);
                            CXSourceLocation llock = clang_getRangeStart(rlock);
                            CXSourceLocation lunlock = clang_getRangeStart(runlock);
                            unsigned int sline;
                            unsigned int scol;
                            unsigned int lline;
                            unsigned int lcol;
                            unsigned int ulline;
                            unsigned int ulcol;
                            clang_getFileLocation(lvar, NULL, &sline, &scol, NULL);
                            clang_getFileLocation(llock, NULL, &lline, &lcol, NULL);
                            clang_getFileLocation(lunlock, NULL, &ulline, &ulcol, NULL);
                            if(sline > lline && sline < ulline) {
                                newvar->locality = CRITLOCAL;
                                newvar->needsreturn = false;
                            } else {
                                struct treeNode* cpnode = refnode;
                                enum CXCursorKind ccxck;
                                ccxck = clang_getCursorKind(refnode->cursor);
                                while(cpnode != NULL && ccxck != CXCursor_FunctionDecl) {
                                    ccxck = clang_getCursorKind(cpnode->cursor);
                                    cpnode = cpnode->parent;
                                }
                                if(cpnode != NULL) {
                                    CXSourceRange cprange = clang_getCursorExtent(cpnode->cursor);
                                    CXSourceLocation cpstart = clang_getRangeStart(cprange);
                                    CXSourceLocation cpend = clang_getRangeEnd(cprange);
                                    unsigned int cpsline;
                                    unsigned int cpscol;
                                    unsigned int cpeline;
                                    unsigned int cpecol;
                                    clang_getFileLocation(cpstart, NULL, &cpsline, &cpscol, NULL);
                                    clang_getFileLocation(cpend, NULL, &cpeline, &cpecol, NULL);
                                    if(sline > cpsline && sline < cpeline) {
                                        newvar->locality = FUNLOCAL;
                                    } else {
                                        newvar->locality = ELSELOCAL;
                                    }
                                }
                            }
                        }
                        if(newvar->locality == FUNLOCAL || (newvar->locality == ELSELOCAL && isptr == NULL)) {
                            struct treeListNode* tlnode = crit->nodesafter;
                            enum accesstype at = NONE;
                            while(tlnode != NULL && at == NONE) {
                                at = findNodeAccess(tlnode->node, namestring, cxtup);
                                tlnode = tlnode->next;
                            }
                            //printf("findNodeAccess = %i\n", at);
                            printf("needsreturn = %i\n", newvar->needsreturn);
                            if(at == READ) {
                                printf("READ\n");
                                newvar->needsreturn = true;
                            } else {
                                printf("NOT READ\n");
                                newvar->needsreturn = false;
                            }
                        }
			CXToken* tokens;
			unsigned int numTokens;
			clang_tokenize(cxtup, range, &tokens, &numTokens);
			if(numTokens > 0) {
			    CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[0]);
			    tstring = clang_getCString(tokenstring);
			    newvar->threadLocal = ((strstr(tstring, "_Thread_local") != NULL) || (strstr(tstring, "thread_local")) != NULL || (strstr(tstring, "__thread") != NULL) || (strstr(tstring, "__declspec(thread)") != NULL));
			    debugNode(node, cxtup);
			    clang_disposeString(tokenstring);
			    clang_disposeTokens(cxtup, tokens, numTokens);
			}
			printf("clang_Cursor_isNull(refcursor) == %i.\n", clang_Cursor_isNull(refcursor));
			struct treeNode* defnode;
			if(clang_Cursor_isNull(refcursor) == false) {
			    defnode = findNode(tree->root, refcursor, cxtup);
			    debugNode(refnode, cxtup);
			    printf("refnode->parent's cxck is %i.\n", clang_getCursorKind(refnode->parent->cursor));
			}
                        printf("needsreturn = %i\n", newvar->needsreturn);
			//printf("%s\n", clang_Cursor_hasAttrs(node->cursor));
			//printf("[p->p->cxck = %i][%s %s]: pointer = %i, threadLocal = %i, global = %i\n\n", cxck2, newvar->typename, newvar->name, newvar->pointer, newvar->threadLocal, newvar->global);
			clang_disposeString(typestring);
			newvar->next = NULL;
			if(crit->accessedvars == NULL) {
			    crit->accessedvars = newvar;
			} else {
			    currvar = crit->accessedvars;
			    while(currvar->next != NULL) {
			        currvar = currvar->next;
			    }
			    currvar->next = newvar;
			}
		    }
		    clang_disposeString(cdisplaystring);
		    /*} else {
		    first = false;
		    }*/
	    }
	} else if(cxck == CXCursor_MemberRefExpr) {
            CXType type = clang_getCursorType(node->cursor);
            CXString typestring2 = clang_getTypeSpelling(type);
            char* tstring = clang_getCString(typestring2);
            if(strcmp(tstring, "pthread_mutex_t") == 0) {
                struct lockListNode* newlock = malloc(sizeof(struct lockListNode));
                CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
                char* namestring = clang_getCString(cdisplaystring);
                newlock->lockname = malloc(strlen(namestring) + 1);    
                strcpy(newlock->lockname, namestring);
                CXCursor refcursor = clang_getCursorReferenced(node->cursor);
                struct treeNode* refnode = findNode(tree->root, refcursor, cxtup);
                newlock->declnode = refnode;
                newlock->next = NULL;
                struct lockListNode* currlock = locklist;
                if(currlock != NULL) {
                    while(currlock->next != NULL) {
                        currlock = currlock->next;
                    }
                    currlock->next = newlock;
                } else {
                    locklist = newlock;
                }
                clang_disposeString(cdisplaystring);
            }
            clang_disposeString(typestring2);
        }
	if(node->children != NULL) {
	    struct treeListNode* childlist = node->children;
	    while(childlist != NULL) {
	        //printf("nextCritRecursive\n");
	        scanCritRecursive(crit, childlist->node, cxtup, isret);
		childlist = childlist->next;
	    }
	    //printf("upCritRecursive\n");
	}
    } else {
        if(node->newContent != NULL) {
	    printf("%s\n", node->newContent);
	} else {
	    printf("Weird node is weird.\n");
	}
    }
     depth--;
}

//enum accesstype {NONE, READ, WRITE};

enum accesstype findNodeAccess(struct treeNode* node, char* name, CXTranslationUnit cxtup) { 
    enum accesstype retval = NONE;
    if(node->validcursor == true) {
        enum CXCursorKind cxck = clang_getCursorKind(node->cursor);
        if(cxck == CXCursor_DeclRefExpr) {
            CXString cdisplaystring = clang_getCursorDisplayName(node->cursor);
            char* namestring = clang_getCString(cdisplaystring);
            if(strcmp(namestring, name)) {
                retval = READ;
            }
            clang_disposeString(cdisplaystring);
	}
    }
    if(retval == NONE && node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
            printf("childlist: %12lX, childlist->node: %12lX\n", childlist, childlist->node);
	    retval = findNodeAccess(childlist->node, name, cxtup);
	    if(retval != NONE) {
	        return retval;
	    }
	    childlist = childlist->next;
	}
	//printf("upCritRecursive\n");
    } else {
        return retval;
    }
}

struct treeNode* findNode(struct treeNode* node, CXCursor cursor, CXTranslationUnit cxtup) {
    depth++;
    struct treeNode* retval = NULL;
    if(node->validcursor == true) {
        if(clang_equalCursors(cursor, node->cursor)) {
	    return node;
	}
    }
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    retval = findNode(childlist->node, cursor, cxtup);
	    if(retval != NULL) {
	        return retval;
	    }
	    childlist = childlist->next;
	}
	//printf("upCritRecursive\n");
    }
    return NULL;
}

void findCrits(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    //debugNode(node, cxtup);
    if(node->modified > 0) {
        if(node->newContent != NULL) {
	    if(strcmp(node->newContent, &("\nstart")) == 0) {
	        struct treeNode* currnode2 = node;
	        while(currnode2->parent != NULL) {
		    currnode2->modifiedNodes = NULL;
		    currnode2 = currnode2->parent;
		}
	        struct criticalSection* newcrit = malloc(sizeof(struct criticalSection));
		//newcrit->returnstmts = NULL;
		newcrit->accessedvars = NULL;
		newcrit->next = NULL;
		struct criticalSection* currcrit = crits;
		while(currcrit != NULL && currcrit->next != NULL) {
		    currcrit = currcrit->next;
		}
		struct treeListNode* currnode = node->parent->children;
		struct treeListNode* lastnode = NULL;
		newcrit->nodelist = NULL;
		newcrit->nodesafter = NULL;
		bool inCrit = false;
		bool first = true;
		bool afterCrit = false;
		struct treeListNode* list;
		struct treeListNode* critlist;
		//struct treeListNode* beforeStart;
		//struct treeListNode* afterEnd;
		while(currnode != NULL) {
		  //printf("0|currnode: %lX\n", currnode);
		  if(currnode->node->newContent != NULL) {
		    //printf("currnode->node->newContent = %s\n", currnode->node->newContent);
		  } else {
		    //printf("currnode->node->newContent = NULL\n");
		  }
		    if(currnode->node->newContent != NULL && (strcmp(currnode->node->newContent, "\nstart") == 0)) {
		      //printf("1\n");
		        //newcrit->containingNode = currnode->node->parent;
		        //lastnode->next = currnode->next;
		        inCrit = true;
			if(first == true) {
			    node->parent->children = currnode->next;
			    newcrit->needsRefactoring = false;
			} else {
			    lastnode->next = currnode->next;
			    newcrit->needsRefactoring = true;
			}
			newcrit->needsWait = false;
			//currnode = currnode->next;
			//newcrit->lockvarNode = currnode->next->node->children->next->node;
		    } else if(currnode->node->newContent != NULL && (strcmp(currnode->node->newContent, "\nend") == 0)) {
		      //printf("2\n");
		        lastnode->next = currnode->next;
		        //currnode = currnode->next;
			//currnode = currnode->next;
		        //printf("-----\n");
		        inCrit = false;
			afterCrit = true;
		    } else if(inCrit == true && afterCrit == false) {
		      //printf("3\n");
		      //debugNode(currnode->node, cxtup);
		        if(newcrit->nodelist == NULL) {
			    //printf("newcrit->nodelist: %lX\n", newcrit->nodelist);
			    newcrit->nodelist = malloc(sizeof(struct treeListNode));
			    critlist = newcrit->nodelist;
			    //printf("critlist: %lX\n", critlist);
			    //printf("newcrit->nodelist: %lX\n", newcrit->nodelist);
			    critlist->node = currnode->node;
			    critlist->next = NULL;
		        } else {
			    //printf("3½\n");
			    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
			    //printf("critlist: %lX\n", critlist);
			    newnode->node = currnode->node;
			    //printf("(cl)node: %lX\n", newnode->node);
			    critlist->next = newnode;
			    //printf("critlist->next: %lX\n", critlist->next);
			    critlist = critlist->next;
			    //printf("critlist: %lX\n", critlist);
			    critlist->next = NULL;
			    //printf("newcrit->nodelist: %lX\n", newcrit->nodelist);
			}
		    } else if(afterCrit == true) {
		      //printf("4\n");
		      //debugNode(currnode->node, cxtup);
			newcrit->needsRefactoring = true;
		        if(newcrit->nodesafter == NULL) {
			  //printf("newcrit->nodesafter: %lX\n", newcrit->nodesafter);
			    newcrit->nodesafter = malloc(sizeof(struct treeListNode));
			    list = newcrit->nodesafter;
			  //printf("list: %lX\n", list);
			  //printf("newcrit->nodesafter: %lX\n", newcrit->nodesafter);
			    list->node = currnode->node;
			    list->next = NULL;
		        } else {
			  //printf("4½\n");
			    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
			  //printf("list: %lX\n", list);
			    newnode->node = currnode->node;
			  //printf("(l)node: %lX\n", newnode->node);
			    list->next = newnode;
			  //printf("list->next: %lX\n", list->next);
			    list = list->next;
			  //printf("list: %lX\n", list);
			    list->next = NULL;
			  //printf("newcrit->nodesafter: %lX\n", newcrit->nodesafter);
			}
		    }
		    lastnode = currnode;
		    currnode = currnode->next;
		    //printf("5\n");
		    first = false;
		}
		//printf("-----\n");
		//printf("newcrit->nodelist: %lX\n", newcrit->nodelist);
		//printf("newcrit->nodesafter: %lX\n", newcrit->nodesafter);
		//newcrit->nodelist = critlist;
		//newcrit->nodesafter = list;
		/*printf("-----\n");
		printf("newcrit->nodelist: %lX\n", newcrit->nodelist);
		printf("newcrit->nodesafter: %lX\n", newcrit->nodesafter);
		printf("newcrit->nodelist->next: %lX\n", newcrit->nodelist->next);
		printf("critlist->next: %lX\n", critlist->next);*/
		if(currcrit == NULL) {
		    crits = newcrit;
		} else {
		    currcrit->next = newcrit;
		}
	    }
        }
    } 
    if(node->children != NULL) {
      //debugNode(node, cxtup);
      //printf("node->children = %lX\n", node->children);
      //printf("node->children->node = %lX\n", node->children->node);
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    findCrits(childlist->node, cxtup);
	    //printf("childlist->next = %lX\n", childlist->next);
	    childlist = childlist->next;
	}
    }
    depth--;
}

int scannodes = 0;

struct treeListNode* scanlist = NULL;

void scanTreeIterative(CXTranslationUnit cxtup);

void scanTreeRecursive(struct treeNode* node, CXTranslationUnit cxtup);

void scanTree(struct treeNode* node, CXTranslationUnit cxtup) {
    scanTreeRecursive(node, cxtup);
    scanTreeIterative(cxtup);
}

void scanTreeIterative(CXTranslationUnit cxtup) {
    struct treeListNode* currnode = scanlist;
    struct treeNode** scanarray = malloc(scannodes*sizeof(struct treeNode));
    int n = 0;
    while(currnode != NULL) {
        scanarray[n] = currnode->node;
        printf("[%i]: 0x%lX\n", n, scanarray[n]);
        //debugNode(scanarray[n], cxtup);
        currnode = currnode->next;
        n++;
    }
    currnode = scanlist;
    struct treeListNode* next = currnode;
    while(next != NULL) {
        currnode = next;
	next = currnode->next;
	free(currnode);
    }
    printf("---\n");
    for(int i = 0; i < scannodes; i++) {
        printf("for(i = %i; i < %i; i++)\n", i, scannodes);
        printf("[%i]: 0x%lX\n", i, scanarray[i]);
	CXString cdisplaystring = clang_getCursorDisplayName(scanarray[i]->cursor);
	char* str = clang_getCString(cdisplaystring);
	printf("Node's CursorDisplayName is \"%s\"\n", str);
	bool start = (strcmp(str, &("pthread_mutex_lock")) == 0);
	bool end = (strcmp(str, &("pthread_mutex_unlock")) == 0);
        printf("(%i|%i)\n", start, end);
        clang_disposeString(cdisplaystring);
        if(start == true) {
            struct treeNode** locklist = calloc(1, scannodes*sizeof(struct treeNode));
            struct treeListNode* nodelist = scanarray[i]->parent->children;
            while(nodelist != NULL) {
                if(nodelist->node->validcursor == true && clang_equalCursors(nodelist->node->cursor, scanarray[i]->cursor)) {
                    break;
                } else {
                    nodelist = nodelist->next;
                }
            }
            //locklist[0] = scanarray[i];
            n = 1;
            currnode = nodelist;
            locklist[0] = currnode->node;
            while(currnode->next != NULL) {
                printf("KUKEN");
                CXString cdisplaystring2 = clang_getCursorDisplayName(currnode->next->node->cursor);
                char* str2 = clang_getCString(cdisplaystring2);
                printf("str2: \"%s\"\n", str2);
                if(strcmp(str2, &("pthread_mutex_lock")) == 0) {
                    locklist[n] = currnode->next->node;
                    n++;
                    i++;
                    currnode = currnode->next;
                } else {
                    break;
                }
            }
            currnode = nodelist;
            bool braek = false;
            n = 0;
            while(currnode != NULL) {
                n++;
                currnode = currnode->next;
            }
            struct treeNode** scanarray2 = malloc(n*sizeof(struct treeNode));
            currnode = nodelist;
            int scanarray2size = n;
            while(currnode != NULL) {
                scanarray2[n-1] = currnode->node;
                printf("[%i]: 0x%lX\n", n-1, scanarray2[n-1]);
                //debugNode(scanarray2[n-1], cxtup);
                currnode = currnode->next;
                n--;
            }
            for(int i = 0; i < scanarray2size; i++) {
                struct treeListNode* currnode = malloc(sizeof(struct treeListNode));
                currnode->node = scanarray2[i];
                CXString cdisplaystring2 = clang_getCursorDisplayName(currnode->node->cursor);
                char* str2 = clang_getCString(cdisplaystring2);
                printf("istr2: \"%s\"\n", str2);
                if(strcmp(str2, &("pthread_mutex_unlock")) == 0) {
                    printf("Node matched!\n");
                    i++;
                    struct treeNode* unlocknode = currnode->node->children->next->node->children->node->children->node;
                    printf("%lX\n", locklist[0]);
                    for(int i = 0; locklist[i] != NULL; i++) {
                        struct treeNode* locknode = locklist[i]->children->next->node->children->node->children->node;
                        if(clang_equalCursors(clang_getCursorReferenced(unlocknode->cursor), clang_getCursorReferenced(locknode->cursor))) {
                            braek = true;
                            struct treeNode* newnode = malloc(sizeof(struct treeNode));
                            newnode->validcursor = false;
                            newnode->childCount = 0;
                            newnode->children = NULL;
                            newnode->modified = 1;
                            newnode->startline = -1;
                            newnode->startcol = -1;
                            newnode->newContent = malloc(7*sizeof(char));
                            strcpy(newnode->newContent, &"\nstart");
                            addChildBefore(locklist[0]->parent, newnode, locklist[0], cxtup);
                            newnode = malloc(sizeof(struct treeNode));
                            newnode->validcursor = false;
                            newnode->childCount = 0;
                            newnode->children = NULL;
                            newnode->modified = 1;
                            newnode->startline = -1;
                            newnode->startcol = -1;
                            newnode->newContent = malloc(5*sizeof(char));
                            strcpy(newnode->newContent, &"\nend");
                            addChildBefore(currnode->node->parent, newnode, currnode->node, cxtup);
                            for(int j = 0; clang_equalCursors(locklist[j]->cursor, locknode->cursor) != 0; j++) {
                                newnode = malloc(sizeof(struct treeNode));
                                newnode->childCount = 0;
                                newnode->children = NULL;
                                newnode->modified = 1;
                                newnode->newContent = printNode(locklist[j], cxtup);
                                newnode->modifiedNodes = NULL;
                                newnode->validcursor = false;
                                addChildBefore(locknode->parent, newnode, locknode, cxtup);
                                clearNode(locklist[j]);
                            }
                        } else {
                            printf("%lX == %lX\n", clang_getCursorReferenced(unlocknode->cursor), clang_getCursorReferenced(locknode->cursor));
                            debugNode(locknode, cxtup);
                            debugNode(unlocknode, cxtup);
                        }
                        if(braek == true) {
                            clang_disposeString(cdisplaystring2);
                            break;
                        }
                    }
                    if(braek == true) {
                        free(currnode);
                        break;
                    }
                }
                clang_disposeString(cdisplaystring2);
                free(currnode);
            }
            free(scanarray2);
            free(locklist);
            printf("%i < %i\n", i, scannodes);
            printf("(i < scannodes) == %i\n", (i < scannodes));
            printf("(0 < scannodes) == %i\n", (0 < scannodes));
        }
    //newnode->parent = node->parent;
    }
    free(scanarray);
}

void scanTreeRecursive(struct treeNode* node, CXTranslationUnit cxtup) {
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
		  /*struct treeNode* newnode = malloc(sizeof(struct treeNode));
		    newnode->validcursor = false;
		    newnode->childCount = 0;
		    newnode->children = NULL;
		    newnode->modified = 1;
		    newnode->startline = -1;
		    newnode->startcol = -1;
		    /*CXSourceLocation endloc = clang_getRangeEnd(range);
		    unsigned eline;
		    unsigned ecolumn;
		    clang_getFileLocation(endloc, NULL, &eline, &ecolumn, NULL);
		    //printf("node: %i, eline: %i, ecolumn: %i\n", newnode, eline, ecolumn);
		    newnode->startline = eline;
		    newnode->startcol = ecolumn;*/
                    /*if(start) {
		      //printf("--START--\n");
		        newnode->newContent = malloc(7*sizeof(char));
			strcpy(newnode->newContent, &"\nstart");
			addChildBefore(node->parent, newnode, node, cxtup);
			//dNode = newnode;
		    } else {
		      //printf("---END---\n");
		        newnode->newContent = malloc(5*sizeof(char));
			strcpy(newnode->newContent, &"\nend");
			addChildBefore(node->parent, newnode, node, cxtup); 
		    }*/
		    //newnode->parent = node->parent;
		    scannodes++;
		    struct treeListNode* newnode = malloc(sizeof(struct treeListNode));
		    newnode->node = node;
		    newnode->next = NULL;
		    if(scanlist == NULL) {
		        scanlist = newnode;
		    } else {
		        struct treeListNode* currnode = scanlist;
		        while(currnode != NULL && currnode->next != NULL) {
			    currnode = currnode->next;
		        }
			currnode->next = newnode;
		    }
		}
		clang_disposeString(cdisplaystring);
	    }
	}
    } 
    if(node->children != NULL) {
        struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    scanTreeRecursive(childlist->node, cxtup);
	    childlist = childlist->next;
	}
    }
    depth--;
}

/*void modifyTree(struct treeNode* node, CXTranslationUnit cxtup) {
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
		    /*CXSourceLocation endloc = clang_getRangeEnd(range);
		    unsigned eline;
		    unsigned ecolumn;
		    clang_getFileLocation(endloc, NULL, &eline, &ecolumn, NULL);
		    //printf("node: %i, eline: %i, ecolumn: %i\n", newnode, eline, ecolumn);
		    newnode->startline = eline;
		    newnode->startcol = ecolumn;*/
                 /*if(start) {
		        newnode->newContent = malloc(41*sizeof(char));
			strcpy(newnode->newContent, &"\n    printf(\"Critical section start!\");");
			addChildAfter(node->parent, newnode, node);
		    } else {
		        newnode->newContent = malloc(39*sizeof(char));
			strcpy(newnode->newContent, &"\n    printf(\"Critical section end!\");");
			addChildBefore(node->parent, newnode, node);
		    }
		    //newnode->parent = node->parent;
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
}*/

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

void debugTree2(struct treeNode* node, CXTranslationUnit cxtup) {
    tnodes++;
    depth++;
    if(depth > moddepth) {
        moddepth = depth;
    }
    if(node->validcursor == true) {
        if(node->modified > 0) {
	    printf("\n%i@%i[\n", node->modified, depth);
	    struct treeListNode* modlist = node->modifiedNodes;
	    while(modlist != NULL) {
                printf("    %s\n", modlist->node->newContent);
                if(modlist->node->validcursor == true) {
                    debugUnmodifiedNode(modlist->node, cxtup);
                }
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
	if(clang_Location_isFromMainFile(rstart) != 0) {
	    printf("VN");
	    for(int i = 0; i < depth; i++) {
	        printf("%s", space);
		//printf("%i", depth);
	    }
            printf("%i@%i, [%i]:%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing ", node->modified, depth, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
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
	    if(node->children != NULL) {
	        if(clang_Location_isFromMainFile(rstart) != 0) {
	            printf("->\n");
		}
	        struct treeListNode* childlist = node->children;
	        while(childlist != NULL) {
		  debugTree2(childlist->node, cxtup);
		    childlist = childlist->next;
		}
	    } else {
	        if(clang_Location_isFromMainFile(rstart) != 0) {
	            printf("\n");
		}
	    }
	} else {
	    printf("BN");          
        }
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
    } else {
        printf("IN");
	printf("\n%i@%i[\n", node->modified, depth);
        tnodes++;
        moddednodes++;
        for(int i = 0; i < depth; i++) {
	    printf("%s", space);
	}
	printf("%i, %s", node->modified, node->newContent);
        printf("\n(L%u:C%u)\n-[]-\n", node->startline, node->startcol);
	if(node->children != NULL) {
	    printf("->\n");
	    struct treeListNode* childlist = node->children;
	    while(childlist != NULL) {
	        debugTree2(childlist->node, cxtup);
	        childlist = childlist->next;
	    }
	} else {
	    printf("\n");
	}
    }
    depth--;
}

void debugTree3(struct treeNode* node, CXTranslationUnit cxtup) {
    tnodes++;
    depth++;
    if(depth > moddepth) {
        moddepth = depth;
    }
    if(node->validcursor == true) {
        if(node->modified > 0) {
	    printf("\n%i@%i[\n", node->modified, depth);
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
	if(clang_Location_isFromMainFile(rstart) != 0) {
	    printf("VN");
	} else {
	    printf("BN");
	}
	for(int i = 0; i < depth; i++) {
	    printf("%s", space);
	    //printf("%i", depth);
	}
	printf("%i@%i, [%i]:%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing ", node->modified, depth, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
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
	        debugTree3(childlist->node, cxtup);
		childlist = childlist->next;
	    }
	} else {
	    printf("\n");
	}
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
    } else {
        printf("IN");
	printf("\n%i@%i[\n", node->modified, depth);
        tnodes++;
        moddednodes++;
        for(int i = 0; i < depth; i++) {
	    printf("%s", space);
	}
	printf("%i, %s", node->modified, node->newContent);
	if(node->children != NULL) {
	    printf("->\n");
	    struct treeListNode* childlist = node->children;
	    while(childlist != NULL) {
	        debugTree3(childlist->node, cxtup);
	        childlist = childlist->next;
	    }
	} else {
	    printf("\n");
	}
    }
    depth--;
}

void debugNode(struct treeNode* node, CXTranslationUnit cxtup) {
    printf("Debugging treeNode: %012lX\n", node);
    if(node->validcursor == true) {
        if(node->modified > 0) {
	  printf("\n%i[\n", node->modified);
	    struct treeListNode* modlist = node->modifiedNodes;
	    /*printf("modlist: %i\n", modlist);
              printf("modnode: %i\n", modlist->node);*/
	    while(modlist != NULL) {
                printf("    %s\n", modlist->node->newContent);
                if(modlist->node->validcursor == true) {
                    debugUnmodifiedNode(modlist->node, cxtup);
                }
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
            printf("%i, [%i]:%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing ", node->modified, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
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
	    CXCursor cursor = clang_getCursorDefinition(node->cursor);
	    range = clang_getCursorExtent(cursor);
	    clang_tokenize(cxtup, range, &tokens, &numTokens);
	    printf("Definiton cursor contains %u tokens (", numTokens);
	    for(int i = 0; i<numTokens; i++) {
	        CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
	        printf("%s ", clang_getCString(tokenstring));
		clang_disposeString(tokenstring);
	    }
	    clang_disposeTokens(cxtup, tokens, numTokens);
	    printf(")");
            printf("\n");
	    //}
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
    } else {
        printf("Modded:\n");
        tnodes++;
        moddednodes++;
        for(int i = 0; i < depth; i++) {
	    printf("%s", space);
	    //printf("%i", depth);
	}
	printf("%i, %s", node->modified, node->newContent);
	printf("\n");
    }
}

void debugUnmodifiedNode(struct treeNode* node, CXTranslationUnit cxtup) {
    printf("Debugging treeNode: %012lX\n", node);
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
	//if(clang_Location_isFromMainFile(rstart) != 0) {
            printf("%i, [%i]:%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing ", node->modified, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
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
	    CXCursor cursor = clang_getCursorDefinition(node->cursor);
	    range = clang_getCursorExtent(cursor);
	    clang_tokenize(cxtup, range, &tokens, &numTokens);
	    printf("Definiton cursor contains %u tokens (", numTokens);
	    for(int i = 0; i<numTokens; i++) {
	        CXString tokenstring = clang_getTokenSpelling(cxtup, tokens[i]);
	        printf("%s ", clang_getCString(tokenstring));
		clang_disposeString(tokenstring);
	    }
	    clang_disposeTokens(cxtup, tokens, numTokens);
	    printf(")");
            printf("\n");
	    //}
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
    } else {
        printf("Modded:\n");
        tnodes++;
        moddednodes++;
        for(int i = 0; i < depth; i++) {
	    printf("%s", space);
	    //printf("%i", depth);
	}
	printf("%i, %s", node->modified, node->newContent);
	printf("\n");
    }
}

void debugCrits(struct criticalSection* crits, CXTranslationUnit cxtup) {
    struct criticalSection* currcrit = crits;
    while(currcrit != NULL) {
        printCrit(currcrit, cxtup);
	currcrit = currcrit->next;
    }
}

void printCrit(struct criticalSection* crit, CXTranslationUnit cxtup) {
    printf("NeedsRefactoring: %i\n", crit->needsRefactoring);
    printf("NeedsWait: %i\n", crit->needsWait);
    //printf("lockvarNode: %012lX\n", crit->lockvarNode);
    //debugNode(crit->lockvarNode, cxtup);
    printf("\n");
    /*printf("lockNode: %012lX\n", crit->lockNode);
    debugNode(crit->lockNode, cxtup);
    printf("\n");
    printf("unlockNode: %012lX\n", crit->unlockNode);
    debugNode(crit->unlockNode, cxtup);
    printf("\n");*/
    if(crit->accessedvars != NULL) {
      //printf("lockvar: %s\n", crit->lockvar->name);
	struct variable* currvar = crit->accessedvars;
	printf("accessedvars: \n");
	while(currvar != NULL) {
	    printf("    %s (locality: %i, threadLocal: %i, pointer: %i, needsreturn: %i)\n", currvar->name, currvar->locality, currvar->threadLocal, currvar->pointer, currvar->needsreturn);
	    currvar = currvar->next;
	}
    } else {
        printf("crit->accessedvars == NULL\n");
    }
    int listsize = 0;
    struct treeListNode* currnode = crit->nodelist;
    while(currnode != NULL) {
        printf("%012lX\n", currnode);
        listsize++;
	currnode = currnode->next;
    }
    printf("nodelist: %012lX\n", crit->nodelist);
    printf("nodelist (%i nodes): \n", listsize);
    currnode = crit->nodelist;
    printf("\n");
    while(currnode != NULL) {
        debugNode(currnode->node, cxtup);
	printf("\n");
	currnode = currnode->next;
    }
    printf("\n");
    listsize = 0;
    currnode = crit->nodesafter;
    while(currnode != NULL) {
        printf("%012lX\n", currnode);
        listsize++;
	currnode = currnode->next;
    }
    printf("nodesafter: %012lX\n", crit->nodesafter);
    printf("nodesafter (%i nodes): \n", listsize);
    currnode = crit->nodesafter;
    printf("\n");
    while(currnode != NULL) {
        debugNode(currnode->node, cxtup);
	printf("\n");
	currnode = currnode->next;
    }
    printf("\n");
}

void freeVariable(struct variable* var) {
  /*printf("kuken\n");
    printf("%064lX");
    printf("&s\n", var->name);
    debugNode(var->decl, tree->cxtup);*/
    free(var->name);
    free(var->typename);
    if(var->next != NULL) {
        freeVariable(var->next);
    }
    free(var);
}

void freeNodeList(struct treeListNode* node) {
  //printf("\nnode: %lX\n", node);
  //printf("\n(%lX == %lX) = %lX", 0, NULL, (0 == NULL));
    if(node != NULL && node->next != NULL) {
        freeNodeList(node->next);
    } 
    free(node);
}

void freeCrit(struct criticalSection* crit) {
    printCrit(crit, tree->cxtup);
    freeVariable(crit->accessedvars);
    freeNodeList(crit->nodelist);
    freeNodeList(crit->nodesafter);
    free(crit);
}

void freeCrits(struct criticalSection* crit) {
    if(crit != NULL && crit->next != NULL) {
        freeCrits(crit->next);
    }
    freeCrit(crit);
}


void printQDUsage() {
    printf("USAGE:\n");
    printf("\n");
    printf("\tqdtrans [FILENAME]\n");
}
