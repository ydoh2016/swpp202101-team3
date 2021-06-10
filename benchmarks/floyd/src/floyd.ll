; ModuleID = '/tmp/a.ll'
source_filename = "test/floyd/src/floyd.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@graph = external dso_local global i16*, align 8
@cycle_pos = external dso_local global i64, align 8
@cycle_len = external dso_local global i64, align 8

; Function Attrs: nounwind uwtable
define dso_local i8* @malloc_upto_8(i64 %x) #0 {
entry:
  %add = add i64 %x, 7
  %div = udiv i64 %add, 8
  %mul = mul i64 %div, 8
  %call = call noalias i8* @malloc(i64 %mul) #4
  ret i8* %call
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

; Function Attrs: nounwind uwtable
define dso_local i16* @advance(i16* %node) #0 {
entry:
  %0 = load i16, i16* %node, align 2
  %conv = zext i16 %0 to i64
  %1 = load i16*, i16** @graph, align 8
  %arrayidx = getelementptr inbounds i16, i16* %1, i64 %conv
  ret i16* %arrayidx
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local void @floyd() #0 {
entry:
  %0 = load i16*, i16** @graph, align 8
  %call = call i16* @advance(i16* %0)
  %1 = load i16*, i16** @graph, align 8
  %call1 = call i16* @advance(i16* %1)
  %call2 = call i16* @advance(i16* %call1)
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %tortoise.0 = phi i16* [ %call, %entry ], [ %call3, %while.body ]
  %hare.0 = phi i16* [ %call2, %entry ], [ %call5, %while.body ]
  %cmp = icmp ne i16* %tortoise.0, %hare.0
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %call3 = call i16* @advance(i16* %tortoise.0)
  %call4 = call i16* @advance(i16* %hare.0)
  %call5 = call i16* @advance(i16* %call4)
  br label %while.cond, !llvm.loop !2

while.end:                                        ; preds = %while.cond
  store i64 0, i64* @cycle_pos, align 8
  %2 = load i16*, i16** @graph, align 8
  br label %while.cond6

while.cond6:                                      ; preds = %while.body8, %while.end
  %tortoise.1 = phi i16* [ %2, %while.end ], [ %call9, %while.body8 ]
  %hare.1 = phi i16* [ %hare.0, %while.end ], [ %call10, %while.body8 ]
  %cmp7 = icmp ne i16* %tortoise.1, %hare.1
  br i1 %cmp7, label %while.body8, label %while.end11

while.body8:                                      ; preds = %while.cond6
  %call9 = call i16* @advance(i16* %tortoise.1)
  %call10 = call i16* @advance(i16* %hare.1)
  %3 = load i64, i64* @cycle_pos, align 8
  %add = add i64 %3, 1
  store i64 %add, i64* @cycle_pos, align 8
  br label %while.cond6, !llvm.loop !5

while.end11:                                      ; preds = %while.cond6
  store i64 1, i64* @cycle_len, align 8
  %call12 = call i16* @advance(i16* %tortoise.1)
  br label %while.cond13

while.cond13:                                     ; preds = %while.body15, %while.end11
  %hare.2 = phi i16* [ %call12, %while.end11 ], [ %call16, %while.body15 ]
  %cmp14 = icmp ne i16* %tortoise.1, %hare.2
  br i1 %cmp14, label %while.body15, label %while.end18

while.body15:                                     ; preds = %while.cond13
  %call16 = call i16* @advance(i16* %hare.2)
  %4 = load i64, i64* @cycle_len, align 8
  %add17 = add i64 %4, 1
  store i64 %add17, i64* @cycle_len, align 8
  br label %while.cond13, !llvm.loop !6

while.end18:                                      ; preds = %while.cond13
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %mul = mul i64 %call, 2
  %call1 = call i8* @malloc_upto_8(i64 %mul)
  %0 = bitcast i8* %call1 to i16*
  store i16* %0, i16** @graph, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i64 %i.0, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %call2 = call i64 (...) @read()
  %conv = trunc i64 %call2 to i16
  %1 = load i16*, i16** @graph, align 8
  %arrayidx = getelementptr inbounds i16, i16* %1, i64 %i.0
  store i16 %conv, i16* %arrayidx, align 2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i64 %i.0, 1
  br label %for.cond, !llvm.loop !7

for.end:                                          ; preds = %for.cond.cleanup
  call void @floyd()
  %2 = load i64, i64* @cycle_pos, align 8
  call void @write(i64 %2)
  %3 = load i64, i64* @cycle_len, align 8
  call void @write(i64 %3)
  ret i32 0
}

declare dso_local i64 @read(...) #3

declare dso_local void @write(i64) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git fa0971b87fb2c9d14d1bba2551e61f02f18f329b)"}
!2 = distinct !{!2, !3, !4}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = distinct !{!5, !3, !4}
!6 = distinct !{!6, !3, !4}
!7 = distinct !{!7, !3, !4}
