; CHECK-LABEL: start main 0
; CHECK: ._defaultBB0
; CHECK-NEXT: br .bb_ne
; CHECK-NEXT: .bb_ne
; CHECK-NEXT: br .bb_ugt
; CHECK-NEXT: .bb_ugt
; CHECK-NEXT: br .bb_sgt
; CHECK-NEXT: .bb_sgt
; CHECK-NEXT: br .bb_uge
; CHECK-NEXT: .bb_uge
; CHECK-NEXT: br .bb_sge
; CHECK-NEXT: .bb_sge
; CHECK-NEXT: br .bb_ult
; CHECK-NEXT: .bb_ult
; CHECK-NEXT: br .bb_slt
; CHECK-NEXT: .bb_slt
; CHECK-NEXT: br .bb_ule
; CHECK-NEXT: .bb_ule
; CHECK-NEXT: br .bb_sle
; CHECK-NEXT: .bb_sle
; CHECK-NEXT: br .bb_true
; CHECK-NEXT: .bb_true
; CHECK-NEXT: ret 1
; CHECK-NEXT: .bb_else
; CHECK-NEXT: ret 0
; CHECK-LABEL: end main
define i32 @main() {
  %cond1 = icmp eq i32 1, 1
  br i1 %cond1, label %bb_ne, label %bb_else
bb_ne:
  %cond2 = icmp ne i32 1, 2
  br i1 %cond2, label %bb_ugt, label %bb_else
bb_ugt:
  %cond3 = icmp ugt i32 2, 1
  br i1 %cond3, label %bb_sgt, label %bb_else
bb_sgt:
  %cond4 = icmp sgt i32 -1, -2
  br i1 %cond4, label %bb_uge, label %bb_else
bb_uge:
  %cond5 = icmp uge i32 2, 1
  br i1 %cond5, label %bb_sge, label %bb_else
bb_sge:
  %cond6 = icmp sge i32 -1, -2
  br i1 %cond6, label %bb_ult, label %bb_else
bb_ult:
  %cond7 = icmp ult i32 1, 2
  br i1 %cond7, label %bb_slt, label %bb_else
bb_slt:
  %cond8 = icmp slt i32 -2, -1
  br i1 %cond8, label %bb_ule, label %bb_else
bb_ule:
  %cond9 = icmp ule i32 1, 2
  br i1 %cond9, label %bb_sle, label %bb_else
bb_sle:
  %cond10 = icmp sle i32 -2, -1
  br i1 %cond10, label %bb_true, label %bb_else
bb_true:
  ret i32 1
bb_else:
  ret i32 0
}