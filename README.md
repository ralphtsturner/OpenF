# OpenF - A Minimal C File & BMP Image Utility Library

OpenF is a lightweight, single-header library for basic **file I/O** and **BMP image** manipulation written in plain C with C++ compatibility. It aims to provide simple, robust file utilities with minimal dependencies — perfect for small projects, quick tools, or embedding in larger codebases.

---

## Features

### File I/O
- Read whole files into memory (`openf_read`)
- Write data to files (`openf_write`)
- Append text to files (`openf_append_text`)
- Check if a file exists (`openf_exists`)
- Get file size (`openf_get_size`)
- Copy files (`openf_copy_file`)
- Merge two files by concatenation (`openf_merge_files`)

### BMP Image Support (24-bit only)
- Load uncompressed 24-bit BMP images (`openf_load_bmp`)
- Save RGB/RGBA BMP images (`openf_save_bmp`)
- Free BMP image memory (`openf_free_image`)

### Utilities
- Safe internal string duplication (`openf_strdup`)
- Initialization and cleanup (`openf_init`, `openf_cleanup`)
- Detailed error handling with `OpenF_Error` enum and error string helper
- Debug printing toggle (`OPENF_DEBUG` macro)

---

## Getting Started

1. **Include `openf.h`** in your project (copy or add as a submodule).

2. Optionally define `OPENF_DEBUG` before including to enable verbose debug prints:

```c
#define OPENF_DEBUG 1
#include "openf.h"
