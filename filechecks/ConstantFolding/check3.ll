; CHECK-LABEL: start main 0
; CHECK: ._defaultBB0
; CHECK-NEXT: br .bb_else
; CHECK-NEXT: .bb_true
; CHECK-NEXT: ret 1
; CHECK-NEXT: .bb_else
; CHECK-NEXT: ret 0
; CHECK-LABEL: end main
define i32 @main() {
  %cond = icmp eq i32 1, 2
  br i1 %cond, label %bb_true, label %bb_else
bb_true:
  ret i32 1
bb_else:
  ret i32 0
}