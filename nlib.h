/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2018 Naman Dixit
 * SPDX-License-Identifier: 0BSD
 * Version: 54
 */

#if !defined(NLIB_H_INCLUDE_GUARD)

/* COMMON ===================================================================== */

/* Platform Identification ---------------------------------------------------- */

# if defined(_MSC_VER)
#  if defined(__clang__)
#   define COMPILER_CLANG
#   define COMPILER_CLANG_WITH_MSVC
#  else
#   define COMPILER_MSVC
#  endif
# elif defined (__GNUC__)
#  if defined(__clang__)
#   define COMPILER_CLANG
#   define COMPILER_CLANG_WITH_GCC
#  else
#   define COMPILER_GCC
#  endif
# elif defined(__clang__)
#  define COMPILER_CLANG
# else
#  error Compiler not supported
# endif

# if defined(_WIN32)
#  define OS_WINDOWS
# elif defined(__linux__)
#  define OS_LINUX
# else
#  error Operating system not supported
# endif

# if defined(COMPILER_MSVC) || defined(COMPILER_CLANG_WITH_MSVC)
#  if defined(_M_IX86)
#   define ARCH_X86
#  elif defined(_M_X64)
#   define ARCH_X64
#  endif
# elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  if defined(__i386__)
#   define ARCH_X86
#  elif defined(__x86_64__)
#   define ARCH_X64
#  endif
# endif

# if !defined(ARCH_X64)  // && !defined(ARCH_X86)
#  error Architecture not supported
# endif

# if defined(ARCH_X86)
#  define BITWIDTH_32
# elif defined(ARCH_X64)
#  define BITWIDTH_64
# else
#  error "Bitwidth not supported"
# endif

// TODO(naman): Check the state of C11 support for MSVC, and update this properly
# if defined(COMPILER_MSVC)
#  if defined(__cplusplus)
#   if __cplusplus == 199711L
#    define LANG_CPP 1998
#   elif __cplusplus == 201402L
#    define LANG_CPP 2014
#   elif __cplusplus == 201703L
#    define LANG_CPP 2017
#   else
#    define LANG_CPP 1998 // A future C++ or bad compiler, just assume C++98 for safety
#   endif
#  elif defined(__STDC_VERSION__)
#   if (__STDC_VERSION__ == 201112) || (__STDC_VERSION__ == 201710)
#    define LANG_C 2011
#   else
#    define LANG_C 2011 // Earliest C version for which MSVC supports __STDC_VERSION__
#   endif
#  elif defined(__STDC__)
#   define LANG_C 1989
#  endif
# elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  if defined(__cplusplus)
#   if __cplusplus == 199711L
#    define LANG_CPP 1998
#   elif __cplusplus == 201103L
#    define LANG_CPP 2011
#   elif __cplusplus == 201402L
#    define LANG_CPP 2014
#   elif __cplusplus == 201703L
#    define LANG_CPP 2017
#   else
#    define LANG_CPP 1998 // A future C++ or bad compiler, just assume C++98 for safety
#   endif
#  elif defined(__STDC_VERSION__) // Using C Language >= 1994 (1989)
#   if (__STDC_VERSION__ == 199409)
#    define LANG_C 1989
#   elif (__STDC_VERSION__ == 199901)
#    define LANG_C 1999
#   elif (__STDC_VERSION__ == 201112) || (__STDC_VERSION__ == 201710)
#    define LANG_C 2011
#   else
#    define LANG_C 1999 // C89 is too primitive to fallback on (C94 is accounted for above)
#   endif
#  elif defined(__STDC__) && !defined(__STDC_VERSION__)
#    define LANG_C 1989 // Since C89 doesn't require definition of __STDC_VERSION__
#  endif
# endif

# if !defined(LANG_C) && !defined(LANG_CPP)
#  error Language not supported
# endif

# if defined(OS_WINDOWS)
#  if defined(ARCH_X86) || defined(ARCH_X64)
#   define ENDIAN_LITTLE
#  else
#   error Could not determine endianness on Windows
#  endif
# elif defined(OS_LINUX)
#  include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ENDIAN_LITTLE
#  elif __BYTE_ORDER == __BIG_ENDIAN
#   define ENDIAN_BIG
#  else
#   error Could not determine endianness on Linux
#  endif
# else
#   error Can not determine endianness, unknown environment
# endif

/* Standard C Headers ----------------------------------------------------- */

// NOTE(naman): Define NLIB_NOLIBC_WINDOWS to prevent the inclusion of libc functions.
# if defined(NLIB_NOLIBC_WINDOWS) && !defined(OS_WINDOWS)
#  error NLIB_NOLIBC_WINDOWS only works on Windows
# endif

# if defined(COMPILER_MSVC)
#  pragma warning(push)
#   pragma warning(disable:4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
# endif

# include <stddef.h>

# if defined(COMPILER_MSVC)
#  pragma warning(pop)
# endif

# include <stdint.h>
# include <inttypes.h>
# include <limits.h>
# include <stdarg.h>
# include <stdnoreturn.h>
# include <float.h>

# if defined(NLIB_NOLIBC_WINDOWS)
#  define NLIB_NOLIBC
# endif

# if !defined(NLIB_NOLIBC)
#  include <stdio.h>
#  include <math.h>
#  include <stdlib.h>
#  include <errno.h>
#  include <string.h>
#  include <assert.h>

#  if (defined(LANG_C) && LANG_C >= 2011) || (defined(LANG_CPP) && LANG_CPP >= 2011)
#   include <stdalign.h>
#  endif

#  if !defined(__STDC_NO_ATOMICS__)
#   include <stdatomic.h>
#  endif

#  if !defined(__STDC_NO_THREADS__)
#   include <threads.h>
#  endif

#  if !defined(__STDC_NO_COMPLEX__)
// #   include <complex.h> UNCOMMENT WHEN NEEDED
#  endif
# endif // !defined(NLIB_NOLIBC)

/* Cross-platform Headers ------------------------------------------------------ */

# include <emmintrin.h> // SSE2 + SSE + MMX (Pentium 4, Athlon 64)

/* Platform dependent Headers -------------------------------------------------- */

# if defined(OS_WINDOWS)
#  if defined(COMPILER_MSVC)
#   pragma warning(push)
#    pragma warning(disable:4255)
#    pragma warning(disable:4668)
#  endif
#  include <Windows.h>
#  if defined(COMPILER_MSVC)
#   pragma warning(pop)
#  endif
# elif defined(OS_LINUX)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/mman.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <pthread.h>
#  include <semaphore.h>
# endif

/* Compiler Extensions --------------------------------------------------------- */

# if defined(COMPILER_MSVC)
#  define fallthrough
# elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  define fallthrough __attribute__((fallthrough))
# endif


/* ===============
 * Primitive Types
 */

typedef int8_t               S8;
typedef int16_t              S16;
typedef int32_t              S32;
typedef int64_t              S64;
typedef int                  Sint;

typedef uint8_t              U8;
typedef uint16_t             U16;
typedef uint32_t             U32;
typedef uint64_t             U64;
typedef unsigned             Uint;

typedef size_t               Size;

typedef uintptr_t            Uptr;
typedef intptr_t             Sptr;
typedef ptrdiff_t            Dptr;

typedef float                F32;
typedef double               F64;

typedef U8                   B8;
typedef U16                  B16;
typedef U32                  B32;
typedef U64                  B64;
# if defined(LANG_C)
#  define true                1U
#  define false               0U
# endif
typedef unsigned char        Byte;
typedef char                 Char;


/* ========================
 * Preprocessor Definitions
 */

# define elemin(array) (sizeof(array)/sizeof((array)[0]))
# define containerof(ptr, type, member)                         \
    ((type*)(void*)(((Byte*)(ptr)) - offsetof(type, member)))

# define isOdd(x)  ((x) % 2 ? true : false)
# define isEven(x) (!isOdd(x))

# define KiB(x) (   (x) * 1024ULL)
# define MiB(x) (KiB(x) * 1024ULL)
# define GiB(x) (MiB(x) * 1024ULL)
# define TiB(x) (GiB(x) * 1024ULL)

# define THOUSAND 1000L
# define MILLION  1000000L
# define BILLION  1000000000L

# define unused_variable(var) (void)var

# define global_variable   static
# define global_immutable  static const
# define persistent_value  static

# define internal_function static
# define header_function   static inline

# if defined(LANG_C)
#  define NLIB_COMPAT_NULL NULL
# else
#  define NLIB_COMPAT_NULL nullptr
# endif


/* =======================
 * @Unicode
 */
# if !defined(NLIB_EXCLUDE_UNICODE)

header_function
B64 unicodeCodepointFromUTF16Surrogate (U16 surrogate, U16 *prev_surrogate, U32 *codepoint)
{
    U16 utf16_hi_surrogate_start = 0xD800;
    U16 utf16_lo_surrogate_start = 0xDC00;
    U16 utf16_surrogate_end = 0xDFFF;

    if ((surrogate >= utf16_hi_surrogate_start) &&
        (surrogate < utf16_lo_surrogate_start)) {
        *prev_surrogate = surrogate;

        return false;
    } else {
        if ((surrogate >= utf16_lo_surrogate_start) &&
            (surrogate <= utf16_surrogate_end)) {
            U16 low_surrogate = surrogate;
            // NOTE(naman): In this line, the numbers get promoted from U16 to S32,
            // so storing them in a U32 results in a inmpicit sign conversion.
            // That is why we are casting manually.
            *codepoint = (U32)((*prev_surrogate - utf16_hi_surrogate_start) << 10U);
            *codepoint |= (low_surrogate - utf16_lo_surrogate_start);
            *codepoint += 0x10000;

            *prev_surrogate = 0;
        } else {
            *codepoint = surrogate;
        }

        return true;
    }
}

// FIXME(naman): Change this to take buffer_size as input, similar to printStringVarArg
// Also, return number of bytes excluding the null byte
header_function
Size unicodeUTF8FromUTF32 (U32 *codepoints, Size codepoint_count, Char *buffer)
{
    if (buffer == NLIB_COMPAT_NULL) {
        Size length = 1; // NOTE(naman): We need one byte for the NUL byte.

        for (Size i = 0; i < codepoint_count; i++) {
            if (codepoints[i] <= 0x7F) {
                length += 1;
            } else if (codepoints[i] <= 0x7FF) {
                length += 2;
            } else if (codepoints[i] <= 0xFFFF) {
                length += 3;
            } else if (codepoints[i] <= 0x10FFFF) {
                length += 4;
            }
        }

        return length;
    } else {
        Size length = 1; // NOTE(naman): We need one byte for the NUL byte.

        for (Size i = 0; i < codepoint_count; i++) {
            if (codepoints[i] <= 0x7F) {
                buffer[0] = (Char)codepoints[i];
                buffer += 1;
                length += 1;
            } else if (codepoints[i] <= 0x7FF) {
                buffer[0] = (Char)(0xC0 | (codepoints[i] >> 6));            /* 110xxxxx */
                buffer[1] = (Char)(0x80 | (codepoints[i] & 0x3F));          /* 10xxxxxx */
                buffer += 2;
                length += 2;
            } else if (codepoints[i] <= 0xFFFF) {
                buffer[0] = (Char)(0xE0 | (codepoints[i] >> 12));           /* 1110xxxx */
                buffer[1] = (Char)(0x80 | ((codepoints[i] >> 6) & 0x3F));   /* 10xxxxxx */
                buffer[2] = (Char)(0x80 | (codepoints[i] & 0x3F));          /* 10xxxxxx */
                buffer += 3;
                length += 3;
            } else if (codepoints[i] <= 0x10FFFF) {
                buffer[0] = (Char)(0xF0 | (codepoints[i] >> 18));           /* 11110xxx */
                buffer[1] = (Char)(0x80 | ((codepoints[i] >> 12) & 0x3F));  /* 10xxxxxx */
                buffer[2] = (Char)(0x80 | ((codepoints[i] >> 6) & 0x3F));   /* 10xxxxxx */
                buffer[3] = (Char)(0x80 | (codepoints[i] & 0x3F));          /* 10xxxxxx */
                buffer += 4;
                length += 4;
            }
        }

        buffer[0] = '\0';

        return length;
    }
}

#  if defined(OS_WINDOWS)

// FIXME(naman): Replace VirtualAlloc/VirtualFree here by memory allocator
header_function
LPWSTR unicodeWin32UTF16FromUTF8 (Char *utf8)
{
    int wcstr_length = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NLIB_COMPAT_NULL, 0);
    LPWSTR wcstr = VirtualAlloc(NLIB_COMPAT_NULL, (DWORD)wcstr_length * sizeof(wchar_t),
                                MEM_COMMIT, PAGE_READWRITE);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wcstr, wcstr_length);

    int normalized_length = NormalizeString(NormalizationC,
                                            wcstr, -1,
                                            NLIB_COMPAT_NULL, 0);
    LPWSTR norm = VirtualAlloc(NLIB_COMPAT_NULL, (DWORD)normalized_length * sizeof(wchar_t),
                               MEM_COMMIT, PAGE_READWRITE);
    NormalizeString(NormalizationC, wcstr, -1, norm, normalized_length);

    VirtualFree(wcstr, 0, MEM_RELEASE);

    return norm;
}

header_function
void unicodeWin32UTF16Dealloc (LPWSTR utf16)
{
    VirtualFree(utf16, 0, MEM_RELEASE);
}

#  endif // OS_WINDOWS

# endif // NLIB_EXCLUDE_UNICODE

/* ==================
 * Print interface partially predeclared
 * (because C can't handle out-of-order declarations)
 */

#  if defined(OS_WINDOWS)
#   if defined(BUILD_DEBUG)
#    define report(...)              printDebugOutput(__VA_ARGS__)
#    define reportv(format, va_list) printDebugOutputV(format, va_list)
#   else // = if !defined(BUILD_DEBUG)
#    define report(...)              err(stderr, __VA_ARGS__)
#    define reportv(format, va_list) errv(stderr, format, va_list)
#   endif // defined(BUILD_DEBUG)
#  elif defined(OS_LINUX)
#   if defined(BUILD_DEBUG)
#    define report(...)              err(__VA_ARGS__)
#    define reportv(format, va_list) errv(format, va_list)
#   else // = if !defined(BUILD_DEBUG)
#    define report(...)              err(__VA_ARGS__)
#    define reportv(format, va_list) errv(format, va_list)
#   endif // defined(BUILD_DEBUG)
#  endif // defined(OS_WINDOWS)
header_function Size err (Char const *format, ...);
header_function Size errv (Char const *format, va_list ap);
header_function Size printDebugOutputV (Char const *format, va_list ap);
header_function Size printDebugOutput (Char const *format, ...);

/* ====================
 * @Debug
 */

# if !defined(NLIB_EXCLUDE_DEBUG)

#  if defined(OS_WINDOWS)
#   if defined(BUILD_DEBUG)
#    define breakpoint() __debugbreak()
#    define quit() breakpoint()
#   else
#    define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#    define quit() ExitProcess(0)
#   endif
#  else
#   if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#    if defined(ARCH_X86) || defined(ARCH_X64)
#     if defined(BUILD_DEBUG)
#      define breakpoint() __asm__ volatile("int $0x03")
#      define quit() breakpoint()
#     else
#      define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#      define quit() exit(0)
#     endif // BUILD_DEBUG
#    endif // Architecture/OS
#   endif // Compiler
#  endif // Operating Systems

# endif // NLIB_EXCLUDE_DEBUG


/* ==============
 * @Claim (assert)
 */

# if !defined(NLIB_EXCLUDE_CLAIM)

#  if defined(BUILD_DEBUG)
#   define claim(cond) claim_(cond, #cond, __FILE__, __LINE__)
header_function
void claim_ (B32 cond,
             Char const *cond_str,
             Char const *filename, U32 line_num)
{
    if (!cond) {
        report("Claim \"%s\" Failed in %s:%u\n\n",
               cond_str, filename, line_num);

        quit();
    }
}
#  else
#   define claim(cond) do { cond; } while (0)
#  endif

# endif // NLIB_EXCLUDE_CLAIM


/* =======================
 * @Printing
 */

# if !defined(NLIB_EXCLUDE_PRINT)

#  if !defined(NLIB_PRINT_MALLOC)
#   if defined(NLIB_MALLOC)
#    define NLIB_PRINT_MALLOC NLIB_MALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Print: NLIB_PRINT_MALLOC required with nolibc"
#   else
#    define NLIB_PRINT_MALLOC malloc
#   endif
#  endif

#  if !defined(NLIB_PRINT_FREE)
#   if defined(NLIB_FREE)
#    define NLIB_PRINT_FREE NLIB_FREE
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Print: NLIB_PRINT_FREE required with nolibc"
#   else
#    define NLIB_PRINT_FREE free
#   endif
#  endif

#  if                                           \
    !defined(NLIB_PRINT_RYU_FLOAT) &&           \
    !defined(NLIB_PRINT_STB_FLOAT) &&           \
    !defined(NLIB_PRINT_BAD_FLOAT) &&           \
    !defined(NLIB_PRINT_NO_FLOAT)
#   define NLIB_PRINT_STB_FLOAT
#  endif

#  if defined(NLIB_PRINT_RYU_FLOAT)
#   include "nlib_print_ryu.h"
#  endif // defined(NLIB_PRINT_RYU_FLOAT)

#  if defined(NLIB_PRINT_STB_FLOAT)
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpadded"
#   endif

global_immutable
struct
{
    S16 temp; // force next field to be 2-byte aligned
    Char pair[201];
} stbfp_digitpair = { 0,
    "00010203040506070809101112131415161718192021222324"
    "25262728293031323334353637383940414243444546474849"
    "50515253545556575859606162636465666768697071727374"
    "75767778798081828384858687888990919293949596979899" };

#   define STBFP_SPECIAL 0x7000

global_immutable
F64 stbfp_bot[23] = {
    1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009,
    1e+010, 1e+011, 1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019,
    1e+020, 1e+021, 1e+022
};

global_immutable
F64 stbfp_negbot[22] = {
    1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
    1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};

global_immutable
F64 stbfp_negboterr[22] = {
    -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020,
    -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
    4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026,
    -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
    -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032,
    2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
    2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,
    -4.8596774326570872e-039
};

global_immutable
F64 stbfp_top[13] = {
    1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230,
    1e+253, 1e+276, 1e+299
};

global_immutable
F64 stbfp_negtop[13] = {
    1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230,
    1e-253, 1e-276, 1e-299
};

global_immutable
F64  stbfp_toperr[13] = {
    8388608,
    6.8601809640529717e+028,
    -7.253143638152921e+052,
    -4.3377296974619174e+075,
    -1.5559416129466825e+098,
    -3.2841562489204913e+121,
    -3.7745893248228135e+144,
    -1.7356668416969134e+167,
    -3.8893577551088374e+190,
    -9.9566444326005119e+213,
    6.3641293062232429e+236,
    -5.2069140800249813e+259,
    -5.2504760255204387e+282
};

global_immutable
F64 stbfp_negtoperr[13] = {
    3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
    -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
    7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
    8.0970921678014997e-317
};

global_immutable
U64 stbfp_powten[20] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL,
    10000000000000000000ULL
};
#   define stbfp_tento19th (1000000000000000000ULL)

#   define stbfp_ddmulthi(oh, ol, xh, yh) do {                          \
        F64 ahi = 0, alo, bhi = 0, blo;                                 \
        S64 bt;                                                         \
        oh = xh * yh;                                                   \
        memcpy(&bt, &xh, 8);                                            \
        bt &= ((~(U64)0) << 27);                                        \
        memcpy(&ahi, &bt, 8);                                           \
        alo = xh - ahi;                                                 \
        memcpy(&bt, &yh, 8);                                            \
        bt &= ((~(U64)0) << 27);                                        \
        memcpy(&bhi, &bt, 8);                                           \
        blo = yh - bhi;                                                 \
        ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo;    \
    } while (0)

#   define stbfp_ddtoS64(ob, xh, xl) do {          \
        F64 ahi = 0, alo, vh, t;                \
        ob = (S64)xh;                           \
        vh = (F64)ob;                           \
        ahi = (xh - vh);                        \
        t = (ahi - xh);                         \
        alo = (xh - (ahi - t)) - (vh + t);      \
        ob += (S64)(ahi + alo + xl);            \
    } while (0)

#   define stbfp_ddrenorm(oh, ol) do {             \
        F64 s;                                  \
        s = oh + ol;                            \
        ol = ol - (s - oh);                     \
        oh = s;                                 \
    } while (0)

#   define stbfp_ddmultlo(oh, ol, xh, xl, yh, yl) do { ol = ol + (xh * yl + xl * yh); } while (0)

#   define stbfp_ddmultlos(oh, ol, xh, yl) do { ol = ol + (xh * yl); } while (0)

header_function
void stbfp_raise_to_power10(F64 base, S32 exponent, // exponent can be -323 to +350
                            F64 *output_high, F64 *output_low)
{
    F64 ph, pl;
    if ((exponent >= 0) && (exponent <= 22)) {
        stbfp_ddmulthi(ph, pl, base, stbfp_bot[exponent]);
    } else {
        S32 e = exponent;
        if (exponent < 0) e = -e;
        S32 et = (e * 0x2c9) >> 14; /* floor( e / 23) */
        if (et > 13) et = 13; /* e=321->et=13, e=322->et=14 */
        S32 eb = e - (et * 23); /* e % 23 if 0 <= e <= 321 */

        ph = base;
        pl = 0.0;
        F64 p2h, p2l;
        if (exponent < 0) {
            if (eb) {
                --eb;
                stbfp_ddmulthi(ph, pl, base, stbfp_negbot[eb]);
                stbfp_ddmultlos(ph, pl, base, stbfp_negboterr[eb]);
            }

            if (et) {
                stbfp_ddrenorm(ph, pl);
                --et;
                stbfp_ddmulthi(p2h, p2l, ph, stbfp_negtop[et]);
                stbfp_ddmultlo(p2h, p2l, ph, pl, stbfp_negtop[et], stbfp_negtoperr[et]);
                ph = p2h;
                pl = p2l;
            }
        } else {
            if (eb) {
                e = eb;
                if (eb > 22)
                    eb = 22;
                e -= eb;
                stbfp_ddmulthi(ph, pl, base, stbfp_bot[eb]);

                if (e) {
                    stbfp_ddrenorm(ph, pl);
                    stbfp_ddmulthi(p2h, p2l, ph, stbfp_bot[e]);
                    stbfp_ddmultlos(p2h, p2l, stbfp_bot[e], pl);
                    ph = p2h;
                    pl = p2l;
                }
            }

            if (et) {
                stbfp_ddrenorm(ph, pl);
                --et;
                stbfp_ddmulthi(p2h, p2l, ph, stbfp_top[et]);
                stbfp_ddmultlo(p2h, p2l, ph, pl, stbfp_top[et], stbfp_toperr[et]);
                ph = p2h;
                pl = p2l;
            }
        }
    }

    stbfp_ddrenorm(ph, pl);
    *output_high = ph;
    *output_low = pl;
}

#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif
#  endif // defined(NLIB_PRINT_STB_FLOAT)

typedef enum Print_Flags {
    Print_Flags_LEFT_JUSTIFIED     = 1u << 0,
    Print_Flags_ALTERNATE_FORM     = 1u << 1,
    Print_Flags_LEADING_PLUS       = 1u << 2,
    Print_Flags_LEADING_SPACE      = 1u << 3,
    Print_Flags_LEADING_ZERO       = 1u << 4,
    Print_Flags_INT8               = 1u << 5,
    Print_Flags_INT16              = 1u << 6,
    Print_Flags_INT64              = 1u << 7,
    Print_Flags_NEGATIVE           = 1u << 8,
    Print_Flags_FLOAT_FIXED        = 1u << 9,
    Print_Flags_FLOAT_EXP          = 1u << 10,
    Print_Flags_FLOAT_HEX          = 1u << 11,
} Print_Flags;

// FIXME(naman): This needs to change to either yield or call a callback every n characters,
// so that functions using this don't have to call it twice (once to get size, then to
// get the actual characters).
header_function
Size printStringVarArg (Char *buffer, Size buffer_size, Char const *format, va_list va)
{
    Char const *fmt = format;
    Char *buf = buffer;
    Size needed_size = 0;
    B32 resume_output = true;

#  define IS_OUTPUT_RESUMABLE(arg__space)       \
    do {                                        \
        if (resume_output &&                    \
            ((buffer == NLIB_COMPAT_NULL) ||                \
             ((Uptr)(buf + (Uptr)arg__space) >= \
              (Uptr)(buffer + buffer_size)))) { \
            resume_output = false;              \
        }                                       \
    } while (0)

    while (true) {
        // Copy everything up to the next % (or end of string)
        while ((fmt[0] != '%') && (fmt[0] != '\0')) {
            IS_OUTPUT_RESUMABLE(1);
            if (resume_output) {
                buf[0] = fmt[0];
                buf++;
            }
            needed_size++;
            fmt++;
        }

        if (fmt[0] == '%') {
            Char const *format_pointer = fmt;

            fmt++;

            // read the modifiers first
            Sint field_width = 0;
            Sint precision = -1;
            Sint trailing_zeroes = 0;
            U32 flags = 0;

            // flags
            while (true) {
                switch (fmt[0]) {
                    case '-': { // if we have left justify
                        flags |= Print_Flags_LEFT_JUSTIFIED;
                        fmt++;
                        continue;
                    } break;
                    case '#': { // if we use alternate form
                        flags |= Print_Flags_ALTERNATE_FORM;
                        fmt++;
                        continue;
                    } break;
                    case '+': { // if we have leading plus
                        flags |= Print_Flags_LEADING_PLUS;
                        fmt++;
                        continue;
                    } break;
                    case ' ': { // if we have leading plus
                        flags |= Print_Flags_LEADING_SPACE;
                        fmt++;
                        continue;
                    } break;
                    case '0': { // if we have leading zero
                        flags |= Print_Flags_LEADING_ZERO;
                        fmt++;
                        goto flags_done;
                    } break;
                    default: {
                        goto flags_done;
                    } break;
                }
            }
        flags_done:

            // get the field width
            if (fmt[0] == '*') {
                field_width = va_arg(va, Sint);
                fmt++;
            } else {
                while ((fmt[0] >= '0') && (fmt[0] <= '9')) {
                    field_width = (field_width * 10) + (fmt[0] - '0');
                    fmt++;
                }
            }

            // get the precision
            if (fmt[0] == '.') {
                fmt++;
                if (fmt[0] == '*') {
                    precision = va_arg(va, Sint);
                    fmt++;
                } else {
                    precision = 0;
                    while ((fmt[0] >= '0') && (fmt[0] <= '9')) {
                        precision = (precision * 10) + (fmt[0] - '0');
                        fmt++;
                    }
                }
            }

            // handle integer size overrides
            switch (fmt[0]) {
                case 'h': { // are we 64-bit
                    flags |= Print_Flags_INT16;
                    fmt++;
                    if (fmt[0] == 'h') {
                        flags &= (U32)~Print_Flags_INT16;
                        flags |= Print_Flags_INT8;
                        fmt++;
                    }
                } break;
                case 'l': { // are we 64-bit
                    flags |= Print_Flags_INT64;
                    fmt++;
                } break;
                case 'z': { // are we 64-bit on size_t?
                    flags |= (sizeof(Size) == 8) ? Print_Flags_INT64 : 0;
                    fmt++;
                } break;
                case 't': { // are we 64-bit on ptrdiff_t?
                    flags |= (sizeof(Dptr) == 8) ? Print_Flags_INT64 : 0;
                    fmt++;
                } break;
                default: {
                } break;
            }

#  define PRINT_STR_SIZE 2048ULL
            Char num_str[PRINT_STR_SIZE];
            Char *str = NLIB_COMPAT_NULL;

            Char head_str[8] = {0};
            Size head_index = 0;

            Char tail_str[8] = {0};
            Size tail_index = 0;

            Size len = 0;

            switch (fmt[0]) { // handle each replacement
                case 's': { // string
                    // get the string
                    str = va_arg(va, Char*);
                    if (str == NLIB_COMPAT_NULL) {
                        str = "null";
                    }

                    // NOTE(naman): By this point, str is most definitely not NLIB_COMPAT_NULL
                    while (str[len] != '\0') {
                        len++;
                    }

                    // clamp to precision
                    // Since precision inits at -1, if none was mentioned, this will not execute
                    if (len > (Size)precision) {
                        len = (Size)precision;
                    }

                    precision = 0;
                } break;

                case 'c': { // char
                    // get the character
                    str = num_str + PRINT_STR_SIZE - 1;
                    *str = (Char)va_arg(va, Sint);
                    len = 1;
                    precision = 0;
                } break;

                case 'n': { // weird write-bytes specifier
                    Sint *d = va_arg(va, Sint*);
                    *d = (Sint)(buf - buffer);
                } break;

                case 'b': case 'B': { // binary
                    B32 upper = (fmt[0] == 'B') ? true : false;

                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    if (flags & Print_Flags_INT8) {
                        num_dec = (U8)num_dec;
                    } else if (flags & Print_Flags_INT16) {
                        num_dec = (U16)num_dec;
                    }

                    str = num_str + PRINT_STR_SIZE;

                    while (true) {
                        U64 n = num_dec & 0x1;
                        num_dec = num_dec >> 1;

                        str--;
                        *str = (n == 1) ? '1' : '0';

                        if ((num_dec != 0) || (((num_str + PRINT_STR_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_STR_SIZE) - (Uptr)str);

                    if (flags & Print_Flags_ALTERNATE_FORM) {
                        head_str[head_index++] = '0';
                        if (upper) {
                            head_str[head_index++] = 'B';
                        } else {
                            head_str[head_index++] = 'b';
                        }
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'o': case 'O': { // octal
                    B32 upper = (fmt[0] == 'O') ? true : false;

                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    if (flags & Print_Flags_INT8) {
                        num_dec = (U8)num_dec;
                    } else if (flags & Print_Flags_INT16) {
                        num_dec = (U16)num_dec;
                    }

                    str = num_str + PRINT_STR_SIZE;

                    while (true) {
                        U64 n = num_dec & 0x7;
                        num_dec = num_dec >> 3;

                        str--;
                        switch (n) {
                            case 0: *str = '0'; break;
                            case 1: *str = '1'; break;
                            case 2: *str = '2'; break;
                            case 3: *str = '3'; break;
                            case 4: *str = '4'; break;
                            case 5: *str = '5'; break;
                            case 6: *str = '6'; break;
                            case 7: *str = '7'; break;
                        }

                        if ((num_dec != 0) || (((num_str + PRINT_STR_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_STR_SIZE) - (Uptr)str);

                    if (flags & Print_Flags_ALTERNATE_FORM) {
                        head_str[head_index++] = '0';
#  if !defined(NLIB_TESTS)
                        if (upper) {
                            head_str[head_index++] = 'O';
                        } else {
                            head_str[head_index++] = 'o';
                        }
#  else
                        unused_variable(upper);
#  endif
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'X':
                case 'x': { // hex
                    B32 upper = (fmt[0] == 'X') ? true : false;

                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    if (flags & Print_Flags_INT8) {
                        num_dec = (U8)num_dec;
                    } else if (flags & Print_Flags_INT16) {
                        num_dec = (U16)num_dec;
                    }

                    str = num_str + PRINT_STR_SIZE;

                    if ((num == 0) && (precision == 0)) {
                        break;
                    }

                    while (true) {
                        U64 n = num_dec & 0xf;
                        num_dec = num_dec >> 4;

                        str--;
                        switch (n) {
                            case 0x0: *str = '0'; break;
                            case 0x1: *str = '1'; break;
                            case 0x2: *str = '2'; break;
                            case 0x3: *str = '3'; break;
                            case 0x4: *str = '4'; break;
                            case 0x5: *str = '5'; break;
                            case 0x6: *str = '6'; break;
                            case 0x7: *str = '7'; break;
                            case 0x8: *str = '8'; break;
                            case 0x9: *str = '9'; break;
                            case 0xA: *str = upper ? 'A' : 'a'; break;
                            case 0xB: *str = upper ? 'B' : 'b'; break;
                            case 0xC: *str = upper ? 'C' : 'c'; break;
                            case 0xD: *str = upper ? 'D' : 'd'; break;
                            case 0xE: *str = upper ? 'E' : 'e'; break;
                            case 0xF: *str = upper ? 'F' : 'f'; break;
                        }

                        if ((num_dec != 0) || (((num_str + PRINT_STR_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_STR_SIZE) - (Uptr)str);

                    if (flags & Print_Flags_ALTERNATE_FORM) {
                        head_str[head_index++] = '0';
                        if (upper) {
                            head_str[head_index++] = 'X';
                        } else {
                            head_str[head_index++] = 'x';
                        }
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'u':
                case 'd': { // integer
                    // get the integer and abs it
                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        S64 i64 = va_arg(va, S64);
                        num = (U64)i64;
                        if ((fmt[0] != 'u') && (i64 < 0)) {
                            num = (U64)-i64;
                            flags |= Print_Flags_NEGATIVE;
                        }
                    } else {
                        S32 i = va_arg(va, S32);
                        num = (U32)i;
                        if ((fmt[0] != 'u') && (i < 0)) {
                            num = (U32)-i;
                            flags |= Print_Flags_NEGATIVE;
                        }
                    }

                    // convert to string
                    U64 num_dec = num;
                    if (flags & Print_Flags_INT8) {
                        num_dec = (U8)num_dec;
                    } else if (flags & Print_Flags_INT16) {
                        num_dec = (U16)num_dec;
                    }

                    str = num_str + PRINT_STR_SIZE;

                    if ((num == 0) && (precision == 0)) {
                        break;
                    }

                    while (num_dec) {
                        str--;
                        *str = (Char)(num_dec % 10) + '0';
                        num_dec /= 10;
                    }

                    // get the length that we copied
                    len = (((Uptr)num_str + PRINT_STR_SIZE) - (Uptr)str);

                    if (len == 0) {
                        --str;
                        *str = '0';
                        len = 1;
                    }

                    if (flags & Print_Flags_NEGATIVE) {
                        head_str[head_index++] = '-';
                    }

                    if (flags & Print_Flags_LEADING_PLUS) {
                        head_str[head_index++] = '+';
                    }

                    if (flags & Print_Flags_LEADING_SPACE) {
                        if (!(flags & Print_Flags_NEGATIVE)) {
                            head_str[head_index++] = ' ';
                        }
                    }

                    if (flags & Print_Flags_LEADING_ZERO) {
                        head_str[head_index++] = '0';
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'p': { // pointer
                    flags |= (sizeof(void*) == 8) ? Print_Flags_INT64 : 0;
                    precision = sizeof(void*) * 2;

                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    str = num_str + PRINT_STR_SIZE;

                    while (true) {
                        U64 n = num_dec & 0xf;
                        num_dec = num_dec >> 4;

                        str--;
                        switch (n) {
                            case 0x0: *str = '0'; break;
                            case 0x1: *str = '1'; break;
                            case 0x2: *str = '2'; break;
                            case 0x3: *str = '3'; break;
                            case 0x4: *str = '4'; break;
                            case 0x5: *str = '5'; break;
                            case 0x6: *str = '6'; break;
                            case 0x7: *str = '7'; break;
                            case 0x8: *str = '8'; break;
                            case 0x9: *str = '9'; break;
                            case 0xA: *str = 'A'; break;
                            case 0xB: *str = 'B'; break;
                            case 0xC: *str = 'C'; break;
                            case 0xD: *str = 'D'; break;
                            case 0xE: *str = 'E'; break;
                            case 0xF: *str = 'F'; break;
                        }

                        if ((num_dec != 0) || (((num_str + PRINT_STR_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_STR_SIZE) - (Uptr)str);
                } break;

                case 'f': case 'F':
                case 'e': case 'E':
                case 'a': case 'A': {
                    switch (fmt[0]) {
                        case 'f': case 'F': { flags |= Print_Flags_FLOAT_FIXED;   } break;
                        case 'e': case 'E': { flags |= Print_Flags_FLOAT_EXP;     } break;
                        case 'a': case 'A': { flags |= Print_Flags_FLOAT_HEX;     } break;
                    }

                    B32 capital = false;
                    switch (fmt[0]) {
                        case 'F': case 'E': case 'A': { capital = true;  } break;
                    }

                    if ((precision < 0) && !(flags & Print_Flags_FLOAT_HEX)) precision = 6;

                    F64 f64 = va_arg(va, F64);
                    union FU64 { F64 f; U64 u;} fu64 = {.f = f64};
                    U64 bits = fu64.u;

                    /* NOTE(naman): 64-bit IEEE-754 Floating Point structure
                       ____________________________________________
                       |  Sign bit | Exponent bits | Mantissa bits|
                       |     1     |       11      |      52      |
                       --------------------------------------------
                    */

#  define            F64_MANTISSA_BITS 52
#  define            F64_EXPONENT_BITS 11
#  define            F64_BIAS 1023

                    // Is the top bit set?
                    B32 negative  = ((bits >> (F64_MANTISSA_BITS + F64_EXPONENT_BITS)) & 1) != 0;
                    // Remove all mantissa bits ------------------ and then reset the sign bit
                    U32 exponent_biased = (((U32)(bits >> F64_MANTISSA_BITS)) &
                                           ((1U << F64_EXPONENT_BITS) - 1U));
                    S32 exponent = (S32)exponent_biased - F64_BIAS;
                    // Reset all bits except for the mantissa bits
                    U64 mantissa  = bits & ((1ULL << F64_MANTISSA_BITS) - 1ULL);

                    if (negative) {
                        flags |= Print_Flags_NEGATIVE;
                    }

                    { // Leading String
                        if (flags & Print_Flags_NEGATIVE) {
                            head_str[head_index++] = '-';
                        } else if (flags & Print_Flags_LEADING_PLUS) {
                            head_str[head_index++] = '+';
                        } else if (flags & Print_Flags_LEADING_SPACE) {
                            if (!(flags & Print_Flags_NEGATIVE)) {
                                head_str[head_index++] = ' ';
                            }
                        }

                        if ((flags & Print_Flags_LEADING_ZERO) &&
                            !(flags & Print_Flags_FLOAT_HEX)) {
                            head_str[head_index++] = '0';
                        }
                    }

                    if (exponent_biased == 0x7FF) { // Handle NaN and Inf
                        if (capital) {
                            if (mantissa) {
                                str = "NAN";
                            } else {
                                str = "INF";
                            }
                        } else {
                            if (mantissa) {
                                str = "nan";
                            } else {
                                str = "inf";
                            }
                        }

                        len = 3;
                    } else if (exponent_biased == 0 && mantissa == 0) {
                        if (flags & Print_Flags_FLOAT_HEX) {
                            head_str[head_index++] = '0';
                            head_str[head_index++] = capital ? 'X' : 'x';
                            head_str[head_index++] = '0';
                        }

                        str = num_str;

                        *str++ = '0';

                        if (precision > 0) {
                            *str++ = '.';
                            trailing_zeroes += precision;
                        }

                        len = (Size)(Dptr)(str - num_str);
                        str = num_str;

                        if (flags & Print_Flags_FLOAT_EXP) {
                            tail_str[tail_index++] = capital ? 'E' : 'e';
                            tail_str[tail_index++] = '+';
                            tail_str[tail_index++] = '0';
                            tail_str[tail_index++] = '0';
                        } else if (flags & Print_Flags_FLOAT_HEX) {
                            tail_str[tail_index++] = capital ? 'P' : 'p';
                            tail_str[tail_index++] = '+';
                            tail_str[tail_index++] = '0';
                        }
                    }

                    if ((str == NLIB_COMPAT_NULL) && (flags & Print_Flags_FLOAT_HEX)) {
                        S32 ex = exponent;
                        B32 denormal = false;

                        if ((exponent_biased == 0) && (mantissa != 0)) { // Denormals
                            denormal = true;
                            ex = -1022;
                        }

                        head_str[head_index++] = '0';
                        head_str[head_index++] = capital ? 'X' : 'x';

                        if (mantissa || precision) {
                            U64 man = mantissa;
                            S32 pr = precision > 0 ? precision : 13;

                            if (denormal == false) {
                                // NOTE(naman): This sets the 53rd bit
                                man |= 1ULL << 52;
                            }

                            /* NOTE(naman): This makes it so that the MSB of mantissa is on
                             * 60th bit, and the normal/denormal bit (that we set above)
                             * is on 61st.
                             *
                             *    bitsInF64 - (bitsInHex * (MANTISSA_BITS/bitsInHex + 1))
                             * => 64 - (4 * (13 + 1))
                             * => 8
                             * (1 at the end comes to handle the 53rd bit)
                             */
                            man <<= 8;

                            // NOTE(naman): This will do the rounding if the precision is
                            // smaller. It does this by incrementing the MSB of mantissa
                            // that won't be printed. Doing that will send a carry forward
                            // if the hex associated with that MSB was >= 8; and not if it was
                            // < 7. Similarly, the carry will propagate further, rounding each
                            // hex to its nearest value, with ties away from zero.
                            if (pr < 13) {
                                const U64 one_at_unprinted_msb_if_precision_is[13] = {
                                    576460752303423488ULL, // 800000000000000
                                    36028797018963968ULL,  // 80000000000000
                                    2251799813685248ULL,   // 8000000000000
                                    140737488355328ULL,    // 800000000000
                                    8796093022208ULL,      // 80000000000
                                    549755813888ULL,       // 8000000000
                                    34359738368ULL,        // 800000000
                                    2147483648ULL,         // 80000000
                                    134217728ULL,          // 8000000
                                    8388608ULL,            // 800000
                                    524288ULL,             // 80000
                                    32768ULL,              // 8000
                                    2048ULL,               // 800
                                };

                                U64 one_at_unprinted_msb  = one_at_unprinted_msb_if_precision_is[pr];

                                U64 ones_after_unprinted_msb = one_at_unprinted_msb - 1ULL;
                                U64 zeroes_at_unprinted = ((Size)-1) ^ ((ones_after_unprinted_msb << 1) | 1);
                                U64 one_at_printed_lsb = one_at_unprinted_msb << 1;

                                // https://indepth.dev/how-to-round-binary-numbers/
                                U64 lower = man & zeroes_at_unprinted;
                                U64 middle = lower | one_at_unprinted_msb;
                                U64 upper = lower + one_at_printed_lsb;

                                if (man < middle) {
                                    man = lower;
                                } else if (man > middle) {
                                    man = upper;
                                } else {
                                    if ((lower & one_at_printed_lsb) == 0) {
                                        man = lower;
                                    } else {
                                        man = upper;
                                    }
                                }
                            }

                            str = num_str;
                            Char const *hexs = capital ? "0123456789ABCDEF" : "0123456789abcdef";

                            // NOTE(naman): This prints 0/1 depending on normal/denormal status
                            *str++ = hexs[(man >> 60) & 0xF];
                            man <<= 4;

                            if (precision) *str++ = '.';

                            S32 n = pr;

                            if (n > 13) n = 13;
                            if (pr > (S32)n) trailing_zeroes = pr - n;

                            U32 count_of_end_zeroes = 0;
                            while (n--) {
                                if ((man >> 60) & 0xF) {
                                    count_of_end_zeroes = 0;
                                } else {
                                    count_of_end_zeroes++;
                                }

                                *str++ = hexs[(man >> 60) & 0xF];
                                man <<= 4;
                            }

                            if (precision >= 0) {
                                count_of_end_zeroes = 0;
                            }

                            len = (Size)(Dptr)(str - num_str - count_of_end_zeroes);
                            str = num_str;
                        } else {
                            len = 0;
                        }

                        {
                            tail_str[tail_index++] = capital ? 'P' : 'p';
                            tail_str[tail_index++] = ex > 0 ? '+' : '-';
                            ex = ex > 0 ? ex : -ex;

                            Char es[4] = {0};
                            Char *ep = &es[elemin(es)];
                            Char el = 0;
                            while (ex) {
                                *--ep = (Char)(ex % 10) + '0';
                                el++;
                                ex /= 10;
                            }

                            while (el) {
                                el--;
                                tail_str[tail_index++] = *ep++;
                            }
                        }
                    }

                    if (str == NLIB_COMPAT_NULL) {
#  if defined(NLIB_PRINT_STB_FLOAT)
                        Char *out = num_str;
                        S32 tens;
                        S32 e;

#   define PRINT_STB_OFFSET 64
                        {
                            S64 sbits = (S64)bits;
                            U32 frac_digits = (U32)precision;

                            F64 value = negative ? -f64 : f64;
                            U32 expo = exponent_biased;

                            if ((exponent_biased == 0) && (mantissa != 0)) { // Denormal
                                // find the right expo for denormals
                                S64 v = (S64)(1ULL << 51);
                                while ((sbits & v) == 0) {
                                    --expo;
                                    v >>= 1;
                                }
                            }

                            { // find the decimal exponent as well as the decimal bits of the value
                                F64 ph, pl;

                                // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
                                tens = (S32)expo - 1023;
                                tens = ((tens < 0) ?
                                        ((tens * 617) / 2048) :
                                        (((tens * 1233) / 4096) + 1));

                                // move the significant bits into position and stick them into an int
                                stbfp_raise_to_power10(value, 18 - tens, &ph, &pl);

                                // get full as much precision from F64-F64 as possible
                                stbfp_ddtoS64(sbits, ph, pl);

                                // check if we undershot
                                if (((U64)sbits) >= stbfp_tento19th)
                                    ++tens;
                            }

                            // now do the rounding in integer land
                            if (flags & Print_Flags_FLOAT_EXP) {
                                frac_digits = frac_digits + 1;
                            } else {
                                frac_digits = ((U32)tens + frac_digits);
                            }

                            if ((frac_digits < 24)) {
                                U32 dg = 1;
                                if ((U64)sbits >= stbfp_powten[9]) {
                                    dg = 10;
                                }

                                while ((U64)sbits >= stbfp_powten[dg]) {
                                    ++dg;
                                    if (dg == 20)
                                        goto noround;
                                }

                                if (frac_digits < dg) {
                                    U64 r;
                                    // add 0.5 at the right position and round
                                    e = (S32)(dg - frac_digits);
                                    if ((U32)e >= 24) goto noround;

                                    r = stbfp_powten[e];
                                    sbits = sbits + ((S64)r / 2);
                                    if ((U64)sbits >= stbfp_powten[dg]) {
                                        ++tens;
                                    }
                                    sbits /= r;
                                }
                            noround:;
                            }

                            // kill long trailing runs of zeros
                            if (sbits) {
                                U32 n;
                                for (;;) {
                                    if (sbits <= 0xffffffff)
                                        break;
                                    if (sbits % 1000)
                                        goto donez;
                                    sbits /= 1000;
                                }
                                n = (U32)sbits;
                                while ((n % 1000) == 0)
                                    n /= 1000;
                                sbits = n;
                            donez:;
                            }

                            // convert to string
                            out += PRINT_STB_OFFSET;
                            e = 0;
                            for (;;) {
                                U32 n;
                                char *o = out - 8;
                                // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
                                if (sbits >= 100000000) {
                                    n = (U32)(sbits % 100000000);
                                    sbits /= 100000000;
                                } else {
                                    n = (U32)sbits;
                                    sbits = 0;
                                }
                                while (n) {
                                    out -= 2;
                                    *(U16 *)(void*)out = *(const U16 *)(const void*)&stbfp_digitpair.pair[(n % 100) * 2];
                                    n /= 100;
                                    e += 2;
                                }
                                if (sbits == 0) {
                                    if ((e) && (out[0] == '0')) {
                                        ++out;
                                        --e;
                                    }
                                    break;
                                }
                                while (out != o) {
                                    *--out = '0';
                                    ++e;
                                }
                            }
                        }

                        // decimal_pos is the number of digits before/after the decimal point.
                        // If it is positive, it means that the most signification digit is left
                        // of the decimal point.
                        // If it is negtive, it means that the most signification digit is right
                        // of the decimal point.
                        S32 decimal_pos = tens;
                        // out contains the decimal representation of all the significant bits
                        // in the floating-point number.
                        Char *sn = out;
                        S32 l = e;

                        str = num_str + PRINT_STB_OFFSET;

                        if (flags & Print_Flags_FLOAT_FIXED) {
                            if (decimal_pos <= 0) { // handle 0.000*000xxxx
                                *str++ = '0';
                                if (precision) *str++ = '.';

                                S32 n = -decimal_pos;
                                if (n > precision) n = precision;

                                S32 i = n;
                                memset(str, '0', (Size)i);
                                str += i;
                                i = 0;

                                if ((S32)(l + n) > precision) l = precision - n;
                                i = l;
                                memcpy(str, sn, (Size)i);
                                str += i;
                                sn += i;
                                i = 0;

                                trailing_zeroes = precision - (n + l);
                            } else {
                                if (decimal_pos >= l) { // handle xxxx000*000.0
                                    S32 n = 0;
                                    memcpy(str, sn, (Size)l);
                                    str += e;
                                    sn += e;
                                    n = l;

                                    if (n < decimal_pos) {
                                        n = decimal_pos - n;
                                        memset(str, '0', (Size)n);
                                        str += n;
                                        n = 0;
                                    }

                                    if (precision) {
                                        *str++ = '.';
                                        trailing_zeroes = precision;
                                    }
                                } else { // handle xxxxx.xxxx000*000
                                    S32 n = 0;
                                    memcpy(str, sn, (Size)decimal_pos);
                                    str += decimal_pos;
                                    sn += decimal_pos;
                                    n = decimal_pos;

                                    if (precision)
                                        *str++ = '.';

                                    if ((l - decimal_pos) > precision)
                                        l = precision + decimal_pos;

                                    if (n < l) {
                                        memcpy(str, sn, (Size)(l - n));
                                        str += (Size)(l - n);
                                        sn += (Size)(l - n);
                                        n += l - n;
                                    }

                                    trailing_zeroes = precision - (l - decimal_pos);
                                }
                            }
                        } else if (flags & Print_Flags_FLOAT_EXP) {
                            *str++ = sn[0];
                            if (precision) *str++ = '.';

                            if ((l - 1) > precision) {
                                l = precision + 1;
                            }

                            S32 n;
                            for (n = 1; n < l; n++) {
                                *str++ = sn[n];
                            }

                            trailing_zeroes = precision - (l - 1);
                            precision = 0;

                            tail_str[tail_index++] = capital ? 'E' : 'e';
                            decimal_pos--;
                            if (decimal_pos < 0) {
                                tail_str[tail_index++] = '-';
                                decimal_pos = -decimal_pos;
                            } else {
                                tail_str[tail_index++] = '+';
                            }

                            n = decimal_pos >= 100 ? 5 : 4;

                            while (true) {
                                tail_str[n] = '0' + (decimal_pos % 10);
                                tail_index++;
                                if (n <= 3) break;
                                --n;
                                decimal_pos /= 10;
                            }
                        }

                        len = (Size)(Dptr)(str - (num_str + PRINT_STB_OFFSET));
                        str = num_str + PRINT_STB_OFFSET;
#  elif defined(NLIB_PRINT_RYU_FLOAT)
                        str = num_str;

                        S32 e2;
                        U64 m2;

                        if (exponent_biased == 0) {
                            e2 = 1 - F64_BIAS - F64_MANTISSA_BITS;
                            m2 = mantissa;
                        } else {
                            e2 = (S32)exponent_biased - F64_BIAS - F64_MANTISSA_BITS;
                            m2 = (1ULL << F64_MANTISSA_BITS) | mantissa;
                        }

                        if (flags & Print_Flags_FLOAT_FIXED) {
                            B32 has_seen_non_zero_digit = false;

                            if (e2 >= -52) {
                                U32 index = e2 < 0 ? 0 : ryu_indexForExponent((U32)e2);
                                U32 p10_bits = ryu_pow10BitsForIndex(index);
                                S32 index_length = (S32)ryu_lengthForIndex(index);

                                for (S32 i = index_length - 1; i >= 0; --i) {
                                    U32 j = p10_bits - (U32)e2;
                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    const U64 *table_lookup = RYU_POW10_SPLIT[RYU_POW10_OFFSET[index] + i];
                                    U32 digits = ryu_mulShift_mod1e9(m2 << 8,
                                                                     table_lookup,
                                                                     (S32)(j + 8));
                                    if (has_seen_non_zero_digit) {
                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                    } else if (digits != 0) {
                                        U32 olength = ryu_decimalLength9(digits);
                                        ryu_append_n_digits(olength, digits, str + len);
                                        len += olength;
                                        has_seen_non_zero_digit = true;
                                    } else {
                                        // skip the digits
                                    }
                                }
                            }

                            if (!has_seen_non_zero_digit) {
                                str[len++] = '0';
                            }

                            if (precision > 0) {
                                str[len++] = '.';
                            }

                            if (e2 >= 0) {
                                if (precision > 0) {
                                    trailing_zeroes += precision;
                                }
                            } else {
                                S32 idx = -e2 / 16;

                                U32 blocks = ((U32)precision / 9) + 1;
                                U32 i = 0;

                                if (blocks <= RYU_MIN_BLOCK_2[idx]) {
                                    i = blocks;
                                    trailing_zeroes += precision;
                                } else if (i < RYU_MIN_BLOCK_2[idx]) {
                                    i = RYU_MIN_BLOCK_2[idx];
                                    memset(str + len, '0', 9 * i);
                                    len += 9 * i;
                                }

                                // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
                                Sint round_up = 0;

                                for (; i < blocks; ++i) {
                                    S32 j = RYU_ADDITIONAL_BITS_2 + (-e2 - 16 * idx);
                                    U32 p = RYU_POW10_OFFSET_2[idx] + i - RYU_MIN_BLOCK_2[idx];

                                    if (p >= RYU_POW10_OFFSET_2[idx + 1]) {
                                        // If the remaining digits are all 0, then we might as well use memset.
                                        // No rounding required in this case.
                                        U32 fill = (U32)precision - 9 * i;
                                        trailing_zeroes += fill;
                                        break;
                                    }

                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    U32 digits = ryu_mulShift_mod1e9(m2 << 8, RYU_POW10_SPLIT_2[p], j + 8);

                                    if (i < blocks - 1) {
                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                    } else {
                                        U32 maximum = (U32)precision - 9 * i;
                                        U32 last_digit = 0;

                                        for (U32 k = 0; k < 9 - maximum; ++k) {
                                            last_digit = digits % 10;
                                            digits /= 10;
                                        }

                                        if (last_digit != 5) {
                                            round_up = last_digit > 5;
                                        } else {
                                            // Is m * 10^(additionalDigits + 1) / 2^(-e2) integer?
                                            S32 required_twos = -e2 - (S32) precision - 1;
                                            B32 any_trailing_zeros = (required_twos <= 0)
                                                || (required_twos < 60 &&
                                                    ryu_multipleOfPowerOf2(m2,
                                                                           (U32)required_twos));
                                            round_up = any_trailing_zeros ? 2 : 1;
                                        }

                                        if (maximum > 0) {
                                            ryu_append_c_digits(maximum, digits, str + len);
                                            len += maximum;
                                        }

                                        break;
                                    }
                                }

                                if (round_up != 0) {
                                    Sint round_index = (Sint)len;
                                    Sint dot_index = 0; // '.' can't be located at index 0
                                    while (true) {
                                        --round_index;
                                        Char c;
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wcomma"
#   endif
                                        if (round_index == -1 || (c = str[round_index], c == '-')) {
                                            str[round_index + 1] = '1';
                                            if (dot_index > 0) {
                                                str[dot_index] = '0';
                                                str[dot_index + 1] = '.';
                                            }
                                            str[len++] = '0';
                                            break;
                                        }
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif
                                        if (c == '.') {
                                            dot_index = round_index;
                                            continue;
                                        } else if (c == '9') {
                                            str[round_index] = '0';
                                            round_up = 1;
                                            continue;
                                        } else {
                                            if (round_up == 2 && c % 2 == 0) {
                                                break;
                                            }
                                            str[round_index] = c + 1;
                                            break;
                                        }
                                    }
                                }
                            }
                        } else if (flags & Print_Flags_FLOAT_EXP) {
                            B32 print_decimal_point = precision > 0;
                            ++precision;

                            U32 digits = 0;
                            U32 printed_digits = 0;
                            U32 available_digits = 0;
                            S32 exp = 0;

                            if (e2 >= -52) {
                                U32 idx = e2 < 0 ? 0 : ryu_indexForExponent((U32)e2);
                                U32 p10bits = ryu_pow10BitsForIndex(idx);
                                S32 index_length = (S32)ryu_lengthForIndex(idx);

                                for (S32 i = index_length - 1; i >= 0; --i) {
                                    U32 j = p10bits - (U32)e2;
                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    const U64 *table_lookup = RYU_POW10_SPLIT[RYU_POW10_OFFSET[idx] + i];
                                    digits = ryu_mulShift_mod1e9(m2 << 8, table_lookup, (S32)(j + 8));

                                    if (printed_digits != 0) {
                                        if (printed_digits + 9 > (Uint)precision) {
                                            available_digits = 9;
                                            break;
                                        }

                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                        printed_digits += 9;
                                    } else if (digits != 0) {
                                        available_digits = ryu_decimalLength9(digits);
                                        exp = i * 9 + (S32) available_digits - 1;

                                        if (available_digits > (Uint)precision) {
                                            break;
                                        }

                                        if (print_decimal_point) {
                                            ryu_append_d_digits(available_digits, digits, str + len);
                                            len += available_digits + 1; // +1 for decimal point
                                        } else {
                                            str[len++] = (Char)('0' + digits);
                                        }

                                        printed_digits = available_digits;
                                        available_digits = 0;
                                    }
                                }
                            }

                            if (e2 < 0 && available_digits == 0) {
                                S32 idx = -e2 / 16;

                                for (S32 i = RYU_MIN_BLOCK_2[idx]; i < 200; ++i) {
                                    S32 j = RYU_ADDITIONAL_BITS_2 + (-e2 - 16 * idx);
                                    U32 p = RYU_POW10_OFFSET_2[idx] + (U32) i - RYU_MIN_BLOCK_2[idx];
                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    digits = (p >= RYU_POW10_OFFSET_2[idx + 1]) ? 0 : ryu_mulShift_mod1e9(m2 << 8, RYU_POW10_SPLIT_2[p], j + 8);

                                    if (printed_digits != 0) {
                                        if (printed_digits + 9 > (Uint)precision) {
                                            available_digits = 9;
                                            break;
                                        }
                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                        printed_digits += 9;
                                    } else if (digits != 0) {
                                        available_digits = ryu_decimalLength9(digits);
                                        exp = -(i + 1) * 9 + (S32) available_digits - 1;
                                        if (available_digits > (Uint)precision) {
                                            break;
                                        }
                                        if (print_decimal_point) {
                                            ryu_append_d_digits(available_digits, digits, str + len);
                                            len += available_digits + 1; // +1 for decimal point
                                        } else {
                                            str[len++] = (Char)('0' + digits);
                                        }
                                        printed_digits = available_digits;
                                        available_digits = 0;
                                    }
                                }
                            }

                            U32 maximum = (U32)precision - printed_digits;

                            if (available_digits == 0) {
                                digits = 0;
                            }
                            U32 last_digit = 0;
                            if (available_digits > maximum) {
                                for (U32 k = 0; k < available_digits - maximum; ++k) {
                                    last_digit = digits % 10;
                                    digits /= 10;
                                }
                            }

                            // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
                            Sint round_up = 0;
                            if (last_digit != 5) {
                                round_up = last_digit > 5;
                            } else {
                                // Is m * 2^e2 * 10^(precision + 1 - exp) integer?
                                // precision was already increased by 1, so we don't need to write + 1 here.
                                S32 rexp = (S32) precision - exp;
                                S32 requiredTwos = -e2 - rexp;
                                B32 any_trailing_zeros = requiredTwos <= 0
                                    || (requiredTwos < 60 && ryu_multipleOfPowerOf2(m2, (U32) requiredTwos));
                                if (rexp < 0) {
                                    S32 requiredFives = -rexp;
                                    any_trailing_zeros = any_trailing_zeros && ryu_multipleOfPowerOf5(m2, (U32) requiredFives);
                                }
                                round_up = any_trailing_zeros ? 2 : 1;
                            }

                            if (printed_digits != 0) {
                                if (digits == 0) {
                                    memset(str + len, '0', maximum);
                                } else {
                                    ryu_append_c_digits(maximum, digits, str + len);
                                }
                                len += maximum;
                            } else {
                                if (print_decimal_point) {
                                    ryu_append_d_digits(maximum, digits, str + len);
                                    len += maximum + 1; // +1 for decimal point
                                } else {
                                    str[len++] = (Char)('0' + digits);
                                }
                            }

                            if (round_up != 0) {
                                Sint round_index = (Sint)len;
                                while (true) {
                                    --round_index;
                                    Char c;
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wcomma"
#   endif
                                    if (round_index == -1 || (c = str[round_index], c == '-')) {
                                        str[round_index + 1] = '1';
                                        ++exp;
                                        break;
                                    }
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif
                                    if (c == '.') {
                                        continue;
                                    } else if (c == '9') {
                                        str[round_index] = '0';
                                        round_up = 1;
                                        continue;
                                    } else {
                                        if (round_up == 2 && c % 2 == 0) {
                                            break;
                                        }
                                        str[round_index] = c + 1;
                                        break;
                                    }
                                }
                            }

                            str[len++] = capital ? 'E' : 'e';
                            if (exp < 0) {
                                str[len++] = '-';
                                exp = -exp;
                            } else {
                                str[len++] = '+';
                            }

                            if (exp >= 100) {
                                S32 c = exp % 10;
                                memcpy(str + len, RYU_DIGIT_TABLE + 2 * (exp / 10), 2);
                                str[len + 2] = (Char) ('0' + c);
                                len += 3;
                            } else {
                                memcpy(str + len, RYU_DIGIT_TABLE + 2 * exp, 2);
                                len += 2;
                            }

                        }
#  elif defined(NLIB_PRINT_BAD_FLOAT)
                        F64 value = (f64 < 0) ? -f64 : f64;
                        B32 power_of_e_nonsense = false;
                        Sint print_exponent = 0;

                        B32 fmt_exponential = (fmt[0] == 'E') || (fmt[0] == 'e');

                        { // Limit the "range" of float
                            // log10(2^64) = 19 = max integral F64 storable in U64 without
                            // loss of information
                            F64 upper_threshold = 1e19;

                            if (fmt_exponential || (value >= upper_threshold)) {
                                power_of_e_nonsense = true;
                                if (value >= 1e256) {
                                    value /= 1e256;
                                    print_exponent += 256;
                                }
                                if (value >= 1e128) {
                                    value /= 1e128;
                                    print_exponent += 128;
                                }
                                if (value >= 1e64) {
                                    value /= 1e64;
                                    print_exponent += 64;
                                }
                                if (value >= 1e32) {
                                    value /= 1e32;
                                    print_exponent += 32;
                                }
                                if (value >= 1e16) {
                                    value /= 1e16;
                                    print_exponent += 16;
                                }
                                if (value >= 1e8) {
                                    value /= 1e8;
                                    print_exponent += 8;
                                }
                                if (value >= 1e4) {
                                    value /= 1e4;
                                    print_exponent += 4;
                                }
                                if (value >= 1e2) {
                                    value /= 1e2;
                                    print_exponent += 2;
                                }
                                if (value >= 1e1) {
                                    value /= 1e1;
                                    print_exponent += 1;
                                }
                            }

                            if (fmt_exponential) {
                                // 10^-(precision-1)
                                // (so that we get atleast one digit)
                                F64 powers_of_10[20] = {
                                    1.000000,
                                    10.000000,
                                    100.000000,
                                    1000.000000,
                                    10000.000000,
                                    100000.000000,
                                    1000000.000000,
                                    10000000.000000,
                                    100000000.000000,
                                    1000000000.000000,
                                    10000000000.000000,
                                    100000000000.000000,
                                    1000000000000.000000,
                                    10000000000000.000000,
                                    100000000000000.000000,
                                    1000000000000000.000000,
                                    10000000000000000.000000,
                                    100000000000000000.000000,
                                    1000000000000000000.000000,
                                    10000000000000000000.000000,
                                };

                                F64 lower_threshold;
                                // lower_threshold = pow(10.0, 1.0 - (F64)precision);
                                S32 power = 1 - precision;
                                if (power < 20) {
                                    lower_threshold = powers_of_10[power];
                                } else {
                                    lower_threshold = 1.0;
                                    for (Sint i = power; i > 0; i--) {
                                        lower_threshold *= 10.0;
                                    }
                                }

                                if (fmt_exponential || (value > 0 && value <= lower_threshold)) {
                                    power_of_e_nonsense = true;
                                    if (value < 1e-255) {
                                        value *= 1e256;
                                        print_exponent -= 256;
                                    }
                                    if (value < 1e-127) {
                                        value *= 1e128;
                                        print_exponent -= 128;
                                    }
                                    if (value < 1e-63) {
                                        value *= 1e64;
                                        print_exponent -= 64;
                                    }
                                    if (value < 1e-31) {
                                        value *= 1e32;
                                        print_exponent -= 32;
                                    }
                                    if (value < 1e-15) {
                                        value *= 1e16;
                                        print_exponent -= 16;
                                    }
                                    if (value < 1e-7) {
                                        value *= 1e8;
                                        print_exponent -= 8;
                                    }
                                    if (value < 1e-3) {
                                        value *= 1e4;
                                        print_exponent -= 4;
                                    }
                                    if (value < 1e-1) {
                                        value *= 1e2;
                                        print_exponent -= 2;
                                    }
                                    if (value < 1e0) {
                                        value *= 1e1;
                                        print_exponent -= 1;
                                    }
                                }
                            }
                        }

                        U64 integral_part = (U64)value;
                        F64 remainder = value - (F64)integral_part;

                        F64 remainder_modified = remainder * 1e19;
                        U64 decimal_part = (U64)remainder_modified;

                        // rounding
                        remainder_modified -= (F64)decimal_part;
                        if (remainder_modified >= 0.5) {
                            decimal_part++;
                            if (decimal_part >= 10000000000000000000ULL) { // 19 zeroes
                                decimal_part = 0;
                                integral_part++;
                                if (print_exponent != 0 && integral_part >= 10) {
                                    print_exponent++;
                                    integral_part = 1;
                                }
                            }
                        }

                        str = num_str + PRINT_STR_SIZE;

                        { // Write print_exponent (if needed)
                            if (power_of_e_nonsense) {
                                // convert to string
                                Sint num_dec = (print_exponent > 0) ? print_exponent : -print_exponent;

                                Char *str_now = str;

                                if (num_dec) {
                                    while (num_dec) {
                                        *--str = (Char)(num_dec % 10) + '0';
                                        num_dec /= 10;
                                    }
                                } else {
                                    *--str = '0';
                                }

                                if ((Uptr)str_now - (Uptr)str == 1) {
                                    *--str = '0';
                                }

                                if (print_exponent < 0) {
                                    *--str = '-';
                                } else {
                                    *--str = '+';
                                }

                                *--str = capital ? 'E' : 'e';
                            }
                        }

                        { // Write decimal part
                            if (precision > 0) {
                                Char tmp_buf[PRINT_STR_SIZE] = {0};
                                Char *tmp = tmp_buf + PRINT_STR_SIZE;
                                Uint width = 19; // TODO(naman): Is this correct?

                                Uint width_iter = width;

                                if ((Size)precision > width) {
                                    // FIXME(naman): Get more decimal digits instead of just
                                    // printing zeroes
                                    Size extra_zeroes = (Size)precision - width;
                                    memset(tmp - extra_zeroes, '0', extra_zeroes);
                                    tmp -= extra_zeroes;
                                }

                                do {
                                    *--tmp = (Char)(decimal_part % 10) + '0';
                                    decimal_part = decimal_part / 10;
                                } while (--width_iter);

                                if ((Uint)precision < width) {
                                    for (Char *temp = tmp_buf + PRINT_STR_SIZE - 1;
                                         (Uptr)temp - (Uptr)tmp >= (Uint)precision;
                                         temp--) {
                                        Size index = 0;

                                        if ((temp[index] - '0') > 5) {
                                            B32 carry = false;
                                            do {
                                                if ((temp[index-1] - '0') == 9) {
                                                    temp[index-1] = '0';
                                                    carry = true;
                                                } else {
                                                    temp[index-1] += 1;
                                                    carry = false;
                                                }
                                                index--;
                                            } while (carry && ((Uptr) temp - index >= (Uptr)tmp));
                                        }
                                    }
                                }

                                str -= precision;
                                memcpy(str, tmp, (Size)precision);

                                *--str = '.';
                            }
                        }

                        { // Write integral
                            do {
                                *--str = (Char)(integral_part % 10) + '0';
                                integral_part = integral_part / 10;
                            } while (integral_part);
                        }

                        // get the length that we copied
                        len = (((Uptr)num_str + PRINT_STR_SIZE) - (Uptr)str);

                        if (len == 0) {
                            *--str = '0';
                            len = 1;
                        }
#  elif defined(NLIB_PRINT_NO_FLOAT)
                        str = num_str + PRINT_STR_SIZE;
                        *--str = fmt[0];
                        len = 1;
#  endif
                    }

                    precision = 0;
                } break;

                case '%': {
                    str = num_str;
                    str[0] = '%';
                    len = 1;
                    precision = 0;
                } break;

                default: { // unknown, just copy code
                    str = num_str;

                    while (format_pointer <= fmt) {
                        str[0] = format_pointer[0];
                        format_pointer++;
                        str++;
                    }

                    len = (Size)(Dptr)(str - num_str);
                    str = num_str;
                    precision = 0;
                } break;
            }

            Size head_size = head_index;
            head_index = 0;
            Size tail_size = tail_index;
            tail_index = 0;

            // get field_width=leading/trailing space, precision=leading zeros
            if ((Size)precision < len) {
                precision = (Sint)len;
            }

            Sint zeros_head_tail = precision + (Sint)head_size + (Sint)tail_size + trailing_zeroes;
            if (field_width < zeros_head_tail) {
                field_width = zeros_head_tail;
            }

            field_width -= zeros_head_tail;
            precision -= len;

            // handle right justify and leading zeros
            if ((flags & Print_Flags_LEFT_JUSTIFIED) == 0) {
                if (flags & Print_Flags_LEADING_ZERO) { // then everything is in precision
                    precision = (field_width > precision) ? field_width : precision;
                    field_width = 0;
                }
            }

            // copy leading spaces
            if ((field_width + precision) > 0) {
                // copy leading spaces (or when doing %8.4d stuff)
                if ((flags & Print_Flags_LEFT_JUSTIFIED) == 0) {
                    IS_OUTPUT_RESUMABLE(field_width);
                    if (resume_output) {
                        memset(buf, ' ', (Size)field_width);
                        buf += field_width;
                    }
                    needed_size += (Size)field_width;
                }

                { // copy the head
                    IS_OUTPUT_RESUMABLE(head_size);
                    if (resume_output) {
                        memcpy(buf, head_str, head_size);
                        buf += head_size;
                    }
                    needed_size += head_size;
                    head_index += head_size;
                }

                { // copy leading zeros
                    IS_OUTPUT_RESUMABLE(precision);
                    if (resume_output) {
                        memset(buf, '0', (Size)precision);
                        buf += precision;
                    }
                    needed_size += (Size)precision;
                }
            }

            { // copy the head
                if (head_index < head_size) {
                    Size repeat = head_size - head_index;
                    IS_OUTPUT_RESUMABLE(repeat);
                    if (resume_output) {
                        memcpy(buf, head_str, repeat);
                        buf += repeat;
                    }
                    head_index += repeat;
                    needed_size += repeat;
                }
            }

            { // copy the string
                IS_OUTPUT_RESUMABLE(len);
                if (resume_output) {
                    memcpy(buf, str, len);
                    buf += len;
                }
                str += len;
                needed_size += len;
            }

            { // copy trailing zeroes
                IS_OUTPUT_RESUMABLE(trailing_zeroes);
                if (resume_output) {
                    memset(buf, '0', (Size)trailing_zeroes);
                    buf += (Size)trailing_zeroes;
                }
                needed_size += (Size)trailing_zeroes;
            }

            { // copy the tail
                IS_OUTPUT_RESUMABLE(tail_size);
                if (resume_output) {
                    memcpy(buf, tail_str, tail_size);
                    buf += tail_size;
                }
                needed_size += tail_size;
            }

            // handle the left justify
            if (flags & Print_Flags_LEFT_JUSTIFIED)
                if (field_width > 0) {
                    IS_OUTPUT_RESUMABLE(field_width);
                    if (resume_output) {
                        memset(buf, ' ', (Size)field_width);
                        buf += field_width;
                    }
                    needed_size += (Size)field_width;
                }

            fmt++;
        } else if (fmt[0] =='\0') {
            break;
        }
    }

    // NOTE(naman): IS_OUTPUT_RESUMABLE always preserves space for 1 null character;
    // thus, the check isn't necessary.
    if (buffer != NLIB_COMPAT_NULL) {
        buf[0] = '\0';
        buf++;
    }

    Size result = needed_size;
    return result;
}

#  if defined(OS_WINDOWS)

header_function
Size printConsole (Sint fd, Char const *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    // FIXME(naman): Remove these double calls once printStringVarArg starts yielding
    Size buffer_size = printStringVarArg(NLIB_COMPAT_NULL, 0, format, ap1);
    Char *buffer = (Char*)NLIB_PRINT_MALLOC(buffer_size + 1);
    printStringVarArg(buffer, buffer_size + 1, format, ap2);

    HANDLE out_stream;
    if (fd == 1) {
        out_stream = GetStdHandle(STD_OUTPUT_HANDLE);
    } else {
        out_stream = GetStdHandle(STD_ERROR_HANDLE);
    }

    if ((out_stream != NLIB_COMPAT_NULL) && (out_stream != INVALID_HANDLE_VALUE)) {
        DWORD written = 0;
        // FIXME(naman): Convert this ASCII/UTF-8 buffer to UTF-16 maybe
        WriteConsoleA(out_stream, buffer, buffer_size, &written, NLIB_COMPAT_NULL);
    }

    NLIB_PRINT_FREE((void*)buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

header_function
Size printDebugOutputV (Char const *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    // FIXME(naman): Remove these double calls once printStringVarArg starts yielding
    Size buffer_size = printStringVarArg(NLIB_COMPAT_NULL, 0, format, ap1);
    Char *buffer = (Char*)NLIB_PRINT_MALLOC(buffer_size + 1);
    printStringVarArg(buffer, buffer_size + 1, format, ap2);

    LPWSTR wcstr = unicodeWin32UTF16FromUTF8(buffer);
    OutputDebugStringW(wcstr);
    unicodeWin32UTF16Dealloc(wcstr);

    nlib_Dealloc((void*)buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

header_function
Size printDebugOutput (Char const *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size result = printDebugOutputV(format, ap);
    va_end(ap);

    return result;
}

#  elif defined(OS_LINUX)

header_function
Size printConsole (Sint fd, Char const *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    // FIXME(naman): Remove these double calls once printStringVarArg starts yielding
    Size buffer_size = printStringVarArg(NLIB_COMPAT_NULL, 0, format, ap1);
    Char *buffer = (Char*)NLIB_PRINT_MALLOC(buffer_size + 1);
    printStringVarArg(buffer, buffer_size + 1, format, ap2);

    if (fd == 1) {
        fputs(buffer, stdout);
    } else {
        fputs(buffer, stderr);
    }

    NLIB_PRINT_FREE((void*)buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

#  endif

header_function
Size printString (Char *buffer, Size buffer_size, Char const *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size result = printStringVarArg(buffer, buffer_size, format, ap);
    va_end(ap);

    return result;
}

header_function
Size say (Char const *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size result = printConsole(1, format, ap);
    va_end(ap);

    return result;
}

header_function
Size sayv (Char const *format, va_list ap)
{
    Size result = printConsole(1, format, ap);
    return result;
}

header_function
Size err (Char const *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size result = printConsole(2, format, ap);
    va_end(ap);

    return result;
}

header_function
Size errv (Char const *format, va_list ap)
{
    Size result = printConsole(2, format, ap);
    return result;
}

# endif // NLIB_EXCLUDE_PRINT

/* ===================
 * @Unit Test
 */

# if !defined(NLIB_EXCLUDE_UNIT_TEST)

#  define utTest(cond) ut_Test(cond, #cond, __FILE__, __LINE__)

header_function
void ut_Test (B32 cond,
              Char *cond_str,
              Char *filename, U32 line_num) {
    if (!(cond)) {
        report("Test Failed: (%s:%u) %s\n", filename, line_num, cond_str);
        quit();
    }
}

# endif // NLIB_EXCLUDE_UNIT_TEST


/* ==================
 * @Bit Operators
 */

# if !defined(NLIB_EXCLUDE_BIT)

// NOTE(naman): Bit vectors are supposed to be zero-indexed.
// NOTE(naman): Base type of bit vectors shouldn't have size > 8 (to prevent shift overflow).

#  define bitToBytes(b) (((b)+(CHAR_BIT-1))/(CHAR_BIT))

#  define bit_I(array, index) ((index)/(CHAR_BIT * sizeof(*(array))))
#  define bit_M(array, index) ((index)%(CHAR_BIT * sizeof(*(array))))
#  define bitSet(array, index)                                          \
    ((array)[bit_I(array, index)] |= (1LLU << bit_M(array, index)))
#  define bitReset(array, index)                                        \
    ((array)[bit_I(array, index)] &= ~(1LLU << bit_M(array, index)))
#  define bitToggle(array, index)                                       \
    ((array)[bit_I(array, index)] ^= ~(1LLU << bit_M(array, index)))
#  define bitTest(array, index)                                         \
    ((array)[bit_I(array, index)] & (1LLU << bit_M(array, index)))

#  if defined(LANG_C) && LANG_C >= 2011
#   define bitFindMSB(x) _Generic((x),                  \
                                  U32: bitFindMSBU32,   \
                                  U64: bitFindMSBU64,   \
        )(x)

#   define bitFindLSB(x) _Generic((x),                  \
                                  U32: bitFindLSBU32,   \
                                  U64: bitFindLSBU64,   \
        )(x)
#  endif // LANG_C

#  if defined(COMPILER_MSVC)

/* _BitScanReverse(&r, x) scans for the first 1-bit from left in x. Once it finds it,
 * it returns the number of bits to the right of the found 1-bit.
 */
header_function
U32 bitFindMSBU32 (U32 x)
{
    unsigned long result = 0;
    Byte found = _BitScanReverse(&result, x);
    result = found ? result : 0;
    return result;
}

header_function
U64 bitFindMSBU64 (U64 x)
{
    unsigned long result = 0;
    Byte found = _BitScanReverse64(&result, x);
    result = found ? result : 0;
    return result;
}

/* _BitScanReverse(&r, x) scans for the first 1-bit from right in x. Once it finds it,
 * it returns the number of bits to the right of the found 1-bit.
 */
header_function
U32 bitFindLSBU32 (U32 x)
{
    unsigned long result = 0;
    Byte found = _BitScanForward(&result, x);
    result = found ? result : (U32)-1;
    return result;
}

header_function
U64 bitFindLSBU64 (U64 x)
{
    unsigned long result = 0;
    Byte found = _BitScanForward64(&result, x);
    result = found ? result : (U32)-1;
    return result;
}

#  elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)

/* __builtin_clz(x) returns the number of leading 0-bits in x, starting from
 * most significant position.
 */
header_function
U32 bitFindMSBU32 (U32 x)
{
    if (x == 0) return (U32)-1;
    U32 result = 32U - (U32)__builtin_clz(x) - 1U;
    return result;
}

header_function
U64 bitFindMSBU64 (U64 x)
{
    if (x == 0) return (U64)-1;
    U64 result = 64LLU - (U64)__builtin_clzll(x) - 1LLU;
    return result;
}

/* __builtin_ctz(x) returns the number of trailing 0-bits in x, starting from
 * least significant position.
 */
header_function
U32 bitFindLSBU32 (U32 x)
{
    if (x == 0) return (U32)-1;
    U32 result = (U32)__builtin_ctz(x);
    return result;
}

header_function
U64 bitFindLSBU64 (U64 x)
{
    if (x == 0) return (U64)-1;
    U64 result = (U64)__builtin_ctzll(x);
    return result;
}

#  endif

# endif // NLIB_EXCLUDE_BIT


/* =================
 * @Maths
 */

# if !defined(NLIB_EXCLUDE_MATH)

#  define Mk_E(type)            (type)2.718281828459045235360287471352662498 /* e */
#  define Mk_LOG2_E(type)       (type)1.442695040888963407359924681001892137 /* log_2 e */
#  define Mk_LOG10_E(type)      (type)0.434294481903251827651128918916605082 /* log_10 e */
#  define Mk_LN_2(type)         (type)0.693147180559945309417232121458176568 /* log_e 2 */
#  define Mk_LN_10(type)        (type)2.302585092994045684017991454684364208 /* log_e 10 */
#  define Mk_PI(type)           (type)3.141592653589793238462643383279502884 /* pi */
#  define Mk_PI_BY_2(type)      (type)1.570796326794896619231321691639751442 /* pi/2 */
#  define Mk_PI_BY_4(type)      (type)0.785398163397448309615660845819875721 /* pi/4 */
#  define Mk_1_BY_PI(type)      (type)0.318309886183790671537767526745028724 /* 1/pi */
#  define Mk_2_BY_PI(type)      (type)0.636619772367581343075535053490057448 /* 2/pi */
#  define Mk_2_BY_SQRT_PI(type) (type)1.128379167095512573896158903121545172 /* 2/sqrt(pi) */
#  define Mk_SQRT_2(type)       (type)1.414213562373095048801688724209698079 /* sqrt(2) */
#  define Mk_1_BY_SQRT_2(type)  (type)0.707106781186547524400844362104849039 /* 1/sqrt(2) */

// FIXME(naman): Make these functions not depend on libm

#  define mMin(x, y) ((x) < (y) ? (x) : (y))
#  define mMax(x, y) ((x) > (y) ? (x) : (y))

#  if defined(LANG_C) && LANG_C >= 2011
#   define mSqrt(x) mGeneric((x),               \
                             F32 : mSqrtF32     \
                             F64 : mSqrtF64     \
        )(x)
#  endif

header_function F32 mSqrtF32 (F32 x) { F32 y = sqrtf(x); return y; }
header_function F64 mSqrtF64 (F64 x) { F64 y = sqrt(x);  return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mPow2(x) mGeneric((x),               \
                             F32 : mPow2F32,    \
                             F64 : mPow2F64,    \
                             U32 : mPow2U32     \
                             U64 : mPow2U64     \
        )(x)
#  endif

header_function F32 mPow2F32 (F32 x) { F32 y = exp2f(x); return y; }
header_function F64 mPow2F64 (F64 x) { F64 y = exp2(x);  return y; }
header_function U32 mPow2U32 (U32 x) { U32 y = 1 << x;   return y; }
header_function U64 mPow2U64 (U64 x) { U64 y = 1 << x;   return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mLog2(x) mGeneric((x),               \
                             F32 : mLog2F32,    \
                             F64 : mLog2F64,    \
                             U32:  mLog2U32,    \
                             U64:  mLog2U64     \
        )(x)
#  endif

header_function F32 mLog2F32 (F32 x) { F32 y = log2f(x); return y; }
header_function F64 mLog2F64 (F64 x) { F64 y = log2(x);  return y; }
header_function U32 mLog2U32 (U32 x) { U32 y = x ? bitFindMSBU32(x) : 0; return y; }
header_function U64 mLog2U64 (U64 x) { U64 y = x ? bitFindMSBU64(x) : 0; return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mIsPowerOf2(x) mGeneric((x),                 \
                                   U32 : mIsPowerOf2U32 \
                                   U64 : mIsPowerOf2U64 \
        )(x)
#  endif

header_function B32 mIsPowerOf2U32 (U32 x) { B32 b = (x & (x - 1)) == 0; return b; }
header_function B64 mIsPowerOf2U64 (U64 x) { B64 b = (x & (x - 1)) == 0; return b; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mNextPowerOf2(x) mGeneric((x),                       \
                                     U32 : mNextPowerOf2U32     \
                                     U64 : mNextPowerOf2U64     \
        )(x)
#  endif

header_function U32 mNextPowerOf2U32 (U32 x) { U32 y = mIsPowerOf2U32(x) ? x : (1U << (mLog2U32(x) + 1U)); return y; }
header_function U64 mNextPowerOf2U64 (U64 x) { U64 y = mIsPowerOf2U64(x) ? x : (1LLU << (mLog2U64(x) + 1LLU)); return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mPrevPowerOf2(x) mGeneric((x),                       \
                                     U32 : mPrevPowerOf2U32     \
                                     U64 : mPrevPowerOf2U64     \
        )(x)
#  endif

header_function U32 mPrevPowerOf2U32 (U32 x) { U32 y = mIsPowerOf2U32(x) ? (1U << (mLog2U32(x) - 1U)) : x; return y; }
header_function U64 mPrevPowerOf2U64 (U64 x) { U64 y = mIsPowerOf2U64(x) ? (1LLU << (mLog2U64(x) - 1LLU)) : x; return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mSin(x) mGeneric((x),                \
                            F32 : mSinF32       \
                            F64 : mSinF64       \
        )(x)
#  endif

header_function F32 mSinF32 (F32 x) { F32 y = sinf(x); return y; }
header_function F64 mSinF64 (F64 x) { F64 y = sin(x);  return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mCos(x) mGeneric((x),                \
                            F32 : mCosF32       \
                            F64 : mCosF64       \
        )(x)
#  endif

header_function F32 mCosF32 (F32 x) { F32 y = cosf(x); return y; }
header_function F64 mCosF64 (F64 x) { F64 y = cos(x);  return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mTan(x) mGeneric((x),                \
                            F32 : mTanF32       \
                            F64 : mTanF64       \
        )(x)
#  endif

header_function F32 mTanF32 (F32 x) { F32 y = tanf(x); return y; }
header_function F64 mTanF64 (F64 x) { F64 y = tan(x);  return y; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mRadians(x) mGeneric((x),                    \
                                F32 : mRadiansF64       \
                                F64 : mRadiansF64       \
        )(x)
#  endif

header_function F32 mRadiansF32 (F32 degrees) { F32 radians = (degrees * Mk_PI(F32)) / 180.0f; return radians; }
header_function F64 mRadiansF64 (F64 degrees) { F64 radians = (degrees * Mk_PI(F64)) / 180.0;  return radians; }

#  if defined(LANG_C) && LANG_C >= 2011
#   define mDegrees(x) mGeneric((x),                    \
                                F32 : mDegreesF32       \
                                F64 : mDegreesF64       \
        )(x)
#  endif

header_function F32 mDegreesF32 (F32 radians) { F32 degrees = (radians * 180.0f) / Mk_PI(F32); return degrees; }
header_function F64 mDegreesF64 (F64 radians) { F64 degrees = (radians * 180.0)  / Mk_PI(F64); return degrees; }

# endif // NLIB_EXCLUDE_MATH


/* ===============================
 * @Random Number Generator
 */

# if !defined(NLIB_EXCLUDE_RANDOM)

#  if defined(COMPILER_MSVC)

/* Linear Congruential Generator
 *
 * If x is the last random number,
 *    m is a number greater than zero that is a power of two,
 *    a is a number between 0 and m,
 * then the next random number is ((x * a) % m).
 *
 * Unfortunately, the lower bits don't have enought randomness in them. The LSB doesn't
 * change at all, the second LSB alternates, the one after that toggles every 2 turns and so
 * on. Therefore, we try to get rid of the LSBs by pulling in some MSBs.
 *
 * NOTE(naman): Seed should be an odd number or the randomness might drop drastically.
 * NOTE(naman): "a" should be equal to 5(mod 8) or 3(mod 8).
 */

header_function
U64 randomU64 (U64 seed)
{
    U64 previous = seed;
    if (previous == 0) {
        // This seed has been tested and should be preferred in normal circumstances.
        previous = 2531011ULL;
    }

    U64 a = 214013ULL;

    U64 upper = 0, lower = 0;
    lower = _umul128(previous, a, &upper);
    U64 log_upper = upper ? mLog2U64(upper) : 0;
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

#  elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)

/* Linear Congruential Generator
 *
 * If x is the last random number,
 *    m is a number greater than zero that is a power of two,
 *    a is a number between 0 and m,
 * then the next random number is ((x * a) % m).
 *
 * Unfortunately, the lower bits don't have enought randomness in them. The LSB doesn't
 * change at all, the second LSB alternates, the one after that toggles every 2 turns and so
 * on. Therefore, we try to get rid of the LSBs by pulling in some MSBs.
 *
 * We do the multiplcation twice because Chi-Square Test indicated that this method
 * gives better randomness. Don't ask.
 *
 * NOTE(naman): Seed should be an odd number or the randomness might drop drastically.
 * NOTE(naman): "a" should be equal to 5(mod 8) or 3(mod 8).
 */

header_function
U64 randomU64 (U64 seed)
{
    U64 previous = seed;
    if (previous == 0) {
        // This seed has been tested and should be preferred in normal circumstances.
        previous = 2531011ULL;
    }

    U64 a = 214013ULL;

    __uint128_t product = (__uint128_t)previous * (__uint128_t)a;
    U64 upper = product >> 64, lower = (U64)product;
    U64 log_upper = upper ? mLog2U64(upper) : 0;
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

#  endif

# endif // NLIB_EXCLUDE_RANDOM

/* ==========================
 * @Hashing Infrastructure
 */

# if !defined(NLIB_EXCLUDE_HASHING)

// TODO(naman): Add hash collision detection

// FNV-1a
header_function
U64 hashString (Char *str)
{
    U64 hash = 0xCBF29CE484222325ULL; // FNV_offset_basis
    for (Size i = 0; str[i] != '\0'; i++) {
        hash = hash ^ (U64)str[i];
        hash = hash * 0x100000001B3ULL; // FNV_prime
    }

    claim(hash != 0);
    return hash;
}

// splitmix64 (xoshiro.di.unimi.it/splitmix64.c)
header_function
U64 hashInteger(U64 x)
{
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;

    claim(x != 0);
    return x;
}

/* Universal Hashing: https://en.wikipedia.org/wiki/Universal_hashing#Avoiding_modular_arithmetic
 *
 * NOTE(naman): Implementation notes
 * w is number of bits in machine word (64 in our case)
 * s is the number of buckets/bins (slots in the hash table) to which the
 *   universe of hashable objects is to be mapped
 * m is log2(s) (=> m = 2^s) and is equal to the number of bits in the final hash
 * a is a random odd positive integer < 2^w (fitting in w bits)
 * b is a random non-negative integer < 2^(w-m) (fitting in (w-m) bits)
 * SEE(naman): https://en.wikipedia.org/wiki/Universal_hashing#Avoiding_modular_arithmetic
 *
 * r is the last random number generated and is just an implementation detail.
 */

typedef struct Hash_Universal {
    U64 a, b, m; /* Hashing constants */

    U64 r; /* Last random number for Universal Hashing */
} Hash_Universal;

header_function
void hashUniversalConstantsUpdate (Hash_Universal *h)
{
    do {
        h->r = randomU64(h->r);
        h->a = h->r;
    } while ((h->a == 0) || ((h->a > 0) && ((h->a & 0x01) != 0x01))); // Make sure that 'a' is odd

    h->r = randomU64(h->r);
    // b should be (64 - m) bits long
    h->b = h->r & (0xFFFFFFFFFFFFFFFFULL >> h->m);
}

header_function
U64 hashUniversal (Hash_Universal h, U64 key)
{
    // NOTE(naman): Remember that we don't want 64-bit hashes, we want hashes < 2^m (s)
    U64 result =  ((h.a * key) + h.b) >> (64 - h.m);
    return result;
}

# endif // NLIB_EXCLUDE_HASHING


/* ===============
 * @String
 */

// TODO(naman): Add a sds-like string library


/* ==================
 * @CString
 */

# if !defined(NLIB_EXCLUDE_CSTRING)

#  if defined(NLIB_NOLIBC)
header_function
Sint strcmp (const Char *s1, const Char *s2)
{
    while(s1[0]) {
        if (s1[0] != s2[0]) break;
        s1++;
        s2++;
    }

    return s1[0] - s2[0];
}

header_function
Sint strncmp (const Char *s1, const Char *s2, Size count)
{
    Sint result = 0;

    for (Size i = 0; i < count; i++)
    {
        if (s1[i] != s2[i]) {
            result = (s1[i] < s2[i]) ? -1 : 1;
            break;
        }
    }

    return result;
}

header_function
Size strlen (const Char *s)
{
    Size len = 0;

    for (Size i = 0; s[i] != '\0'; i++) {
        len++;
    }

    len++;

    return len;
}
#  endif

header_function
B32 streq (const Char *str1, const Char *str2)
{
    B32 result = (strcmp(str1, str2) == 0);
    return result;
}

header_function
B32 strneq (const Char *str1, const Char *str2, Size count)
{
    B32 result = (strncmp(str1, str2, count) == 0);
    return result;
}

header_function
Size strprefix (Char *str, Char *pre)
{
    Size lenpre = strlen(pre);
    Size lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0;
    } else {
        if (strneq(pre, str, lenpre)) {
            return lenpre;
        } else {
            return 0;
        }
    }
}

header_function
Size strsuffix (Char *str, Char *suf)
{
    Size lensuf = strlen(suf);
    Size lenstr = strlen(str);

    if (lenstr < lensuf) {
        return 0;
    } else {
        if (strneq(suf, str + (lenstr - lensuf), lensuf)) {
            return lensuf;
        } else {
            return 0;
        }
    }
}

# endif // NLIB_EXCLUDE_STRING


/* ===============
 * @Memory
 */

# if !defined(NLIB_EXCLUDE_MEMORY)

#  define MEM_MAX_ALIGN_MINUS_ONE (alignof(max_align_t) - 1u)
#  define memAlignUp(p) (((p) + MEM_MAX_ALIGN_MINUS_ONE) & (~ MEM_MAX_ALIGN_MINUS_ONE))
#  define memAlignDown(p) (memAlignUp((p) - MEM_MAX_ALIGN_MINUS_ONE))

// TODO(naman): Add memory allocators back, making sure that they are thread-safe, reentrant


# endif // NLIB_EXCLUDE_MEMORY


/* =========================================================
 * Data Structures =========================================
 * =========================================================
 */

/* ==============
 * Strechy Buffer
 */

/* API ----------------------------------------
 * Size  sbufAdd         (T *ptr, T elem)
 * void  sbufDelete      (T *ptr)
 * void  sbufClear       (T *ptr) : Make everything zero
 * Size  sbufResize      (T *ptr, Size new_size) : Increase size of the underlying buffer
 * T*    sbufOnePastLast (T *ptr) : Return the pointer to the first empty element
 *
 * T*    sbufCreate      (T, void *memory, Size memory_size) : Create a fixed-size buffer
 *
 * Size  sbufSizeof      (T *ptr)
 * Size  sbufElemin      (T *ptr)
 * Size  sbufMaxSizeof   (T *ptr)
 * Size  sbufMaxElemin   (T *ptr)
 *
 * Size  sbufUnsortedRemove (T* ptr, Size index)
 *
 * T*    sbufPrint       (Char *ptr, Char *format, ...)
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_SBUF)

#  if !defined(NLIB_SBUF_MALLOC)
#   if defined(NLIB_MALLOC)
#    define NLIB_SBUF_MALLOC NLIB_MALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Sbuf: NLIB_SBUF_MALLOC required with nolibc"
#   else
#    define NLIB_SBUF_MALLOC malloc
#   endif
#  endif

#  if !defined(NLIB_SBUF_REALLOC)
#   if defined(NLIB_REALLOC)
#    define NLIB_SBUF_REALLOC NLIB_REALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Sbuf: NLIB_SBUF_REALLOC required with nolibc"
#   else
#    define NLIB_SBUF_REALLOC realloc
#   endif
#  endif

#  if !defined(NLIB_SBUF_FREE)
#   if defined(NLIB_FREE)
#    define NLIB_SBUF_FREE NLIB_FREE
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Sbuf: NLIB_SBUF_FREE required with nolibc"
#   else
#    define NLIB_SBUF_FREE free
#   endif
#  endif

typedef struct Sbuf_Header {
    Size cap; // NOTE(naman): Maximum number of elements that can be stored
    Size len; // NOTE(naman): Count of elements actually stored
    void *userdata;
    B64 fixed; // Can't use allocations/reallocation
    alignas(alignof(max_align_t)) Byte buf[];
} Sbuf_Header;

#  define sbuf_GetHeader(sb) ((Sbuf_Header*)(void*)((Byte*)(sb) - offsetof(Sbuf_Header, buf)))

#  define sbuf_Len(sb)         ((sb) ? sbuf_GetHeader(sb)->len : 0U)
#  define sbuf_Cap(sb)         ((sb) ? sbuf_GetHeader(sb)->cap : 0U)

#  define sbufCreate(t, m, s)  sbuf_CreateFixed(sizeof(t), m, s)
// NOTE(naman): In sbufAdd, we check if there is enough space even after growing the sbuf
// to deal with fixed sbufs (since sbuf_Grow will simply return in that case).
// Also, if there wasn;t enough space, we return existing length just to avoid returning void
// which is not allowed by C99.
#  define sbufAdd(sb, ...)     ((sb) = sbuf_Grow((sb), sizeof(*(sb))),  \
                                ((sbuf_Len(sb) + 1) <= sbuf_Cap(sb) ?   \
                                 ((sb)[sbuf_Len(sb)] = (__VA_ARGS__),   \
                                  ((sbuf_GetHeader(sb))->len)++) :      \
                                 (claim(!(sbuf_GetHeader(sb)->fixed) && \
                                        "Fixed sbuf's space full"),     \
                                  (sbuf_GetHeader(sb))->len)))
#  define sbufDelete(sb)       ((sb) && !(sbuf_GetHeader(sb)->fixed) ?  \
                                (NLIB_SBUF_FREE(sbuf_GetHeader(sb)), (sb) = NULL) : \
                                ((sb) ? ((sb) = NULL) : 0))
#  define sbufClear(sb)        ((sb) ?                                  \
                                (memset((sb), 0, sbufSizeof(sb)),       \
                                 sbuf_GetHeader(sb)->len = 0) :         \
                                0)
#  define sbufResize(sb, n)    (((n) > sbufMaxElemin(sb)) ?             \
                                ((sb) = sbuf_Resize(sb, n, sizeof(*(sb)))) : \
                                0)

#  define sbufSizeof(sb)       (sbuf_Len(sb) * sizeof(*(sb)))
#  define sbufElemin(sb)       (sbuf_Len(sb))
#  define sbufMaxSizeof(sb)    (sbuf_Cap(sb) * sizeof(*(sb)))
#  define sbufMaxElemin(sb)    (sbuf_Cap(sb))
#  define sbufOnePastLast(sb)  ((sb) + sbuf_Len(sb))

# define sbufPrint(sb, ...)       ((sb) = sbuf_Print((sb), __VA_ARGS__))
# define sbufPrintSized(sb, s...) ((sb) = sbuf_PrintSized((sb), s, __VA_ARGS__))

# define sbufUnsortedRemove(sb, i) (((sb)[(i)] = (sb)[sbuf_Len(sb) - 1]), \
                                   ((sbuf_GetHeader(sb)->len)--))

#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wcast-align"
#  endif

header_function
void* sbuf_CreateFixed (Size elem_size, void *memory, Size memory_size)
{
    Sbuf_Header *header = (Sbuf_Header *)memory;

    header->cap = (memory_size - sizeof(*header)) / elem_size;
    header->len = 0;
    header->fixed = true;

    return header->buf;
}

header_function
void* sbuf_Grow (void *buf, Size elem_size)
{
    if ((buf != NULL) && sbuf_GetHeader(buf)->fixed) return buf;

    if ((sbuf_Len(buf) + 1) <= sbuf_Cap(buf)) {
        return buf;
    } else {
        Size new_cap = mMax(2 * sbuf_Cap(buf), 4U);

        Size new_size = (new_cap * elem_size) + memAlignUp(sizeof(Sbuf_Header));

        Sbuf_Header *new_header = NULL;

        if (buf != NULL) {
            new_header = (Sbuf_Header *)NLIB_SBUF_REALLOC(sbuf_GetHeader(buf), new_size);
        } else {
            new_header = (Sbuf_Header *)NLIB_SBUF_MALLOC(new_size);
            *new_header = (Sbuf_Header){.len = 0,
                                        .userdata = NULL};
        }

        new_header->cap = new_cap;

        return new_header->buf;
    }
}

header_function
void* sbuf_Resize (void *buf, Size elem_count, Size elem_size)
{
    if ((buf != NULL) && sbuf_GetHeader(buf)->fixed) return buf;

    Size new_cap = elem_count;

    Size new_size = (new_cap * elem_size) + sizeof(Sbuf_Header);

    Sbuf_Header *new_header = NULL;

    if (buf != NULL) {
        new_header = (Sbuf_Header *)NLIB_SBUF_REALLOC(sbuf_GetHeader(buf), new_size);
    } else {
        new_header = (Sbuf_Header *)NLIB_SBUF_MALLOC(new_size);
        *new_header = (Sbuf_Header){.cap = 0,
                                    .len = 0,
                                    .userdata = NULL};
    }

    new_header->cap = new_cap;

    return new_header->buf;
}

#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wformat-nonliteral"
#  endif
#  if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
__attribute__((format(__printf__, 2, 3)))
#  endif
header_function
Char* sbuf_Print (Char *buf, const Char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    size_t cap = sbufMaxElemin(buf) - sbufElemin(buf);
    Size n = 1 + printStringVarArg(sbufOnePastLast(buf), cap, fmt, args);
    va_end(args);

    if ((Size)n > cap) {
        sbufResize(buf, sbufSizeof(buf) + n);
        size_t new_cap = sbufMaxSizeof(buf) - sbufSizeof(buf);
        va_start(args, fmt);
        n = 1 + printStringVarArg(sbufOnePastLast(buf), new_cap, fmt, args);
        va_end(args);
    }

    sbuf_GetHeader(buf)->len += (n - 1);
    return buf;
}

header_function
Char* sbuf_PrintSized (Char *buf, Size size, const Char *fmt, ...)
{
    size_t cap = sbufMaxElemin(buf) - sbufElemin(buf);

    if (size > cap) {
        sbufResize(buf, sbufSizeof(buf) + size);
    }

    size_t new_cap = sbufMaxSizeof(buf) - sbufSizeof(buf);
    va_list args;
    va_start(args, fmt);
    n = 1 + printStringVarArg(sbufOnePastLast(buf), new_cap, fmt, args);
    va_end(args);

    sbuf_GetHeader(buf)->len += (n - 1);
    return buf;
}
#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic pop // -Wformat-nonliteral
#  endif

#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic pop // -Wcast-align
#  endif
# endif // defined(LANG_C)


/* ===============================
 * Intrusive Circular Doubly Linked List
 * Inspired from https://github.com/torvalds/linux/blob/master/include/linux/list.h
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_LIST)
typedef struct List_Node {
    struct List_Node *next, *prev;
} List_Node;

/* To get the node (container struct) holding the linked list node */
#  define listThisNode(node, type, member) containerof(&node,      type, member)
#  define listNextNode(node, type, member) containerof(node->next, type, member)
#  define listPrevNode(node, type, member) containerof(node->prev, type, member)

/* Create and initialize the node statically
 *
 * node n = {
 *     .data = 40,
 *     .list = listNodeInitCompound(n.list),
 * };
 */
#  define listNodeInitCompound(name) {&(name), &(name)}

/* Initialize the node only
 *
 * node *n = malloc(sizeof(*n));
 * n->data = 40;
 * listNodeInit(&n->list);
 */
#  define listNodeInit(ptr) do{(ptr)->next = (ptr); (ptr)->prev = (ptr);}while(0)

header_function
void list__Add (List_Node *new, List_Node *prev, List_Node *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

header_function
void listAddAfter (List_Node *new, List_Node *after_this)
{
    list__Add(new, after_this, after_this->next);
}

header_function
void listAddBefore (List_Node *new, List_Node *before_this)
{
    list__Add(new, before_this->prev, before_this);
}

header_function
void list__RemoveNodeBetween (List_Node * prev, List_Node * next)
{
    next->prev = prev;
    prev->next = next;
}

header_function
void listRemove (List_Node *entry)
{
    list__RemoveNodeBetween(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

header_function
void listRemoveAndInit (List_Node *entry)
{
    list__RemoveNodeBetween(entry->prev, entry->next);
    listNodeInit(entry);
}

header_function
void listReplace(List_Node *old, List_Node *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

header_function
void listReplaceAndInit(List_Node *old, List_Node *new)
{
        listReplace(old, new);
        listNodeInit(old);
}

header_function
void listSwap(List_Node *entry1, List_Node *entry2)
{
    List_Node *pos = entry2->prev;

    listRemove(entry2);
    listReplace(entry1, entry2);
    if (pos == entry1) pos = entry2;
    listAddAfter(entry1, pos);
}

header_function
void listMoveAfter (List_Node *list, List_Node *after_this)
{
    list__RemoveNodeBetween(list->prev, list->next);
    listAddAfter(list, after_this);
}

header_function
void listMoveBefore (List_Node *list, List_Node *before_this)
{
        list__RemoveNodeBetween(list->prev, list->next);
        listAddBefore(list, before_this);
}

header_function
B32 listIsEmpty (List_Node *node)
{
    B32 result = (node->next == node);
    return result;
}

// Splice in a List list, between the Nodes node and node->next
header_function
void list__Splice (List_Node *list, List_Node *node)
{
    List_Node *first = list->next;
    List_Node *last = list->prev;
    List_Node *at = node->next;

    first->prev = node;
    node->next = first;

    last->next = at;
    at->prev = last;
}

// Splice in a List list, between the Nodes node and node->next
header_function
void listSplice (List_Node *list, List_Node *node)
{
    if (!listIsEmpty(list)) list__Splice(list, node);
}

header_function
void listSpliceInit (List_Node *list, List_Node *node)
{
    if (!listIsEmpty(list)) {
        list__Splice(list, node);
        listNodeInit(list);
    }
}

# define LIST_LOOP(idx, node)                                           \
    for (List_Node *(idx) = (node)->next; (idx) != (node); (idx) = (idx)->next)
# define LIST_LOOP_REVERSE(idx, node)                                   \
    for (List_Node *(idx) = (node)->prev; (idx) != (node); pos = pos->prev)
# endif // defined(LANG_C)


/* ==============
 * Interning
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_INTERN)
#  define INTERN_EQUALITY(func_name) B32 func_name (void *a, void *b, Size b_index)
typedef INTERN_EQUALITY(Intern_Equality_Function);

typedef struct Intern {
    struct Intern_List {
        Size *indices;
        U8 *secondary_hashes;
    } lists[256];
} Intern;

header_function
B32 internCheck (Intern *it, U8 hash1, U8 hash2,
                 void *datum, void *data, Intern_Equality_Function *eqf,
                 Size *result)
{
    if (it->lists[hash1].secondary_hashes != NULL) {
        // Our data has probably been inserted already.
        // (or atleast some data with same hash has been inserted :)
        for (Size i = 0;
             i < sbufElemin(it->lists[hash1].secondary_hashes);
             i++) {
            Size index = it->lists[hash1].indices[i];
            if ((it->lists[hash1].secondary_hashes[i] == hash2) && eqf(datum, data, index)) {
                // This is our data, return it
                if (result != NULL) {
                    *result = index;
                }
                return true;
            }
        }

        return false;
    } else {
        return false;
    }
}

header_function
void internData (Intern *it, U8 hash1, U8 hash2, Size index)
{
    sbufAdd(it->lists[hash1].secondary_hashes, hash2);
    sbufAdd(it->lists[hash1].indices, index);
    utTest(sbufElemin(it->lists[hash1].secondary_hashes) == sbufElemin(it->lists[hash1].indices));
}

header_function
U8 internStringPearsonHash (void *buffer, Size len, B32 which)
{
    // NOTE(naman): Pearson's hash for 8-bit hashing
    // https://en.wikipedia.org/wiki/Pearson_hashing
    persistent_value U8 hash_lookup_table1[256] =
        {
            // 0-255 shuffled in any (random) order suffices
            98,    6,   85, 150,  36,  23, 112, 164, 135, 207, 169,   5,  26,  64, 165, 219, // 01
            61,   20,   68,  89, 130,  63,  52, 102,  24, 229, 132, 245,  80, 216, 195, 115, // 02
            90,  168,  156, 203, 177, 120,   2, 190, 188,   7, 100, 185, 174, 243, 162,  10, // 03
            237,  18,  253, 225,   8, 208, 172, 244, 255, 126, 101,  79, 145, 235, 228, 121, // 04
            123, 251,   67, 250, 161,   0, 107,  97, 241, 111, 181,  82, 249,  33,  69,  55, // 05
            59,  153,   29,   9, 213, 167,  84,  93,  30,  46, 94,   75, 151, 114,  73, 222, // 06
            197,  96,  210,  45,  16, 227, 248, 202,  51, 152, 252, 125,  81, 206, 215, 186, // 07
            39,  158,  178, 187, 131, 136,   1,  49,  50,  17, 141,  91,  47, 129,  60,  99, // 08
            154,  35,   86, 171, 105,  34,  38, 200, 147,  58,  77, 118, 173, 246,  76, 254, // 09
            133, 232,  196, 144, 198, 124,  53,   4, 108,  74, 223, 234, 134, 230, 157, 139, // 10
            189, 205,  199, 128, 176,  19, 211, 236, 127, 192, 231,  70, 233,  88, 146,  44, // 11
            183, 201,   22,  83,  13, 214, 116, 109, 159,  32,  95, 226, 140, 220,  57,  12, // 12
            221,  31,  209, 182, 143,  92, 149, 184, 148,  62, 113,  65,  37,  27, 106, 166, // 13
            3,    14,  204,  72,  21,  41,  56,  66,  28, 193,  40, 217,  25,  54, 179, 117, // 14
            238,  87,  240, 155, 180, 170, 242, 212, 191, 163,  78, 218, 137, 194, 175, 110, // 15
            43,  119,  224,  71, 122, 142,  42, 160, 104,  48, 247, 103,  15,  11, 138, 239, // 16
        };

    persistent_value U8 hash_lookup_table2[256] =
        {
            251, 175, 119, 215,  81,  14,  79, 191, 103,  49, 181, 143, 186, 157,   0, 232, // 01
            31,   32,  55,  60, 152,  58,  17, 237, 174,  70, 160, 144, 220,  90,  57, 223, // 02
            59,    3,  18, 140, 111, 166, 203, 196, 134, 243, 124,  95, 222, 179, 197,  65, // 03
            180,  48,  36,  15, 107,  46, 233, 130, 165,  30, 123, 161, 209,  23,  97,  16, // 04
            40,   91, 219,  61, 100,  10, 210, 109, 250, 127,  22, 138,  29, 108, 244,  67, // 05
            207,   9, 178, 204,  74,  98, 126, 249, 167, 116,  34,  77, 193, 200, 121,   5, // 06
            20,  113,  71,  35, 128,  13, 182,  94,  25, 226, 227, 199,  75,  27,  41, 245, // 07
            230, 224,  43, 225, 177,  26, 155, 150, 212, 142, 218, 115, 241,  73,  88, 105, // 08
            39,  114,  62, 255, 192, 201, 145, 214, 168, 158, 221, 148, 154, 122,  12,  84, // 09
            82,  163,  44, 139, 228, 236, 205, 242, 217,  11, 187, 146, 159,  64,  86, 239, // 10
            195,  42, 106, 198, 118, 112, 184, 172,  87,   2, 173, 117, 176, 229, 247, 253, // 11
            137, 185,  99, 164, 102, 147,  45,  66, 231,  52, 141, 211, 194, 206, 246, 238, // 12
            56,  110,  78, 248,  63, 240, 189,  93,  92,  51,  53, 183,  19, 171,  72,  50, // 13
            33,  104, 101,  69,   8, 252,  83, 120,  76, 135,  85,  54, 202, 125, 188, 213, // 14
            96,  235, 136, 208, 162, 129, 190, 132, 156,  38,  47,   1,   7, 254,  24,   4, // 15
            216, 131,  89,  21,  28, 133,  37, 153, 149,  80, 170,  68,   6, 169, 234, 151, // 16
        };

    Char *string = buffer;

    U8 hash = (U8)len;
    for (Size i = 0; i < len; i++) {
        if (which == true) {
            hash = hash_lookup_table1[hash ^ string[i]];
        } else {
            hash = hash_lookup_table2[hash ^ string[i]];
        }
    }

    return hash;
}

typedef struct Intern_String {
    Intern intern;
    Char **strings;
} Intern_String;

header_function
INTERN_EQUALITY(internStringEquality) {
    Char *sa = a;
    Char **ss = b;
    Char *sb = ss[b_index];
    B32 result = streq(sa, sb);
    return result;
}

header_function
Char *internString (Intern_String *is, Char *str)
{
    U8 hash1 = internStringPearsonHash(str, strlen(str), true);
    U8 hash2 = internStringPearsonHash(str, strlen(str), false);

    Size index = 0;

    if (internCheck(&is->intern, hash1, hash2, str, is->strings, internStringEquality, &index)) {
        Char *result = is->strings[index];
        return result;
    } else {
        Size index_new = sbufElemin(is->strings);

        Char *str_new = NULL;
        for (Char *s = str; s[0] != '\0'; s++) {
            sbufAdd(str_new, s[0]);
        }
        sbufAdd(str_new, '\0');

        sbufAdd(is->strings, str_new);

        internData(&is->intern, hash1, hash2, index_new);
        Char *result = is->strings[index_new];
        return result;
    }
}

header_function
Char *internStringCheck (Intern_String *is, Char *str)
{
    U8 hash1 = internStringPearsonHash(str, strlen(str), true);
    U8 hash2 = internStringPearsonHash(str, strlen(str), false);

    Size index = 0;

    if (internCheck(&is->intern, hash1, hash2, str, is->strings, &internStringEquality, &index)) {
        Char *result = is->strings[index];
        return result;
    } else {
        return NULL;
    }
}

#  if defined(BUILD_DEBUG)

header_function
void internStringDebugPrint (Intern_String *is)
{
    for (Size i = 0; i < elemin(is->intern.lists); i++) {
        for (Size j = 0; j < sbufElemin(is->intern.lists[i].indices); j++) {
            report("%s\n", is->strings[is->intern.lists[i].indices[j]]);
        }
    }
}

#  endif

typedef struct Intern_Integer {
    Intern intern;
    U64 *integers;
} Intern_Integer;

header_function
INTERN_EQUALITY(internIntegerEquality) {
    U64 ia = ((U64*)a)[0];
    U64 ib = ((U64*)b)[b_index];

    B32 result = (ia == ib);
    return result;
}

// SEE(naman): https://stackoverflow.com/a/8546542
header_function
U8 internIntegerHash8Bit (U64 key, B32 which)
{
    U8 result = 0;
    U64 q = 0;

    // NOTE(naman): Both q are prime.
    if (which == true) {
        q = 33149;
    } else {
        q = 146519;
    }

    Byte *b = (Byte*)(&key);
    for (Size i = 0; i < sizeof(key); i++) {
        result += (U8)((U64)(b[i]) * q);
    }

    return result;
}

header_function
U64 internInteger (Intern_Integer *ii, U64 num)
{
    U8 hash1 = internIntegerHash8Bit(num, true);
    U8 hash2 = internIntegerHash8Bit(num, false);

    U64 num_copy = num; // Just in case
    Size index = 0;

    if (internCheck(&ii->intern, hash1, hash2,
                    &num_copy, ii->integers, &internIntegerEquality, &index)) {
        return num;
    } else {
        sbufAdd(ii->integers, num);
        internData(&ii->intern, hash1, hash2, sbufElemin(ii->integers));
        return num;
    }
}

header_function
U64 internIntegerCheck (Intern_Integer *ii, U64 num)
{
    U8 hash1 = internIntegerHash8Bit(num, true);
    U8 hash2 = internIntegerHash8Bit(num, false);

    U64 num_copy = num; // Just in case
    Size index = 0;

    if (internCheck(&ii->intern, hash1, hash2,
                    &num_copy, ii->integers, &internIntegerEquality, &index)) {
        return num;
    } else {
        return false;
    }
}
# endif // NLIB_EXCLUDE_INTERN

/* ==============
 * Hash Table
 */

/* API ---------------------------------------------------
 * NOTE(naman): Zero key and value are considered invalid.
 *
 * Hash_Table htCreate (Size min_slots);
 * void       htDelete (Hash_Table ht);
 * U64        htInsert (Hash_Table *ht, U64 key, U64 value);
 * U64        htLookup (Hash_Table *ht, U64 key);
 * U64        htRemove (Hash_Table *ht, U64 key);
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_HASH_TABLE)

typedef struct Hash_Table {
    Hash_Universal univ;
    U64 *keys;
    U64 *values;
    Size slot_count;
    Size slot_filled;
    B64 fixed; // Use fixed memory
    U64 collision_count;
} Hash_Table;

#  if !defined(NLIB_HASH_TABLE_MALLOC)
#   if defined(NLIB_MALLOC)
#    define NLIB_HASH_TABLE_MALLOC NLIB_MALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Hash_Table: NLIB_HASH_TABLE_MALLOC required with nolibc"
#   else
#    define NLIB_HASH_TABLE_MALLOC malloc
#   endif
#  endif

#  if !defined(NLIB_HASH_TABLE_FREE)
#   if defined(NLIB_FREE)
#    define NLIB_HASH_TABLE_FREE NLIB_FREE
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Hash_Table: NLIB_HASH_TABLE_FREE required with nolibc"
#   else
#    define NLIB_HASH_TABLE_FREE free
#   endif
#  endif

header_function
Hash_Table ht_Create (Size slots_atleast, void *memory, Size memory_size)
{
    claim((memory == NULL) || (slots_atleast == 0)); // Can't have fixed memory without fixed slots

    Hash_Table ht = {0};

    if (memory) {
        // NOTE(naman): We are willing to waste some memory because not going overboard
        // is more important. However, the client code should try to provide
        // memory_size such that memory_size/16 is power of 2.
        Size slots = memory_size / (sizeof(*(ht.keys)) + sizeof(*(ht.values)));
        ht.slot_count = mPrevPowerOf2U64(slots);
    } else {
        // NOTE(naman): We try to make the initial hash table a bit larger than expected.
        // The reason for this is that if we have only a small amount of elements, we would
        // just use a associative array instead of a hash table to do the lookup.
        Size slots = mMax(slots_atleast, 64U);
        ht.slot_count = mNextPowerOf2U64(slots);
    }

    ht.univ.m = mLog2U64(ht.slot_count);
    hashUniversalConstantsUpdate(&ht.univ);

    if (memory) {
        ht.fixed = true;
        ht.keys = memory;
        ht.values = (void*)((Byte*)memory + ht.slot_count * sizeof((ht.keys)));
    } else {
        ht.keys     = NLIB_HASH_TABLE_MALLOC(ht.slot_count * sizeof(*(ht.keys)));
        ht.values   = NLIB_HASH_TABLE_MALLOC(ht.slot_count * sizeof(*(ht.values)));
    }

    memset(ht.keys,   0, ht.slot_count * sizeof(*(ht.keys)));
    memset(ht.values, 0, ht.slot_count * sizeof(*(ht.values)));

    return ht;
}

header_function
Hash_Table htCreate (void)
{
    Hash_Table ht = ht_Create(0, NULL, 0);
    return ht;
}

header_function
Hash_Table htCreateWithSlots (Size slots)
{
    Hash_Table ht = ht_Create(slots, NULL, 0);
    return ht;
}

header_function
Hash_Table htCreateFixed (void *memory, Size memory_size)
{
    Hash_Table ht = ht_Create(0, memory, memory_size);
    return ht;
}

header_function
void htDelete (Hash_Table ht)
{
    if (ht.fixed) {
        claim(false && "Trying to free a fixed-memory hash table");
    } else {
        NLIB_HASH_TABLE_FREE(ht.keys);
        NLIB_HASH_TABLE_FREE(ht.values);
    }
}

header_function
B32 ht_LinearProbeSearch (Hash_Table *ht, U64 key, U64 hash, Size *value)
{
    Size index = 0;
    B32 found = false;

    for (Size i = 0; !found && (i < ht->slot_count); ++i) {
        index = (hash + i) % (ht->slot_count);
        if (ht->keys[index] == key) {
            found = true;
            break;
        }
    }

    *value = index;

    return found;
}

header_function
U64 ht_LinearProbeInsertion (Hash_Table *ht,
                             U64 hash, U64 key, U64 value)
{
    U64 result_value = value;

    for (Size i = 0; i < ht->slot_count; ++i) {
        Size index = (hash + i) % (ht->slot_count);
        if ((ht->keys[index] == key) || (ht->values[index] == 0))  {
            result_value = ht->values[index];
            ht->keys[index] = key;
            ht->values[index] = value;
            break;
        } else {
            ht->collision_count++;
        }
    }

    return result_value;
}

header_function
U64 htInsert (Hash_Table *ht, U64 key, U64 value)
{
    // FIXME(naman): Figure out the correct condition on which to resize on.
    const Uint max_collisions_allowed = (Uint)ht->slot_count;

    if ((key == 0) || (value == 0)) return 0;

    if (ht->fixed) {
        if (ht->slot_filled >= ht->slot_count) {
            return 0;
        }
    } else if ((ht->collision_count > max_collisions_allowed) ||
               ((2 * ht->slot_filled) >= ht->slot_count)) {
        Size slot_count_prev = ht->slot_count;
        U64 *keys   = ht->keys;
        U64 *values = ht->values;

        if ((2 * ht->slot_filled) >= ht->slot_count) { // Only increase size if need be
            ht->univ.m = ht->univ.m + 1;
            ht->slot_count = 1ULL << (ht->univ.m);
            hashUniversalConstantsUpdate(&(ht->univ));
        }

        ht->keys   = NLIB_HASH_TABLE_MALLOC(ht->slot_count * sizeof(*(ht->keys)));
        ht->values = NLIB_HASH_TABLE_MALLOC(ht->slot_count * sizeof(*(ht->values)));

        memset(ht->keys,   0, ht->slot_count * sizeof(*(ht->keys)));
        memset(ht->values, 0, ht->slot_count * sizeof(*(ht->values)));

        for (Size i = 0; i < slot_count_prev; ++i) {
            U64 key_i   = keys[i];
            U64 value_i = values[i];
            if (value_i != 0) {
                U64 hash_new = hashUniversal(ht->univ, key_i);
                ht_LinearProbeInsertion(ht, hash_new, key_i, value_i);
            }
        }

        NLIB_HASH_TABLE_FREE(keys);
        NLIB_HASH_TABLE_FREE(values);

        ht->collision_count = 0;
    }

    U64 hash = hashUniversal(ht->univ, key);
    U64 result_value = ht_LinearProbeInsertion(ht, hash, key, value);
    if (result_value == 0) {
        ht->slot_filled++;
    }

    return result_value;
}

header_function
U64 htLookup (Hash_Table *ht, U64 key)
{
    if (key == 0) return 0;

    Size location = 0;
    U64 result_value = 0;

    U64 hash = hashUniversal(ht->univ, key);
    if (ht_LinearProbeSearch(ht, key, hash, &location)) {
        result_value = ht->values[location];
    }

    return result_value;
}

header_function
U64 htRemove (Hash_Table *ht, U64 key)
{
    if (key == 0) return 0;

    Size location = 0;
    U64 result_value = 0;

    U64 hash = hashUniversal(ht->univ, key);
    if (ht_LinearProbeSearch(ht, key, hash, &location)) {
        result_value = ht->values[location];
    }

    if (result_value != 0) {
        ht->values[location] = 0;
        ht->keys[location] = 0;
        ht->slot_filled -= 1;
    }

    return result_value;
}

# endif // NLIB_EXCLUDE_HASH_TABLE

/* ==============
 * Map
 * (also works as a sparse-set)
 */

/* API ----------------------------------------
 * void  mapInsert     (T *ptr, U64 key, T value)
 * void  mapRemove     (T *ptr, U64 key)
 * B32   mapExists     (T *ptr, U64 key)
 * T     mapLookup     (T *ptr, U64 key)
 * T*    mapGetRef     (T *ptr, U64 key)
 * void  mapDelete     (T *ptr)
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_MAP)
typedef struct Map_Userdata {
    Hash_Table table;
    Size *free_list;
} Map_Userdata;

#  if !defined(NLIB_MAP_MALLOC)
#   if defined(NLIB_MALLOC)
#    define NLIB_MAP_MALLOC NLIB_MALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Map: NLIB_MAP_MALLOC required with nolibc"
#   else
#    define NLIB_MAP_MALLOC malloc
#   endif
#  endif

#  if !defined(NLIB_MAP_FREE)
#   if defined(NLIB_FREE)
#    define NLIB_MAP_FREE NLIB_FREE
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Map: NLIB_MAP_FREE required with nolibc"
#   else
#    define NLIB_MAP_FREE free
#   endif
#  endif

#  define mapInsert(_map, _key, _value) do {                            \
        Sbuf_Header *shdr = NULL;                                       \
        if ((_map) == NULL) {                                           \
            (_map) = sbuf_Grow((_map),                                  \
                               sizeof(*(_map)));                        \
            shdr = sbuf_GetHeader(_map);                                \
            (shdr->len)++;                                              \
            shdr->userdata = NLIB_MAP_MALLOC(sizeof(Map_Userdata));     \
            *(Map_Userdata*)(shdr->userdata) = (Map_Userdata){0};       \
            ((Map_Userdata*)shdr->userdata)->table = htCreate();        \
        } else {                                                        \
            shdr = sbuf_GetHeader((_map));                              \
        }                                                               \
                                                                        \
        Size insertion_index = 0;                                       \
                                                                        \
        if ((_key) != 0) {                                              \
            if (sbufElemin(((Map_Userdata*)shdr->userdata)->free_list) > 0) { \
                (_map)[((Map_Userdata*)shdr->userdata)->free_list[0]] = (_value); \
                insertion_index = ((Map_Userdata*)shdr->userdata)->free_list[0]; \
                sbufUnsortedRemove(((Map_Userdata*)shdr->userdata)->free_list, 0); \
            } else {                                                    \
                sbufAdd((_map), (_value));                              \
                insertion_index = sbufElemin((_map)) - 1;               \
            }                                                           \
                                                                        \
            htInsert(&((Map_Userdata*)shdr->userdata)->table, (_key), insertion_index); \
        }                                                               \
    } while (0)

header_function
B32 mapExists (void *map, U64 key) {
    if (map != NULL) {
        Sbuf_Header *shdr = sbuf_GetHeader(map);
        Size index = htLookup(&((Map_Userdata*)shdr->userdata)->table, key);
        if (index != 0) {
            return true;
        }
    }
    return false;
}

#  define mapLookup(_map, _key) ((_map)[(htLookup(&((Map_Userdata*)(sbuf_GetHeader(_map))->userdata)->table, \
                                                (_key)))])
#  define mapGetRef(_map, _key) (&mapLookup((_map), (_key)))

#  define mapRemove(_map, _key) do {                                    \
        if ((_map) != NULL) {                                           \
            Sbuf_Header *shdr = sbuf_GetHeader(_map);                   \
            Size index = htLookup(&((Map_Userdata*)shdr->userdata)->table, \
                                  (_key));                              \
            sbufAdd(((Map_Userdata*)shdr->userdata)->free_list, index); \
            htRemove(&((Map_Userdata*)shdr->userdata)->table, (_key));  \
        }                                                               \
    } while (0)

#  define mapDelete(_map) do {                                  \
        Sbuf_Header *shdr = sbuf_GetHeader(_map);               \
        htDelete(((Map_Userdata*)shdr->userdata)->table);       \
        sbufDelete(((Map_Userdata*)shdr->userdata)->free_list); \
        NLIB_MAP_FREE(shdr->userdata);                          \
        sbufDelete(_map);                                       \
    } while (0)
# endif // LANG_C

/* ==============
 * Concurrent Ring (Lock-based Multi-producer Multi-consumer)
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_RING_LOCKED)

#  if !defined(NLIB_RING_LOCKED_MALLOC)
#   if defined(NLIB_MALLOC)
#    define NLIB_RING_LOCKED_MALLOC NLIB_MALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Ring_Locked: NLIB_RING_LOCKED_MALLOC required with nolibc"
#   else
#    define NLIB_RING_LOCKED_MALLOC malloc
#   endif
#  endif

#  if !defined(NLIB_RING_LOCKED_FREE)
#   if defined(NLIB_FREE)
#    define NLIB_RING_LOCKED_FREE NLIB_FREE
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Ring_Locked: NLIB_RING_LOCKED_FREE required with nolibc"
#   else
#    define NLIB_RING_LOCKED_FREE free
#   endif
#  endif

#  if defined(OS_LINUX)

typedef struct Ring_Locked__Head {
    sem_t fill_count; // How many are filled?
    sem_t empty_count; // How many are empty?
    pthread_mutex_t buffer_lock;
    Size buffer_size;
    Size buffer_write_cursor;
    Size buffer_read_cursor;
    alignas(alignof(max_align_t)) Byte buffer[];
} Ring_Locked__Head;

#   define ringLockedCreate(type, size) ringLocked__Create(sizeof(type), size)

header_function
void* ringLocked__Create (Size elemsize, Size buffersize)
{
    Size ring_size = elemsize * buffersize;
    Size header_size = memAlignUp(sizeof(Ring_Locked__Head));
    Size total_size = header_size + ring_size;

    Ring_Locked__Head *head = NLIB_RING_LOCKED_MALLOC(total_size);
    *head = (Ring_Locked__Head){0};

    sem_init(&head->fill_count, 0, 0);
    sem_init(&head->empty_count, 0, (Uint)buffersize);
    pthread_mutex_init(&head->buffer_lock, NULL);

    head->buffer_size = buffersize;

    void *result = (Byte*)(void*)head + header_size;
    return result;
}

#   define ringLocked__GetHead(r) containerof(r, Ring_Locked__Head, buffer)

#   define ringLockedPush(ring, elem) do {                         \
        Ring_Locked__Head *head = ringLocked__GetHead(ring);    \
                                                                \
        sem_wait(&head->empty_count);                           \
        pthread_mutex_lock(&head->buffer_lock);                 \
                                                                \
        ring[head->buffer_write_cursor] = elem;                 \
        head->buffer_write_cursor++;                            \
        head->buffer_write_cursor %= head->buffer_size;         \
                                                                \
        pthread_mutex_unlock(&head->buffer_lock);               \
        sem_post(&head->fill_count);                            \
    } while (0)

#   define ringLockedPull(ring, dest) do {                                 \
        Ring_Locked__Head *head = ringLocked__GetHead(ring);            \
                                                                        \
        sem_wait(&head->fill_count);                                    \
        pthread_mutex_lock(&head->buffer_lock);                         \
                                                                        \
        memcpy(dest, ring + head->buffer_read_cursor, sizeof(*ring));   \
        head->buffer_read_cursor++;                                     \
        head->buffer_read_cursor %= head->buffer_size;                  \
                                                                        \
        pthread_mutex_unlock(&head->buffer_lock);                       \
        sem_post(&head->empty_count);                                   \
    } while (0)

// FIXME(naman): Can this function be called if some threads are stuck inside Pull?
header_function
void ringLockedDelete (void *ring)
{
    Ring_Locked__Head *head = ringLocked__GetHead(ring);
    NLIB_RING_LOCKED_FREE(head);
}

#  elif defined(OS_WINDOWS)
// TODO(naman): This
#  endif
# endif // LANG_C


/* ==============
 * Concurrent Queue (Lock-based Unbounded Multi-producer Multi-consumer)
 */

# if defined(LANG_C) && !defined(NLIB_EXCLUDE_QUEUE_LOCKED)

#  if !defined(NLIB_QUEUE_LOCKED_MALLOC)
#   if defined(NLIB_MALLOC)
#    define NLIB_QUEUE_LOCKED_MALLOC NLIB_MALLOC
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Queue_Locked: NLIB_QUEUE_LOCKED_MALLOC required with nolibc"
#   else
#    define NLIB_QUEUE_LOCKED_MALLOC malloc
#   endif
#  endif

#  if !defined(NLIB_QUEUE_LOCKED_FREE)
#   if defined(NLIB_FREE)
#    define NLIB_QUEUE_LOCKED_FREE NLIB_FREE
#   elif defined(NLIB_NOLIBC)
#    error "nlib: Queue_Locked: NLIB_QUEUE_LOCKED_FREE required with nolibc"
#   else
#    define NLIB_QUEUE_LOCKED_FREE free
#   endif
#  endif

#  if defined(OS_LINUX)

typedef List_Node Queue_Locked_Entry;

typedef struct Queue_Locked__Head {
    pthread_mutex_t list_lock;
    pthread_cond_t  list_filled_signal;
    Queue_Locked_Entry list;
} Queue_Locked__Head;

header_function
Queue_Locked_Entry* queueLockedCreate (void)
{
    Queue_Locked__Head *qh = calloc(1, sizeof(*qh));
    Queue_Locked_Entry *qe = &qh->list;
    listNodeInit(qe);

    pthread_mutex_init(&qh->list_lock,          NULL);
    pthread_cond_init (&qh->list_filled_signal, NULL);

    return qe;
}

#   define queueLocked__GetHead(q) containerof(q, Queue_Locked__Head, list)

// NOTE(naman): When a thread in pthread_cond_wait is cancelled, it is first
// brought out of the wait, meaning that it first regains the mutex. This
// cleanup function will make sure that in case of cancellation, the
// thread will unlock the mutex so that no other thread waiting on the
// same condition variable will deadlock.
header_function
void queueLocked__CondWaitCleaner (void *arg)
{
    pthread_mutex_unlock(arg);
}

#   define queueLockedEnqueue(queue, qiptr) do {                   \
        Queue_Locked__Head *head = queueLocked__GetHead(queue); \
                                                                \
        pthread_mutex_lock(&head->list_lock);                   \
                                                                \
        listAddBefore(qiptr, &head->list);                      \
                                                                \
        pthread_cond_broadcast(&head->list_filled_signal);      \
        pthread_mutex_unlock(&head->list_lock);                 \
    } while (0)

#   define queueLockedDequeue(queue, qiptr) do {                   \
        Queue_Locked__Head *head = queueLocked__GetHead(queue); \
                                                                \
        pthread_mutex_lock(&head->list_lock);                   \
                                                                \
        while (listIsEmpty(&head->list)) {                      \
            pthread_cleanup_push(&queueLocked__CondWaitCleaner,  \
                                 &head->list_lock);             \
            pthread_cond_wait(&head->list_filled_signal,        \
                              &head->list_lock);                \
            pthread_cleanup_pop(0);                             \
        }                                                       \
                                                                \
        Queue_Locked_Entry *que = head->list.next;              \
        listRemoveAndInit(que);                                 \
        qiptr = que;                                            \
                                                                \
        pthread_mutex_unlock(&head->list_lock);                 \
    } while (0)

// FIXME(naman): Can this function be called if some threads are stuck inside Pull?
header_function
void queueLockedDelete (void *queue)
{
    Queue_Locked__Head *head = queueLocked__GetHead(queue);
    NLIB_QUEUE_LOCKED_FREE(head);
}

#  elif defined(OS_WINDOWS)
// TODO(naman): This
#  endif
# endif // LANG_C

#define NLIB_H_INCLUDE_GUARD
#endif // NLIB_H_INCLUDE_GUARD
