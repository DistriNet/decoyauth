#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <time.h>
#include "util.h"
#include "encode.h"
#include "weaver.h"

#define NUM_POINTS 1000

#define A   "115792089210356248762697446949407573530086143415290314195533631308867097853948"
#define B   "41058363725152142129326129780047268409114441015993725554835256314039467401291"
#define P   "115792089210356248762697446949407573530086143415290314195533631308867097853951"

int main()
{
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BN_CTX* ctx = BN_CTX_new();
    EC_POINT** points = read_points("points.txt", group, NUM_POINTS);

    if(points)
    {
        BIGNUM* a = BN_new();
        BN_dec2bn(&a, A);
        BIGNUM* b = BN_new();
        BN_dec2bn(&b, B);
        BIGNUM* prime = BN_new();
        BN_dec2bn(&prime, P);
        
        BIGNUM** hashes = read_hashes("hashes.txt", NUM_POINTS);
        if(hashes)
        {
            BIGNUM** u_values = (BIGNUM**) malloc(NUM_POINTS * sizeof(BIGNUM*));
            BIGNUM** v_values = (BIGNUM**) malloc(NUM_POINTS * sizeof(BIGNUM*));
            
            for(int i = 0; i < NUM_POINTS; i++)
            {
                BIGNUM** pair = point_to_values(points[i], group, a, b, prime);
                u_values[i] = pair[0];
                v_values[i] = pair[1];
                free(pair);
            }
            
            clock_t start_time = clock();

            BIGNUM** matrix = precompute(hashes, NUM_POINTS, prime, ctx);

            clock_t end_time = clock();
            double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("precomputation              %f ms\n", 1000 * time_spent);

            start_time = clock();

            BIGNUM** c_u = weave(u_values, matrix, NUM_POINTS, prime, ctx);
            BIGNUM** c_v = weave(v_values, matrix, NUM_POINTS, prime, ctx);
            
            end_time = clock();
            time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("weave (total)       %f ms\n", 1000 * time_spent);
            printf("weave (per point)   %f ms\n", 1000 * time_spent / NUM_POINTS);

            start_time = clock();
            for(int i = 0; i < NUM_POINTS; i++)
            {
                BIGNUM* rec_u = evaluate(c_u, hashes[i], NUM_POINTS, prime, ctx);
                BIGNUM* rec_v = evaluate(c_v, hashes[i], NUM_POINTS, prime, ctx);
                BIGNUM** pair = (BIGNUM**) malloc(2 * sizeof(BIGNUM*));
                pair[0] = rec_u;
                pair[1] = rec_v;

                EC_POINT* recovered_point = values_to_point(pair, group, a, b, prime);
                BN_free(rec_u);
                BN_free(rec_v);
                free(pair);
                EC_POINT_free(recovered_point);
            }
            
            end_time = clock();
            time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("evaluation (total)          %f ms\n", 1000 * time_spent);
            printf("evaluation (per point)      %f ms\n", 1000 * time_spent / NUM_POINTS);
            
            for(int i = 0; i < NUM_POINTS; i++)
            {
                BN_free(u_values[i]);
                BN_free(v_values[i]);
            }
            free(u_values);
            free(v_values);
            
            for(int i = 0; i < NUM_POINTS * NUM_POINTS; i++)
            {
                BN_free(matrix[i]);
            }
            free(matrix);
            
            for(int i = 0; i < NUM_POINTS; i++)
            {
                BN_free(c_u[i]);
            }
            free(c_u);
            
            for(int i = 0; i < NUM_POINTS; i++)
            {
                BN_free(c_v[i]);
            }
            free(c_v);
            
            BN_free(a);
            BN_free(b);
            BN_free(prime);
            
            EC_GROUP_free(group);
            BN_CTX_free(ctx);

            free_points(points, NUM_POINTS);
            free_hashes(hashes, NUM_POINTS);
        }
        else
        {
            EC_GROUP_free(group);
            BN_CTX_free(ctx);
        }
    }
    else
    {
        EC_GROUP_free(group);
        BN_CTX_free(ctx);
    }
}
