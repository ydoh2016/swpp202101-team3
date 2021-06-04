; ModuleID = 'check1.ll'
source_filename = "check1.ll"

define i64 @main() {
  %hptr = call i64* @malloc(i64 24)
  %hptr0 = getelementptr inbounds i64, i64* %hptr, i64 0 
  %hptr1 = getelementptr inbounds i64, i64* %hptr, i64 1
  %hptr2 = getelementptr inbounds i64, i64* %hptr, i64 2
  
  ;CHECK: [[R1:r[0-9]+]] [[R2:r[0-9]+]] [[R3:r[0-9]+]] _ _ _ _ _ = vload 8 [[R4:r[0-9]+]] 0
  ;CHECK-NOT: [[R5:r[0-9]+]] = load 8 [[R6:r[0-9]+]] 0 
  %load1 = load i64, i64* %hptr0
  %load2 = load i64, i64* %hptr1
  %load3 = load i64, i64* %hptr2

  %load4 = add i64 %load1, %load2
  %load5 = mul i64 %load3, %load4
  
  ret i64 %load5
}

declare i64* @malloc(i64)

