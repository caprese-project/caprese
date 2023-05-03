auto str = "Hello, World!";

void putc(char ch) {
  asm volatile("mv a7, %0" : : "r"(1));
  asm volatile("mv a0, %0" : : "r"(ch));
  asm volatile("ecall");
}

extern "C" int main() {
  while(true) {
    putc('H');
    putc('e');
    putc('l');
    putc('l');
    putc('o');
    putc('\n');
  }
  return 0;
}
