; ModuleID = 'check1.ll'
source_filename = "check1.ll"

define i64 @main() {
  %hptr = call i64* @malloc(i64 24)

  %hptr0 = getelementptr inbounds i64, i64* %hptr, i64 0 
  store i64 10, i64* %hptr0
  %hptr1 = getelementptr inbounds i64, i64* %hptr, i64 1
  store i64 20, i64* %hptr1
  %hptr2 = getelementptr inbounds i64, i64* %hptr, i64 2
  store i64 30, i64* %hptr2

  %load1 = load i64, i64* %hptr0
  %load2 = load i64, i64* %hptr1
  %load3 = load i64, i64* %hptr2

  %load4 = add i64 %load1, %load2
  %load5 = mul i64 %load3, %load4
  
  ret i64 %load5
}

declare i64* @malloc(i64)

