define void @main(i64 %arr) {
entry:
    %test = alloca [64 x i64]
    %i = mul i64 4, %arr
    %arrayidx0 = getelementptr inbounds [64 x i64], [64 x i64]* %test, i64 0, i64 %i
    ;CHECK-NOT: switch [[R4:r[0-9]+]] 0 .store0.0 1 .store0.1 2 .store0.2 3 .store0.3 4 .store0.4 5 .store0.5 6 .store0.6 7 .store0.7 8 .store0.8 9 .store0.9 10 .store0.10 11 .store0.11 12 .store0.12 13 .store0.13 14 .store0.14 15 .store0.15 16 .store0.16 17 .store0.17 18 .store0.18 19 .store0.19 20 .store0.20 21 .store0.21 22 .store0.22 23 .store0.23 24 .store0.24 25 .store0.25 26 .store0.26 .normalStore0
    store i64 0, i64* %arrayidx0
    ret void
}