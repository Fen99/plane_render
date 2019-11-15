project(threadpool)
set(THREADPOOL_SRC
    src/threadpool.cpp
)
set(THREADPOOL_DEPENDENCES common)

build_static_lib(THREADPOOL_SRC THREADPOOL_DEPENDENCES)