#include "qdtrans.h"

int counter = 0;

int depth = 0;

int main(int argc, char *argv[]) {
    filename = "./tests/testfiles/Test0.c";
    tree = generateTree(filename);
    //printf("qdtranscxtup: %lx\n", tree->cxtup);
    scanTree(tree->root, tree->cxtup);
    //findCrits(tree->root, tree->cxtup);
    //scanCrits(tree->cxtup);
    //debugCrits(crits, tree->cxtup);
    //refactorCrits(tree->root, tree->cxtup);
    //printf("Snopp!\n");
    //depth = 0;
    //printTree(tree);
    traverseTree(tree->root, tree->cxtup);
    //printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", tree->error, tree->nodes, tree->unmodifiedDepth);
    //printf("Final max depth: %i\n", moddepth);
    //freeCrits(crits);
    disposeTree(tree);
END:
    return (counter - 3);
}

int traverseTree (struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    if(node->children != NULL) {
	struct treeListNode* childlist = node->children;
	while(childlist != NULL) {
	    if(childlist->node->modified > 0) {
	        if(depth == 1) {
		     counter++;
	        }
	        checkTree(childlist->node, cxtup);
	    }
	    childlist = childlist->next;
	}
    depth--;
    }
}

int checkTree (struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
    depth++;
    struct treeListNode* childlist = node->children->node->children;
    struct treeNode* nodeToTest = childlist->next->next;
    if ((nodeToTest->node->modified == 1) && (strcmp(nodeToTest->node->newContent, &"\nstart") == 0)) {
        counter++:
    }
    nodeToTest = nodeToTest->next->next->next;
    if ((nodeToTest->node->modified == 1) && (strcmp(nodeToTest->node->newContent, &"\nend") == 0)) {
        counter++:
    }
    depth--;
    depth--;
}
