#pragma once

#define ARCH_IS_BIG_ENDIAN	0	// for INTEL CPU
//#define ARCH_IS_BIG_ENDIAN	1	// for MOTOROLA CPU

typedef unsigned char md5_byte_t; /* 8-bit byte */
typedef unsigned int md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */

	bool operator==(md5_state_s &sMd5)
	{
		if(memcmp(this, &sMd5, sizeof(md5_state_s)) == 0)
			return true;
		return false;
	}
} md5_state_t;

class CMD5
{
public:
	CMD5();
	~CMD5();
	/* Initialize the algorithm. */
	void init(md5_state_t *pms);
	/* Append a string to the message. */
	void append(md5_state_t *pms, const md5_byte_t *data, int nbytes);
	/* Finish the message and return the digest. */
	void finish(md5_state_t *pms, md5_byte_t digest[16]);
	void process(md5_state_t *pms, const md5_byte_t *data /*[64]*/);
};