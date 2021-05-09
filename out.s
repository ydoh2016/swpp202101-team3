start f 3:
.bb_1:
  r1 = icmp eq arg1 arg2 32 
  r1 = sub arg2 arg3 32 
  br r1 .bb_2 .bb_3 
.bb_2:
  r1 = mul arg1 arg2 32 
  r1 = add arg1 r1 32 
  ret r1 
.bb_3:
  r1 = mul arg1 arg2 32 
  r1 = add arg2 r1 32 
  ret r1 
end f

start main 0:
._defaultBB0:
  r1 = call f 1 2 3 
  ret 0 
end main

