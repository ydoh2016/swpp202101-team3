; CHECK-LABEL: start main 2
; CHECK: ._defaultBB0
; CHECK-NEXT: [[COND:r[0-9]+]] = icmp sgt arg1 arg2 32
; CHECK-NEXT: br [[COND]] .bb_true .bb_else
; CHECK: .bb_true
; CHECK-NEXT: ret 0
; CHECK: .bb_else
; CHECK-NEXT: ret 1
; CHECK-LABEL: end main
define i32 @main(i32 %a, i32 %b) {
  %c = add i32 1, 2
  %cond = icmp sgt i32 %a, %b
  br i1 %cond, label %bb_true, label %bb_else
bb_true:
  %d = srem i32 %c, 3
  ret i32 %d
bb_else:
  %e = srem i32 %c, 2
  ret i32 %e
}