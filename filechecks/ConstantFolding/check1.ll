; CHECK-LABEL: start main 2
; CHECK: ._defaultBB0
; CHECK-NEXT: ret 1
; CHECK-LABEL: end main
define i32 @main(i32 %a, i32 %b) {
  %c = add i32 1, 2
  %d = sub i32 %c, 1
  %e = mul i32 %d, 3
  %f = udiv i32 %e, 2
  %g = urem i32 %f, 4
  %h = srem i32 %g, 2
  %i = udiv i32 %h, 1
  %j = sdiv i32 %i, 1
  ret i32 %j
}