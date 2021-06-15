; CHECK-LABEL: start main 0
; CHECK: ._defaultBB0
; CHECK-NEXT: ret 4
; CHECK-LABEL: end main
define i32 @main(i32 %a, i32 %b) {
  ret i32 4
}