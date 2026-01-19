#pragma once
#include <time.h>
#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
#include <string>
#endif

/* ================= USER CONFIG ================= */
#ifndef LOG_ENABLE_TIMING
#define LOG_ENABLE_TIMING 1
#endif

#ifndef LOG_OUTPUT_TREE
#define LOG_OUTPUT_TREE 1
#endif

#ifndef LOG_OUTPUT_CSV
#define LOG_OUTPUT_CSV 1
#endif
/* =============================================== */

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct log_timer
    {
        const char *name;
        const char *category;
        struct timespec start;
        int depth;
    } log_timer_t;

    /* ---------- Configurable outputs ---------- */
    static FILE *time_it_tree_file = NULL;
    static FILE *time_it_csv_file = NULL;
    static int time_it_enable_tree = LOG_OUTPUT_TREE;
    static int time_it_enable_csv = LOG_OUTPUT_CSV;

    static inline void time_it_set_tree_file(FILE *f) { time_it_tree_file = f; }
    static inline void time_it_set_csv_file(FILE *f) { time_it_csv_file = f; }
    static inline void time_it_set_tree(int enable) { time_it_enable_tree = enable; }
    static inline void time_it_set_csv(int enable) { time_it_enable_csv = enable; }

    /* ---------- Helper: print elapsed in scientific notation, exponent multiple of 3 ---------- */
    static inline void print_scientific(FILE *out, long elapsed_ns)
    {
        double value = (double)elapsed_ns;
        int exp3 = 0;

        if (value == 0)
        {
            fprintf(out, "0.0e0");
            return;
        }

        double log10v = log10(value);
        exp3 = ((int)floor(log10v / 3.0)) * 3;
        double scaled = value / pow(10.0, exp3);
        fprintf(out, "%.3fe%d", scaled, exp3);
    }

    /* ---------- Logging function ---------- */
    static inline void log_timer_end(log_timer_t *t, long elapsed_ns)
    {
        FILE *tree_out = time_it_tree_file ? time_it_tree_file : stderr;
        FILE *csv_out = time_it_csv_file ? time_it_csv_file : stderr;

        if (time_it_enable_tree)
        {
            for (int i = 0; i < t->depth; ++i)
                fputs("    ", tree_out);
            fprintf(tree_out, "%s [%s]: ", t->name, t->category);
            print_scientific(tree_out, elapsed_ns);
            fputs("\n", tree_out);
        }

        if (time_it_enable_csv)
        {
            fprintf(csv_out, "%d,%s,%s,", t->depth, t->name, t->category);
            print_scientific(csv_out, elapsed_ns);
            fputs("\n", csv_out);
        }
    }

#ifdef __cplusplus
}
#endif

/* ---------- Function name capture ---------- */
#if defined(__cplusplus)
#if defined(__GNUC__) || defined(__clang__)
#define LOG_FUNC_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define LOG_FUNC_NAME __FUNCSIG__
#else
#define LOG_FUNC_NAME __func__
#endif
#else
#define LOG_FUNC_NAME __func__
#endif

/* ---------- Zero-cost disable ---------- */
#if !LOG_ENABLE_TIMING
#define TIME_IT(cat) ((void)0)
#define SET_TIME_IT_OUTPUT_FILE_BASENAME(name) ((void)0)
#else

/* ---------- Thread-local depth ---------- */
#if defined(__cplusplus)
static thread_local int log_depth = 0;
#else
static _Thread_local int log_depth = 0;
#endif

/* ---------- C++ RAII ---------- */
#ifdef __cplusplus

class TimeItTimer
{
public:
    TimeItTimer(const char *name, const char *category)
    {
        t_.name = name;
        t_.category = category;
        t_.depth = log_depth++;
        clock_gettime(CLOCK_MONOTONIC, &t_.start);
    }
    TimeItTimer(const std::string &name, const std::string &category) : TimeItTimer(name.c_str(), category.c_str()) {}
    TimeItTimer(const std::string &name, const char *category) : TimeItTimer(name.c_str(), category) {}
    TimeItTimer(const char *name, const std::string &category) : TimeItTimer(name, category.c_str()) {}

    ~TimeItTimer()
    {
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        long elapsed_ns =
            (end.tv_sec - t_.start.tv_sec) * 1000000000L +
            (end.tv_nsec - t_.start.tv_nsec);

        log_timer_end(&t_, elapsed_ns); // log BEFORE decreasing depth
        log_depth--;
    }

private:
    log_timer_t t_;
};

#define TIME_IT(cat) TimeItTimer __time_it__(LOG_FUNC_NAME, cat)

/* ---------- C++ RAII for basename files ---------- */
class TimeItBasenameFiles
{
public:
    TimeItBasenameFiles(const char *basename)
    {
        char tree_path[512];
        char csv_path[512];
        snprintf(tree_path, sizeof(tree_path), "%s.log", basename);
        snprintf(csv_path, sizeof(csv_path), "%s.csv", basename);

        tree_f_ = fopen(tree_path, "a");
        csv_f_ = fopen(csv_path, "a");

        if (!tree_f_)
            tree_f_ = stderr;
        if (!csv_f_)
            csv_f_ = stderr;

        time_it_tree_file = tree_f_;
        time_it_csv_file = csv_f_;
    }
    TimeItBasenameFiles(const std::string &basename) : TimeItBasenameFiles((char *)basename.c_str()) {}
    ~TimeItBasenameFiles()
    {
        if (tree_f_ && tree_f_ != stderr)
            fclose(tree_f_);
        if (csv_f_ && csv_f_ != stderr)
            fclose(csv_f_);
        time_it_tree_file = stderr;
        time_it_csv_file = stderr;
    }

private:
    FILE *tree_f_;
    FILE *csv_f_;
};

#define SET_TIME_IT_OUTPUT_FILE_BASENAME(name) TimeItBasenameFiles __time_it_files__(name)

#elif defined(__GNUC__) || defined(__clang__)

/* ---------- C cleanup ---------- */
static inline void log_timer_cleanup(log_timer_t *t)
{
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_ns =
        (end.tv_sec - t->start.tv_sec) * 1000000000L +
        (end.tv_nsec - t->start.tv_nsec);

    log_timer_end(t, elapsed_ns); // BEFORE decreasing depth
    log_depth--;
}

#define TIME_IT(cat)                                 \
    log_timer_t __time_it__                          \
        __attribute__((cleanup(log_timer_cleanup))); \
    __time_it__.name = LOG_FUNC_NAME;                \
    __time_it__.category = cat;                      \
    __time_it__.depth = log_depth++;                 \
    clock_gettime(CLOCK_MONOTONIC, &__time_it__.start)

/* ---------- C basename files cleanup ---------- */
static inline void time_it_file_cleanup(FILE **f)
{
    if (f && *f && *f != stderr)
        fclose(*f);
}

#define SET_TIME_IT_OUTPUT_FILE_BASENAME(basename)                         \
    FILE *__time_it_tree__ __attribute__((cleanup(time_it_file_cleanup))); \
    FILE *__time_it_csv__ __attribute__((cleanup(time_it_file_cleanup)));  \
    char __tree_path__[512];                                               \
    char __csv_path__[512];                                                \
    snprintf(__tree_path__, sizeof(__tree_path__), "%s.log", basename);    \
    snprintf(__csv_path__, sizeof(__csv_path__), "%s.csv", basename);      \
    __time_it_tree__ = fopen(__tree_path__, "a");                          \
    __time_it_csv__ = fopen(__csv_path__, "a");                            \
    time_it_tree_file = __time_it_tree__ ? __time_it_tree__ : stderr;      \
    time_it_csv_file = __time_it_csv__ ? __time_it_csv__ : stderr;

#else
#error "TIME_IT requires C++ or GCC/Clang cleanup support"
#endif

#endif /* LOG_ENABLE_TIMING */
