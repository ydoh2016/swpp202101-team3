define i32 @main() {
; CHECK-LABEL:  start main 0:
; CHECK: [[REG1:r[0-9]+]] = mul 2 1 32
; CHECK-NEXT: br .bb_true
; CHECK: .bb_true:
; CHECK-NEXT: br .bb_exit
; CHECK: .bb_false:
; CHECK-NEXT: br .bb_exit
; CHECK: .bb_exit:
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end main
    %y = add i32 1, 1
    %z = add i32 %y, 2
    %w = mul i32 %y, 2
    %cond = icmp eq i32 %y, %z
    br i1 %cond, label %bb_true, label %bb_false
bb_true:
    %x.0 = add i32 %y, %z
    br label %bb_exit
bb_false:
    %x.1 = add i32 %y, %z
    br label %bb_exit
bb_exit:
    %x = phi i32 [%x.0, %bb_true], [%y, %bb_false]
    ret i32 %y 
}