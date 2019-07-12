#ifndef H_WUTILS_H
#define H_WUTILS_H

#ifdef __cplusplus
extern "C" {
#endif
#undef assert

#ifdef NDEBUG
// TODO check if this is ok to remove the calls completely from the binary
#define assert(__e) (0)
#define log_debug(__s) (0)
#define log_warn(__s) (0)
#define log_error(__s) (0)

#else /* NDEBUG*/ 

#if MYNEWT_VAL(OS_CRASH_FILE_LINE)
    #define assert(__e) ((__e) ? (void)0 : wassert_fn(__FILE__, __LINE__))
#else
    #define assert(__e) ((__e) ? (void)0 : wassert_fn(NULL, NULL))
#endif
#define log_debug log_debug_fn
#define log_warn log_warn_fn
#define log_error log_error_fn

#endif /* NDEBUG */


// Utility functions tried and tested
void wassert_fn(const char* f, const char* l);
// logging
void log_debug_fn(const char* sl, ...);
void log_warn_fn(const char* sl, ...);
void log_error_fn(const char* sl, ...);

#ifdef __cplusplus
}
#endif

#endif  /* H_WUTILS_H */
