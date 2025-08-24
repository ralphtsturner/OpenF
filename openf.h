#ifndef OPENF_H
#define OPENF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-----------------------------------
  Configurations & Debug
------------------------------------*/

#ifndef OPENF_DEBUG
#define OPENF_DEBUG 0
#endif

#if OPENF_DEBUG
#include <assert.h>
#define OPENF_DBG_PRINT(fmt, ...) fprintf(stderr, "[openf DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define OPENF_DBG_PRINT(fmt, ...)
#endif

/*-----------------------------------
  Error Codes
------------------------------------*/

typedef enum {
    OPENF_OK = 0,
    OPENF_ERR_NULL_ARG,
    OPENF_ERR_OPEN_FAILED,
    OPENF_ERR_SEEK_FAILED,
    OPENF_ERR_READ_FAILED,
    OPENF_ERR_WRITE_FAILED,
    OPENF_ERR_MEM_ALLOC,
    OPENF_ERR_INVALID_FORMAT,
    OPENF_ERR_UNSUPPORTED,
    OPENF_ERR_CLOSE_FAILED,
    OPENF_ERR_FILE_EXISTS,
    OPENF_ERR_FILE_NOT_FOUND,
    OPENF_ERR_GENERAL_FAILURE
} OpenF_Error;

/*-----------------------------------
  File struct and management
------------------------------------*/

typedef struct {
    char* data;      // Allocated content buffer
    size_t size;     // Size of data buffer in bytes
} OpenF_File;

/* Initialize OpenF_File from a string content */
static inline OpenF_Error openf_init_file(OpenF_File* file, const char* content) {
    if (!file) return OPENF_ERR_NULL_ARG;
    if (!content) {
        file->data = NULL;
        file->size = 0;
        return OPENF_OK;
    }
    size_t len = strlen(content);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return OPENF_ERR_MEM_ALLOC;
    memcpy(buf, content, len);
    buf[len] = '\0';
    file->data = buf;
    file->size = len;
    OPENF_DBG_PRINT("openf_init_file: allocated %zu bytes", len);
    return OPENF_OK;
}

/* Free OpenF_File */
static inline void openf_free_file(OpenF_File* file) {
    if (!file) return;
    if (file->data) {
        free(file->data);
        OPENF_DBG_PRINT("openf_free_file: freed %zu bytes", file->size);
        file->data = NULL;
        file->size = 0;
    }
}

/*-----------------------------------
  String duplication utility
------------------------------------*/

static inline OpenF_Error openf_strdup(const char* src, char** out_dup) {
    if (!src || !out_dup) return OPENF_ERR_NULL_ARG;
    size_t len = strlen(src);
    char* dup = (char*)malloc(len + 1);
    if (!dup) return OPENF_ERR_MEM_ALLOC;
    memcpy(dup, src, len);
    dup[len] = '\0';
    *out_dup = dup;
    OPENF_DBG_PRINT("openf_strdup: duplicated string '%s'", src);
    return OPENF_OK;
}

/*-----------------------------------
  File operations with double checks
------------------------------------*/

/* Read entire file into allocated buffer */
static inline OpenF_Error openf_read(const char* path, OpenF_File* out_file) {
    if (!path || !out_file) return OPENF_ERR_NULL_ARG;

    FILE* f = fopen(path, "rb");
    if (!f) {
        OPENF_DBG_PRINT("openf_read: failed to open '%s'", path);
        return OPENF_ERR_OPEN_FAILED;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return OPENF_ERR_SEEK_FAILED;
    }

    long filesize = ftell(f);
    if (filesize < 0) {
        fclose(f);
        return OPENF_ERR_SEEK_FAILED;
    }

    rewind(f);

    char* buffer = (char*)malloc(filesize + 1);
    if (!buffer) {
        fclose(f);
        return OPENF_ERR_MEM_ALLOC;
    }

    size_t read_bytes = fread(buffer, 1, filesize, f);
    if (read_bytes != (size_t)filesize) {
        free(buffer);
        fclose(f);
        return OPENF_ERR_READ_FAILED;
    }

    if (fclose(f) != 0) {
        free(buffer);
        return OPENF_ERR_CLOSE_FAILED;
    }

    buffer[filesize] = '\0';

    out_file->data = buffer;
    out_file->size = filesize;

    OPENF_DBG_PRINT("openf_read: read %zu bytes from '%s'", filesize, path);

    return OPENF_OK;
}

/* Write raw data to file, overwrite if exists */
static inline OpenF_Error openf_write(const char* path, const char* data, size_t size) {
    if (!path || !data) return OPENF_ERR_NULL_ARG;

    FILE* f = fopen(path, "wb");
    if (!f) {
        OPENF_DBG_PRINT("openf_write: failed to open '%s' for writing", path);
        return OPENF_ERR_OPEN_FAILED;
    }

    size_t written = fwrite(data, 1, size, f);
    if (written != size) {
        fclose(f);
        OPENF_DBG_PRINT("openf_write: write size mismatch for '%s'", path);
        return OPENF_ERR_WRITE_FAILED;
    }

    if (fclose(f) != 0) {
        return OPENF_ERR_CLOSE_FAILED;
    }

    OPENF_DBG_PRINT("openf_write: wrote %zu bytes to '%s'", size, path);

    return OPENF_OK;
}

/* Append null-terminated text to file */
static inline OpenF_Error openf_append_text(const char* path, const char* text) {
    if (!path || !text) return OPENF_ERR_NULL_ARG;

    FILE* f = fopen(path, "ab");
    if (!f) {
        OPENF_DBG_PRINT("openf_append_text: failed to open '%s' for append", path);
        return OPENF_ERR_OPEN_FAILED;
    }

    size_t len = strlen(text);
    size_t written = fwrite(text, 1, len, f);
    if (written != len) {
        fclose(f);
        return OPENF_ERR_WRITE_FAILED;
    }

    if (fclose(f) != 0) {
        return OPENF_ERR_CLOSE_FAILED;
    }

    OPENF_DBG_PRINT("openf_append_text: appended %zu bytes to '%s'", len, path);

    return OPENF_OK;
}

/* Check if file exists */
static inline int openf_exists(const char* path) {
    if (!path) return 0;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* Get file size */
static inline OpenF_Error openf_get_size(const char* path, size_t* out_size) {
    if (!path || !out_size) return OPENF_ERR_NULL_ARG;

    FILE* f = fopen(path, "rb");
    if (!f) return OPENF_ERR_FILE_NOT_FOUND;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return OPENF_ERR_SEEK_FAILED;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return OPENF_ERR_SEEK_FAILED;
    }

    *out_size = (size_t)size;

    if (fclose(f) != 0) {
        return OPENF_ERR_CLOSE_FAILED;
    }

    OPENF_DBG_PRINT("openf_get_size: '%s' size is %zu bytes", path, *out_size);

    return OPENF_OK;
}

/* Copy file */
static inline OpenF_Error openf_copy_file(const char* src, const char* dest) {
    if (!src || !dest) return OPENF_ERR_NULL_ARG;

    FILE* fsrc = fopen(src, "rb");
    if (!fsrc) return OPENF_ERR_FILE_NOT_FOUND;

    FILE* fdest = fopen(dest, "wb");
    if (!fdest) {
        fclose(fsrc);
        return OPENF_ERR_OPEN_FAILED;
    }

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        size_t written = fwrite(buffer, 1, bytes, fdest);
        if (written != bytes) {
            fclose(fsrc);
            fclose(fdest);
            return OPENF_ERR_WRITE_FAILED;
        }
    }

    if (ferror(fsrc)) {
        fclose(fsrc);
        fclose(fdest);
        return OPENF_ERR_READ_FAILED;
    }

    fclose(fsrc);
    if (fclose(fdest) != 0) return OPENF_ERR_CLOSE_FAILED;

    OPENF_DBG_PRINT("openf_copy_file: copied '%s' to '%s'", src, dest);

    return OPENF_OK;
}

/* Merge two files (concatenate) */
static inline OpenF_Error openf_merge_files(const char* out, const char* a, const char* b) {
    if (!out || !a || !b) return OPENF_ERR_NULL_ARG;

    FILE* fout = fopen(out, "wb");
    if (!fout) return OPENF_ERR_OPEN_FAILED;

    const char* files[2] = {a, b};
    for (int i = 0; i < 2; i++) {
        FILE* fin = fopen(files[i], "rb");
        if (!fin) {
            fclose(fout);
            return OPENF_ERR_FILE_NOT_FOUND;
        }

        char buffer[8192];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
            size_t written = fwrite(buffer, 1, bytes, fout);
            if (written != bytes) {
                fclose(fin);
                fclose(fout);
                return OPENF_ERR_WRITE_FAILED;
            }
        }
        fclose(fin);

        if (ferror(fin)) {
            fclose(fout);
            return OPENF_ERR_READ_FAILED;
        }
    }

    if (fclose(fout) != 0) return OPENF_ERR_CLOSE_FAILED;

    OPENF_DBG_PRINT("openf_merge_files: merged '%s' and '%s' into '%s'", a, b, out);

    return OPENF_OK;
}

/*-----------------------------------
  BMP 24-bit image support
------------------------------------*/

#pragma pack(push,1)
typedef struct {
    unsigned short bfType;      // 'BM'
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} OpenF_BMPFileHeader;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} OpenF_BMPInfoHeader;
#pragma pack(pop)

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char* pixels; // RGB 24-bit, row-major bottom-up
} OpenF_Image;

/* Load uncompressed 24-bit BMP */
static inline OpenF_Error openf_load_bmp(const char* path, OpenF_Image** out_image) {
    if (!path || !out_image) return OPENF_ERR_NULL_ARG;

    FILE* f = fopen(path, "rb");
    if (!f) return OPENF_ERR_FILE_NOT_FOUND;

    OpenF_BMPFileHeader file_header;
    if (fread(&file_header, sizeof(file_header), 1, f) != 1) {
        fclose(f);
        return OPENF_ERR_READ_FAILED;
    }

    if (file_header.bfType != 0x4D42) { // 'BM'
        fclose(f);
        return OPENF_ERR_INVALID_FORMAT;
    }

    OpenF_BMPInfoHeader info_header;
    if (fread(&info_header, sizeof(info_header), 1, f) != 1) {
        fclose(f);
        return OPENF_ERR_READ_FAILED;
    }

    if (info_header.biBitCount != 24 || info_header.biCompression != 0) {
        fclose(f);
        return OPENF_ERR_UNSUPPORTED;
    }

    if (info_header.biWidth <= 0 || info_header.biHeight == 0) {
        fclose(f);
        return OPENF_ERR_INVALID_FORMAT;
    }

    unsigned int width = (unsigned int)info_header.biWidth;
    unsigned int height = (unsigned int)(info_header.biHeight < 0 ? -info_header.biHeight : info_header.biHeight);

    size_t row_size = ((width * 3 + 3) / 4) * 4;
    size_t pixel_data_size = row_size * height;

    unsigned char* pixels = (unsigned char*)malloc(width * height * 3);
    if (!pixels) {
        fclose(f);
        return OPENF_ERR_MEM_ALLOC;
    }

    if (fseek(f, file_header.bfOffBits, SEEK_SET) != 0) {
        free(pixels);
        fclose(f);
        return OPENF_ERR_SEEK_FAILED;
    }

    unsigned char* row_data = (unsigned char*)malloc(row_size);
    if (!row_data) {
        free(pixels);
        fclose(f);
        return OPENF_ERR_MEM_ALLOC;
    }

    int top_down = info_header.biHeight < 0 ? 1 : 0;

    for (unsigned int y = 0; y < height; y++) {
        if (fread(row_data, 1, row_size, f) != row_size) {
            free(row_data);
            free(pixels);
            fclose(f);
            return OPENF_ERR_READ_FAILED;
        }
        unsigned int target_y = top_down ? y : (height - 1 - y);
        for (unsigned int x = 0; x < width; x++) {
            unsigned int dst_idx = (target_y * width + x) * 3;
            unsigned int src_idx = x * 3;
            // BMP stores as BGR, convert to RGB
            pixels[dst_idx + 0] = row_data[src_idx + 2];
            pixels[dst_idx + 1] = row_data[src_idx + 1];
            pixels[dst_idx + 2] = row_data[src_idx + 0];
        }
    }

    free(row_data);
    fclose(f);

    OpenF_Image* img = (OpenF_Image*)malloc(sizeof(OpenF_Image));
    if (!img) {
        free(pixels);
        return OPENF_ERR_MEM_ALLOC;
    }

    img->width = width;
    img->height = height;
    img->pixels = pixels;
    *out_image = img;

    OPENF_DBG_PRINT("openf_load_bmp: loaded '%s' %ux%u pixels", path, width, height);

    return OPENF_OK;
}

/* Save RGB 24-bit BMP (uncompressed) */
static inline OpenF_Error openf_save_bmp(const char* path, const OpenF_Image* image) {
    if (!path || !image || !image->pixels) return OPENF_ERR_NULL_ARG;

    FILE* f = fopen(path, "wb");
    if (!f) return OPENF_ERR_OPEN_FAILED;

    unsigned int width = image->width;
    unsigned int height = image->height;

    size_t row_size = ((width * 3 + 3) / 4) * 4;
    size_t pixel_data_size = row_size * height;
    size_t file_size = sizeof(OpenF_BMPFileHeader) + sizeof(OpenF_BMPInfoHeader) + pixel_data_size;

    OpenF_BMPFileHeader file_header = {0};
    file_header.bfType = 0x4D42; // 'BM'
    file_header.bfSize = (unsigned int)file_size;
    file_header.bfOffBits = sizeof(OpenF_BMPFileHeader) + sizeof(OpenF_BMPInfoHeader);

    OpenF_BMPInfoHeader info_header = {0};
    info_header.biSize = sizeof(OpenF_BMPInfoHeader);
    info_header.biWidth = (int)width;
    info_header.biHeight = (int)height;
    info_header.biPlanes = 1;
    info_header.biBitCount = 24;
    info_header.biCompression = 0;
    info_header.biSizeImage = (unsigned int)pixel_data_size;
    info_header.biXPelsPerMeter = 0;
    info_header.biYPelsPerMeter = 0;

    if (fwrite(&file_header, sizeof(file_header), 1, f) != 1) {
        fclose(f);
        return OPENF_ERR_WRITE_FAILED;
    }

    if (fwrite(&info_header, sizeof(info_header), 1, f) != 1) {
        fclose(f);
        return OPENF_ERR_WRITE_FAILED;
    }

    unsigned char* row_data = (unsigned char*)malloc(row_size);
    if (!row_data) {
        fclose(f);
        return OPENF_ERR_MEM_ALLOC;
    }

    for (unsigned int y = 0; y < height; y++) {
        memset(row_data, 0, row_size);
        unsigned int src_y = height - 1 - y;
        for (unsigned int x = 0; x < width; x++) {
            size_t dst_idx = x * 3;
            size_t src_idx = (src_y * width + x) * 3;
            // Convert RGB to BGR for BMP
            row_data[dst_idx + 0] = image->pixels[src_idx + 2];
            row_data[dst_idx + 1] = image->pixels[src_idx + 1];
            row_data[dst_idx + 2] = image->pixels[src_idx + 0];
        }
        if (fwrite(row_data, 1, row_size, f) != row_size) {
            free(row_data);
            fclose(f);
            return OPENF_ERR_WRITE_FAILED;
        }
    }

    free(row_data);

    if (fclose(f) != 0) return OPENF_ERR_CLOSE_FAILED;

    OPENF_DBG_PRINT("openf_save_bmp: saved '%s' %ux%u pixels", path, width, height);

    return OPENF_OK;
}

/* Free image struct */
static inline void openf_free_image(OpenF_Image** image) {
    if (!image || !*image) return;
    if ((*image)->pixels) free((*image)->pixels);
    free(*image);
    *image = NULL;
    OPENF_DBG_PRINT("openf_free_image: image freed");
}

/*-----------------------------------
  Initialization and Cleanup (dummy for extensibility)
------------------------------------*/

static inline OpenF_Error openf_init(void) {
    OPENF_DBG_PRINT("openf_init: called");
    /* For now no real global init, placeholder */
    return OPENF_OK;
}

static inline void openf_cleanup(void) {
    OPENF_DBG_PRINT("openf_cleanup: called");
    /* For now no real global cleanup, placeholder */
}

/*-----------------------------------
  Internal debug assert macro (optional)
------------------------------------*/
#if OPENF_DEBUG
#define OPENF_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "[openf ASSERT] %s failed at %s:%d\n", msg, __FILE__, __LINE__); \
        assert(cond); \
    } \
} while(0)
#else
#define OPENF_ASSERT(cond, msg) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif // OPENF_H
