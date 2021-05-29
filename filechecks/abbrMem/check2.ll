; ModuleID = 'check2.ll'
source_filename = "check2.ll"

define i64 @main() {
  %hptr = call i64* @malloc(i64 24)
  %hptr0 = getelementptr inbounds i64, i64* %hptr, i64 0 
  %hptr1 = getelementptr inbounds i64, i64* %hptr, i64 1
  %hptr2 = getelementptr inbounds i64, i64* %hptr, i64 2

  store i64 10, i64* %hptr0
  store i64 20, i64* %hptr1
  store i64 30, i64* %hptr2

; CHECK-NOT: store 8 10 [[R1:r[0-9]+]] 0 
; CHECK-NOT: store 8 20 [[R2:r[0-9]+]] 0 
; CHECK-NOT: store 8 30 [[R3:r[0-9]+]] 0 
; CHECK: vstore 8 10 20 30 _ _ _ _ _ [[R4:r[0-9]+]] 0
  
  ret i64 0
}

declare i64* @malloc(i64)