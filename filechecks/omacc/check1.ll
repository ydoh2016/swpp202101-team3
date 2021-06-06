define void @main() {
entry:
    %test = alloca [10 x i64]
    br label %for.cond
for.cond:
    %i = phi i64 [0, %entry],[%inc, %for.inc]
    %cmp = icmp slt i64 %i, 10
    br i1 %cmp, label %for.body, label %next
for.body:
    %arrayidx0 = getelementptr inbounds [10 x i64], [10 x i64]* %test, i64 0, i64 %i
    ;CHECK: switch [[R4:r[0-9]+]] 0 .store0.0 1 .store0.1 2 .store0.2 3 .store0.3 4 .store0.4 5 .store0.5 6 .store0.6 7 .store0.7 8 .store0.8 9 .store0.9 .normalStore0
    store i64 0, i64* %arrayidx0
    br label %for.inc
for.inc:
    %inc = add nsw i64 %i,1
    br label %for.cond
next:
    br label %for.cond2
for.cond2:
    %i.0 = phi i64 [0, %next],[%inc.0, %for.inc2]
    %cmp.0 = icmp slt i64 %i.0, 10
    br i1 %cmp.0, label %for.body2, label %exit
for.body2:
    %arrayidx1 = getelementptr inbounds [10 x i64], [10 x i64]* %test, i64 0, i64 %i
    ;CHECK: switch [[R4:r[0-9]+]] 0 .load0.0 1 .load0.1 2 .load0.2 3 .load0.3 4 .load0.4 5 .load0.5 6 .load0.6 7 .load0.7 8 .load0.8 9 .load0.9 .normalLoad0
    %val = load i64, i64* %arrayidx1
    %val.0 = add i64 %val, 1
    ;CHECK: switch [[R4:r[0-9]+]] 0 .store1.0 1 .store1.1 2 .store1.2 3 .store1.3 4 .store1.4 5 .store1.5 6 .store1.6 7 .store1.7 8 .store1.8 9 .store1.9 .normalStore1
    store i64 %val.0, i64* %arrayidx1
    br label %for.inc2
for.inc2:
    %inc.0 = add nsw i64 %i,1
    br label %for.cond2
exit:
    ret void
}