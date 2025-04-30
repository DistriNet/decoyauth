import random

from curves import *
from util import *

def ec_add(P: tuple[int, int], Q: tuple[int, int], a: int, prime: int):
	# TODO: Directly use EccPoint where possible?
	Pc = ECC.EccPoint(P[0], P[1], "secp256r1")
	Qc = ECC.EccPoint(Q[0], Q[1], "secp256r1")
	R = Pc + Qc
	# ECC.EccPoint results in IntegerGMP
	return int(R.x), int(R.y)
    
def ec_sub(P: tuple[int, int], Q: tuple[int, int], a: int, prime: int):
	# TODO: Directly use EccPoint where possible?
	Pc = ECC.EccPoint(P[0], P[1], "secp256r1")
	Qc = ECC.EccPoint(Q[0], Q[1], "secp256r1")
	R = Pc + (- Qc)
	# ECC.EccPoint results in IntegerGMP
	return int(R.x), int(R.y)

def g(x: int, a: int, b: int, prime: int):
    return (x**3 + a * x + b) % prime

def X_0(u: int, a: int, b: int, prime: int):
    u_square = u * u % prime
    return (-(b * invert_mod(a, prime)) * (0x1 + invert_mod(u_square * u_square - u_square, prime))) % prime

def X_1(u: int, a: int, b: int, prime: int):
    return (-((u * u) % prime) * X_0(u, a, b, prime)) % prime

def f(u: int, a: int, b: int, prime: int):
    # If u in {-1, 0, 1}, return infinity
    if u == prime - 0x1 and u == 0x0 and u == 0x1:
        return (0, 0)
    
    # If u not in {-1, 0, 1} and g(X_0(u)) is a square, return (X_0(u), sqrt(g(X_0(u))))
    x_0 = X_0(u, a, b, prime)
    g_x = g(x_0, a, b, prime)
    y = modular_sqrt(g_x, prime)

    if y is not None:
        return (x_0, y)
    
    # If u not in {-1, 0, 1} and g(X_0(u)) is not a square, return (X_1(u), -sqrt(g(X_1(u))))
    x_1 = X_1(u, a, b, prime)
    g_x = g(x_1, a, b, prime)
    y = modular_sqrt(g_x, prime)
    assert y is not None

    return (x_1, prime - y)

def calc_v(q: tuple[int, int], j: int, a: int, b: int, prime: int):
    x, y = q[0], q[1]
    omega = ((a * invert_mod(b, prime) * x) + 0x1) % prime
    omega_square = omega * omega % prime
    
    sqrt = modular_sqrt(omega_square - 4 * omega, prime)
    if sqrt is None:
        return None
    
    sqrt = sqrt if j == 0 or j == 1 else prime - sqrt

    if is_quadratic_residue(y, prime):
        multiply = invert_mod(2 * omega, prime)
    else:
        multiply = invert_mod(2, prime)

    res_sqrt = modular_sqrt((omega + sqrt) * multiply, prime)
    if res_sqrt is None:
        return None
    
    output = res_sqrt if j == 0 or j == 2 else (prime - res_sqrt)
    return output

# -------------------------------------------------------------------------------------

def point_to_values_for_u_j(point: tuple[int, int], a: int, b: int, prime: int, u: int, j: int):
    if u == prime - 0x1 or u == 0x0 or u == 0x1:
        return None

    f_val = f(u, a, b, prime)
    q = ec_sub(point, f_val, a, prime)
    if q == (0, 0):
        return None
    
    return calc_v(q, j, a, b, prime)


def point_to_values(point: tuple[int, int], a: int, b: int, prime: int):
    """
    Based on the following paper and existing implementations:
    - https://github.com/codahale/elligator-squared/blob/main/src/lib.rs#L25
    - Elligator Squared Uniform Points on Elliptic Curves of Prime Order as Uniform Random Strings
      by Mehdi Tibouchi. See https://eprint.iacr.org/2014/043.pdf
    - https://github.com/dignifiedquire/bcrypto/blob/master/lib/js/elliptic.js
    """
    for i in range(1000):
        u = random.randint(1, secp256r1_p-1)
        j = random.randint(0, 4)
        v = point_to_values_for_u_j(point, a, b, prime, u, j)
        if v is None: continue

        return u, v
        
    print("ERROR: Could not find a valid candidate, suspect RNG failure")
    return None

def values_to_point(u: int, v :int, a: int, b: int, prime: int):
    f_u = f(u, a, b, prime)
    f_v = f(v, a, b, prime)
    
    point_u = (f_u[0], f_u[1])
    point_v = (f_v[0], f_v[1])
    return ec_add(point_u, point_v, a, prime)

