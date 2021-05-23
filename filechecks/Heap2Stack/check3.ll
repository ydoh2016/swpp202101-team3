;CHECK-LABEL: start f5 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end f5
define i64* @f5() {
    %a = call noalias i8* @malloc(i64 8)
    %b = bitcast i8* %a to i64*
    ret i64* %b
}
;CHECK-LABEL: start f4 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end f4
define i64* @f4() {
    %a = call i64* @f5()
    ret i64* %a
}
;CHECK-LABEL: start f3 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end f3
define i64* @f3() {
    %a = call i64* @f4()
    ret i64* %a
}
;CHECK-LABEL: start f2 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end f2
define i64* @f2() {
    %a = call i64* @f3()
    ret i64* %a
}
;CHECK-LABEL: start f1 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end f1
define i64* @f1() {
    %a = call i64* @f2()
    ret i64* %a
}
;CHECK-LABEL: start main 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end main
define i64 @main() {
    %a = call i64* @f1()
    %b = bitcast i64* %a to i8*
    call void @free(i8* %b)
    ret i64 0
}

declare void @free(i8*)
declare noalias i8* @malloc(i64)