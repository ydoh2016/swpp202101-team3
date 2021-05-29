define i32 @func2(i32 %arg1, i32 %arg2, i32 %arg3) {
; CHECK-LABEL: start func2 3
;
; The entry block proceeds to bb_1 unconditionally and dominates it.
; Merge the entry block and bb_1, then remove bb_1.
  %cond1 = icmp eq i32 %arg1, %arg2
  br label %bb_1

bb_1:
  %a = sub i32 %arg2, %arg3
  br i1 %cond1, label %bb_2, label %bb_3

; Do not optimize because there is a phi node in bb_4
bb_2:
;CHECK: br .bb_4
  br label %bb_4

bb_3:
;CHECK: br .bb_4
  br label %bb_4

bb_4:
  %p = phi i32 [%arg1, %bb_2], [%arg2, %bb_3]
  %b = mul i32 %arg1, %arg2
  %pb = add i32 %p, %b
  ret i32 %pb
}

define void @main() {
  %retVal = call i32 @func2(i32 1, i32 2, i32 3)
  ret void
}