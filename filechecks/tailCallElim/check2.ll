; CHECK-LABEL:  start f1 1:
; CHECK: .entry:
; CHECK-NEXT: [[REG4:r[0-9]+]] = mul arg1 1 32
; CHECK-NEXT: [[REG3:r[0-9]+]] = mul 1 1 32
; CHECK-NEXT: br .tailrecurse
; CHECK: .tailrecurse:
; CHECK-NEXT: [[REG1:r[0-9]+]] = icmp sgt [[REG4]] 0 32
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
; CHECK-LABEL: end f1
; CHECK-LABEL:  start f 2:
; CHECK: .entry:
; CHECK-NEXT: [[REG5:r[0-9]+]] = mul arg1 1 32
; CHECK-NEXT: [[REG3:r[0-9]+]] = mul 0 1 1
; CHECK-NEXT: br .tailrecurse
; CHECK: .tailrecurse:
; CHECK-NEXT: [[REG1:r[0-9]+]] = icmp eq [[REG5]] arg2 32
; CHECK-NEXT: br [[REG1]] .then .else
; CHECK: .then:
; CHECK-NEXT: [[REG2:r[0-9]+]] = add [[REG5]] 1 32
; CHECK-NEXT: [[REG1]] = call f1 [[REG2]]
; CHECK-NEXT: [[REG1]] = mul [[REG1]] [[REG5]] 32
; CHECK-NEXT: [[REG1]] = select [[REG3]] [[REG4]] [[REG1]]
; CHECK-NEXT: [[REG4:r[0-9]+]] = mul [[REG1]] 1 32
; CHECK-NEXT: [[REG5]] = mul [[REG2]] 1 32
; CHECK-NEXT: [[REG3]] = mul 1 1 1
; CHECK-NEXT: br .tailrecurse
; CHECK: .else:
; CHECK-NEXT: [[REG1]] = select [[REG3]] [[REG4]] 1
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end f
; CHECK-LABEL:  start main 2:
; CHECK: [[REG1:r[0-9]+]] = call f arg1 arg2
; CHECK-NEXT: ret [[REG1]]
; CHECK-LABEL: end main
define i32 @f1(i32 %x) {
entry:
	%tmp.1 = icmp sgt i32 %x, 0
	br i1 %tmp.1, label %then, label %else
then:
	%tmp.6 = add i32 %x, 1
	%tmp.4 = call i32 @f1( i32 %tmp.6)
	%tmp.7 = mul i32 %tmp.4, %x
	ret i32 %tmp.7
else:
	ret i32 1
}
define i32 @f(i32 %x, i32 %y) {
entry:
	%tmp.1 = icmp eq i32 %x, %y
	br i1 %tmp.1, label %then, label %else
then:
	%tmp.6 = add i32 %x, 1
	%tmp.4 = call i32 @f1( i32 %tmp.6)
    %tmp.5 = call i32 @f( i32 %tmp.6, i32 %y)
	%tmp.7 = mul i32 %tmp.4, %x
	ret i32 %tmp.7
else:
	ret i32 1
}
define i32 @main(i32 %x, i32 %y) {
	%v = call i32 @f(i32 %x, i32 %y)
	ret i32 %v
}