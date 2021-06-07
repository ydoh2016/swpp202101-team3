define void @main() {
;CHECK: start main 0

entry:
;CHECK: [[DEFAULTBB:.[a-z0-9]+]]:
;CHECK-NEXT: [[E:r[0-9]+]] = mul 0 1 32
;CHECK-NEXT: br .loop
  br label %loop
loop:
;CHECK: .loop:
;CHECK_NEXT: r2 = add r1 1 32
;CHECK_NEXT: r1 = icmp ne r2 1024 32
;CHECK_NEXT: br r1 .exit .loop.splitted
  %iv = phi i32 [ 0, %entry ], [ %inc, %loop ]
  %inc = add i32 %iv, 1
  %exitcnd = icmp eq i32 %inc, 1024
  br i1 %exitcnd, label %loop, label %exit

exit:
;CHECK: .exit:
;CHECK_NEXT: ret 0
  ret void
}

declare dso_local void @write(i64) #1
