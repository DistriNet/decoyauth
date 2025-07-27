#include "util.h"
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

void print_bignum(const BIGNUM *bn)
{
	char *dec_str = BN_bn2dec(bn);
	if (dec_str) {
		fprintf(stderr, "%s", dec_str);
		OPENSSL_free(dec_str);
	} else {
#ifdef DEBUG_PRINTS
		fprintf(stderr, "Error converting BIGNUM to decimal string\n");
#endif
	}
}

void print_ec_point(const EC_POINT *ep)
{
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();
	BN_CTX *ctx = BN_CTX_new();
	EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
	EC_POINT_get_affine_coordinates(group, ep, x, y, ctx);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);
	fprintf(stderr, "(");
	print_bignum(x);
	fprintf(stderr, ", ");
	print_bignum(y);
	fprintf(stderr, ")");
	BN_free(x);
	BN_free(y);
}

BIGNUM **read_hashes(const char *filename, const int num_points)
{
	BIGNUM **hashes;
	char buffer[256];
	FILE *file;
	int i = 0;

	file = fopen(filename, "r");
	if (file == NULL) {
		printf("Could not open file %s\n", filename);
		return NULL;
	}

	hashes = (BIGNUM **) malloc(num_points * sizeof(BIGNUM *));
	if (hashes == NULL) {
		fprintf(stderr, "Error allocating memory for points\n");
		fclose(file);
		return NULL;
	}

	while (fgets(buffer, sizeof(buffer), file) && i < num_points) {
		BIGNUM *bn = BN_new();
		if (bn == NULL) {
			fprintf(stderr, "Error allocating BIGNUM\n");
			fclose(file);
			free_hashes(hashes, num_points);
			return NULL;
		}

		if (!BN_dec2bn(&bn, buffer)) {
			fprintf(stderr,
				"Error converting string to BIGNUM\n");
			BN_free(bn);
			fclose(file);
			free_hashes(hashes, num_points);
			return NULL;
		}

		hashes[i++] = bn;
	}

	fclose(file);
	return hashes;
}

EC_POINT **read_points(const char *filename, EC_GROUP *group,
		       const int num_points)
{
	EC_POINT **points;
	FILE *file;
	char buffer[1024];
	int i = 0;

	file = fopen(filename, "r");
	if (file == NULL) {
		printf("Could not open file %s\n", filename);
		return NULL;
	}

	points = (EC_POINT **) malloc(num_points * sizeof(EC_POINT *));
	if (points == NULL) {
		fprintf(stderr, "Error allocating memory for points\n");
		fclose(file);
		return NULL;
	}

	while (fgets(buffer, sizeof(buffer), file)) {
		char *x_str = strtok(buffer, " ");
		char *y_str = strtok(NULL, " ");

		if (x_str && y_str) {
			BIGNUM *x = BN_new();
			BIGNUM *y = BN_new();

			BN_dec2bn(&x, x_str);
			BN_dec2bn(&y, y_str);

			EC_POINT *point = EC_POINT_new(group);
			EC_POINT_set_affine_coordinates(group, point, x,
							y, NULL);

			points[i] = point;

			BN_free(x);
			BN_free(y);

			if (++i == num_points) {
				break;
			}
		}
	}

	fclose(file);
	return points;
}

void free_hashes(BIGNUM **hashes, const int num_points)
{
	for (int i = 0; i < num_points; i++) {
		if (hashes[i] != NULL)
			BN_free(hashes[i]);
	}
	free(hashes);
}

void free_points(EC_POINT **points, const int num_points)
{
	for (int i = 0; i < num_points; i++) {
		if (points[i] != NULL)
			EC_POINT_free(points[i]);
	}
	free(points);
}

void store(const char *filename, double value, int comma)
{
	FILE *fp = fopen(filename, "a");
	if (fp != NULL) {
		if (comma)
			fprintf(fp, ",%lf", value);
		else
			fprintf(fp, "%lf", value);
		fclose(fp);
	}
}

void storeln(const char *filename, double value, int comma)
{
	FILE *fp = fopen(filename, "a");
	if (fp != NULL) {
		if (comma)
			fprintf(fp, ",%lf\n", value);
		else
			fprintf(fp, "%lf\n", value);
		fclose(fp);
	}
}

void export_bignums(const char *filename, BIGNUM **bignums, int count)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n",
			filename);
	}

	for (int i = 0; i < count; i++) {
		char *dec_str = BN_bn2dec(bignums[i]);
		if (dec_str) {
			// printf("%s\n", dec_str);
			fprintf(fp, "%s\n", dec_str);
			OPENSSL_free(dec_str);
		} else {
			fprintf(stderr,
				"Error converting BIGNUM to decimal string\n");
		}
	}
	fclose(fp);
}

BIGNUM **import_bignums(const char *filename, int count)
{
	BIGNUM **bignums;
	FILE *fp;
	char buffer[1024];
	int i = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for reading\n",
			filename);
		return NULL;
	}

	bignums = (BIGNUM **) malloc(count * sizeof(BIGNUM *));
	if (bignums == NULL) {
		fprintf(stderr,
			"Error allocating memory for BIGNUMs\n");
		fclose(fp);
		return NULL;
	}

	while (i < count && fgets(buffer, sizeof(buffer), fp)) {
		// Remove newline character if present
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';
		}

		BIGNUM *bn = BN_new();
		if (bn == NULL) {
			fprintf(stderr, "Error allocating BIGNUM\n");
			// Free already allocated BIGNUMs
			for (int j = 0; j < i; j++) {
				BN_free(bignums[j]);
			}
			free(bignums);
			fclose(fp);
			return NULL;
		}

		if (!BN_dec2bn(&bn, buffer)) {
			fprintf(stderr,
				"Error converting string to BIGNUM\n");
			BN_free(bn);
			// Free already allocated BIGNUMs
			for (int j = 0; j < i; j++) {
				BN_free(bignums[j]);
			}
			free(bignums);
			fclose(fp);
			return NULL;
		}

		bignums[i++] = bn;
	}

	fclose(fp);

	if (i < count) {
		fprintf(stderr,
			"Warning: Only %d out of %d BIGNUMs were imported\n",
			i, count);
	}

	return bignums;
}
