#include "ewx_helper.h"

const char *ewx_inet_ntop(const uint32_t *src, char *dst)
{
	static const char *fmt = "%u.%u.%u.%u";
	const unsigned char *p = (const unsigned char *) src;

	sprintf(dst, fmt, p[0], p[1], p[2], p[3]);

	return (dst);
}

int ewx_inet_pton(const char *src, uint32_t *dst)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[4], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int new = *tp * 10 + (pch - digits);

			if (new > 255)
				return (1);
			*tp = new;
			if (! saw_digit) {
				if (++octets > 4)
					return (1);
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (1);
			*++tp = 0;
			saw_digit = 0;
		} else
			return (1);
	}
	if (octets < 4)
		return (1);
	memcpy(dst, tmp, 4);
	return (0);
}

