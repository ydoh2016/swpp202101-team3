define dso_local i32 @_Z3fibi(i32 %n) #0 {
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
  %call = call i32 @_Z3fibi(i32 %sub)
  %sub4 = sub nsw i32 %n, 2
  %call5 = call i32 @_Z3fibi(i32 %sub4)
  %add = add nsw i32 %call, %call5
  br label %return

return:
  %retval.0 = phi i32 [ 0, %if.then ], [ 1, %if.then2 ], [ %add, %if.else3 ]
  ret i32 %retval.0
}

define i32 @inlinable() {
  entry:
    ret i32 0 
}

define dso_local i32 @main() #1 {
entry:
;CHECK-LABEL: .entry:
;CHECK: [[R1:r[0-9]+]] = call _Z3fibi 10 
;CHECK-NEXT: br .entry1 
;CHECK-LABEL: .entry.split:
;CHECK: ret 0 
;CHECK-LABEL: .entry1:
;CHECK: [[R2:r[0-9]+]] = mul 0 1 32 
;CHECK-NEXT: br .entry.split 
;
  %call = call i32 @_Z3fibi(i32 10)
  %call2 = call i32 @inlinable()
  ret i32 0
}