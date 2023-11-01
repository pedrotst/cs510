
/* An example that finds all primes between 2 and limit. */

define tests (limit) {
    auto num, p, root, i, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20

    prime[1] = 2;
    prime[2] = 3;
	p1[1] = 1;
	p2[2] = 1;
	p3[1] = 1;
	p4[1] = 1;
	p5[1] = 1;
	p6[1] = 1;
	p7[1] = 1;
	p8[1] = 1;
	p9[1] = 1;
	p0[1] = 1;
	p11[1] = 1;
	p21[2] = 1;
	p31[1] = 1;

    num = 2;
    if (limit >= 2) print "prime 1 = 2\n"
    if (limit >= 3) print "prime 2 = 3\n";
    scale = 0;

	for ( i = 1; i < limit; i ++) {
		prime[i] = i;
	}
}


print "\ntyping 'tests (10)' will construct 10 elements array.\n"
