; ModuleID = 'test5.c'
source_filename = "test5.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque

@stdin = external global %struct._IO_FILE*, align 8
@c = dso_local global i8 0, align 1, !dbg !0
@d = dso_local global i8 0, align 1, !dbg !5
@x = dso_local global i8* null, align 8, !dbg !8
@y = dso_local global i8* null, align 8, !dbg !11
@z = dso_local global i8* null, align 8, !dbg !13

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !23 {
  %1 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8, !dbg !28
  %2 = call i32 @getc(%struct._IO_FILE* noundef %1), !dbg !29
  %3 = trunc i32 %2 to i8, !dbg !29
  store i8 %3, i8* @c, align 1, !dbg !30
  %4 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8, !dbg !31
  %5 = call i32 @getc(%struct._IO_FILE* noundef %4), !dbg !32
  %6 = trunc i32 %5 to i8, !dbg !32
  store i8 %6, i8* @d, align 1, !dbg !33
  %7 = call noalias i8* @malloc(i64 noundef 10) #3, !dbg !34
  store i8* %7, i8** @x, align 8, !dbg !35
  %8 = call noalias i8* @malloc(i64 noundef 10) #3, !dbg !36
  store i8* %8, i8** @y, align 8, !dbg !37
  %9 = load i8*, i8** @y, align 8, !dbg !38
  store i8* %9, i8** @z, align 8, !dbg !39
  %10 = load i8*, i8** @x, align 8, !dbg !40
  call void @free(i8* noundef %10) #3, !dbg !41
  %11 = load i8*, i8** @z, align 8, !dbg !42
  call void @free(i8* noundef %11) #3, !dbg !43
  ret i32 0, !dbg !44
}

declare i32 @getc(%struct._IO_FILE* noundef) #1

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 noundef) #2

; Function Attrs: nounwind
declare void @free(i8* noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!15, !16, !17, !18, !19, !20, !21}
!llvm.ident = !{!22}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 4, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "Ubuntu clang version 14.0.0-1ubuntu1", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "test5.c", directory: "/homes/pdacost/cs510/proj2/testcases", checksumkind: CSK_MD5, checksum: "997512bc3c9a0714f1b81bbcfd2529fc")
!4 = !{!0, !5, !8, !11, !13}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 4, type: !7, isLocal: false, isDefinition: true)
!7 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!8 = !DIGlobalVariableExpression(var: !9, expr: !DIExpression())
!9 = distinct !DIGlobalVariable(name: "x", scope: !2, file: !3, line: 5, type: !10, isLocal: false, isDefinition: true)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!11 = !DIGlobalVariableExpression(var: !12, expr: !DIExpression())
!12 = distinct !DIGlobalVariable(name: "y", scope: !2, file: !3, line: 6, type: !10, isLocal: false, isDefinition: true)
!13 = !DIGlobalVariableExpression(var: !14, expr: !DIExpression())
!14 = distinct !DIGlobalVariable(name: "z", scope: !2, file: !3, line: 7, type: !10, isLocal: false, isDefinition: true)
!15 = !{i32 7, !"Dwarf Version", i32 5}
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 1, !"wchar_size", i32 4}
!18 = !{i32 7, !"PIC Level", i32 2}
!19 = !{i32 7, !"PIE Level", i32 2}
!20 = !{i32 7, !"uwtable", i32 1}
!21 = !{i32 7, !"frame-pointer", i32 2}
!22 = !{!"Ubuntu clang version 14.0.0-1ubuntu1"}
!23 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 9, type: !24, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !27)
!24 = !DISubroutineType(types: !25)
!25 = !{!26}
!26 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!27 = !{}
!28 = !DILocation(line: 10, column: 12, scope: !23)
!29 = !DILocation(line: 10, column: 7, scope: !23)
!30 = !DILocation(line: 10, column: 5, scope: !23)
!31 = !DILocation(line: 11, column: 12, scope: !23)
!32 = !DILocation(line: 11, column: 7, scope: !23)
!33 = !DILocation(line: 11, column: 5, scope: !23)
!34 = !DILocation(line: 13, column: 7, scope: !23)
!35 = !DILocation(line: 13, column: 5, scope: !23)
!36 = !DILocation(line: 14, column: 7, scope: !23)
!37 = !DILocation(line: 14, column: 5, scope: !23)
!38 = !DILocation(line: 16, column: 7, scope: !23)
!39 = !DILocation(line: 16, column: 5, scope: !23)
!40 = !DILocation(line: 18, column: 8, scope: !23)
!41 = !DILocation(line: 18, column: 3, scope: !23)
!42 = !DILocation(line: 19, column: 8, scope: !23)
!43 = !DILocation(line: 19, column: 3, scope: !23)
!44 = !DILocation(line: 20, column: 1, scope: !23)
