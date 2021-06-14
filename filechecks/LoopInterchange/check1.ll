;CHECK-LABEL: start main 0
define i32 @main() {
  %1 = alloca i32
  %2 = alloca [4 x [5 x i32]]
  %3 = alloca i32
  %4 = alloca i32
  %5 = alloca i32
  store i32 0, i32* %1
  store i32 0, i32* %3
  br label %6
6:
  store i32 0, i32* %4
  br label %7
7:
  %8 = load i32, i32* %4
  %9 = icmp slt i32 %8, 4
  br i1 %9, label %10, label %30
10:
  %11 = load i32, i32* %3
  %12 = add nsw i32 %11, 1
  store i32 %12, i32* %3
  store i32 0, i32* %5
  br label %13
13:
  %14 = load i32, i32* %5
  %15 = icmp slt i32 %14, 5
  br i1 %15, label %16, label %26
16:
  %17 = load i32, i32* %4
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds [4 x [5 x i32]], [4 x [5 x i32]]* %2, i64 0, i64 %18
  %20 = load i32, i32* %5
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds [5 x i32], [5 x i32]* %19, i64 0, i64 %21
  store i32 0, i32* %22
  br label %23
23:
  %24 = load i32, i32* %5
  %25 = add nsw i32 %24, 1
  store i32 %25, i32* %5
  br label %13
26:
  br label %27
27:
  %28 = load i32, i32* %4
  %29 = add nsw i32 %28, 1
  store i32 %29, i32* %4
  br label %7
30:
  ret i32 0
}