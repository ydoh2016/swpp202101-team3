define i32 @multiple_returns(i32 %arg) {
  %cond = icmp eq i32 %arg, 0
  br i1 %cond, label %end1,  label %end2

end1:
  ret i32 0

end2:
  ret i32 2
}

define void @main() {
;CHECK_LABEL: ._defaultBB0:
;CHECK: br ._defaultBB1 
;CHECK-LABEL: ..split:
;CHECK: ret 0 
;CHECK-LABEL: ._defaultBB1:
;CHECK: [[R1:r[0-9]+]] = icmp eq 5 0 32 
;CHECK-NEXT: br [[R1:r[0-9]+]] .end1 .end2 
;CHECK-LABEL: .end1:
;CHECK: [[R2:r[0-9]+]] = mul 0 1 32 
;CHECK-NEXT: br ..split 
;CHECK-LABEL: .end2:
;CHECK: [[R3:r[0-9]+]] = mul 2 1 32 
;CHECK-NEXT: br ..split 
;
  %result = call i32 @multiple_returns(i32 5)
  ret void
}