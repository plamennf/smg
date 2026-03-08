#pragma once

#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>
#define DoDebugBreak() __debugbreak()
#define COMPILER_MSVC
#define HAS_SUPPORT_FOR_INTRINSICS
#endif

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef double   f64;
typedef float    f32;

// Copy-paste from https://gist.github.com/andrewrk/ffb272748448174e6cdb4958dae9f3d8
// Defer macro/thing.

#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

template<typename T>
struct ExitScope {
    T lambda;
    ExitScope(T lambda):lambda(lambda){}
    ~ExitScope(){lambda();}
    ExitScope(const ExitScope&);
  private:
    ExitScope& operator =(const ExitScope&);
};
 
class ExitScopeHelp {
  public:
    template<typename T>
        ExitScope<T> operator+(T t){ return t;}
};
 
#define defer const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

#define ArrayCount(arr) (sizeof(arr)/sizeof((arr)[0]))
#define Max(x, y) ((x) > (y) ? (x) : (y))
#define Min(x, y) ((x) < (y) ? (x) : (y))

#define Bit(x) (1 << (x))
#define Square(x) ((x)*(x))

#if defined(BUILD_DEBUG) || defined(BUILD_RELEASE)
#define ASSERTIONS_ENABLED
#endif

#ifdef ASSERTIONS_ENABLED
#define Assert(expr) if (expr) {} else { DoDebugBreak(); }
#else
#define Assert(expr)
#endif

// From https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators
#define IsPowerOfTwo(x) ((x != 0) && ((x & (x - 1)) == 0))

#define Kilobytes(x) ((x)*1024LL)
#define Megabytes(x) (Kilobytes(x)*1024LL)
#define Gigabytes(x) (Megabytes(x)*1024LL)
#define Terabytes(x) (Gigabytes(x)*1024LL)


// From https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/PlatformDetection.h

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection

// Strings
int string_length(char *s);
char *copy_string(char *s);
bool strings_match(char *a, char *b);

bool is_end_of_line(char c);
bool is_space(char c);

char *find_character_from_right(char *s, char c);
char *find_character_from_left(char *s, char c);

// Logging
void init_logging(char *log_filepath = "log.txt");
void shutdown_logging();

void logprintf(char *fmt, ...);

// Memory Arena


// From https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators

#define MEMORY_ARENA_DEFAULT_ALIGNMENT (2 * sizeof(void *))

struct Memory_Arena {
    s64 size;
    s64 occupied;
    u8 *data;
};

void ma_init(Memory_Arena *arena, s64 size);
void ma_reset(Memory_Arena *arena);

#define MaAllocStruct(arena, Type, ...) (Type *)ma_alloc(arena, sizeof(Type), __VA_ARGS__)
#define MaAllocArray(arena, Type, Count, ...) (Type *)ma_alloc(arena, (Count) * sizeof(Type), __VA_ARGS__)

void *ma_alloc(Memory_Arena *arena, s64 size, bool zero_memory = true, s64 alignment = MEMORY_ARENA_DEFAULT_ALIGNMENT);

// Temporary storage

void init_temporary_storage(s64 size);
void reset_temporary_storage();

bool is_temporary_storage_initialized();

s64 get_temporary_storage_mark();
void set_temporary_storage_mark(s64 mark);

#define TAllocStruct(Type, ...) (Type *)talloc(sizeof(Type), __VA_ARGS__)
#define TAllocArray(Type, Count, ...) (Type *)talloc((Count) * sizeof(Type), __VA_ARGS__)

void *talloc(s64 size, s64 alignment = MEMORY_ARENA_DEFAULT_ALIGNMENT);

// Misc
void clamp(int *v, int a, int b);
void clamp(float *v, float a, float b);
u64 round_to_next_power_of_2(u64 v);

bool file_exists(char *filepath);
