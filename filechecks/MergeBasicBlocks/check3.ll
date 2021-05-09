define i32 @func3(i32 %arg1, i32 %arg2) {
; CHECK: start func3 2
; CHECK-NEXT: .bb_1:
; CHECK-NEXT: [[COND1:r[0-9]+]] = icmp eq arg1 arg2 32
; CHECK-NEXT: [[A:r[0-9]+]] = add arg1 arg2 32
; CHECK-NEXT: [[B:r[0-9]+]] = sub arg1 arg2 32
; CHECK-NEXT: [[C:r[0-9]+]] = mul arg1 arg2 32
; CHECK-NEXT: [[D:r[0-9]+]] = mul [[A]] [[B]] 32
; CHECK-NEXT: [[E:r[0-9]+]] = mul [[C]] [[D]] 32
; CHECK-NEXT: [[F:r[0-9]+]] = mul [[D]] [[E]] 32
; CHECK-NEXT: ret [[F]]
; CHECK-NEXT: end func3
;                
; The basic blocks are all combined together
  %cond1 = icmp eq i32 %arg1, %arg2
  br label %bb_1

bb_4:
  %d = mul i32 %a, %b
  %e = mul i32 %c, %d
  %f = mul i32 %d, %e
  ret i32 %f

bb_3:
  %c = mul i32 %arg1, %arg2
  br label %bb_4

bb_2:
  %b = sub i32 %arg1, %arg2
  br label %bb_3

bb_1:
  %a = add i32 %arg1, %arg2
  br label %bb_2
}

define void @main() {
  %retVal = call i32 @func3(i32 5, i32 3)
  ret void
}