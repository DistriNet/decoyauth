#include <gtest/gtest.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "util.h"
#include "encode.h"
#include "weaver.h"

// #define DEBUG_PRINTS

#define A   "115792089210356248762697446949407573530086143415290314195533631308867097853948"
#define B   "41058363725152142129326129780047268409114441015993725554835256314039467401291"
#define P   "115792089210356248762697446949407573530086143415290314195533631308867097853951"

void test_cycle(const int num_points)
{
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BN_CTX* ctx = BN_CTX_new();
    EC_POINT** points = read_points("points.txt", group, num_points);

    if(points)
    {
        BIGNUM* a = BN_new();
        BN_dec2bn(&a, A);
        BIGNUM* b = BN_new();
        BN_dec2bn(&b, B);
        BIGNUM* prime = BN_new();
        BN_dec2bn(&prime, P);
        
        BIGNUM** hashes = read_hashes("hashes.txt", num_points);
        if(hashes)
        {
            BIGNUM** u_values = (BIGNUM**) malloc(num_points * sizeof(BIGNUM*));
            BIGNUM** v_values = (BIGNUM**) malloc(num_points * sizeof(BIGNUM*));
            
            for(int i = 0; i < num_points; i++)
            {
                EXPECT_EQ(EC_POINT_is_on_curve(group, points[i], ctx), 1);
                BIGNUM** pair = es_encode(points[i], group, a, b, prime);
                EC_POINT* recovered_point = es_decode(pair, group, a, b, prime);
                EXPECT_EQ(EC_POINT_cmp(group, recovered_point, points[i], ctx), 0);
                EC_POINT_free(recovered_point);
                u_values[i] = pair[0];
                v_values[i] = pair[1];
                free(pair);
            }
            
            BIGNUM** matrix = precompute(hashes, num_points, prime, ctx);
            BIGNUM** c_u = weave(u_values, matrix, num_points, prime, ctx);
            BIGNUM** c_v = weave(v_values, matrix, num_points, prime, ctx);
            
            int cmp_values[num_points];
            for(int i = 0; i < num_points; i++)
            {
                cmp_values[i] = -1;
            }
            
            for(int i = 0; i < num_points; i++)
            {
                BIGNUM* rec_u = evaluate(c_u, hashes[i], num_points, prime, ctx);
                EXPECT_EQ(BN_cmp(rec_u, u_values[i]), 0);
                BIGNUM* rec_v = evaluate(c_v, hashes[i], num_points, prime, ctx);
                EXPECT_EQ(BN_cmp(rec_v, v_values[i]), 0);

                BIGNUM** pair = (BIGNUM**) malloc(2 * sizeof(BIGNUM*));
                pair[0] = rec_u;
                pair[1] = rec_v;

#ifdef DEBUG_PRINTS
                fprintf(stderr, "u     = ");
                print_bignum(u_values[i]);
                fprintf(stderr, "\nv     = ");
                print_bignum(v_values[i]);
                fprintf(stderr, "\nrec_u = ");
                print_bignum(rec_u);
                fprintf(stderr, "\nrec_v = ");
                print_bignum(rec_v);
                fprintf(stderr, "\n");
#endif

                EC_POINT* recovered_point = es_decode(pair, group, a, b, prime);
                BN_free(rec_u);
                BN_free(rec_v);
                free(pair);

                int cmp = EC_POINT_cmp(group, recovered_point, points[i], ctx);
#ifdef DEBUG_PRINTS
                if(cmp)
                {
                    fprintf(stderr, "recovered = ");
                    print_ec_point(recovered_point);
                    fprintf(stderr, "\npoint     = ");
                    print_ec_point(points[i]);
                    fprintf(stderr, "\n");
                }
#endif
                EC_POINT_free(recovered_point);
                EXPECT_EQ(cmp, 0);
            }
            
            for(int i = 0; i < num_points; i++)
            {
                BN_free(u_values[i]);
                BN_free(v_values[i]);
            }
            free(u_values);
            free(v_values);
            
            for(int i = 0; i < num_points * num_points; i++)
            {
                BN_free(matrix[i]);
            }
            free(matrix);
            
            for(int i = 0; i < num_points; i++)
            {
                BN_free(c_u[i]);
            }
            free(c_u);
            
            for(int i = 0; i < num_points; i++)
            {
                BN_free(c_v[i]);
            }
            free(c_v);
            
            BN_free(a);
            BN_free(b);
            BN_free(prime);
            
            EC_GROUP_free(group);
            BN_CTX_free(ctx);

            free_points(points, num_points);
            free_hashes(hashes, num_points);
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

TEST(combined, full_10)
{
    test_cycle(10);
}

TEST(combined, full_100)
{
    test_cycle(100);
}

// TEST(combined, full_500)
// {
//     test_cycle(500);
// }

// TEST(combined, full_1000)
// {
//     test_cycle(1000);
// }
