
define dso_local i32 @main() #0 !dbg !9 {
entry:
  %retval = alloca i32, align 4
  %arr = alloca [4 x [5 x i32]], align 16
  %count = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @llvm.dbg.declare(metadata [4 x [5 x i32]]* %arr, metadata !14, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.declare(metadata i32* %count, metadata !20, metadata !DIExpression()), !dbg !21
  store i32 0, i32* %count, align 4, !dbg !21
  call void @llvm.dbg.declare(metadata i32* %i, metadata !22, metadata !DIExpression()), !dbg !24
  store i32 0, i32* %i, align 4, !dbg !24
  br label %for.cond, !dbg !25

for.cond:                                         ; preds = %for.inc7, %entry
  %0 = load i32, i32* %i, align 4, !dbg !26
  %cmp = icmp slt i32 %0, 4, !dbg !28
  br i1 %cmp, label %for.body, label %for.end9, !dbg !29

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %count, align 4, !dbg !30
  %inc = add nsw i32 %1, 1, !dbg !30
  store i32 %inc, i32* %count, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata i32* %j, metadata !32, metadata !DIExpression()), !dbg !34
  store i32 0, i32* %j, align 4, !dbg !34
  br label %for.cond1, !dbg !35

for.cond1:                                        ; preds = %for.inc, %for.body
  %2 = load i32, i32* %j, align 4, !dbg !36
  %cmp2 = icmp slt i32 %2, 5, !dbg !38
  br i1 %cmp2, label %for.body3, label %for.end, !dbg !39

for.body3:                                        ; preds = %for.cond1
  %3 = load i32, i32* %count, align 4, !dbg !40
  %dec = add nsw i32 %3, -1, !dbg !40
  store i32 %dec, i32* %count, align 4, !dbg !40
  %4 = load i32, i32* %i, align 4, !dbg !42
  %idxprom = sext i32 %4 to i64, !dbg !43
  %arrayidx = getelementptr inbounds [4 x [5 x i32]], [4 x [5 x i32]]* %arr, i64 0, i64 %idxprom, !dbg !43
  %5 = load i32, i32* %j, align 4, !dbg !44
  %idxprom4 = sext i32 %5 to i64, !dbg !43
  %arrayidx5 = getelementptr inbounds [5 x i32], [5 x i32]* %arrayidx, i64 0, i64 %idxprom4, !dbg !43
  store i32 0, i32* %arrayidx5, align 4, !dbg !45
  br label %for.inc, !dbg !46

for.inc:                                          ; preds = %for.body3
  %6 = load i32, i32* %j, align 4, !dbg !47
  %inc6 = add nsw i32 %6, 1, !dbg !47
  store i32 %inc6, i32* %j, align 4, !dbg !47
  br label %for.cond1, !dbg !48, !llvm.loop !49

for.end:                                          ; preds = %for.cond1
  br label %for.inc7, !dbg !52

for.inc7:                                         ; preds = %for.end
  %7 = load i32, i32* %i, align 4, !dbg !53
  %inc8 = add nsw i32 %7, 1, !dbg !53
  store i32 %inc8, i32* %i, align 4, !dbg !53
  br label %for.cond, !dbg !54, !llvm.loop !55

for.end9:                                         ; preds = %for.cond
  ret i32 0, !dbg !57
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
