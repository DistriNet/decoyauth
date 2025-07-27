#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <cstring>
#include "gtest/gtest.h"
#include "weaver.h"

TEST(weave, test1)
{
    std::string x_str[3] = {"1", "3", "8"};
    std::string y_str[3] = {"10", "5", "7"};
    int cmp_values[3] = {-1, -1, -1};

    BIGNUM** x_values = (BIGNUM**) malloc(3 * sizeof(BIGNUM*));
    BIGNUM** y_values = (BIGNUM**) malloc(3 * sizeof(BIGNUM*));

    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, "11");

    for(int i = 0; i < 3; i++)
    {
        x_values[i] = BN_new();
        BN_dec2bn(&x_values[i], x_str[i].c_str());

        y_values[i] = BN_new();
        BN_dec2bn(&y_values[i], y_str[i].c_str());
    }

    BIGNUM** matrix = precompute(x_values, 3, prime, ctx);
    BIGNUM** vals = weave(y_values, matrix, 3, prime, ctx);

    for (int i = 0; i < 3; i++)
    {
        BIGNUM* y = evaluate(vals, x_values[i], 3, prime, ctx);
        cmp_values[i] = BN_cmp(y, y_values[i]);
        BN_free(y);
    }

    for(int i = 0; i < 9; i++)
    {
        BN_free(matrix[i]);
    }
    free(matrix);

    for(int i = 0; i < 3; i++)
    {
        BN_free(vals[i]);
    }
    free(vals);

    for(int i = 0; i < 3; i++)
    {
        BN_free(x_values[i]);
        BN_free(y_values[i]);
    }
    free(x_values);
    free(y_values);

    BN_free(prime);
    BN_CTX_free(ctx);

    for(int i = 0; i < 3; i++)
    {
        EXPECT_EQ(cmp_values[i], 0);
    }
}
