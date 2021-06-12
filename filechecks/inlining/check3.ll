define dso_local i32 @fib(i32 %n) #0 {
entry:
  %cmp = icmp eq i32 %n, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:
  br label %return

if.else:
  %cmp1 = icmp eq i32 %n, 1
  br i1 %cmp1, label %if.then2, label %if.else3

if.then2:
  br label %return

if.else3:
  %sub = sub nsw i32 %n, 1
  %call = call i32 @fib(i32 %sub)
  %sub4 = sub nsw i32 %n, 2
  %call5 = call i32 @fib(i32 %sub4)
  %add = add nsw i32 %call, %call5
  br label %return

return:
  %retval.0 = phi i32 [ 0, %if.then ], [ 1, %if.then2 ], [ %add, %if.else3 ]
  ret i32 %retval.0
}

define dso_local i32 @main() #1 {
;CHECK-LABEL:.entry:
;CHECK: br .entry1 
;CHECK-LABEL: .entry.split:
;CHECK: ret 0 
;CHECK-LABEL: .entry1:
;CHECK: br [[R:r[0-9]+]] .if.then .if.else 
;CHECK-LABEL: .if.then:
;CHECK: br .return 
;CHECK-LABEL: .if.else:
;CHECK: br [[R:r[0-9]+]] .if.then2 .if.else3 
;CHECK-LABEL: .if.then2:
;CHECK: br .return 
;CHECK-LABEL: .if.else3:
;CHECK: [[R:r[0-9]+]] = call fib [[R:r[0-9]+]] 
;CHECK: br .return 
;CHECK-LABEL: .return:
;CHECK: br .entry.split

entry: 
  %call = call i32 @fib(i32 10)
  ret i32 0
}