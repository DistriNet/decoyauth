#include <benchmark/benchmark.h>
#include <iostream>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include "util.h"
#include "encode.h"
#include "weaver.h"

#define A "115792089210356248762697446949407573530086143415290314195533631308867097853948"
#define B "41058363725152142129326129780047268409114441015993725554835256314039467401291"
#define P "115792089210356248762697446949407573530086143415290314195533631308867097853951"

void pin_thread_to_cpu(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << std::endl;
    }
    
    // Verify that thread is pinned (only print in verbose mode or first run)
    static bool verified = false;
    if (!verified) {
        CPU_ZERO(&cpuset);
        pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        
        for (int i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &cpuset)) {
                std::cout << "Thread pinned to CPU core " << i << std::endl;
                verified = true;
                break;
            }
        }
    }
}

extern "C" BIGNUM** load_hashes(int num_points) {
    return read_hashes("hashes.txt", num_points);
}

extern "C" EC_POINT** load_points(int num_points, EC_GROUP* group) {
    return read_points("points.txt", group, num_points);
}

extern "C" BIGNUM** test_weave(BIGNUM** y_values, BIGNUM** matrix, int num_elements, BIGNUM* prime, BN_CTX* ctx) {
    return weave(y_values, matrix, num_elements, prime, ctx);
}

extern "C" BIGNUM* test_evaluate(BIGNUM** vals, BIGNUM* x, int num_elements, BIGNUM* prime, BN_CTX* ctx) {
    return evaluate(vals, x, num_elements, prime, ctx);
}

extern "C" BIGNUM** test_precompute(BIGNUM** x_values, int num_elements, const BIGNUM* prime, BN_CTX* ctx) {
    return precompute(x_values, num_elements, prime, ctx);
}

extern "C" BIGNUM*** load_encoded_points(int num_points) {
    BIGNUM** bns = import_bignums("encoded_points.txt", num_points * 2);
    BIGNUM*** result = (BIGNUM***) malloc(num_points * sizeof(BIGNUM**));
    for (int i = 0; i < num_points; i++) {
        result[i] = (BIGNUM**) malloc(2 * sizeof(BIGNUM*));
        result[i][0] = bns[2 * i];
        result[i][1] = bns[2 * i + 1];
    }
    free(bns);
    return result;
}

static void BM_encoding(benchmark::State &state)
{
    pin_thread_to_cpu(3);

    BIGNUM *a = BN_new();
    BN_dec2bn(&a, A);
    BIGNUM *b = BN_new();
    BN_dec2bn(&b, B);
    BIGNUM *prime = BN_new();
    BN_dec2bn(&prime, P);
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);

    int num_points = 10000;
    EC_POINT** points = load_points(num_points, group);

    int i = 0;
    for (auto _ : state) {
        BIGNUM** result = es_encode(points[i++ % num_points], group, a, b, prime);
        state.PauseTiming();
        if (result != NULL) {
            BN_free(result[0]);
            BN_free(result[1]);
            free(result);
        }
        state.ResumeTiming();
    }

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    for (int i = 0; i < num_points; i++)
        EC_POINT_free(points[i]);
    free(points);
}

static void BM_decoding(benchmark::State &state)
{
    pin_thread_to_cpu(3);
    
    BIGNUM *a = BN_new();
    BN_dec2bn(&a, A);
    BIGNUM *b = BN_new();
    BN_dec2bn(&b, B);
    BIGNUM *prime = BN_new();
    BN_dec2bn(&prime, P);
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);

    int num_points = 10000;
    BIGNUM*** points = load_encoded_points(num_points);

    int i = 0;
    for (auto _ : state) {
        EC_POINT* result = es_decode(points[i++ % num_points], group, a, b, prime);
        state.PauseTiming();
        if (result != NULL)
            EC_POINT_free(result);
        state.ResumeTiming();
    }

    BN_free(a);
    BN_free(b);
    BN_free(prime);
    EC_GROUP_free(group);

    for (int i = 0; i < num_points; i++) {
        BN_free(points[i][0]);
        BN_free(points[i][1]);
        free(points[i]);
    }
    free(points);
}

static void BM_precompute(benchmark::State &state)
{
    pin_thread_to_cpu(3);
    
    int num_points = state.range(0);
    BIGNUM** hashes = load_hashes(num_points);

    BIGNUM *prime = BN_new();
    BN_dec2bn(&prime, P);
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BN_CTX *ctx = BN_CTX_new();

    for (auto _ : state) {
        BIGNUM** result = test_precompute(hashes, num_points, prime, ctx);
        state.PauseTiming();
        if (result != NULL) {
            for (int i = 0; i < num_points; i++)
                BN_free(result[i]);
            free(result);
            result = NULL;
        }
        state.ResumeTiming();
    }

    BN_free(prime);
    BN_CTX_free(ctx);
    EC_GROUP_free(group);
    
    for (int i = 0; i < num_points; i++)
        BN_free(hashes[i]);
    free(hashes);
}

static void BM_weave(benchmark::State &state)
{
    pin_thread_to_cpu(3);
    
    int num_points = state.range(0);
    char matrix_filename[256] = {0};
    sprintf(matrix_filename, "../matrices/matrix_%d.txt", (int) num_points);
    BIGNUM **matrix = import_bignums(matrix_filename, num_points * num_points);

    BIGNUM *prime = BN_new();
    BN_dec2bn(&prime, P);
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BN_CTX *ctx = BN_CTX_new();

    BIGNUM*** points = load_encoded_points(num_points);
    BIGNUM** y_values = (BIGNUM**) malloc(num_points * sizeof(BIGNUM*));
    for (int i = 0; i < num_points; i++) {
        y_values[i] = points[i][0];
    }

    for (auto _ : state) {
        BIGNUM** result = test_weave(y_values, matrix, num_points, prime, ctx);
        state.PauseTiming();
        if (result != NULL) {
            for (int i = 0; i < num_points; i++)
                BN_free(result[i]);
            free(result);
            result = NULL;
        }
        state.ResumeTiming();
    }

    BN_free(prime);
    BN_CTX_free(ctx);
    EC_GROUP_free(group);
    free(y_values);
    
    for (int i = 0; i < num_points; i++) {
        BN_free(points[i][0]);
        BN_free(points[i][1]);
        free(points[i]);
    }
    free(points);
}

static void BM_evaluate(benchmark::State &state)
{
    pin_thread_to_cpu(3);
    
    int num_points = state.range(0);
    char vals_filename[256] = {0};
    sprintf(vals_filename, "../values/vals_%d.txt", (int) num_points);
    BIGNUM** vals = import_bignums(vals_filename, num_points);

    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM** pwd = load_hashes(1);

    for (auto _ : state) {
        BIGNUM* result = test_evaluate(vals, *pwd, num_points, prime, ctx);
        state.PauseTiming();
        if (result != NULL)
            BN_free(result);
        state.ResumeTiming();
    }

    BN_free(prime);
    BN_CTX_free(ctx);
    EC_GROUP_free(group);
    BN_free(*pwd);
    free(pwd);
    
    for (int i = 0; i < num_points; i++)
        BN_free(vals[i]);
    free(vals);
}

template <class Func>
void CustomArguments(Func* benchmark) {
    // Argument here is the number of points: 3, 10, 20, 30, 40, 50, 100, 200, 500, 1000
    benchmark->Arg(3);
    benchmark->DenseRange(10, 50, 10);
    benchmark->Arg(100);
    benchmark->Arg(200);
    benchmark->Arg(500);
    benchmark->Arg(1000);
}

BENCHMARK(BM_encoding);
BENCHMARK(BM_decoding);
BENCHMARK(BM_precompute)->Apply(CustomArguments);
BENCHMARK(BM_weave)->Apply(CustomArguments);
BENCHMARK(BM_evaluate)->Apply(CustomArguments);

BENCHMARK_MAIN();
