define void @main() {
entry:
    %test = alloca [64 x i64]
    br label %for.cond
for.cond:
    %i = phi i64 [0, %entry],[%inc, %for.inc]
    %cmp = icmp slt i64 %i, 64
    br i1 %cmp, label %for.body, label %next
for.body:
    %arrayidx0 = getelementptr inbounds [64 x i64], [64 x i64]* %test, i64 0, i64 %i
    ;CHECK: switch [[R4:r[0-9]+]] 0 .store0.0 1 .store0.1 2 .store0.2 3 .store0.3 4 .store0.4 5 .store0.5 6 .store0.6 7 .store0.7 8 .store0.8 9 .store0.9 10 .store0.10 11 .store0.11 12 .store0.12 13 .store0.13 14 .store0.14 15 .store0.15 16 .store0.16 17 .store0.17 18 .store0.18 19 .store0.19 20 .store0.20 21 .store0.21 22 .store0.22 23 .store0.23 24 .store0.24 25 .store0.25 26 .store0.26 .normalStore0
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
    %arrayidx1 = getelementptr inbounds [64 x i64], [64 x i64]* %test, i64 0, i64 %i
    ;CHECK: switch [[R4:r[0-9]+]] 0 .load0.0 1 .load0.1 2 .load0.2 3 .load0.3 4 .load0.4 5 .load0.5 6 .load0.6 7 .load0.7 8 .load0.8 9 .load0.9 10 .load0.10 11 .load0.11 12 .load0.12 13 .load0.13 14 .load0.14 15 .load0.15 16 .load0.16 17 .load0.17 18 .load0.18 19 .load0.19 20 .load0.20 21 .load0.21 22 .load0.22 23 .load0.23 24 .load0.24 25 .load0.25 26 .load0.26 .normalLoad0
    %val = load i64, i64* %arrayidx1
    %val.0 = add i64 %val, 1
    ;CHECK: switch [[R4:r[0-9]+]] 0 .store1.0 1 .store1.1 2 .store1.2 3 .store1.3 4 .store1.4 5 .store1.5 6 .store1.6 7 .store1.7 8 .store1.8 9 .store1.9 10 .store1.10 11 .store1.11 12 .store1.12 13 .store1.13 14 .store1.14 15 .store1.15 16 .store1.16 17 .store1.17 18 .store1.18 19 .store1.19 20 .store1.20 21 .store1.21 22 .store1.22 23 .store1.23 24 .store1.24 25 .store1.25 26 .store1.26 .normalStore1
    store i64 %val.0, i64* %arrayidx1
    br label %for.inc2
for.inc2:
    %inc.0 = add nsw i64 %i,1
    br label %for.cond2
exit:
    ret void
}