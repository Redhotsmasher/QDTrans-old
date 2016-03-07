#include <clang-c/Index.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

char* kinds[701];

char* space = "  ";

void printTree(struct treeNode* node, CXTranslationUnit cxtup) {
    depth++;
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
    char* str1 = kinds[cursorkind];
    char* str2 = clang_getCString(typestring);
    char* str3 = clang_getCString(cdisplaystring);
    char* str4 = clang_getCString(cspellstring);
    if(clang_Location_isFromMainFile(rstart) != 0) {
      for(int i = 0; i < depth; i++) {
	  printf("%s", space);
	  //printf("%i", depth);
	}
        printf("%s([%i]):%s \"%s\"; %s (L%u:C%u-L%u:C%u), containing ", str1, cursorkind, str2, str3, str4, *curlines, *curcols, *curlinee, *curcole);
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
	clang_disposeString(typestring);
	clang_disposeString(cdisplaystring);
	clang_disposeString(cspellstring);
	free(curlines);
	free(curlinee);
	free(curcols);
	free(curcole);
	if(node->children != NULL) {
	    if(clang_Location_isFromMainFile(rstart) != 0) {
	        printf("->\n");
	    }
	    struct treeListNode* childlist = node->children;
	    while(childlist != NULL) {
	        printTree(childlist->node, cxtup);
		childlist = childlist->next;
	    }
	} else {
	    if(clang_Location_isFromMainFile(rstart) != 0) {
	        printf("\n");
	    }
	}
    }
    depth--;
}

int main(int argc, char *argv[]) {
  
    if(argc == 2) {
        filename = argv[1];
    } else {
      //filename = "Test1.c";
        printUsage();
        goto END;
    }
  

  //filename = "Test1t.c";
    filefile = fopen(filename, "r+");
  
    kinds[1] = "CXCursor_UnexposedDecl";
    kinds[2] = "CXCursor_StructDecl";
    kinds[3] = "CXCursor_UnionDecl";
    kinds[4] = "CXCursor_ClassDecl";
    kinds[5] = "CXCursor_EnumDecl";
    kinds[6] = "CXCursor_FieldDecl";
    kinds[7] = "CXCursor_EnumConstantDecl";
    kinds[8] = "CXCursor_FunctionDecl";
    kinds[9] = "CXCursor_VarDecl";
    kinds[10] = "CXCursor_ParmDecl";
    kinds[11] = "CXCursor_ObjCInterfaceDecl";
    kinds[12] = "CXCursor_ObjCCategoryDecl";
    kinds[13] = "CXCursor_ObjCProtocolDecl";
    kinds[14] = "CXCursor_ObjCPropertyDecl";
    kinds[15] = "CXCursor_ObjCIvarDecl";
    kinds[16] = "CXCursor_ObjCInstanceMethodDecl";
    kinds[17] = "CXCursor_ObjCClassMethodDecl";
    kinds[18] = "CXCursor_ObjCImplementationDecl";
    kinds[19] = "CXCursor_ObjCCategoryImplDecl";
    kinds[20] = "CXCursor_TypedefDecl";
    kinds[21] = "CXCursor_CXXMethod";
    kinds[22] = "CXCursor_Namespace";
    kinds[23] = "CXCursor_LinkageSpec";
    kinds[24] = "CXCursor_Constructor";
    kinds[25] = "CXCursor_Destructor";
    kinds[26] = "CXCursor_ConversionFunction";
    kinds[27] = "CXCursor_TemplateTypeParameter";
    kinds[28] = "CXCursor_NonTypeTemplateParameter";
    kinds[29] = "CXCursor_TemplateTemplateParameter";
    kinds[30] = "CXCursor_FunctionTemplate";
    kinds[31] = "CXCursor_ClassTemplate";
    kinds[32] = "CXCursor_ClassTemplatePartialSpecialization";
    kinds[33] = "CXCursor_NamespaceAlias";
    kinds[34] = "CXCursor_UsingDirective";
    kinds[35] = "CXCursor_UsingDeclaration";
    kinds[36] = "CXCursor_TypeAliasDecl";
    kinds[37] = "CXCursor_ObjCSynthesizeDecl";
    kinds[38] = "CXCursor_ObjCDynamicDecl";
    kinds[39] = "CXCursor_CXXAccessSpecifier";

    kinds[40] = "CXCursor_ObjCSuperClassRef";
    kinds[41] = "CXCursor_ObjCProtocolRef";
    kinds[42] = "CXCursor_ObjCClassRef";
    kinds[43] = "CXCursor_TypeRef";
    kinds[44] = "CXCursor_CXXBaseSpecifier";
    kinds[45] = "CXCursor_TemplateRef";
    kinds[46] = "CXCursor_NamespaceRef";
    kinds[47] = "CXCursor_MemberRef";
    kinds[48] = "CXCursor_LabelRef";
    kinds[49] = "CXCursor_OverloadedDeclRef";

    kinds[50] = "CXCursor_VariableRef";

    kinds[70] = "CXCursor_InvalidFile";
    kinds[71] = "CXCursor_NoDeclFound";
    kinds[72] = "CXCursor_NotImplemented";
    kinds[73] = "CXCursor_InvalidCode";

    kinds[100] = "CXCursor_UnexposedExpr";
    kinds[101] = "CXCursor_DeclRefExpr";
    kinds[102] = "CXCursor_MemberRefExpr";
    kinds[103] = "CXCursor_CallExpr";
    kinds[104] = "CXCursor_ObjCMessageExpr";
    kinds[105] = "CXCursor_BlockExpr";
    kinds[106] = "CXCursor_IntegerLiteral";
    kinds[107] = "CXCursor_FloatingLiteral";
    kinds[108] = "CXCursor_ImaginaryLiteral";
    kinds[109] = "CXCursor_StringLiteral";
    kinds[110] = "CXCursor_CharacterLiteral";
    kinds[111] = "CXCursor_ParenExpr";
    kinds[112] = "CXCursor_UnaryOperator";
    kinds[113] = "CXCursor_ArraySubscriptExpr";
    kinds[114] = "CXCursor_BinaryOperator";
    kinds[115] = "CXCursor_CompoundAssignOperator";
    kinds[116] = "CXCursor_ConditionalOperator";
    kinds[117] = "CXCursor_CStyleCastExpr";
    kinds[118] = "CXCursor_CompoundLiteralExpr";
    kinds[119] = "CXCursor_InitListExpr";
    kinds[120] = "CXCursor_AddrLabelExpr";
    kinds[120] = "CXCursor_StmtExpr";
    kinds[122] = "CXCursor_GenericSelectionExpr";
    kinds[123] = "CXCursor_GNUNullExpr";
    kinds[124] = "CXCursor_CXXStaticCastExpr";
    kinds[125] = "CXCursor_CXXDynamicCastExpr";
    kinds[126] = "CXCursor_CXXReinterpretCastExpr";
    kinds[127] = "CXCursor_CXXConstCastExpr";
    kinds[128] = "CXCursor_CXXFunctionalCastExpr";
    kinds[129] = "CXCursor_CXXTypeidExpr";
    kinds[130] = "CXCursor_CXXBoolLiteralExpr";
    kinds[131] = "CXCursor_CXXNullPtrLiteralExpr";
    kinds[132] = "CXCursor_CXXThisExpr";
    kinds[133] = "CXCursor_CXXThrowExpr";
    kinds[134] = "CXCursor_CXXNewExpr";
    kinds[135] = "CXCursor_CXXDeleteExpr";
    kinds[136] = "CXCursor_UnaryExpr";
    kinds[137] = "CXCursor_ObjCStringLiteral";
    kinds[138] = "CXCursor_ObjCEncodeExpr";
    kinds[139] = "CXCursor_ObjCSelectorExpr";
    kinds[140] = "CXCursor_ObjCProtocolExpr";
    kinds[141] = "CXCursor_ObjCBridgedCastExpr";
    kinds[142] = "CXCursor_PackExpansionExpr";
    kinds[143] = "CXCursor_SizeOfPackExpr";
    kinds[144] = "CXCursor_LambdaExpr";
    kinds[145] = "CXCursor_ObjCBoolLiteralExpr";
    kinds[146] = "CXCursor_ObjCSelfExpr";

    kinds[200] = "CXCursor_UnexposedStmt";
    kinds[201] = "CXCursor_LabelStmt";
    kinds[202] = "CXCursor_CompoundStmt";
    kinds[203] = "CXCursor_CaseStmt";
    kinds[204] = "CXCursor_DefaultStmt";
    kinds[205] = "CXCursor_IfStmt";
    kinds[206] = "CXCursor_SwitchStmt";
    kinds[207] = "CXCursor_WhileStmt";
    kinds[208] = "CXCursor_DoStmt";
    kinds[209] = "CXCursor_ForStmt";
    kinds[210] = "CXCursor_GotoStmt";
    kinds[211] = "CXCursor_IndirectGotoStmt";
    kinds[212] = "CXCursor_ContinueStmt";
    kinds[213] = "CXCursor_BreakStmt";
    kinds[214] = "CXCursor_ReturnStmt";
    kinds[215] = "CXCursor_AsmStmt";
    kinds[216] = "CXCursor_ObjCAtTryStmt";
    kinds[217] = "CXCursor_ObjCAtCatchStmt";
    kinds[218] = "CXCursor_ObjCAtFinallyStmt";
    kinds[219] = "CXCursor_ObjCAtThrowStmt";
    kinds[220] = "CXCursor_ObjCAtSynchronizedStmt";
    kinds[221] = "CXCursor_ObjCAutoreleasePoolStmt";
    kinds[222] = "CXCursor_ObjCForCollectionStmt";
    kinds[223] = "CXCursor_CXXCatchStmt";
    kinds[224] = "CXCursor_CXXTryStmt";
    kinds[225] = "CXCursor_CXXForRangeStmt";
    kinds[226] = "CXCursor_SEHTryStmt";
    kinds[227] = "CXCursor_SEHExceptStmt";
    kinds[228] = "CXCursor_SEHFinallyStmt";
    kinds[229] = "CXCursor_MSAsmStmt";
    kinds[230] = "CXCursor_NullStmt";
    kinds[231] = "CXCursor_DeclStmt";
    kinds[232] = "CXCursor_OMPParallelDirective";
    kinds[233] = "CXCursor_OMPSimdDirective";
    kinds[234] = "CXCursor_OMPForDirective";
    kinds[235] = "CXCursor_OMPSectionsDirective";
    kinds[236] = "CXCursor_OMPSectionDirective";
    kinds[237] = "CXCursor_OMPSingleDirective";
    kinds[238] = "CXCursor_OMPParallelForDirective";
    kinds[239] = "CXCursor_OMPParallelSectionsDirective";
    kinds[240] = "CXCursor_OMPTaskDirective";
    kinds[241] = "CXCursor_OMPMasterDirective";
    kinds[242] = "CXCursor_OMPCriticalDirective";
    kinds[243] = "CXCursor_OMPTaskyieldDirective";
    kinds[244] = "CXCursor_OMPBarrierDirective";
    kinds[245] = "CXCursor_OMPTaskwaitDirective";
    kinds[246] = "CXCursor_OMPFlushDirective";
    kinds[247] = "CXCursor_SEHLeaveStmt";
    kinds[248] = "CXCursor_OMPOrderedDirective";
    kinds[249] = "CXCursor_OMPAtomicDirective";
    kinds[250] = "CXCursor_OMPForSimdDirective";
    kinds[251] = "CXCursor_OMPParallelForSimdDirective";
    kinds[252] = "CXCursor_OMPTargetDirective";
    kinds[253] = "CXCursor_OMPTeamsDirective";
    kinds[254] = "CXCursor_OMPTaskgroupDirective";
    kinds[255] = "CXCursor_OMPCancellationPointDirective";
    kinds[256] = "CXCursor_OMPCancelDirective";

    kinds[300] = "CXCursor_TranslationUnit";

    kinds[400] = "CXCursor_UnexposedAttr";
    kinds[401] = "CXCursor_IBActionAttr";
    kinds[402] = "CXCursor_IBOutletAttr";
    kinds[403] = "CXCursor_IBOutletCollectionAttr";
    kinds[404] = "CXCursor_CXXFinalAttr";
    kinds[405] = "CXCursor_CXXOverrideAttr";
    kinds[406] = "CXCursor_AnnotateAttr";
    kinds[407] = "CXCursor_AsmLabelAttr";
    kinds[408] = "CXCursor_PackedAttr";
    kinds[409] = "CXCursor_PureAttr";
    kinds[410] = "CXCursor_ConstAttr";
    kinds[411] = "CXCursor_NoDuplicateAttr";
    kinds[412] = "CXCursor_CUDAConstantAttr";
    kinds[413] = "CXCursor_CUDADeviceAttr";
    kinds[414] = "CXCursor_CUDAGlobalAttr";
    kinds[415] = "CXCursor_CUDAHostAttr";
    kinds[416] = "CXCursor_CUDASharedAttr";       

    kinds[500] = "CXCursor_PreprocessingDirective";
    kinds[501] = "CXCursor_MacroDefinition";
    kinds[502] = "CXCursor_MacroExpansion";
    kinds[503] = "CXCursor_InclusionDirective";

    kinds[600] = "CXCursor_ModuleImportDecl";

    kinds[700] = "CXCursor_OverloadCandidate";
    
    
    /*CXIndex cxi = clang_createIndex(1, 0);
    unsigned flags = (CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_Incomplete);
    //const char* filename = "/home/redhotsmasher/QDTrans/src/Test1.c";
    //CXTranslationUnit cxtu = clang_createTranslationUnitFromSourceFile (cxi, filename, 0, NULL, 0, NULL);
    CXTranslationUnit cxtup = clang_createTranslationUnit(cxi, filename);
    printf("cxi: %lx, cxtup: %lx\n", cxi, cxtup);
    enum CXErrorCode error = clang_parseTranslationUnit2(cxi, filename, NULL, 0, NULL, 0, flags, &cxtup);
    file = clang_getFile (cxtup, filename);
    printf("cxi: %lx, cxtup: %lx\n", cxi, cxtup);
    CXCursor cursor = clang_getTranslationUnitCursor(cxtup);
    tree = malloc(sizeof(struct treeNode));
    tree->cursor = cursor;
    tree->children = NULL;
    currentnode = tree;
    clang_visitChildren(cursor, visitor, NULL);
    visitRecursive(tree->children);*/
    struct nodeTree* tree = generateTree(filename);
    printTree(tree->root, tree->cxtup);
    //clang_disposeTranslationUnit(cxtup);
    //clang_disposeIndex(cxi);
    disposeTree(tree);
    printf("\nError Code: %i\nTotal nodes: %i\nMaximum depth: %i\n", tree->error, tree->nodes, tree->unmodifiedDepth);
END:
    return 0;
}

int printUsage() {
    printf("USAGE:\n");
    printf("\n");
    printf("\tdumptree [FILENAME]\n");
}
