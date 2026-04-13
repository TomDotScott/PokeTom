#ifndef ASSERT_H
#define ASSERT_H

#define DEBUG_BREAK() do { __debugbreak(); } while(0);
#define FORCE_CRASH() do { *((volatile int*)0) = 0; } while(0);

#define ASSERT(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::fprintf(stderr,                                               \
                "[[ASSERT]] %s:%d\n"                                           \
                "Condition failed: %s\n",                                      \
                __FILE__, __LINE__, #cond);                                    \
            std::fflush(stderr);                                               \
            DEBUG_BREAK();                                                     \
        }                                                                      \
    } while (0)

#define ASSERT_MSG(cond, msg, ...)                                             \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::fprintf(stderr,                                               \
                "[[ASSERT]] %s:%d\n"                                           \
                "Condition failed: %s\n"                                       \
                "msg: " msg "\n\n",                                            \
                __FILE__, __LINE__, #cond, ##__VA_ARGS__);                     \
            std::fflush(stderr);                                               \
            DEBUG_BREAK();                                                     \
        }                                                                      \
    } while (0)

#endif
