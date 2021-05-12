; CHECK-LABEL:  start f 2:
; CHECK: [[REG3:r[0-9]+]] = add arg1 arg2 32
; CHECK-NEXT: [[REG2:r[0-9]+]] = add arg1 4294967295 32
; CHECK-NEXT: [[REG1:r[0-9]+]] = icmp sle [[REG2]] 0 32
; CHECK-NEXT: br [[REG1]] .bb_recur .bb_exit
; CHECK: .bb_recur:
; CHECK: [[REG1:r[0-9]+]] = call f [[REG2]] [[REG3]]
; CHECK-NEXT:  ret [[REG1]]
; CHECK: .bb_exit:
; CHECK-NEXT: ret [[REG3]]
; CHECK-LABEL: end f
; CHECK-LABEL:  start main 0:
; CHECK: [[REG1:r[0-9]+]] = call f 0 1
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end main
define internal i32 @f(i32 %x, i32 %y, i32 %z) {
    %t = add i32 %x, %y
    %tt = add i32 %x, -1
    %cond = icmp sle i32 %tt, 0
    br i1 %cond, label %bb_recur, label %bb_exit
bb_recur:
    %1 = call i32 @f(i32 %tt, i32 %t, i32 %z)
    ret i32 %1
bb_exit:
    ret i32 %t
}

define i32 @main() {
    %x = call i32 @f(i32 0, i32 1, i32 2)
    ret i32 %x
}