define i32 @f() {
; CHECK-LABEL:  start f 0:
; CHECK: [[REG1:r[0-9]+]] = add 1 1 32
; CHECK-NEXT: [[REG1]] = mul [[REG1]] 2 32
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end f
    %y = add i32 1, 1
    %z = add i32 %y, 2
    %w = mul i32 %y, 2
    ret i32 %w 
}