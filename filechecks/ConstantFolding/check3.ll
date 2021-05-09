; CHECK-LABEL: start f 0:
; CHECK: br .bb_else
; CHECK: .bb_true
; CHECK-NEXT: ret 5
; CHECK: .bb_else
; CHECK-NEXT: ret 7
; CHECK-LABEL: end f
define i32 @f() {
  %a = add i32 1, 2
  %cond = icmp eq i32 %a, 2
  br i1 %cond, label %bb_true, label %bb_else
bb_true:
  %b = add i32 2, 3
  ret i32 %b
bb_else:
  %c = add i32 3, 4
  ret i32 %c
}