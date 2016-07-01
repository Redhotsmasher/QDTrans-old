#include "../src/qdtrans.h"

int counter = 0;

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
    debugTree2(tree->root, tree->cxtup);
    if (tree->root->modified != 2) {
        counter = 1;
    }
    //printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", tree->error, tree->nodes, tree->unmodifiedDepth);
    //printf("Final max depth: %i\n", moddepth);
    //freeCrits(crits);
    disposeTree(tree);
    return counter;
}
