define i32 @func(i32 %arg1, i32 %arg2, i32 %arg3) {
; CHECK-LABEL: start func 3
;
; The entry block proceeds to bb_1 unconditionally and dominates it.
; Merge the entry block and bb_1, then remove bb_1.
  %cond1 = icmp eq i32 %arg1, %arg2
  br label %bb_1

bb_1:
  %a = sub i32 %arg2, %arg3
  br i1 %cond1, label %bb_2, label %bb_3

; bb_2 and bb_3 proceed to bb_4 unconditionally but does not dominate it.
; Merge (bb_2, bb_4) and (bb_3, bb_4), then keep bb_4 as it was.

bb_2:
  br label %bb_4
;CHECK: mul arg1 arg2 32
;CHECK-NEXT: add arg1 [[R1:r[0-9]]] 32
;CHECK-NEXT: ret [[R1]]
;CHECK-NOT: br [[BB4:.bb_[0-9]]]

bb_3:
  br label %bb_4
;CHECK: mul arg1 arg2 32
;CHECK-NEXT: add arg2 [[R2:r[0-9]]] 32
;CHECK-NEXT: ret [[R2]]
;CHECK-NOT: br [[BB5:.bb_[0-9]]]

bb_4:
  %p = phi i32 [%arg1, %bb_2], [%arg2, %bb_3]
  %b = mul i32 %arg1, %arg2
  %pb = add i32 %p, %b
  ret i32 %pb
}

define void @main() {
  %retVal = call i32 @func(i32 1, i32 2, i32 3)
  ret void
}