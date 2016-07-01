#include "../src/qdtrans.h"

int counter = 0;

//struct criticalSection* crits = NULL;

int main(int argc, char *argv[]) {
    filename = "./tests/testfiles/Test0.c";
    tree = generateTree(filename);
    //printf("qdtranscxtup: %lx\n", tree->cxtup);
    scanTree(tree->root, tree->cxtup);
    printTree(tree);
    findCrits(tree->root, tree->cxtup);
    debugCrits(crits, tree->cxtup);
    scanCrits(tree->cxtup);
    //refactorCrits(tree->root, tree->cxtup);
    //printf("Snopp!\n");
    //depth = 0;
    //printTree(tree);
    if(crits->accessedvars == NULL) {
        printf("crits->accessedvars == NULL. This is bad.\n");
    } else if(strcmp(crits->accessedvars->name, &"num") == 0) {
	counter++;
    }
    //printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", tree->error, tree->nodes, tree->unmodifiedDepth);
    //printf("Final max depth: %i\n", moddepth);
    //freeCrits(crits);
    disposeTree(tree);
END:
    return (counter - 1);
}
