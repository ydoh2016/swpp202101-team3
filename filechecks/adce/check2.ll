define i32 @main() {
; CHECK-LABEL:  start main 0:
; CHECK: ret 0
; CHECK-LABEL: end main
    %y = add i32 1, 1
    %z = add i32 %y, 2
    %w = mul i32 %y, 2
    %cond = icmp eq i32 %y, %z
    ret i32 0
}