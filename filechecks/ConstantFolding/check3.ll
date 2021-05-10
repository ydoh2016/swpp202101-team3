; CHECK-LABEL: start main 2
; CHECK: ._defaultBB0
; CHECK-NEXT: br .bb_else
; CHECK-LABEL: end main
define i32 @main(i32 %a, i32 %b) {
  %cond = icmp eq i32 %a, %b
  br i1 %cond, label %bb_true, label %bb_else
bb_true:
  ret i32 1
bb_else:
  ret i32 0
}