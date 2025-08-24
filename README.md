# OpenF — A Minimal File & BMP Utility Library for C/C++

**OpenF** is a single-header, zero-dependency C library that provides basic file manipulation and 24-bit BMP image support. It is ideal for simple tools, embedded utilities, and lightweight applications where you want full control and minimal runtime cost.

Written mostly from scratch, OpenF is intentionally minimal yet powerful enough to handle real-world file operations without the overhead of full frameworks or bloated libraries.

---

## 🚀 Features

### 📂 File I/O
- `openf_init()` / `openf_cleanup()` — Global setup & teardown (optional, future-proof).
- `openf_read(path, &out)` — Read entire file into memory.
- `openf_write(path, data, size)` — Write raw data to file.
- `openf_append_text(path, text)` — Append a null-terminated string to file.
- `openf_exists(path)` — Check if a file exists.
- `openf_get_size(path, &out)` — Get file size in bytes.
- `openf_copy_file(src, dest)` — Copy file contents from source to destination.
- `openf_merge_files(out, a, b)` — Concatenate two files into a new one.
- `openf_free_file(&file)` — Free memory allocated by `openf_read`.

### 🖼️ BMP Image (24-bit only)
- `openf_load_bmp(path, &out_image)` — Load 24-bit uncompressed BMP into memory.
- `openf_save_bmp(path, image)` — Save image as 24-bit BMP.
- `openf_free_image(&image)` — Free image memory.

### 🔧 Utilities
- `openf_strdup(s)` — Safe internal string duplicator.
- Optional debug output via `#define OPENF_DEBUG 1`.

---

## 📦 Getting Started

### 📁 Include the Header

Just drop `openf.h` into your project and include it:

```c
#include "openf.h"
```

If you're using C++, wrap it like this:

```c
extern "C" {
    #include "openf.h"
}
```

To enable debug logging, define OPENF_DEBUG before including the header:

```c
#define OPENF_DEBUG 1
#include "openf.h"
```

## 🧪 Example Usage (C++)

```c
#include <iostream>
#include <cstring>
#include "openf.h"

int main() {
    if (openf_init() != OPENF_OK) {
        std::cerr << "Failed to initialize OpenF.\n";
        return 1;
    }

    const char* filename = "file1.txt";
    const char* content = "Hello openf!\n";

    // Write to file
    if (openf_write(filename, content, strlen(content)) != OPENF_OK) {
        std::cerr << "Write failed!\n";
        return 1;
    }

    // Read file
    OpenF_File file = {nullptr, 0};
    if (openf_read(filename, &file) != OPENF_OK) {
        std::cerr << "Read failed!\n";
        return 1;
    }

    std::cout << "Read " << file.size << " bytes: ";
    std::cout.write(file.data, file.size);
    std::cout << "\n";

    // Clean up
    openf_free_file(&file);
    openf_cleanup();
    return 0;
}
```

## ❗ Important Notes

BMP Format: Only uncompressed 24-bit .bmp images are supported.

Memory Management: Files and images read with OpenF must be freed using ``` openf_free_file() ``` or ``` openf_free_image() ```.

Thread Safety: The library is not thread-safe — avoid concurrent calls.

C++ Compatible: Fully usable in C++ via ``` extern "C" ```.

Error Handling: All functions return meaningful OpenF_Error codes. Use openf_error_str() to convert them to readable strings.

## ⚠️ Error Codes
Code	Meaning

``` OPENF_OK ``` Operation succeeded 

``` OPENF_ERR_NULL_ARG ```        Null pointer passed 

``` OPENF_ERR_OPEN_FAILED	```     File could not be opened 

``` OPENF_ERR_SEEK_FAILED	```     Failed to seek in file 

``` OPENF_ERR_READ_FAILED	```     Read operation failed 

``` OPENF_ERR_WRITE_FAILED ```    Write operation failed 

``` OPENF_ERR_CLOSE_FAILED ```    fclose failed

``` OPENF_ERR_MEM_ALLOC ```	      Memory allocation failed 

``` OPENF_ERR_FILE_EXISTS ```	    File already exists (optional) 

``` OPENF_ERR_FILE_NOT_FOUND ```	File does not exist 

``` OPENF_ERR_INVALID_FORMAT ```	Invalid BMP format 

``` OPENF_ERR_UNSUPPORTED ```	    Feature not supported 

``` OPENF_ERR_GENERAL_FAILURE ```	Catch-all for other errors 

Use this helper for diagnostics:

```c
const char* msg = openf_error_str(error_code);
printf("Error: %s\n", msg);
```

## 🧼 Best Practices

Always check return codes.

Always free allocated file/image data.

Wrap usage in ``` openf_init() ``` / ``` openf_cleanup() ``` for safety (especially if extended).

## 📝 License

This project is licensed under the MIT License. You are free to use, modify, distribute, and include it in commercial or open-source projects.
