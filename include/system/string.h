#ifndef SHIM_STRING_H
#define SHIM_STRING_H

#define bcmp(s1, s2, n)       mkbi3_(bcmp, s1, s2, n)
#define bcopy(src, dest, n)   mkbi3_(bcopy, src, dest, n)
#define bzero(s, n)           mkbi2_(bzero, s, n)
mkdepbi2_(char *, index, const char *, s, int, c)
#define memchr(s, c, n)       mkbi3_(memchr, s, c, n)
#define memcmp(s1, s2, n)     mkbi3_(memcmp, s1, s2, n)
#define memcpy(dest, src, n)  mkbi3_(memcpy, dest, src, n)
#define memmove(dest, src, n) mkbi3_(memmove, dest, src, n)
#define mempcpy(dest, src, n) mkbi3_(mempcpy, dest, src, n)
#define memset(s, c, n)       mkbi3_(memset, s, c, n)
mkdepbi2_(char *, rindex, const char *, s, int, c)
mkdepbi2_(char *, stpcpy, char *, dest, const char *, src)
#define stpncpy(dest, src, n) mkbi3_(stpncpy, dest, src, n)
mkdepbi2_(int, strcasecmp, const char *, s1, const char *, s2)
mkdepbi2_(char *, strcat, char *, dest, const char *, src)
mkdepbi2_(char *, strchr, const char *, s, int, c)
mkdepbi2_(int, strcmp, const char *, s1, const char *, s2)
mkdepbi2_(char *, strcpy, char *, dest, const char *, src)
mkdepbi2_(size_t, strcspn, const char *, s, const char *, reject)
mkdepbi1_(char *, strdup, const char *, s)
#define strndup(s, n) mkbi2_(strndup, s, n)
mkdepbi1_(size_t, strlen, const char *, s)
#define strncasecmp(s1, s2, n) \
	mkbi3_(strncasecmp, const char *, s1, const char *, s2, size_t, n)
#define strncat(dest, src, n) mkbi3_(strncat, dest, src, n)
#define strncmp(s1, s2, n)    mkbi3_(strncmp, s1, s2, n)
#define strncpy(dest, src, n) mkbi3_(strncpy, dest, src, n)
#define strnlen(s1, n)        mkbi2_(strnlen, s1, n)
mkdepbi2_(char *, strpbrk, const char *, s, const char *, accept)
mkdepbi2_(char *, strrchr, const char *, s, int, c)
mkdepbi2_(size_t, strspn, const char *, s, const char *, accept)
mkdepbi2_(char *, strstr, const char *, haystack, const char *, needle)

static inline __attribute__((unused)) CHAR8 *
translate_slashes(CHAR8 *out, const char *str)
{
	int i;
	int j;
	if (str == NULL || out == NULL)
		return NULL;

	for (i = 0, j = 0; str[i] != '\0'; i++, j++) {
		if (str[i] == '\\') {
			out[j] = '/';
			if (str[i + 1] == '\\')
				i++;
		} else
			out[j] = str[i];
	}
	out[j] = '\0';
	return out;
}

static inline UNUSED CHAR8 *
strchrnul(const CHAR8 *s, int c)
{
	unsigned int i;

	if (s == NULL)
		return NULL;

	for (i = 0; s[i] != '\000' && s[i] != c; i++)
		;

	return (CHAR8 *)&s[i];
}

#endif /* SHIM_STRING_H */
