//-----------------------------------------------------------------------------
// MEKA - LIBMY.C
// Various helper functions - Code
//-----------------------------------------------------------------------------
// FIXME: many of those functions are now useless, outdated or not efficient
//-----------------------------------------------------------------------------

#include "shared.h"

//-----------------------------------------------------------------------------

unsigned short *StrCpyU16(unsigned short *s1, unsigned short *s2)
{
	unsigned short  *r = s1;

	while (*s2)
		*s1++ = *s2++;
	*s1 = EOSTR;
	return (r);
}

int     StrLenU16(const unsigned short *s)
{
	int    i = 0;
	while (*s++)
		i++;
	return (i);
}

char   *StrNDup(const char *src, int n)
{
	int    n2;
	char  *ret, *dst;

	n2 = strlen (src);
	if (n2 < n)
		n = n2;
	ret = dst = (char*)malloc (sizeof (char) * (n + 1));
	while (*src && n --)
		*dst++ = *src++;
	*dst = EOSTR;
	return (ret);
}

unsigned short   *StrDupToU16(const char *src)
{
	u16* ret = (u16*)malloc (sizeof (unsigned short) * (strlen (src) + 1));
	u16* dst = ret;
	while (*src)
	{
		*dst++ = *src++;
	}
	*dst = EOSTR;
	return (ret);
}

unsigned short   *StrNDupToU16(const char *src, int n)
{
	int n2 = strlen (src);
	if (n2 < n)
		n = n2;
	u16* ret = (u16*)malloc (sizeof (unsigned short) * (n + 1));
	u16* dst = ret;
	while (*src && n --)
	{
		*dst++ = *src++;
	}
	*dst = EOSTR;
	return (ret);
}

bool	StrIsNull(const char *s)
{
	if (s == 0 || *s == EOSTR)
		return true;
	return false;
}

void	StrUpper(char *s)
{
	char c;
	while ((c = *s) != 0)
	{
		if (c >= 'a' && c <= 'z')
		{
			c = c - 'a' + 'A';
			*s = c;
		}
		s++;
	}
}

void	StrLower(char *s)
{
	char c;
	while ((c = *s) != 0)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = c - 'A' + 'a';
			*s = c;
		}
		s++;
	}
}

void StrReplace (char *s, char c1, char c2)
{
	while (*s)
	{
		if (*s == c1)
			*s = c2;
		s++;
	}
}

int	StrMatch (const char *src, const char *wildcards)
{
	int nbr = 0;
	for (int val = 0; ; val++)
	{
		if (wildcards[val] == '*')
		{
			int i = 0;
			do
			{
				nbr = nbr + StrMatch(src + val + i, wildcards + val + 1);
				i++;
			}
			while (src[val + i - 1] != EOSTR);
			return (nbr);
		}
		else
		{
			if (wildcards[val] != src[val])
				return (0);
			if (wildcards[val] == EOSTR)
				return (1);
		}
	}
}

void    StrChomp (char *s)
{
	int   last;

	last = strlen(s) - 1;
	while (last >= 0 && (s[last] == '\n' || s[last] == '\r'))
	{
		s[last] = EOSTR;
		last -= 1;
	}
}

void    StrTrim (char *s)
{
	char * s1 = s;
	char * s2 = s;
	while (*s2 == ' ' || *s2 == '\t')
		s2++;
	if (s1 != s2)
	{
		while (*s2 != EOSTR)
			*s1++ = *s2++;
		*s1 = EOSTR;
	}
	StrTrimEnd(s);
}

void     StrTrimEnd(char *s)
{
	int i = strlen(s) - 1;
	while (i > 0 && (s[i] == ' ' || s[i] == '\t'))
	{
		s[i] = EOSTR;
		i--;
	}
}

void    StrRemoveBlanks(char *s)
{
	char *dst;

	dst = s;
	while (*s != EOSTR)
	{
		if (*s == ' ' || *s == '\t')
			s++;
		else
			*dst++ = *s++;
	}
	*dst = EOSTR;
}

void    Write_Bits_Field(int v, int n_bits, char* out_field)
{
	char* p = out_field;

	for (int bit_idx = n_bits-1; bit_idx >= 0; bit_idx--)
	{
		*p++ = (v & (1 << bit_idx)) ? '1' : '0';
		if (bit_idx != 0 && (bit_idx & 7) == 0)
			*p++ = '.';
	}
	*p = EOSTR;
}
