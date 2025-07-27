#include "gtest/gtest.h"
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "util.h"
#include "encode.h"

#define A   "115792089210356248762697446949407573530086143415290314195533631308867097853948"
#define B   "41058363725152142129326129780047268409114441015993725554835256314039467401291"
#define P   "115792089210356248762697446949407573530086143415290314195533631308867097853951"

// #define DEBUG_PRINTS

TEST(x_0, test1)
{
    BIGNUM* u = BN_new();
    BN_dec2bn(&u, "107698393190582369789940241844786910414546117189221584948777358459955759916154");

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "80132941452981742527194707771963140633568971999969381239125043823644434638911");

    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);

    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);

    BIGNUM* result = X_0(u, a, b, prime);
    int cmp_val = BN_cmp(result, expected);

    BN_free(u);
    BN_free(expected);
    BN_free(result);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(x_1, test1)
{
    BIGNUM* u = BN_new();
    BN_dec2bn(&u, "107698393190582369789940241844786910414546117189221584948777358459955759916154");

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "105802709781551059029653857118310285418460105649947430327400511850673870075758");

    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);

    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);

    BIGNUM* result = X_1(u, a, b, prime);
    int cmp_val = BN_cmp(result, expected);

    BN_free(u);
    BN_free(expected);
    BN_free(result);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(g, test1)
{
    BIGNUM* x = BN_new();
    BN_dec2bn(&x, "74633025145443874835202050178576493479652141839188712030954629964645663843148");

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "36631462249462266181966305836312028879302499912460784520884566741954153406810");

    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);

    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);

    BIGNUM* result = g(x, a, b, prime);
    int cmp_val = BN_cmp(result, expected);

    BN_free(x);
    BN_free(expected);
    BN_free(result);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(f, test1)
{
    BIGNUM* u = BN_new();
    BN_dec2bn(&u, "107698393190582369789940241844786910414546117189221584948777358459955759916154");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BIGNUM* expected_x = BN_new();
    BN_dec2bn(&expected_x, "80132941452981742527194707771963140633568971999969381239125043823644434638911");
    BIGNUM* expected_y = BN_new();
    BN_dec2bn(&expected_y, "33684532066648903150815515976854275382923162816237194553367378967514287581759");
    EC_POINT* expected = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, expected, expected_x, expected_y, NULL);
    BN_free(expected_x);
    BN_free(expected_y);
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    EC_POINT* result = f(u, group, a, b, prime);
    BN_free(u);

    int cmp_val = EC_POINT_cmp(group, result, expected, NULL);
    EC_GROUP_free(group);
    EC_POINT_free(expected);
    EC_POINT_free(result);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(f, test2)
{
    BIGNUM* u = BN_new();
    BN_dec2bn(&u, "24998129243780766980572089171172925592606352632077101700409213790009970587439");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    BIGNUM* expected_x = BN_new();
    BN_dec2bn(&expected_x, "8374303575921993913679096622019263579752502445757631307479545585504175939117");
    BIGNUM* expected_y = BN_new();
    BN_dec2bn(&expected_y, "85727828049363477579394508289563607261092977759248282071333815909058310559883");
    EC_POINT* expected = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, expected, expected_x, expected_y, NULL);
    BN_free(expected_x);
    BN_free(expected_y);
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    EC_POINT* result = f(u, group, a, b, prime);
    BN_free(u);

    int cmp_val = EC_POINT_cmp(group, result, expected, NULL);
    EC_GROUP_free(group);
    EC_POINT_free(expected);
    EC_POINT_free(result);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(calc_v, test1)
{
    BIGNUM* q_x = BN_new();
    BN_dec2bn(&q_x, "95637831262659725491757352816596807869363243896143490851233831429075705581671");
    BIGNUM* q_y = BN_new();
    BN_dec2bn(&q_y, "92295012450266247646725114414297555035943519530517215064501765702732847773271");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* q = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, q, q_x, q_y, NULL);
    BN_free(q_x);
    BN_free(q_y);

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "11703262432241649285534580503297167065115598731204786139607364589766255439652");
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    BIGNUM* result = calc_v(q, 0, group, a, b, prime);
    EC_POINT_free(q);
    EC_GROUP_free(group);

    int cmp_val = BN_cmp(result, expected);
    BN_free(result);
    BN_free(expected);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(calc_v, test2)
{
    BIGNUM* q_x = BN_new();
    BN_dec2bn(&q_x, "95637831262659725491757352816596807869363243896143490851233831429075705581671");
    BIGNUM* q_y = BN_new();
    BN_dec2bn(&q_y, "92295012450266247646725114414297555035943519530517215064501765702732847773271");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* q = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, q, q_x, q_y, NULL);
    BN_free(q_x);
    BN_free(q_y);

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "104088826778114599477162866446110406464970544684085528055926266719100842414299");
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    BIGNUM* result = calc_v(q, 1, group, a, b, prime);
    EC_POINT_free(q);
    EC_GROUP_free(group);

    int cmp_val = BN_cmp(result, expected);
    BN_free(result);
    BN_free(expected);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(calc_v, test3)
{
    BIGNUM* q_x = BN_new();
    BN_dec2bn(&q_x, "95637831262659725491757352816596807869363243896143490851233831429075705581671");
    BIGNUM* q_y = BN_new();
    BN_dec2bn(&q_y, "92295012450266247646725114414297555035943519530517215064501765702732847773271");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* q = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, q, q_x, q_y, NULL);
    BN_free(q_x);
    BN_free(q_y);

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "64784640168550942811371183932955074366210907529330664778174062627302837840825");
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    BIGNUM* result = calc_v(q, 2, group, a, b, prime);
    EC_POINT_free(q);
    EC_GROUP_free(group);

    int cmp_val = BN_cmp(result, expected);
    BN_free(result);
    BN_free(expected);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(calc_v, test4)
{
    BIGNUM* q_x = BN_new();
    BN_dec2bn(&q_x, "95637831262659725491757352816596807869363243896143490851233831429075705581671");
    BIGNUM* q_y = BN_new();
    BN_dec2bn(&q_y, "92295012450266247646725114414297555035943519530517215064501765702732847773271");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* q = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, q, q_x, q_y, NULL);
    BN_free(q_x);
    BN_free(q_y);

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "51007449041805305951326263016452499163875235885959649417359568681564260013126");
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    BIGNUM* result = calc_v(q, 3, group, a, b, prime);
    EC_POINT_free(q);
    EC_GROUP_free(group);

    int cmp_val = BN_cmp(result, expected);
    BN_free(result);
    BN_free(expected);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}

TEST(calc_v, test5)
{
    BIGNUM* q_x = BN_new();
    BN_dec2bn(&q_x, "42752386123681091781520306989765444327905610539227483185968618637732770216890");
    BIGNUM* q_y = BN_new();
    BN_dec2bn(&q_y, "17868722974474255602141851956864065980150888661933202632387638617957549184146");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* q = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, q, q_x, q_y, NULL);
    BN_free(q_x);
    BN_free(q_y);

    BIGNUM* expected = BN_new();
    BN_dec2bn(&expected, "48071332441591625029364332877742831533316743594285558878990210650512498948990");
    
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);
    
    BIGNUM* result = calc_v(q, 1, group, a, b, prime);
    EC_POINT_free(q);
    EC_GROUP_free(group);

    int cmp_val = BN_cmp(result, expected);
    BN_free(result);
    BN_free(expected);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp_val, 0);
}


TEST(encode, decode)
{
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);

    BIGNUM* expected_x = BN_new();
    BN_dec2bn(&expected_x, "65806355583802351726491629782483873926370084931339497194066072319463673170168");
    BIGNUM* expected_y = BN_new();
    BN_dec2bn(&expected_y, "82644964625339147662735488253695614323153588769978943963476249488026677744124");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* expected = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, expected, expected_x, expected_y, NULL);
    BN_free(expected_x);
    BN_free(expected_y);

    BIGNUM** encoded = (BIGNUM**) malloc(2 * sizeof(BIGNUM*));
    encoded[0] = BN_new();
    BN_dec2bn(&encoded[0], "97945056622653298015081862479932987806690569858371314855998309191291337515864");
    encoded[1] = BN_new();
    BN_dec2bn(&encoded[1], "18052566651904244286302490488482292752139620615679439020636077745050056827647");

    EC_POINT* decoded = es_decode(encoded, group, a, b, prime);
    ASSERT_NE(decoded, nullptr);

    BN_CTX* ctx = BN_CTX_new();
    int cmp = EC_POINT_cmp(group, expected, decoded, ctx);

    BN_free(encoded[0]);
    BN_free(encoded[1]);
    free(encoded);
    BN_CTX_free(ctx);

    EC_POINT_free(expected);
    EC_POINT_free(decoded);
    EC_GROUP_free(group);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp, 0);
}

TEST(encode, encode_decode)
{
    BIGNUM* a = BN_new();
    BN_dec2bn(&a, A);
    BIGNUM* b = BN_new();
    BN_dec2bn(&b, B);
    BIGNUM* prime = BN_new();
    BN_dec2bn(&prime, P);

    BIGNUM* point_x = BN_new();
    BN_dec2bn(&point_x, "8517691224314295442851788324363290124830494657161065724563951638507853653482");
    BIGNUM* point_y = BN_new();
    BN_dec2bn(&point_y, "14364232192640213915146118647318971956571632776157127840070843282816720716733");

	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT* point = EC_POINT_new(group);
    EC_POINT_set_affine_coordinates(group, point, point_x, point_y, NULL);
    BN_free(point_x);
    BN_free(point_y);

    BIGNUM** encoded = es_encode(point, group, a, b, prime);
    ASSERT_NE(encoded, nullptr);

    EC_POINT* decoded = es_decode(encoded, group, a, b, prime);
    ASSERT_NE(decoded, nullptr);

    BN_CTX* ctx = BN_CTX_new();
    int cmp = EC_POINT_cmp(group, point, decoded, ctx);

    BN_free(encoded[0]);
    BN_free(encoded[1]);
    free(encoded);
    BN_CTX_free(ctx);

    EC_POINT_free(point);
    EC_POINT_free(decoded);
    EC_GROUP_free(group);

    BN_free(a);
    BN_free(b);
    BN_free(prime);

    EXPECT_EQ(cmp, 0);
}
