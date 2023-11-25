//
// Created by zrk on 2023/11/10.
//

#include "tick.h"
#include <cstdio>
#include <algorithm>
#include <immintrin.h>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <iostream>


void throw_err(const char* message)
{
    printf("#################\n");
    printf("# %s #", message);
    printf("#################\n");
    exit(1);
}


float randf()
{
    // return rand() % 10;
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.0f - 0.5f; // NOLINT(*-msc50-cpp)
}


void print(const float* const M, size_t N)
{
    char paddedString[256];
    for (size_t i = 0; i < N; i++) {
        if (i == 0) printf("[ "); else printf("  ");
        for (size_t j = 0; j < N; j++) {
            auto m = M[i*N + j];
            sprintf(paddedString, "%.4f", m);
            printf("%s ", paddedString);
        }
        if (i == N-1) printf(" ]");
        printf("\n");
    }
}


void naive_mm(size_t N, bool bPrint=false, size_t NTest=100)
{
    auto* A = new float[N*N];
    auto* B = new float[N*N];
    auto* C = new float[N*N];
    for (size_t i = 0; i < N*N; i++) {
        A[i] = randf();
        B[i] = randf();
        C[i] = 0.0f;
    }
    auto fn = [&]() -> void {
        // FIXME: C[i] = 0.0f;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                for (size_t k = 0; k < N; k++) {
                    auto a = A[i*N + k];
                    auto b = B[k*N + j];
                    C[i*N + j] += a*b;
                }
            }
        }
    };
    fn();

    if (!bPrint) {
        Tick tick;
        tick.Start("naive_mm");
        for (size_t nt = 0; nt < NTest; nt++) {
            fn();
        }
        tick.End(1.0/(double)NTest);
    } else {
        print(A, N); print(B, N); print(C, N);
    }

    delete[] A;
    delete[] B;
    delete[] C;
}


template<size_t TILE=16>  // cache line size = 64 = 16 * sizeof(float)
void tiled_only_mm(size_t N, bool bPrint=false, size_t NTest=100)
{
    if ((N % TILE) != 0)
        throw_err("(N % TILE) != 0");

    auto* A = new float[N*N];
    auto* B = new float[N*N];
    auto* C = new float[N*N];
    for (size_t i = 0; i < N*N; i++) {
        A[i] = randf();
        B[i] = randf();
        C[i] = 0.0f;
    }

    auto fn = [&]() -> void {
        // FIXME: C[i] = 0.0f;
        for (size_t I = 0; I < N; I+=TILE) {
            for (size_t K = 0; K < N; K+=TILE) {          //  SWAP  (K, J)
                for (size_t J = 0; J < N; J+=TILE) {      //  SWAP  (K, J)

                    // C_IJ = A_IK * B_KJ
                    for (size_t i = 0; i < TILE; i++) {
                        for (size_t k = 0; k < TILE; k++) {
                            for (size_t j = 0; j < TILE; j++) {
                                C[(I+i)*N + J+j] += A[(I+i)*N + K+k] * B[(K+k)*N + J+j];
                            }
                        }
                    }

                }
            }
        }
    };
    fn();

    if (!bPrint) {
        Tick tick;
        tick.Start("tiled_only_mm");
        for (size_t nt = 0; nt < NTest; nt++) {
            fn();
        }
        tick.End(1.0/(double)NTest);
    } else {
        print(A, N); print(B, N); print(C, N);
    }

    delete[] A;
    delete[] B;
    delete[] C;
}


void simd_only_mm_256(size_t N, bool bPrint=false, size_t NTest=100)
{
#ifdef AVX2_ENABLED
    const size_t ALIGN = 8;
    if ((N % ALIGN) != 0)
        throw_err("(N % ALIGN) != 0");

    auto* A = (float*)_aligned_malloc(N*N*sizeof(float), sizeof(float)*ALIGN);
    auto* B = (float*)_aligned_malloc(N*N*sizeof(float), sizeof(float)*ALIGN);
    auto* C = (float*)_aligned_malloc(N*N*sizeof(float), sizeof(float)*ALIGN);
    auto* c = (float*)_aligned_malloc(ALIGN*sizeof(float), sizeof(float)*ALIGN);

    for (size_t i = 0; i < N*N; i++) {
        A[i] = randf();
        B[i] = randf();
        C[i] = 0.0f;
    }

    auto fn = [&]() -> void {
        // FIXME: C[i] = 0.0f;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                for (size_t k = 0; k < N; k+=ALIGN) {
                    __m256 _a = _mm256_load_ps(&A[i*N + k]);
                    __m256 _b = _mm256_set_ps(
                            B[(k+7)*N + j],B[(k+6)*N + j],
                            B[(k+5)*N + j],B[(k+4)*N + j],
                            B[(k+3)*N + j],B[(k+2)*N + j],
                            B[(k+1)*N + j],B[(k+0)*N + j]
                    );
                    __m256 _c = _mm256_dp_ps(_a, _b, 0xFF);
                    _mm256_store_ps(c, _c);
                    C[i*N + j] += c[0] + c[4];
                }
            }
        }
    };
    fn();

    if (!bPrint) {
        Tick tick;
        tick.Start("simd_only_mm_256");
        for (size_t nt = 0; nt < NTest; nt++) {
            fn();
        }
        tick.End(1.0/(double)NTest);
    } else {
        print(A, N); print(B, N); print(C, N);
    }

    _aligned_free(A);
    _aligned_free(B);
    _aligned_free(C);
    _aligned_free(c);
#else
    printf("AVX2 is disabled.\n");
#endif
}


template<size_t THREAD=8>
void threading_only_mm(size_t N, bool bPrint=false, size_t NTest=100)
{
    const size_t BLOCK = N / THREAD;

    auto* A = new float[N*N];
    auto* B = new float[N*N];
    auto* C = new float[N*N];
    for (size_t i = 0; i < N*N; i++) {
        A[i] = randf();
        B[i] = randf();
        C[i] = 0.0f;
    }

    auto base_fn = [&](size_t X, size_t Y) -> void {
        // FIXME: C[i] = 0.0f;
        for (size_t i = X; i < Y; i++) {
            for (size_t j = 0; j < N; j++) {
                for (size_t k = 0; k < N; k++) {
                    auto a = A[i*N + k];
                    auto b = B[k*N + j];
                    C[i*N + j] += a*b;
                }
            }
        }
    };
    if (bPrint) base_fn(0, N);

    auto fn = [&]() -> void {
        std::thread groups[THREAD];
        for (size_t i = 0; i < THREAD; i++) {
            groups[i] = std::thread(base_fn, i*BLOCK, i*BLOCK+BLOCK);  // std::move(t)
        }

        for (auto& t : groups) {
            t.join();
        }
    };
    if(!bPrint) fn();

    if (!bPrint) {
        Tick tick;
        tick.Start("threading_only_mm");
        for (size_t nt = 0; nt < NTest; nt++) {
            fn();
        }
        tick.End(1.0/(double)NTest);
    } else {
        print(A, N); print(B, N); print(C, N);
    }

    delete[] A;
    delete[] B;
    delete[] C;
}


template<size_t THREAD=8, size_t TILE=16>
void full_mm_256(size_t N, bool bPrint=false, size_t NTest=100)
{
#ifdef AVX2_ENABLED
    const size_t ALIGN = 8;
    const size_t BLOCK = N / THREAD;

//    if (BLOCK < TILE)
//        throw_err("N/THREAD is smaller than TILE");
    if ((N % ALIGN) != 0)
        throw_err("(N % ALIGN) != 0");
    if (TILE < ALIGN || TILE % ALIGN != 0)  // TILE >= 8 (256 bits)
        throw_err("TILE < ALIGN || TILE % ALIGN != 0");

    auto* A = (float*)_aligned_malloc(N*N*sizeof(float), sizeof(float)*ALIGN);
    auto* B = (float*)_aligned_malloc(N*N*sizeof(float), sizeof(float)*ALIGN);
    auto* C = (float*)_aligned_malloc(N*N*sizeof(float), sizeof(float)*ALIGN);

    // NOTE: share frequently read/write memory will induce bus contention, and thus reduce the performance
    // auto* c = (float*)_aligned_malloc(TILE*sizeof(float), sizeof(float)*ALIGN);

    for (size_t i = 0; i < N*N; i++) {
        A[i] = randf();
        B[i] = randf();
        C[i] = 0.0f;
    }

    auto base_fn = [&](size_t X, size_t Y) -> void {
        auto* c = (float*)_aligned_malloc(TILE*sizeof(float), sizeof(float)*ALIGN);
        // FIXME: C[i] = 0.0f;
        for (size_t I = X; I < Y; I+=TILE) {
            for (size_t K = 0; K < N; K+=TILE) {          //  SWAP  (K, J)
                for (size_t J = 0; J < N; J+=TILE) {      //  SWAP  (K, J)

                    // C_IJ = A_IK * B_KJ
                    for (size_t i = 0; i < TILE && (I+i) < Y; i++) {
                        for (size_t k = 0; k < TILE; k++) {
                            float a = A[(I+i)*N + K+k];

                            __m256 _a = _mm256_set1_ps(a);

                            for (size_t j = 0; j < TILE; j+=ALIGN) {
                                __m256 _b = _mm256_load_ps(&B[(K+k)*N + J]);
                                __m256 _c = _mm256_mul_ps(_a, _b);
                                _mm256_store_ps(&c[j], _c);
                            }
                            for (size_t j = 0; j < TILE; j++) {
                                C[(I+i)*N + J+j] += c[j];
                            }
                        }
                    }

                }
            }
        }
        _aligned_free(c);
    };
    if (bPrint) base_fn(0, N);

    auto fn = [&]() -> void {
        std::thread groups[THREAD];
        for (size_t i = 0; i < THREAD; i++) {
            groups[i] = std::thread(base_fn, i*BLOCK, i*BLOCK+BLOCK);  // std::move(t)
        }

        for (auto& t : groups) {
            t.join();
        }
    };
    if (!bPrint) fn();

    if (!bPrint) {
        Tick tick;
        tick.Start("full_mm_256");
        for (size_t nt = 0; nt < NTest; nt++) {
            fn();
        }
        tick.End(1.0/(double)NTest);
    } else {
        print(A, N); print(B, N); print(C, N);
    }

    _aligned_free(A);
    _aligned_free(B);
    _aligned_free(C);
#else
    printf("AVX2 is disabled.\n");
#endif
}


extern void cs_mm(size_t N, bool bPrint=false, size_t NTest=100);

int main() {

    auto print_fn = []() -> void {
        printf(" =========== naive =========== \n");
        srand(230027+1055); // NOLINT(*-msc51-cpp)
        naive_mm(8, true);
        printf(" ============================= \n");

        printf(" =========== tiled =========== \n");
        srand(230027+1055); // NOLINT(*-msc51-cpp)
        tiled_only_mm<4>(8, true);
        printf(" ============================ \n");

        printf(" ============ simd =========== \n");
        srand(230027+1055); // NOLINT(*-msc51-cpp)
        simd_only_mm_256(8, true);
        printf(" ============================= \n");

        printf(" ============ threading =========== \n");
        srand(230027+1055); // NOLINT(*-msc51-cpp)
        threading_only_mm<2>(8, true);
        printf(" ================================== \n");

        printf(" ============ full =========== \n");
        srand(230027+1055); // NOLINT(*-msc51-cpp)
        full_mm_256<1, 8>(8, true);
        printf(" ============================== \n");

        printf(" ============ compute shader =========== \n");
        srand(230027+1055); // NOLINT(*-msc51-cpp)
        cs_mm(8, true);
        printf(" ======================================= \n");
    };

    size_t NTest;
    size_t N = 1024;  // 768  1024  1280  1536

    auto nop_fn = [&]() -> void {
        // naive:
        naive_mm(N, false, NTest);

        // cache:
        tiled_only_mm<64>(N, false, NTest);

        // vectorize:
        simd_only_mm_256(N, false, NTest);

        // threading:
        threading_only_mm<16>(N, false, NTest);

        // full:
        full_mm_256<16, 64>(N, false, NTest);

        // compute shader:
        cs_mm(N, false, NTest);
    };

    std::string cmd;
    std::cout << "print[Y|n]:";
    std::cin >> cmd;

    if (cmd == "Y" || cmd == "y") {
        print_fn();
    } else {
        std::cout << "Number of tests:";
        int tests = 0;
        scanf("%d", &tests);
        if (tests == 0) return 0;

        NTest = tests;
        nop_fn();
    }

    return 0;
}
