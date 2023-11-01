; ModuleID = 'test1.c'
source_filename = "test1.c"
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

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !19 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8, !dbg !24
  %3 = call i32 @getc(%struct._IO_FILE* noundef %2), !dbg !25
  %4 = trunc i32 %3 to i8, !dbg !25
  store i8 %4, i8* @c, align 1, !dbg !26
  %5 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8, !dbg !27
  %6 = call i32 @getc(%struct._IO_FILE* noundef %5), !dbg !28
  %7 = trunc i32 %6 to i8, !dbg !28
  store i8 %7, i8* @d, align 1, !dbg !29
  %8 = call noalias i8* @malloc(i64 noundef 10) #3, !dbg !30
  store i8* %8, i8** @x, align 8, !dbg !31
  br label %9, !dbg !32

9:                                                ; preds = %30, %0
  %10 = load i8, i8* @d, align 1, !dbg !33
  %11 = sext i8 %10 to i32, !dbg !33
  %12 = icmp slt i32 %11, 50, !dbg !34
  br i1 %12, label %13, label %35, !dbg !32

13:                                               ; preds = %9
  %14 = load i8, i8* @c, align 1, !dbg !35
  %15 = sext i8 %14 to i32, !dbg !35
  %16 = add nsw i32 %15, 1, !dbg !35
  %17 = trunc i32 %16 to i8, !dbg !35
  store i8 %17, i8* @c, align 1, !dbg !35
  br label %18, !dbg !37

18:                                               ; preds = %29, %13
  %19 = load i8, i8* @c, align 1, !dbg !38
  %20 = sext i8 %19 to i32, !dbg !38
  %21 = icmp sgt i32 %20, 15, !dbg !39
  br i1 %21, label %22, label %30, !dbg !37

22:                                               ; preds = %18
  %23 = load i8, i8* @c, align 1, !dbg !40
  %24 = sext i8 %23 to i32, !dbg !40
  %25 = load i8, i8* @d, align 1, !dbg !43
  %26 = sext i8 %25 to i32, !dbg !43
  %27 = icmp sgt i32 %24, %26, !dbg !44
  br i1 %27, label %28, label %29, !dbg !45

28:                                               ; preds = %22
  br label %30, !dbg !46

29:                                               ; preds = %22
  br label %18, !dbg !37, !llvm.loop !47

30:                                               ; preds = %28, %18
  %31 = load i8, i8* @d, align 1, !dbg !50
  %32 = sext i8 %31 to i32, !dbg !50
  %33 = sub nsw i32 %32, 1, !dbg !50
  %34 = trunc i32 %33 to i8, !dbg !50
  store i8 %34, i8* @d, align 1, !dbg !50
  br label %9, !dbg !32, !llvm.loop !51

35:                                               ; preds = %9
  %36 = load i32, i32* %1, align 4, !dbg !53
  ret i32 %36, !dbg !53
}

declare i32 @getc(%struct._IO_FILE* noundef) #1

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!11, !12, !13, !14, !15, !16, !17}
!llvm.ident = !{!18}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 4, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "Ubuntu clang version 14.0.0-1ubuntu1", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "test1.c", directory: "/homes/pdacost/cs510/proj2/testcases", checksumkind: CSK_MD5, checksum: "9f496ced5f9cd39b31e02947766ce8a3")
!4 = !{!0, !5, !8}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 4, type: !7, isLocal: false, isDefinition: true)
!7 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!8 = !DIGlobalVariableExpression(var: !9, expr: !DIExpression())
!9 = distinct !DIGlobalVariable(name: "x", scope: !2, file: !3, line: 5, type: !10, isLocal: false, isDefinition: true)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!11 = !{i32 7, !"Dwarf Version", i32 5}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 7, !"PIC Level", i32 2}
!15 = !{i32 7, !"PIE Level", i32 2}
!16 = !{i32 7, !"uwtable", i32 1}
!17 = !{i32 7, !"frame-pointer", i32 2}
!18 = !{!"Ubuntu clang version 14.0.0-1ubuntu1"}
!19 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 8, type: !20, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !23)
!20 = !DISubroutineType(types: !21)
!21 = !{!22}
!22 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!23 = !{}
!24 = !DILocation(line: 9, column: 12, scope: !19)
!25 = !DILocation(line: 9, column: 7, scope: !19)
!26 = !DILocation(line: 9, column: 5, scope: !19)
!27 = !DILocation(line: 10, column: 12, scope: !19)
!28 = !DILocation(line: 10, column: 7, scope: !19)
!29 = !DILocation(line: 10, column: 5, scope: !19)
!30 = !DILocation(line: 12, column: 7, scope: !19)
!31 = !DILocation(line: 12, column: 5, scope: !19)
!32 = !DILocation(line: 14, column: 3, scope: !19)
!33 = !DILocation(line: 14, column: 10, scope: !19)
!34 = !DILocation(line: 14, column: 12, scope: !19)
!35 = !DILocation(line: 16, column: 7, scope: !36)
!36 = distinct !DILexicalBlock(scope: !19, file: !3, line: 14, column: 17)
!37 = !DILocation(line: 18, column: 5, scope: !36)
!38 = !DILocation(line: 18, column: 12, scope: !36)
!39 = !DILocation(line: 18, column: 14, scope: !36)
!40 = !DILocation(line: 20, column: 11, scope: !41)
!41 = distinct !DILexicalBlock(scope: !42, file: !3, line: 20, column: 11)
!42 = distinct !DILexicalBlock(scope: !36, file: !3, line: 19, column: 5)
!43 = !DILocation(line: 20, column: 15, scope: !41)
!44 = !DILocation(line: 20, column: 13, scope: !41)
!45 = !DILocation(line: 20, column: 11, scope: !42)
!46 = !DILocation(line: 21, column: 9, scope: !41)
!47 = distinct !{!47, !37, !48, !49}
!48 = !DILocation(line: 22, column: 5, scope: !36)
!49 = !{!"llvm.loop.mustprogress"}
!50 = !DILocation(line: 24, column: 7, scope: !36)
!51 = distinct !{!51, !32, !52, !49}
!52 = !DILocation(line: 25, column: 5, scope: !19)
!53 = !DILocation(line: 27, column: 1, scope: !19)
