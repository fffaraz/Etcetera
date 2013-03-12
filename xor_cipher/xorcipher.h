
void xorcipher(const unsigned char *key, char *input, char *output, size_t len)
{
	const unsigned char *keyp = key;
	size_t i;

	for(i=0; i<len; ++i)
	{
		output[i] = input[i] ^ *keyp++;
		if(!*keyp) keyp=key;
	}
}
