define i32 @main() {
; CHECK-LABEL:  start main 0:
; CHECK: [[REG1:r[0-9]+]] = mul 2 1 32
; CHECK-NEXT: [[REG1]] = mul [[REG1]] 2 32
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end main
    %y = add i32 1, 1
    %z = add i32 %y, 2
    %w = mul i32 %y, 2
    ret i32 %w 
}