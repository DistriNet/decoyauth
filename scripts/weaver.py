from util import *

def precompute(xs, prime):
    n = len(xs)

    a = [0 for _ in range(n)]
    a[0] = -(xs[0] + xs[1]) % prime
    a[1] = (xs[0] * xs[1]) % prime

    for i in range(2, n):
        a[i] = (-xs[i] * a[i-1]) % prime
        for m in range(i-1, 0, -1):
            a[m] = ((a[m] % prime) - (xs[i] * a[m-1]) % prime) % prime
        a[0] = (a[0] - xs[i]) % prime

    p = [1 for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if j != i:
                p[i] = (p[i] % prime) * ((xs[i] - xs[j]) % prime)

    matrix = [[0 for _ in range(n)] for _ in range(n)]

    for i in range(n):
        b = [1]
        for j in range(n-1):
            b.append((a[j] % prime) + (xs[i] * b[j]) % prime)
        for j in range(n):
            matrix[j][i] = (b[n-1-j] % prime) * invert_mod(p[i], prime)
            
    return matrix

def waever(ys, matrix, prime):
    n = len(ys)
    
    c = []
    for i in range(n):
        c.append(0)
        for j in range(n):
            c[i] = (c[i] % prime) + ((matrix[i][j] * ys[j]) % prime)

    return c

def eval_weave(c, x, prime=None):
    result = c[-1]
    for k in range(len(c)-2, -1, -1):
        result = c[k] + x * result
    return result if prime is None else (result % prime)

