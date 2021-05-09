; CHECK-LABEL:  start f 4:
; CHECK: .bb_entry:
; CHECK-NEXT: [[REG3:r[0-9]+]] = mul arg4 1 32
; CHECK-NEXT: [[REG4:r[0-9]+]] = mul arg2 1 32
; CHECK-NEXT: [[REG5:r[0-9]+]] = mul arg1 1 32
; CHECK-NEXT: br .tailrecurse
; CHECK: .tailrecurse:
; CHECK-NEXT: [[REG1:r[0-9]+]] = icmp eq [[REG5]] 0 32
; CHECK-NEXT: br [[REG1]] .bb_true .bb_else
; CHECK: .bb_true:
; CHECK-NEXT: ret 0
; CHECK: .bb_else:
; CHECK-NEXT: [[REG1]] = add [[REG5]] 4294967295 32
; CHECK-NEXT: call f [[REG1]] [[REG4]] [[REG3]] arg3
; CHECK-NEXT: [[REG5]] = mul [[REG1]] 1 32
; CHECK-NEXT: [[REG6:r[0-9]+]] = mul [[REG3]] 1 32
; CHECK-NEXT: [[REG3]] = mul [[REG4]] 1 32
; CHECK-NEXT: [[REG4]] = mul [[REG6]] 1 32
; CHECK-NEXT: br .tailrecurse
; CHECK-LABEL: end f
define void @f(i32 %disk, i32 %src, i32 %dest, i32 %spare) {
bb_entry:
    %cond = icmp eq i32 %disk, 0
    br i1 %cond, label %bb_true, label %bb_else
bb_true:
    ret void
bb_else:
    %disk.0 = add i32 %disk, -1
    call void @f( i32 %disk.0, i32 %src, i32 %spare, i32 %dest)
    call void @f( i32 %disk.0, i32 %spare, i32 %dest, i32 %src)
    ret void
}