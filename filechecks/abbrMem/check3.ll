; ModuleID = 'check1.ll'
source_filename = "check1.ll"

define i64 @main() {
  %hptr = call i64* @malloc(i64 24)

  %a = add i64 0, 0

  %b = mul i64 %a, 5
  %c = mul i64 %a, 5
  %d = mul i64 %a, 5

  %e = add i64 %b, 0 
  %f = add i64 %c, 1 
  %g = add i64 %d, 2
  
  %hptr0 = getelementptr inbounds i64, i64* %hptr, i64 %e 
  %hptr1 = getelementptr inbounds i64, i64* %hptr, i64 %f
  %hptr2 = getelementptr inbounds i64, i64* %hptr, i64 %g 

  store i64 10, i64* %hptr0
  store i64 20, i64* %hptr1
  store i64 30, i64* %hptr2

  ; CHECK: vstore 8 10 20 30 _ _ _ _ _ [[R1:r[0-9]+]] 0 
  ; CHECK: [[R2:r[0-9]+]] [[R3:r[0-9]+]] [[R4:r[0-9]+]] _ _ _ _ _ = vload 8 [[R5:r[0-9]+]] 0 

  %load1 = load i64, i64* %hptr0
  %load2 = load i64, i64* %hptr1
  %load3 = load i64, i64* %hptr2

  %load4 = add i64 %load1, %load2
  %load5 = mul i64 %load3, %load4
  
  ret i64 %load5
}

declare i64* @malloc(i64)