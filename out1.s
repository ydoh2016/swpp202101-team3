start func1 3:
.bb_1:
  r2 = icmp eq arg1 arg2 32 
  r1 = sub arg2 arg3 32 
  br r2 .bb_2 .bb_3 
.bb_2:
  r1 = mul arg1 arg2 32 
  ret r1 
.bb_3:
  r1 = mul arg1 arg2 32 
  ret r1 
end func1

start main 0:
._defaultBB0:
  r1 = call func1 10 5 10 
  ret 0 
end main

