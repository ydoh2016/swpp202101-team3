#ifndef SWPP_ASM_INTERPRETER_SIZE_H
#define SWPP_ASM_INTERPRETER_SIZE_H


enum MSize {
  MSize1 = 0,
  MSize2,
  MSize4,
  MSize8
};

enum Size{
  Size1 = 0,
  Size8,
  Size16,
  Size32,
  Size64
};

enum VSize {
  VSize2 = 0,
  VSize4,
  VSize8
};

int msize_of(MSize msize);

int bw_of(Size size);

int vsize_of(VSize vsize);

#endif //SWPP_ASM_INTERPRETER_SIZE_H
