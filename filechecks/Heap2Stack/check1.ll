;CHECK-LABEL: start main 0
;CHECK-NOT: malloc
;CHECK-NOT: free
;CHECK-NOT: bitcast
;CHECK-LABEL: end main

define void @main() {
    %a = call noalias i8* @malloc(i64 8)
    %b = bitcast i8* %a to i64*
    %c = call noalias i8* @malloc(i64 8)
    %d = bitcast i8* %c to i64*  
    %e = bitcast i64* %b to i8*
    %f = bitcast i64* %d to i8*
    call void @free(i8* %e)
    call void @free(i8* %f)
    ret void
}

declare void @free(i8*)
declare noalias i8* @malloc(i64)
