; CHECK-LABEL:  start h 2:
; CHECK: [[REG1:r[0-9]+]] = add arg1 arg2 32
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end h
; CHECK-LABEL:  start f 2:
; CHECK: [[REG1:r[0-9]+]] = call h arg1 arg2
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end f
; CHECK-LABEL:  start main 0:
; CHECK: [[REG1:r[0-9]+]] = call f 1 2
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end main
define internal i32 @h(i32 %x, i32 %y, i32 %z) {
    %b = add i32 %y, %z
    ret i32 %b
}

define internal i32 @f(i32 %x, i32 %y, i32 %z) {
    %t = call i32 @h(i32 %x, i32 %y, i32 %z)
    ret i32 %t
}

define i32 @main() {
    %x = call i32 @f(i32 0, i32 1, i32 2)
    ret i32 %x
}