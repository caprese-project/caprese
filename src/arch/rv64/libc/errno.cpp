namespace {
  int __errno_value;
} // namespace

extern "C" {
  int* __errno() {
    return &__errno_value;
  }
}
