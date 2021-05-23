start main 0:
._defaultBB0:
  ; Init stack pointer
  sp = sub sp 16 64 
  r2 = add sp 0 64 
  r1 = add sp 8 64 
  free r1 
  free r2 
  sp = add sp 16 64 
  ret 0 
end main

