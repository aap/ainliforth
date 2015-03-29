pt: forth.sed forth.11 lda
	sed -f forth.sed < forth.11 | ./lda >pt

lda: lda.c
	cc lda.c -o lda
