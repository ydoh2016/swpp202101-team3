start func3 2:
.bb_1:
  r1 = icmp eq arg1 arg2 32 
  r1 = add arg1 arg2 32 
  r2 = sub arg1 arg2 32 
  r3 = mul arg1 arg2 32 
  r2 = mul r1 r2 32 
  r1 = mul r3 r2 32 
  r1 = mul r2 r1 32 
  ret r1 
end func3

start main 0:
._defaultBB0:
  r1 = call func3 5 3 
  ret 0 
end main

