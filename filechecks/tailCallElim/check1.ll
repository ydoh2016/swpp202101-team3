; CHECK-LABEL:  start f 2:
; CHECK: .entry:
; CHECK-NEXT: [[REG4:r[0-9]+]] = mul arg1 1 32
; CHECK-NEXT: [[REG3:r[0-9]+]] = mul 1 1 32
; CHECK-NEXT: br .tailrecurse
; CHECK: .tailrecurse:
; CHECK-NEXT: [[REG1:r[0-9]+]] = icmp eq [[REG4]] arg2 32
; CHECK-NEXT: br [[REG1]] .then .else
; CHECK: .then:
; CHECK-NEXT: [[REG2:r[0-9]+]] = add [[REG4]] 1 32
; CHECK-NEXT: [[REG1]] = mul [[REG3]] [[REG4]] 32
; CHECK-NEXT: [[REG3]] = mul [[REG1]] 1 32
; CHECK-NEXT: [[REG4]] = mul [[REG2]] 1 32
; CHECK-NEXT: br .tailrecurse
; CHECK: .else:
; CHECK-NEXT: [[REG1]] = mul [[REG3]] 1 32
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end f
define i32 @f(i32 %x, i32 %y) {
entry:
	%tmp.1 = icmp eq i32 %x, %y
	br i1 %tmp.1, label %then, label %else
then:
	%tmp.6 = add i32 %x, 1
	%tmp.4 = call i32 @f( i32 %tmp.6, i32 %y )
	%tmp.7 = mul i32 %tmp.4, %x
	ret i32 %tmp.7
else:
	ret i32 1
}