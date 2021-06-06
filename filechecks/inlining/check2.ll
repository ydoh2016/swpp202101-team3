declare i8* @malloc(i64)
declare void @free(i8*)

define dso_local void @swap(i32* %a, i32* %b) {
entry:
  %0 = load i32, i32* %a, align 4
  %1 = load i32, i32* %b, align 4
  store i32 %1, i32* %a, align 4
  store i32 %0, i32* %b, align 4
  ret void
}


define i32 @main() {
;CHECK-LABEL: start main 0
entry:
    %ptr.0 = call i8* @malloc(i64 32)
    %ptr.1 = bitcast i8* %ptr.0 to i32*

    %ptr.2 = call i8* @malloc(i64 32)
    %ptr.3 = bitcast i8* %ptr.2 to i32*

    store i32 3, i32* %ptr.1
    store i32 5, i32* %ptr.3
;CHECK-NOT: call swap [[RLEFT:r[0-9]+]] [[RRIGHT:r[0-9]+]]
;CHECK: br .entry1
;CHECK-LABEL: .entry.split:
;CHECK-LABEL: .entry1:
;CHECK: br .entry.split
    call void @swap(i32* %ptr.1, i32* %ptr.3)
    %div.0 = load i32, i32* %ptr.1
    %div.1 = load i32, i32* %ptr.3

    %result = urem i32 %div.0, %div.1

    call void @free(i8* %ptr.0)
    call void @free(i8* %ptr.2)
    
    ret i32 %result
}