; CHECK-LABEL:  start main 2:
; CHECK: .entry:
; CHECK-NEXT: [[REG1:r[0-9]+]] = mul arg1 1 32
; CHECK-NEXT: [[REG3:r[0-9]+]] = mul arg1 1 32
; CHECK-NEXT: br .for.header
; CHECK: .for.header:
; CHECK-NEXT: [[REG2:r[0-9]+]] = icmp eq [[REG3]] arg2 32
; CHECK-NEXT: [[REG4:r[0-9]+]] = mul [[REG1]] 1 32
; CHECK-NEXT: [[REG1]] = mul [[REG3]] 1 32
; CHECK-NEXT: [[REG3]] = mul [[REG4]] 1 32
; CHECK-NEXT: br [[REG2]] .ops.1 .body
; CHECK: .body:
; CHECK-NEXT: [[REG2]] = add [[REG1]] [[REG3]] 32
; CHECK-NEXT: br .latch
; CHECK: .latch:
; CHECK-NEXT: [[REG1]] = add [[REG3]] 1 32
; CHECK-NEXT: [[REG3]] = mul [[REG1]] 1 32
; CHECK-NEXT: [[REG1]] = mul [[REG2]] 1 32
; CHECK-NEXT: br .for.header
; CHECK: .ops.1:
; CHECK-NEXT: [[REG1]] = add [[REG1]] arg2 32
; CHECK-NEXT: ret [[REG3]]
; CHECK-LABEL: end main
define i32 @main(i32 %a, i32 %b) {
entry:
  br label %for.header
for.header:
  %i = phi i32 [%a, %entry], [%i.next, %latch]
  %total = phi i32 [%a, %entry], [%total.next, %latch]
  %cond = icmp eq i32 %i, %b
  br i1 %cond, label %ops.1, label %body
body:
  %power = add nsw i32 %a,%b
  %total.next = add nsw i32 %total,%i
  br label %latch
latch:
  %i.next = add nsw i32 %i, 1
  br label %for.header
ops.1:
  %check = add nsw i32 %i, %b
  ret i32 %total
}
