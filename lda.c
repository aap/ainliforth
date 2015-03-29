#include <stdio.h>
#include <stdint.h>

int origin, count;
uint8_t data[0x10000];
int nextc;

int last;

void
writeblock(void)
{
	uint8_t h[7];
	int i;
	int checksum;

	h[0] = 1;
	h[1] = 0;
	h[2] = count+6 & 0xFF;
	h[3] = ((count+6) >> 8) & 0xFF;
	h[4] = origin & 0xFF;
	h[5] = (origin >> 8) & 0xFF;

	fprintf(stderr, "%o %o -> %o\n", origin, count, origin+count);
	if(origin < last && count)
		fprintf(stderr, "^ warning ^\n");
//	fprintf(stderr, "\t\ts/%o/%o/g\n", origin, last);
	last = origin+count;

	checksum = 0;
	for(i = 0; i < 6; i++)
		checksum += h[i];
	for(i = 0; i < count; i++)
		checksum += data[i];
	h[6] = -checksum;
	fwrite(h, 6, 1, stdout);
	fwrite(data, count, 1, stdout);
	fwrite(&h[6], 1, 1, stdout);
}

int
ch(void)
{
	int c;
	if(nextc){
		c = nextc;
		nextc = 0;
		return c;
	}
tail:
	c = getchar();
	if(c == ';'){
		while(c = getchar(), c != '\n')
			if(c == EOF) return c;
		goto tail;
	}
	if(c == '\t' || c == '\n')
		c = ' ';
	return c;
}

int
chsp(void)
{
	int c;
	while(c = ch(), c == ' ');
	return c;
}

/* prefix: h = hex, d = dec, b = bin */
int
num(void)
{
	int base;
	int n;
	int c;

	base = 8;
	n = 0;
	c = ch();
	if(c == 'h') base = 16;
	else if(c == 'd') base = 10;
	else if(c == 'b') base = 2;
	else nextc = c;
	while(c = ch(), c != EOF){
		if(c >= '0' && c <= '9')
			n = n*base + c-'0';
		else if(c >= 'A' && c <= 'F')
			n = n*base + c-'A'+10;
		else if(c >= 'a' && c <= 'f')
			n = n*base + c-'a'+10;
		else{
			nextc = c;
			break;
		}
	}
	return n;
}

int
expr(void)
{
	int c;
	int n;
	n = num();
	c = chsp();
	switch(c){
	case '+':
		return n + expr();
	case '-':
		return n - expr();
	case '|':
		return n | expr();
	case '&':
		return n & expr();
	default:
		nextc = c;
		return n;
	}
}

int
main()
{
	int c;

	origin = -1;
	while(c = chsp(), c != EOF){
		switch(c){
		case ':':
			if(origin > 0)
				writeblock();
			origin = expr();
			count = 0;
			break;

		case '.':
			c = expr();
			goto byte;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'h':
			nextc = c;
			c = expr();
			goto word;

		case '\'':
			c = ch();
			goto byte;

		case '"':
			c = ch();
			c |= ch() << 8;
			goto word;

		byte:
			data[count++] = c;
			break;

		word:
			data[count++] = c & 0xFF;
			data[count++] = (c >> 8) & 0xFF;
			break;

		default:
			fprintf(stderr, "unknown char: %c 0x%x\n", c, c);
		}
	}
	writeblock();
	return 0;
}
