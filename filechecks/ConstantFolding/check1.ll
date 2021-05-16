; CHECK-LABEL: start main 2
; CHECK: ._defaultBB0
; CHECK-NEXT: ret 2
; CHECK-LABEL: end main
define i32 @main(i32 %a, i32 %b) {
  %c = add i32 10, 20
  %d = sub i32 %c, 2
  %e = mul i32 %d, 4
  %f = urem i32 %e, 40
  %g = mul i32 %f, -1
  %h = srem i32 %g, 20
  %i = mul i32 %h, -1
  %j = sdiv i32 %i, 2
  %k = mul i32 %j, -1
  %l = sdiv i32 %k, -3
  ret i32 %l
}