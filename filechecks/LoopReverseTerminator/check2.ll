define void @main(i32 %n) {
;CHECK: start main 1

entry:
;CHECK: [[DEFAULTBB:.[a-z0-9]+]]:
;CHECK-NEXT: [[E:r[0-9]+]] = mul 0 1 32
;CHECK-NEXT: br .body
  br label %body

body:
;CHECK: .body:
;CHECK-NEXT: br .latch
  %i = phi i32 [ 0, %entry ], [ %i.next, %latch ]
  br label %latch

latch:
;CHECK: .latch:
;CHECK-NEXT: r2 = add r3 1 32 
;CHECK-NEXT: r1 = icmp sge r2 arg1 32
;CHECK-NEXT: r3 = mul r2 1 32 
;CHECK-NEXT: br r1 .exit .body
  %i.next = add nsw i32 %i, 1
  %cond = icmp slt i32 %i.next, %n
  br i1 %cond, label %body, label %exit

exit:
;CHECK: .exit:
;CHECK-NEXT: ret 0 
  ret void
}