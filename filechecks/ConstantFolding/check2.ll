; CHECK-LABEL: start f 0:
; CHECK-NEXT: [[REG1:r[0-9]+]] = add arg1 arg2 32
; CHECK: .bb_true
; CHECK-NEXT: ret 0
; CHECK: .bb_else
; CHECK-NEXT: ret 1
; CHECK-LABEL: end f
define i32 @f(i32 %a, i32 %b) {
  %c = add i32 1, 2
  %cond = icmp sgt i32 %a, %b
  br i1 %cond, label %bb_true, label %bb_else
bb_true:
  %d = srem i32 %c, 1
  ret i32 %d
bb_else:
  %e = srem i32 %c, 2
  ret i32 %e
}