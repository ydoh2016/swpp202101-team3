define void @f(i32 %disk, i32 %src, i32 %dest, i32 %spare) {
bb_entry:
    %cond = icmp eq i32 %disk, 0
    br i1 %cond, label %bb_true, label %bb_else
bb_true:
    ret void
bb_else:
    %disk.0 = add i32 %disk, -1
    call void @f( i32 %disk.0, i32 %src, i32 %spare, i32 %dest)
    call void @f( i32 %disk.0, i32 %spare, i32 %dest, i32 %src)
    ret void
}