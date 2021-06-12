define i32 @func1(i32 %arg1, i32 %arg2, i32 %arg3) {
;CHECK: start func1 3
;CHECK-NEXT: [[DEFAULTBB:.[a-z0-9]+]]:
;CHECK-NEXT: [[COND1:r[0-9]+]] = icmp eq arg1 arg2 32
;CHECK-NEXT: [[A:r[0-9]+]] = sub arg2 arg3 32
;CHECK: .bb_2:
;CHECK: .bb_3:
;CHECK: ret [[R:r[0-9]+]]
  %cond1 = icmp eq i32 %arg1, %arg2
  br label %bb_1

bb_1:
  %a = sub i32 %arg2, %arg3
  br i1 %cond1, label %bb_2, label %bb_3

bb_2:
  br label %bb_3

bb_3:
  %b = mul i32 %arg1, %arg2
  ret i32 %b
}

define void @main() {
  %retVal = call i32 @func1(i32 10, i32 5, i32 10)
  ret void
}
