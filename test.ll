define i1 @main(i32 %a, i32 %b) {
entry:
  %c = add i32 %a, %b
  %d = sub i32 %a, %b
  br label %for.cond

for.cond:
  %cond = icmp eq i32 %c, %d
  br i1 %cond, label %bb1, label %bb2

bb1:
  ret i1 %cond

bb2:
  ret i1 %cond
}