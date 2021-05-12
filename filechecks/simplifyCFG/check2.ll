declare void @h1()
declare void @h2()
declare void @h3()

; CHECK-LABEL:  start main 1:
; CHECK: .entry:
; CHECK-NEXT: switch arg1 10 .A 20 .B 30 .C .ELSE3
; CHECK: .A:
; CHECK-NEXT: call h1
; CHECK-NEXT: ret 0
; CHECK: .B:
; CHECK-NEXT: call h2
; CHECK-NEXT: ret 0
; CHECK: .C:
; CHECK-NEXT: call h3
; CHECK-NEXT: ret 0
; CHECK: .ELSE3:
; CHECK-NEXT: call h2
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
    %c2 = icmp eq i32 %val, 20
    br i1 %c2, label %B, label %ELSE2
B:
    call void @h2()
    ret void
ELSE2:
    %c3 = icmp eq i32 %val, 30
    br i1 %c3, label %C, label %ELSE3
C:
    call void @h3()
    ret void
ELSE3:
    call void @h2()
    ret void
}