#include <openssl/rand.h>
#include <openssl/bn.h>
#include "encode.h"

// #define FIXED_U_VALUE "97945056622653298015081862479932987806690569858371314855998309191291337515864"
// #define FIXED_J_VALUE 2
// #define DEBUG_PRINTS

BIGNUM* generate_random_bn(BIGNUM* prime) {
    BIGNUM* rand_bn = BN_new();
    if(rand_bn == NULL)
        return NULL;

    // Generate a random number between 1 and prime-1
    if(!BN_rand_range(rand_bn, prime)) {
        BN_free(rand_bn);
        return NULL;
    }

    // Ensure the random number is at least 1
    if(BN_is_zero(rand_bn)) {
        BN_one(rand_bn);
    }

    return rand_bn;
}

int generate_j() {
    unsigned char buffer[1];
    if (RAND_bytes(buffer, sizeof(buffer)) != 1)
        return -1;

    int random_number = buffer[0] % 4;
    return random_number;
}

bool is_valid_u(BIGNUM* u, BIGNUM* prime) {
    BIGNUM* one = BN_new();
    BIGNUM* zero = BN_new();
    BIGNUM* minus_one = BN_new();

    BN_zero(zero);
    BN_one(one);
    BN_sub(minus_one, prime, one);

    bool result = true;

    if(BN_cmp(u, zero) == 0 || BN_cmp(u, one) == 0 || BN_cmp(u, minus_one) == 0)
        result = false;

    BN_free(zero);
    BN_free(one);
    BN_free(minus_one);

    return result;
}

BIGNUM* g(BIGNUM* x, BIGNUM* a, BIGNUM* b, BIGNUM* prime) {
    BIGNUM* result = BN_new();
    BIGNUM* three = BN_new();
    BN_dec2bn(&three, "3");

    BN_CTX* ctx = BN_CTX_new();
    BN_mod_exp(result, x, three, prime, ctx);
    BN_free(three);

    BIGNUM* mul = BN_new();
    BN_mod_mul(mul, a, x, prime, ctx);
    BN_mod_add(result, result, mul, prime, ctx);
    BN_free(mul);

    BN_mod_add(result, result, b, prime, ctx);
    BN_CTX_free(ctx);
    return result;
}

BIGNUM* X_0(BIGNUM* u, BIGNUM* a, BIGNUM* b, BIGNUM* prime) {
    BIGNUM* one = BN_new();
    BN_one(one);
    
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* u_square = BN_new();
    BN_mod_sqr(u_square, u, prime, ctx);

    // temp_inv = 0x1 + BN_mod_inverse(u_square * u_square - u_square, prime, ctx)
    BIGNUM* u_square_square = BN_new();
    BN_mod_sqr(u_square_square, u_square, prime, ctx);

    BIGNUM* temp = BN_new();
    BN_mod_sub(temp, u_square_square, u_square, prime, ctx);
    BN_free(u_square_square);
    BN_free(u_square);

    BIGNUM* temp_inv = BN_new();
    BN_mod_inverse(temp_inv, temp, prime, ctx);
    BN_free(temp);
    BIGNUM* temp_sum = BN_new();
    BN_mod_add(temp_sum, one, temp_inv, prime, ctx);
    BN_free(temp_inv);
    BN_free(one);

    // b_a_inv = b * a_inv % prime
    BIGNUM* a_inv = BN_new();
    BN_mod_inverse(a_inv, a, prime, ctx);
    BIGNUM* b_a_inv = BN_new();
    BN_mod_mul(b_a_inv, b, a_inv, prime, ctx);
    BN_free(a_inv);

    // result = (b_a_inv * temp) % prime
    BIGNUM* result = BN_new();
    BN_mod_mul(result, b_a_inv, temp_sum, prime, ctx);
    BN_free(b_a_inv);
    BN_free(temp_sum);

    // result = -result
    BN_sub(result, prime, result);
    BN_CTX_free(ctx);
    return result;
}

BIGNUM* X_1(BIGNUM* u, BIGNUM* a, BIGNUM* b, BIGNUM* prime) {
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* x_0 = X_0(u, a, b, prime);
    BIGNUM* u_square = BN_new();
    BN_mod_sqr(u_square, u, prime, ctx);

    BIGNUM* result = BN_new();
    BN_mod_mul(result, u_square, x_0, prime, ctx);
    BN_free(u_square);
    BN_free(x_0);

    BN_sub(result, prime, result);
    BN_CTX_free(ctx);
    return result;
}

EC_POINT* f(BIGNUM* u, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* prime) {
    BIGNUM* one = BN_new();
    BN_one(one);
    BIGNUM* zero = BN_new();
    BN_zero(zero);
    BIGNUM* minus_one = BN_new();
    BN_sub(minus_one, prime, one);
    
    BN_CTX* ctx = BN_CTX_new();
    EC_POINT* result = EC_POINT_new(group);

    if(!BN_cmp(u, one) || !BN_cmp(u, zero) || !BN_cmp(u, minus_one)) {
        EC_POINT_set_to_infinity(group, result);
        BN_free(one);
        BN_free(zero);
        BN_free(minus_one);
        BN_CTX_free(ctx);
        return result;
    }

    BN_free(one);
    BN_free(zero);
    BN_free(minus_one);

    BIGNUM* x_0 = X_0(u, a, b, prime);
    BIGNUM* g_0 = g(x_0, a, b, prime);

    BIGNUM* y = BN_new();
    BIGNUM* is_mod_sqrt = BN_mod_sqrt(y, g_0, prime, ctx);
    BN_free(g_0);

    if(is_mod_sqrt != NULL) {
        EC_POINT_set_affine_coordinates_GFp(group, result, x_0, y, NULL);
        BN_free(x_0);
        BN_free(y);
        BN_CTX_free(ctx);
        return result;
    } else {
        BN_free(x_0);
    }

    BIGNUM* x_1 = X_1(u, a, b, prime);
    BIGNUM* g_1 = g(x_1, a, b, prime);
    
    is_mod_sqrt = BN_mod_sqrt(y, g_1, prime, ctx);
    BN_free(g_1);

    if(is_mod_sqrt != NULL) {
        BIGNUM* y_inv = BN_new();
        BN_sub(y_inv, prime, y);
        BN_free(y);
        EC_POINT_set_affine_coordinates_GFp(group, result, x_1, y_inv, NULL);
        BN_free(y_inv);
        BN_free(x_1);
        BN_CTX_free(ctx);
        return result;
    }

    BN_free(x_1);
    BN_free(y);
    EC_POINT_free(result);
    BN_CTX_free(ctx);
    return NULL;
}

BIGNUM* calc_v(EC_POINT* q, int j, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* prime) {
    BIGNUM* result = NULL;

    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    EC_POINT_get_affine_coordinates(group, q, x, y, ctx);
    
    BIGNUM* one = BN_new();
    BN_one(one);
    BIGNUM* two = BN_new();
    BN_dec2bn(&two, "2");
    BIGNUM* four = BN_new();
    BN_dec2bn(&four, "4");

    BIGNUM* b_inv = BN_new();
    BN_mod_inverse(b_inv, b, prime, ctx);
    
    BIGNUM* omega = BN_new();
    BN_mod_mul(omega, a, b_inv, prime, ctx);
    BN_free(b_inv);
    BN_mod_mul(omega, omega, x, prime, ctx);
    BN_mod_add(omega, omega, one, prime, ctx);
    BN_free(one);

    BIGNUM* omega_square = BN_new();
    BN_mod_sqr(omega_square, omega, prime, ctx);

    BIGNUM* subtractor = BN_new();
    BN_mod_mul(subtractor, four, omega, prime, ctx);
    BN_free(four);

    BIGNUM* sqrt = BN_new();
    BN_sub(sqrt, omega_square, subtractor);
    BN_free(subtractor);
    BN_free(omega_square);

    BIGNUM* is_mod_sqrt = BN_mod_sqrt(sqrt, sqrt, prime, ctx);
    if(is_mod_sqrt != NULL) {
        if(j != 0 && j != 1)
            BN_sub(sqrt, prime, sqrt);

        BIGNUM* temp = BN_new();
        is_mod_sqrt = BN_mod_sqrt(temp, y, prime, ctx);
        BN_free(temp);

        BIGNUM* multiply = BN_new();
        if(is_mod_sqrt != NULL) {
            BN_mod_mul(multiply, two, omega, prime, ctx);
            BN_mod_inverse(multiply, multiply, prime, ctx);
        } else {
            BN_mod_inverse(multiply, two, prime, ctx);
        }

        temp = BN_new();
        BN_mod_add(temp, omega, sqrt, prime, ctx);
        BN_mod_mul(temp, temp, multiply, prime, ctx);

        is_mod_sqrt = BN_mod_sqrt(temp, temp, prime, ctx);
        BN_free(multiply);

        if(is_mod_sqrt != NULL) {
            if(j != 0 && j != 2)
                BN_sub(temp, prime, temp);
            result = temp;
        } else {
            BN_free(temp);
        }
    }

    BN_free(x);
    BN_free(y);
    BN_free(omega);
    BN_free(two);
    BN_free(sqrt);
    BN_CTX_free(ctx);
    return result;
}

BIGNUM** point_to_values(EC_POINT* point, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* prime) {
    for(int i = 0; i < 1000; i++) {
        if(point == NULL || prime == NULL)
            return NULL;
    
        BIGNUM* u = generate_random_bn(prime);
        if(u == NULL)
            continue;

        if(!is_valid_u(u, prime)) {
            BN_free(u);
            continue;
        }

        EC_POINT* f_val = f(u, group, a, b, prime);
        EC_POINT_invert(group, f_val, NULL);

        EC_POINT* diff = EC_POINT_new(group);
        EC_POINT_add(group, diff, point, f_val, NULL);
        EC_POINT_free(f_val);

        if(EC_POINT_is_at_infinity(group, diff)) {
            EC_POINT_free(diff);
            BN_free(u);
            continue;
        }
        
        int j = generate_j();
        if(j < 0)
            return NULL;

        BIGNUM* v = calc_v(diff, j, group, a, b, prime);
        if(!v) {
            EC_POINT_free(diff);
            BN_free(u);
            continue;
        }

        EC_POINT_free(diff);

        BIGNUM** output = (BIGNUM**) malloc(2 * sizeof(BIGNUM*));
        output[0] = u;
        output[1] = v;

        return output;
    }
}

EC_POINT* values_to_point(BIGNUM** encoded_point, EC_GROUP* group, BIGNUM* a, BIGNUM* b, BIGNUM* p) {
    BIGNUM* u = encoded_point[0];
    BIGNUM* v = encoded_point[1];

    if(!u) {
        if(v) BN_free(v);
        free(encoded_point);
        return NULL;
    }

    if(!v) {
        BN_free(u);
        free(encoded_point);
        return NULL;
    }

    EC_POINT* f_u = f(u, group, a, b, p);
    EC_POINT* f_v = f(v, group, a, b, p);

    EC_POINT* result = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    EC_POINT_add(group, result, f_u, f_v, ctx);

    EC_POINT_free(f_u);
    EC_POINT_free(f_v);
    BN_CTX_free(ctx);

    return result;
}
