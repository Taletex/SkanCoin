#ifndef DEBUG_FLAG

#define DEBUG_FLAG 1  // Todo: cercare di passare una variabile che viene indicata nel cmakelist

#define DEBUG_INFO(msg) \
    fprintf(stderr, "> Debug: %s in file %s riga %d\n", __func__, __FILE__, __LINE__); \
    if(msg != "") fprintf(stderr, "  %s\n", msg);

#endif
