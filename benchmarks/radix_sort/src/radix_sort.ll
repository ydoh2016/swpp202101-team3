; ModuleID = '/tmp/a.ll'
source_filename = "radix_sort/src/radix_sort.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx11.0.0"

; Function Attrs: nounwind ssp uwtable
define dso_local i8* @malloc_upto_8(i64 %x) #0 {
entry:
  %add = add i64 %x, 7
  %div = udiv i64 %add, 8
  %mul = mul i64 %div, 8
  %call = call noalias i8* @malloc(i64 %mul) #4
  ret i8* %call
}

; Function Attrs: allocsize(0)
declare noalias i8* @malloc(i64) #1

; Function Attrs: nounwind ssp uwtable
define dso_local i32 @getMax(i32* %arr, i32 %n) #0 {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 0
  %0 = load i32, i32* %arrayidx, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %mx.0 = phi i32 [ %0, %entry ], [ %mx.1, %for.inc ]
  %i.0 = phi i32 [ 1, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64
  %arrayidx1 = getelementptr inbounds i32, i32* %arr, i64 %idxprom
  %1 = load i32, i32* %arrayidx1, align 4
  %cmp2 = icmp sgt i32 %1, %mx.0
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %idxprom3 = sext i32 %i.0 to i64
  %arrayidx4 = getelementptr inbounds i32, i32* %arr, i64 %idxprom3
  %2 = load i32, i32* %arrayidx4, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %mx.1 = phi i32 [ %2, %if.then ], [ %mx.0, %for.body ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond.cleanup
  ret i32 %mx.0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind ssp uwtable
define dso_local void @countSort(i32* %arr, i32 %n, i32 %exp) #0 {
entry:
  %count = alloca [10 x i32], align 16
  %conv = sext i32 %n to i64
  %mul = mul i64 4, %conv
  %call = call i8* @malloc_upto_8(i64 %mul)
  %0 = bitcast i8* %call to i32*
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %count, i64 0, i64 %idxprom
  store i32 0, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !7

for.end:                                          ; preds = %for.cond
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc11, %for.end
  %i.1 = phi i32 [ 0, %for.end ], [ %inc12, %for.inc11 ]
  %cmp3 = icmp slt i32 %i.1, %n
  br i1 %cmp3, label %for.body5, label %for.end13

for.body5:                                        ; preds = %for.cond2
  %idxprom6 = sext i32 %i.1 to i64
  %arrayidx7 = getelementptr inbounds i32, i32* %arr, i64 %idxprom6
  %1 = load i32, i32* %arrayidx7, align 4
  %div = sdiv i32 %1, %exp
  %rem = srem i32 %div, 10
  %idxprom8 = sext i32 %rem to i64
  %arrayidx9 = getelementptr inbounds [10 x i32], [10 x i32]* %count, i64 0, i64 %idxprom8
  %2 = load i32, i32* %arrayidx9, align 4
  %inc10 = add nsw i32 %2, 1
  store i32 %inc10, i32* %arrayidx9, align 4
  br label %for.inc11

for.inc11:                                        ; preds = %for.body5
  %inc12 = add nsw i32 %i.1, 1
  br label %for.cond2, !llvm.loop !8

for.end13:                                        ; preds = %for.cond2
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc22, %for.end13
  %i.2 = phi i32 [ 1, %for.end13 ], [ %inc23, %for.inc22 ]
  %cmp15 = icmp slt i32 %i.2, 10
  br i1 %cmp15, label %for.body17, label %for.end24

for.body17:                                       ; preds = %for.cond14
  %sub = sub nsw i32 %i.2, 1
  %idxprom18 = sext i32 %sub to i64
  %arrayidx19 = getelementptr inbounds [10 x i32], [10 x i32]* %count, i64 0, i64 %idxprom18
  %3 = load i32, i32* %arrayidx19, align 4
  %idxprom20 = sext i32 %i.2 to i64
  %arrayidx21 = getelementptr inbounds [10 x i32], [10 x i32]* %count, i64 0, i64 %idxprom20
  %4 = load i32, i32* %arrayidx21, align 4
  %add = add nsw i32 %4, %3
  store i32 %add, i32* %arrayidx21, align 4
  br label %for.inc22

for.inc22:                                        ; preds = %for.body17
  %inc23 = add nsw i32 %i.2, 1
  br label %for.cond14, !llvm.loop !9

for.end24:                                        ; preds = %for.cond14
  %sub25 = sub nsw i32 %n, 1
  br label %for.cond26

for.cond26:                                       ; preds = %for.inc47, %for.end24
  %i.3 = phi i32 [ %sub25, %for.end24 ], [ %dec48, %for.inc47 ]
  %cmp27 = icmp sge i32 %i.3, 0
  br i1 %cmp27, label %for.body29, label %for.end49

for.body29:                                       ; preds = %for.cond26
  %idxprom30 = sext i32 %i.3 to i64
  %arrayidx31 = getelementptr inbounds i32, i32* %arr, i64 %idxprom30
  %5 = load i32, i32* %arrayidx31, align 4
  %idxprom32 = sext i32 %i.3 to i64
  %arrayidx33 = getelementptr inbounds i32, i32* %arr, i64 %idxprom32
  %6 = load i32, i32* %arrayidx33, align 4
  %div34 = sdiv i32 %6, %exp
  %rem35 = srem i32 %div34, 10
  %idxprom36 = sext i32 %rem35 to i64
  %arrayidx37 = getelementptr inbounds [10 x i32], [10 x i32]* %count, i64 0, i64 %idxprom36
  %7 = load i32, i32* %arrayidx37, align 4
  %sub38 = sub nsw i32 %7, 1
  %idxprom39 = sext i32 %sub38 to i64
  %arrayidx40 = getelementptr inbounds i32, i32* %0, i64 %idxprom39
  store i32 %5, i32* %arrayidx40, align 4
  %idxprom41 = sext i32 %i.3 to i64
  %arrayidx42 = getelementptr inbounds i32, i32* %arr, i64 %idxprom41
  %8 = load i32, i32* %arrayidx42, align 4
  %div43 = sdiv i32 %8, %exp
  %rem44 = srem i32 %div43, 10
  %idxprom45 = sext i32 %rem44 to i64
  %arrayidx46 = getelementptr inbounds [10 x i32], [10 x i32]* %count, i64 0, i64 %idxprom45
  %9 = load i32, i32* %arrayidx46, align 4
  %dec = add nsw i32 %9, -1
  store i32 %dec, i32* %arrayidx46, align 4
  br label %for.inc47

for.inc47:                                        ; preds = %for.body29
  %dec48 = add nsw i32 %i.3, -1
  br label %for.cond26, !llvm.loop !10

for.end49:                                        ; preds = %for.cond26
  br label %for.cond50

for.cond50:                                       ; preds = %for.inc58, %for.end49
  %i.4 = phi i32 [ 0, %for.end49 ], [ %inc59, %for.inc58 ]
  %cmp51 = icmp slt i32 %i.4, %n
  br i1 %cmp51, label %for.body53, label %for.end60

for.body53:                                       ; preds = %for.cond50
  %idxprom54 = sext i32 %i.4 to i64
  %arrayidx55 = getelementptr inbounds i32, i32* %0, i64 %idxprom54
  %10 = load i32, i32* %arrayidx55, align 4
  %idxprom56 = sext i32 %i.4 to i64
  %arrayidx57 = getelementptr inbounds i32, i32* %arr, i64 %idxprom56
  store i32 %10, i32* %arrayidx57, align 4
  br label %for.inc58

for.inc58:                                        ; preds = %for.body53
  %inc59 = add nsw i32 %i.4, 1
  br label %for.cond50, !llvm.loop !11

for.end60:                                        ; preds = %for.cond50
  %11 = bitcast i32* %0 to i8*
  call void @free(i8* %11)
  ret void
}

declare void @free(i8*) #3

; Function Attrs: nounwind ssp uwtable
define dso_local void @radixSort(i32* %arr, i32 %n) #0 {
entry:
  %call = call i32 @getMax(i32* %arr, i32 %n)
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %exp.0 = phi i32 [ 1, %entry ], [ %mul, %for.inc ]
  %div = sdiv i32 %call, %exp.0
  %cmp = icmp sgt i32 %div, 0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  call void @countSort(i32* %arr, i32 %n, i32 %exp.0)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %mul = mul nsw i32 %exp.0, 10
  br label %for.cond, !llvm.loop !12

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: nounwind ssp uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %mul = mul i64 4, %call
  %call1 = call i8* @malloc_upto_8(i64 %mul)
  %0 = bitcast i8* %call1 to i32*
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %conv = sext i32 %i.0 to i64
  %cmp = icmp ult i64 %conv, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %call3 = call i64 (...) @read()
  %conv4 = trunc i64 %call3 to i32
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %idxprom
  store i32 %conv4, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !13

for.end:                                          ; preds = %for.cond.cleanup
  %conv5 = trunc i64 %call to i32
  call void @radixSort(i32* %0, i32 %conv5)
  br label %for.cond7

for.cond7:                                        ; preds = %for.inc16, %for.end
  %i6.0 = phi i32 [ 0, %for.end ], [ %inc17, %for.inc16 ]
  %conv8 = sext i32 %i6.0 to i64
  %cmp9 = icmp ult i64 %conv8, %call
  br i1 %cmp9, label %for.body12, label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond7
  br label %for.end18

for.body12:                                       ; preds = %for.cond7
  %idxprom13 = sext i32 %i6.0 to i64
  %arrayidx14 = getelementptr inbounds i32, i32* %0, i64 %idxprom13
  %1 = load i32, i32* %arrayidx14, align 4
  %conv15 = sext i32 %1 to i64
  call void @write(i64 %conv15)
  br label %for.inc16

for.inc16:                                        ; preds = %for.body12
  %inc17 = add nsw i32 %i6.0, 1
  br label %for.cond7, !llvm.loop !14

for.end18:                                        ; preds = %for.cond.cleanup11
  %2 = bitcast i32* %0 to i8*
  call void @free(i8* %2)
  ret i32 0
}

declare i64 @read(...) #3

declare void @write(i64) #3

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { allocsize(0) "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }
attributes #3 = { "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { allocsize(0) }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 11, i32 1]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
!4 = distinct !{!4, !5, !6}
!5 = !{!"llvm.loop.mustprogress"}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = distinct !{!7, !5, !6}
!8 = distinct !{!8, !5, !6}
!9 = distinct !{!9, !5, !6}
!10 = distinct !{!10, !5, !6}
!11 = distinct !{!11, !5, !6}
!12 = distinct !{!12, !5, !6}
!13 = distinct !{!13, !5, !6}
!14 = distinct !{!14, !5, !6}
