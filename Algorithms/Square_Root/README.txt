How to interpret the square root benchmark results
Documentation by yipeng@cs.princeton.edu

==================================================

For problem size n=2, solution should be
[x0, b1, b0, a1, a0] = [0, 1, 0, 1, 1]
Where the b values indicate the coefficients in the polynomial B = b1 * x + b0 * 1, which in this case would be B = x, the polynomial for which we're finding the square root
The Grover algorithm solution is A = a1 * x + a0 * 1 = x + 1.
The solution is correct because we're working in a modular space defined over GF(2), like so:
A*A mod x^2 + x + 1
= (x+1)(x+1) mod x^2 + x + 1
= x^2 + 2x + 1 mod x^2 + x + 1
= x
= B
So we've found that sqrt(B) = A mod x^2 + x + 1

==================================================

For problem size n=3, solution should be
[x1, x0, b2, b1, b0, a2, a1, a0] = [0, 0, 0, 1, 0, 1, 1, 0]
In this case B = b2 * x^2 + b1 * x + b0 * 1 = x
And A = a2 * x^2 + a1 * x + a0 * 1 = x^2 + x.
To verify the solution,
A*A mod x^3 + x + 1
= (x^2+x)(x^2+x) mod x^3 + x + 1
= x^4 + 2x^3 + x^2 mod x^3 + x + 1
= -3x - 2
= x = B
The last two lines here are true because we are working in GF(2).
So we've found that sqrt(B) = A mod x^3 + x + 1

==================================================

For problem size n=4, solution should be
[x2, x1, x0, b3, b2, b1, b0, a3, a2, a1, a0] = [0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1]
In this case B = b3 * x^3 + b2 * x^2 + b1 * x + b0 * 1 = x
And A = a3 * x^3 + a2 * x^2 + a1 * x + a0 * 1 = x^2 + 1.
To verify the solution,
A*A mod x^3 + x + 1
= (x^2+1)(x^2+1) mod x^4 + x + 1
= x^4 + 2x^2 + 1 mod x^4 + x + 1
= 2x^2 - x
= x = B
The last two lines here are true because we are working in GF(2).
So we've found that sqrt(B) = A mod x^4 + x + 1

==================================================
