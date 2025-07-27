#include "weaver.h"

BIGNUM **precompute(BIGNUM **x_values, int num_elements, const BIGNUM *prime,
		    BN_CTX *ctx)
{
	if (x_values == NULL || prime == NULL || ctx == NULL)
		return NULL;

	BIGNUM **a = (BIGNUM **) malloc(sizeof(BIGNUM *) * num_elements);
	for (int i = 0; i < num_elements; i++)
		a[i] = BN_new();

	BN_mod_add(a[0], x_values[0], x_values[1], prime, ctx);
	BN_set_negative(a[0], 1);
	BN_mod_mul(a[1], x_values[0], x_values[1], prime, ctx);

	for (int i = 2; i < num_elements; i++) {
		BIGNUM *neg_x = BN_new();
		BN_copy(neg_x, x_values[i]);
		BN_set_negative(neg_x, 1);
		BN_mod_mul(a[i], neg_x, a[i - 1], prime, ctx);
		BN_free(neg_x);

		for (int m = i - 1; m > 0; m--) {
			BIGNUM *mul = BN_new();
			BN_mod_mul(mul, x_values[i], a[m - 1], prime, ctx);
			BN_mod_sub(a[m], a[m], mul, prime, ctx);
			BN_free(mul);
		}

		BN_mod_sub(a[0], a[0], x_values[i], prime, ctx);
	}

	BIGNUM **p = (BIGNUM **) malloc(sizeof(BIGNUM *) * num_elements);
	for (int i = 0; i < num_elements; i++)
		p[i] = BN_new();

	for (int i = 0; i < num_elements; i++) {
		BN_dec2bn(&p[i], "1");
		for (int j = 0; j < num_elements; j++) {
			if (j != i) {
				BIGNUM *sub = BN_new();
				BN_mod_sub(sub, x_values[i], x_values[j], prime,
					   ctx);
				BN_mod_mul(p[i], p[i], sub, prime, ctx);
				BN_free(sub);
			}
		}
	}

	BIGNUM **matrix =
	    (BIGNUM **) malloc(sizeof(BIGNUM *) * num_elements * num_elements);
	if (matrix == NULL) {
		printf("ERROR: matrix could not be allocated");
		for (int i = 0; i < num_elements; i++) {
			BN_free(a[i]);
			BN_free(p[i]);
		}
		return NULL;
	}

	for (int i = 0; i < num_elements; i++) {
		for (int j = 0; j < num_elements; j++)
			matrix[i * num_elements + j] = BN_new();
	}

	for (int i = 0; i < num_elements; i++) {
		BIGNUM **b =
		    (BIGNUM **) malloc(sizeof(BIGNUM *) * num_elements);
		for (int i = 0; i < num_elements; i++)
			b[i] = BN_new();

		BN_dec2bn(&b[0], "1");
		for (int j = 0; j < num_elements - 1; j++) {
			BIGNUM *mul = BN_new();
			BN_mod_mul(mul, x_values[i], b[j], prime, ctx);
			BN_mod_add(b[j + 1], a[j], mul, prime, ctx);
			BN_free(mul);
		}

		for (int j = 0; j < num_elements; j++) {
			BIGNUM *invert = BN_new();
			BN_mod_inverse(invert, p[i], prime, ctx);
			BN_mod_mul(matrix[j * num_elements + i],
				   b[num_elements - 1 - j], invert, prime, ctx);
			BN_free(invert);
		}

		for (int i = 0; i < num_elements; i++)
			BN_free(b[i]);
		free(b);
	}

	for (int i = 0; i < num_elements; i++) {
		BN_free(a[i]);
		BN_free(p[i]);
	}

	free(a);
	free(p);

	return matrix;
}

BIGNUM **weave(BIGNUM **y_values, BIGNUM **matrix, int num_elements,
		     BIGNUM *prime, BN_CTX *ctx)
{
	if (y_values == NULL || matrix == NULL || prime == NULL || ctx == NULL)
		return NULL;

	BIGNUM **c = (BIGNUM **) malloc(sizeof(BIGNUM *) * num_elements);
	if (c == NULL) {
		printf("ERROR: c could not be allocated.");
		return NULL;
	}

	for (int i = 0; i < num_elements; i++)
		c[i] = BN_new();

	for (int i = 0; i < num_elements; i++) {
		BN_dec2bn(&c[i], "0");
		for (int j = 0; j < num_elements; j++) {
			BIGNUM *mul = BN_new();
			BN_mod_mul(mul, matrix[i * num_elements + j],
				   y_values[j], prime, ctx);
			BN_mod_add(c[i], c[i], mul, prime, ctx);
			BN_free(mul);
		}
	}

	return c;
}

BIGNUM *evaluate(BIGNUM **vals, BIGNUM *x, int num_elements, BIGNUM *prime,
		 BN_CTX *ctx)
{
	BIGNUM *result = BN_new();
	BN_copy(result, vals[num_elements - 1]);
	for (int k = num_elements - 2; k > -1; k--) {
		BIGNUM *mul = BN_new();
		BN_mod_mul(mul, x, result, prime, ctx);
		BN_mod_add(result, vals[k], mul, prime, ctx);
		BN_free(mul);
	}
	return result;
}
