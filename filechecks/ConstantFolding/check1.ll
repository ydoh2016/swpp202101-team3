; CHECK-LABEL: start f 0:
; CHECK-NEXT: ret 6
; CHECK-LABLE: end f
define i32 @f() {
  %a = add i32 1, 2
  %b = sub i32 %a, 1
  %c = mul i32 %b, 3
  ret i32 %c
}