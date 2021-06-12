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
  br i1 %9, label %10, label %32
10:
  %11 = load i32, i32* %3
  %12 = add nsw i32 %11, 1
  store i32 %12, i32* %3
  store i32 0, i32* %5
  br label %13
13:
  %14 = load i32, i32* %5
  %15 = icmp slt i32 %14, 5
  br i1 %15, label %16, label %28
16:
  %17 = load i32, i32* %3
  %18 = add nsw i32 %17, -1
  store i32 %18, i32* %3
  %19 = load i32, i32* %4
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [4 x [5 x i32]], [4 x [5 x i32]]* %2, i64 0, i64 %20
  %22 = load i32, i32* %5
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [5 x i32], [5 x i32]* %21, i64 0, i64 %23
  store i32 0, i32* %24
  br label %25
25:
  %26 = load i32, i32* %5
  %27 = add nsw i32 %26, 1
  store i32 %27, i32* %5
  br label %13
28:
  br label %29
29:
  %30 = load i32, i32* %4
  %31 = add nsw i32 %30, 1
  store i32 %31, i32* %4
  br label %7
32:
  ret i32 0
}