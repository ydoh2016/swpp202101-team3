;CHECK-LABEL: start main 0
define i32 @main() {
  %1 = alloca i32
  %2 = alloca [4 x [5 x i32]]
  %3 = alloca i32
  %4 = alloca i32
  store i32 0, i32* %1
  br label %5
5:
  store i32 0, i32* %3
  br label %6
6:
  %7 = load i32, i32* %3
  %8 = icmp slt i32 %7, 4
  br i1 %8, label %9, label %27
for.body:
  store i32 0, i32* %4
  br label %10
10:
  %11 = load i32, i32* %4
  %12 = icmp slt i32 %11, 5
  br i1 %12, label %13, label %23
13:
  %14 = load i32, i32* %3
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds [4 x [5 x i32]], [4 x [5 x i32]]* %2, i64 0, i64 %15
  %17 = load i32, i32* %4
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds [5 x i32], [5 x i32]* %16, i64 0, i64 %18
  store i32 0, i32* %19
  br label %20
20:
  %21 = load i32, i32* %4
  %22 = add nsw i32 %21, 1
  store i32 %22, i32* %4
  br label %10
23:
  br label %24
24:
  %25 = load i32, i32* %3
  %26 = add nsw i32 %25, 1
  store i32 %26, i32* %3
  br label %5
27:
  ret i32 0
}