declare void @h1()
declare void @h2()
declare void @h3()

; CHECK-LABEL:  start main 1:
; CHECK: .entry:
; CHECK-NEXT: [[REG1:r[0-9]+]] = icmp eq arg1 10 32
; CHECK-NEXT: br [[REG1]] .A .ELSE1
; CHECK: .A:
; CHECK-NEXT: call h1
; CHECK-NEXT: ret 0
; CHECK: .ELSE1:
; CHECK-NEXT: [[REG1]] = add arg1 20 32
; CHECK-NEXT: [[REG1]] = icmp eq arg1 20 32
; CHECK-NEXT: br [[REG1]] .B .ELSE2
; CHECK: .B:
; CHECK-NEXT: call h2
; CHECK-NEXT: ret 0
; CHECK: .ELSE2:
; CHECK-NEXT: call h3
; CHECK-NEXT: ret 0
; CHECK-LABEL: end main

define void @main(i32 %val) nounwind {
entry:
    %c1 = icmp eq i32 %val, 10
    br i1 %c1, label %A, label %ELSE1
A:
    call void @h1()
    ret void
ELSE1:
    %d = add i32 %val, 20
    %c2 = icmp eq i32 %val, 20
    br i1 %c2, label %B, label %ELSE2
B:
    call void @h2()
    ret void

ELSE2:
    call void @h3()
    ret void
}