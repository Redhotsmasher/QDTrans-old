#include "clang+llvm-3.7.0-x86_64-linux-gnu-ubuntu-14.04/include/clang-c/Index.h"
#include <stdio.h>

CXFile* file;
unsigned* curline;
unsigned* parline;
unsigned* curline2;
unsigned* parline2;
unsigned* curcol;
unsigned* parcol;
unsigned* curcol2;
unsigned* parcol2;

char* kinds[202] = {"CXCursor_UnexposedDecl", 
"CXCursor_StructDecl", 
"CXCursor_UnionDecl", 
"CXCursor_ClassDecl", 
"CXCursor_EnumDecl", 
"CXCursor_FieldDecl", 
"CXCursor_EnumConstantDecl", 
"CXCursor_FunctionDecl", 
"CXCursor_VarDecl", 
"CXCursor_ParmDecl", 
"CXCursor_ObjCInterfaceDecl", 
"CXCursor_ObjCCategoryDecl", 
"CXCursor_ObjCProtocolDecl", 
"CXCursor_ObjCPropertyDecl", 
"CXCursor_ObjCIvarDecl", 
"CXCursor_ObjCInstanceMethodDecl", 
"CXCursor_ObjCClassMethodDecl", 
"CXCursor_ObjCImplementationDecl", 
"CXCursor_ObjCCategoryImplDecl", 
"CXCursor_TypedefDecl", 
"CXCursor_CXXMethod", 
"CXCursor_Namespace", 
"CXCursor_LinkageSpec", 
"CXCursor_Constructor", 
"CXCursor_Destructor", 
"CXCursor_ConversionFunction", 
"CXCursor_TemplateTypeParameter", 
"CXCursor_NonTypeTemplateParameter", 
"CXCursor_TemplateTemplateParameter", 
"CXCursor_FunctionTemplate", 
"CXCursor_ClassTemplate", 
"CXCursor_ClassTemplatePartialSpecialization", 
"CXCursor_NamespaceAlias", 
"CXCursor_UsingDirective", 
"CXCursor_UsingDeclaration", 
"CXCursor_TypeAliasDecl", 
"CXCursor_ObjCSynthesizeDecl", 
"CXCursor_ObjCDynamicDecl", 
"CXCursor_CXXAccessSpecifier", 
"CXCursor_FirstDecl", 
"CXCursor_LastDecl", 
"CXCursor_FirstRef", 
"CXCursor_ObjCSuperClassRef", 
"CXCursor_ObjCProtocolRef", 
"CXCursor_ObjCClassRef", 
"CXCursor_TypeRef", 
"CXCursor_CXXBaseSpecifier", 
"CXCursor_TemplateRef", 
"CXCursor_NamespaceRef", 
"CXCursor_MemberRef", 
"CXCursor_LabelRef", 
"CXCursor_OverloadedDeclRef", 
"CXCursor_VariableRef", 
"CXCursor_LastRef", 
"CXCursor_FirstInvalid", 
"CXCursor_InvalidFile", 
"CXCursor_NoDeclFound", 
"CXCursor_NotImplemented", 
"CXCursor_InvalidCode", 
"CXCursor_LastInvalid", 
"CXCursor_FirstExpr", 
"CXCursor_UnexposedExpr", 
"CXCursor_DeclRefExpr", 
"CXCursor_MemberRefExpr", 
"CXCursor_CallExpr", 
"CXCursor_ObjCMessageExpr", 
"CXCursor_BlockExpr", 
"CXCursor_IntegerLiteral", 
"CXCursor_FloatingLiteral", 
"CXCursor_ImaginaryLiteral", 
"CXCursor_StringLiteral", 
"CXCursor_CharacterLiteral", 
"CXCursor_ParenExpr", 
"CXCursor_UnaryOperator", 
"CXCursor_ArraySubscriptExpr", 
"CXCursor_BinaryOperator", 
"CXCursor_CompoundAssignOperator", 
"CXCursor_ConditionalOperator", 
"CXCursor_CStyleCastExpr", 
"CXCursor_CompoundLiteralExpr", 
"CXCursor_InitListExpr", 
"CXCursor_AddrLabelExpr", 
"CXCursor_StmtExpr", 
"CXCursor_GenericSelectionExpr", 
"CXCursor_GNUNullExpr", 
"CXCursor_CXXDynamicCastExpr", 
"CXCursor_CXXReinterpretCastExpr", 
"CXCursor_CXXConstCastExpr", 
"CXCursor_CXXFunctionalCastExpr", 
"CXCursor_CXXTypeidExpr", 
"CXCursor_CXXBoolLiteralExpr", 
"CXCursor_CXXNullPtrLiteralExpr", 
"CXCursor_CXXThisExpr", 
"CXCursor_CXXThrowExpr", 
"CXCursor_CXXNewExpr", 
"CXCursor_CXXDeleteExpr", 
"CXCursor_UnaryExpr", 
"CXCursor_ObjCStringLiteral", 
"CXCursor_ObjCEncodeExpr", 
"CXCursor_ObjCSelectorExpr", 
"CXCursor_ObjCProtocolExpr", 
"CXCursor_ObjCBridgedCastExpr", 
"CXCursor_PackExpansionExpr", 
"CXCursor_SizeOfPackExpr", 
"CXCursor_LambdaExpr", 
"CXCursor_ObjCBoolLiteralExpr", 
"CXCursor_ObjCSelfExpr", 
"CXCursor_OMPArraySectionExpr", 
"CXCursor_LastExpr", 
"CXCursor_FirstStmt", 
"CXCursor_UnexposedStmt", 
"CXCursor_CompoundStmt", 
"CXCursor_CaseStmt", 
"CXCursor_DefaultStmt", 
"CXCursor_IfStmt", 
"CXCursor_SwitchStmt", 
"CXCursor_WhileStmt", 
"CXCursor_DoStmt", 
"CXCursor_ForStmt", 
"CXCursor_GotoStmt", 
"CXCursor_IndirectGotoStmt", 
"CXCursor_ContinueStmt", 
"CXCursor_BreakStmt", 
"CXCursor_ReturnStmt", 
"CXCursor_GCCAsmStmt", 
"CXCursor_AsmStmt", 
"CXCursor_ObjCAtTryStmt", 
"CXCursor_ObjCAtCatchStmt", 
"CXCursor_ObjCAtFinallyStmt", 
"CXCursor_ObjCAtThrowStmt", 
"CXCursor_ObjCAtSynchronizedStmt", 
"CXCursor_ObjCAutoreleasePoolStmt", 
"CXCursor_ObjCForCollectionStmt", 
"CXCursor_CXXCatchStmt", 
"CXCursor_CXXTryStmt", 
"CXCursor_CXXForRangeStmt", 
"CXCursor_SEHTryStmt", 
"CXCursor_SEHExceptStmt", 
"CXCursor_SEHFinallyStmt", 
"CXCursor_MSAsmStmt", 
"CXCursor_NullStmt", 
"CXCursor_DeclStmt", 
"CXCursor_OMPParallelDirective", 
"CXCursor_OMPSimdDirective", 
"CXCursor_OMPForDirective", 
"CXCursor_OMPSectionsDirective", 
"CXCursor_OMPSectionDirective", 
"CXCursor_OMPSingleDirective", 
"CXCursor_OMPParallelForDirective", 
"CXCursor_OMPParallelSectionsDirective", 
"CXCursor_OMPTaskDirective", 
"CXCursor_OMPMasterDirective", 
"CXCursor_OMPCriticalDirective", 
"CXCursor_OMPTaskyieldDirective", 
"CXCursor_OMPBarrierDirective", 
"CXCursor_OMPTaskwaitDirective", 
"CXCursor_OMPFlushDirective", 
"CXCursor_SEHLeaveStmt", 
"CXCursor_OMPOrderedDirective", 
"CXCursor_OMPAtomicDirective", 
"CXCursor_OMPForSimdDirective", 
"CXCursor_OMPParallelForSimdDirective", 
"CXCursor_OMPTargetDirective", 
"CXCursor_OMPTeamsDirective", 
"CXCursor_OMPTaskgroupDirective", 
"CXCursor_OMPCancellationPointDirective", 
"CXCursor_OMPCancelDirective", 
"CXCursor_OMPTargetDataDirective", 
"CXCursor_LastStmt", 
"CXCursor_TranslationUnit", 
"CXCursor_FirstAttr", 
"CXCursor_UnexposedAttr", 
"CXCursor_IBActionAttr", 
"CXCursor_IBOutletAttr", 
"CXCursor_IBOutletCollectionAttr", 
"CXCursor_CXXFinalAttr", 
"CXCursor_CXXOverrideAttr", 
"CXCursor_AnnotateAttr", 
"CXCursor_AsmLabelAttr", 
"CXCursor_PackedAttr", 
"CXCursor_PureAttr", 
"CXCursor_ConstAttr", 
"CXCursor_NoDuplicateAttr", 
"CXCursor_CUDAConstantAttr", 
"CXCursor_CUDADeviceAttr", 
"CXCursor_CUDAGlobalAttr", 
"CXCursor_CUDAHostAttr", 
"CXCursor_CUDASharedAttr", 
"CXCursor_VisibilityAttr", 
"CXCursor_LastAttr", 
"CXCursor_PreprocessingDirective", 
"CXCursor_MacroDefinition", 
"CXCursor_MacroExpansion", 
"CXCursor_MacroInstantiation", 
"CXCursor_InclusionDirective", 
"CXCursor_FirstPreprocessing", 
"CXCursor_LastPreprocessing", 
"CXCursor_ModuleImportDecl", 
"CXCursor_TypeAliasTemplateDecl", 
"CXCursor_FirstExtraDecl", 
"CXCursor_LastExtraDecl", 
"CXCursor_OverloadCandidate"};

enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    CXSourceRange curext = clang_getCursorExtent(cursor);
    CXSourceRange parext = clang_getCursorExtent(parent);
    CXSourceLocation curloc1 = clang_getRangeStart(curext);
    CXSourceLocation parloc1 = clang_getRangeStart(parext);
    CXSourceLocation curloc2 = clang_getRangeStart(curext);
    CXSourceLocation parloc2 = clang_getRangeStart(parext);
    clang_getFileLocation(curloc1, file, curline, curcol, NULL);
    clang_getFileLocation(parloc1, file, parline, parcol, NULL);
    clang_getFileLocation(curloc2, file, curline2, curcol2, NULL);
    clang_getFileLocation(parloc2, file, parline2, parcol2, NULL);
    char* str2 = kinds[clang_getCursorKind(cursor)];
    char* str1 = kinds[clang_getCursorKind(parent)];
    printf("%s@%d:%d-%d:%d->%s@%d:%d-%d:%d\n", str1, curline, curcol, curline2, curcol2, str2, parline, parcol, parline2, parcol2);
    return CXChildVisit_Recurse;
}

enum CXChildVisitResult (*visitor)(CXCursor, CXCursor, CXClientData) = &visit;

int main() {
    CXIndex cxi;
    const char* filename = "tests/Test1.c";
    //CXTranslationUnit cxtu = clang_createTranslationUnitFromSourceFile (cxi, filename, 0, NULL, 0, NULL);
    CXTranslationUnit cxtup = clang_parseTranslationUnit (cxi, filename, NULL, 0, NULL, 0, 0);
    file = clang_getFile (cxtup, filename);
    CXCursor cursor = clang_getTranslationUnitCursor(cxtup);
    curline = malloc(sizeof(unsigned));
    parline = malloc(sizeof(unsigned));
    curline2 = malloc(sizeof(unsigned));
    parline2 = malloc(sizeof(unsigned));
    curcol = malloc(sizeof(unsigned));
    parcol = malloc(sizeof(unsigned));
    curcol2 = malloc(sizeof(unsigned));
    parcol2 = malloc(sizeof(unsigned));
    clang_visitChildren(cursor, visitor, NULL);
    free(file);
    free(curline);
    free(parline);
    free(curline2);
    free(parline2);
    free(curcol);
    free(parcol);
    free(curcol2);
    free(parcol2);
    return 0;
}

