#include <stdio.h>
#include <string.h>
#include "qdtrans.h"

int main(int argc, char *argv[]) {
    if(argc == 2) {
        filename = argv[1];
    } else {
        //filename = "Test1.c";
        printQDUsage();
        goto END;
    }
    tree = generateTree(filename);
    //printf("qdtranscxtup: %lx\n", tree->cxtup);
    //printf("crits: %012lX\n", crits);
    scanTree(tree->root, tree->cxtup);
    //printf("crits: %012lX\n", crits);
    findCrits(tree->root, tree->cxtup);
    scanCrits(tree->cxtup);
    //debugCrits(crits, tree->cxtup);
    refactorCrits(tree->root, tree->cxtup);
    //printf("Snopp!\n");
    depth = 0;
    printf("\n\n---\n\n");
    debugTree2(tree->root, tree->cxtup);
    printf("\n\n---\n\n");
    /*char* outfilename = filename+strlen(filename)-1;
    sprintf(outfilename, "qd.c");
    outfilename = filename;
    printf("Saving to %s...\n", outfilename);
    freopen(outfilename, "w", stdout);*/
    printTree(tree);
    //fclose(stdout);
    printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", tree->error, tree->nodes, tree->unmodifiedDepth);
    printf("Final max depth: %i\n", moddepth);
    freeCrits(crits);
    disposeTree(tree);
END:
    return 0;
}
