/*
 * Copyright (C) 1997-2004, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file strings.c
 * String Utility Routine Source File
 *
 * This file contains string utility functions.
 *
 * @author Michael Jennings <mej@eterm.org>
 */

static const char cvs_ident[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libast_internal.h>

#if !(HAVE_MEMMEM)
/* Find first occurance of bytestring needle of size needlelen in memory region
   haystack of size haystacklen */
void *
memmem(const void *haystack, register size_t haystacklen, const void *needle, register size_t needlelen)
{
    register char *hs = (char *) haystack;
    register char *n = (char *) needle;
    register unsigned long i;
    register size_t len = haystacklen - needlelen;

    for (i = 0; i < len; i++) {
        if (!memcmp(hs + i, n, needlelen)) {
            return (hs + i);
        }
    }
    return (NULL);
}
#endif

#if !(HAVE_STRNLEN)
size_t
strnlen(register const char *s, size_t maxlen)
{
    register size_t n;

    if (!s)
        return 0;
    for (n = 0; *s && n < maxlen; s++, n++);
    return n;
}
#endif

#if !(HAVE_USLEEP)
void
usleep(unsigned long usec)
{
    struct timeval delay;

    delay.tv_sec = 0;
    delay.tv_usec = usec;
    select(0, NULL, NULL, NULL, &delay);

}

#endif

/***** Not needed ******
#if !(HAVE_NANOSLEEP)
__inline__ void
nanosleep(unsigned long nsec) {
    usleep(nsec / 1000);
}
#endif
************************/

#if !(HAVE_STRCASESTR)
char *
strcasestr(const char *haystack, register const char *needle)
{
    register const char *t;
    register size_t len = strlen(needle);

    for (t = haystack; t && *t; t++) {
        if (!strncasecmp(t, needle, len)) {
            return ((char *) t);
        }
    }
    return (NULL);
}
#endif

#if !(HAVE_STRCASECHR)
char *
strcasechr(const char *haystack, register const char needle)
{
    register const char *t;

    for (t = haystack; t && *t; t++) {
        if (tolower(*t) == tolower(needle)) {
            return ((char *) t);
        }
    }
    return (NULL);
}
#endif

#if !(HAVE_STRCASEPBRK)
char *
strcasepbrk(const char *haystack, register const char *needle)
{
    register const char *t;

    for (t = haystack; t && *t; t++) {
        if (strcasechr(needle, *t)) {
            return ((char *) t);
        }
    }
    return (NULL);
}
#endif

#if !(HAVE_STRREV)
char *
strrev(register char *str)
{
    register int i, j;

    i = strlen(str);
    for (j = 0, i--; i > j; i--, j++) {
        (void) SWAP(str[j], str[i]);
    }
    return (str);

}
#endif

#if !(HAVE_STRSEP)
char *
strsep(char **str, register char *sep)
{

    register char *s = *str;
    char *sptr;

    D_STRINGS(("strsep(%s, %s) called.\n", *str, sep));
    sptr = s;
    for (; *s && !strchr(sep, *s); s++);
    if (!*s) {
        if (s != sptr) {
            *str = s;
            D_STRINGS(("Reached end of string with token \"%s\" in buffer\n", sptr));
            return (sptr);
        } else {
            D_STRINGS(("Reached end of string\n"));
            return ((char *) NULL);
        }
    }
    *s = 0;
    *str = s + 1;
    D_STRINGS(("Got token \"%s\", *str == \"%s\"\n", sptr, *str));
    return (sptr);
}
#endif

/**
 * Returns a portion of a larger string.
 */
spif_charptr_t
spiftool_substr(spif_charptr_t str, spif_int32_t idx, spif_int32_t cnt)
{
    spif_charptr_t newstr;
    spif_uint32_t start_pos, char_count;
    spif_uint32_t len;

    REQUIRE_RVAL(str != SPIF_NULL_TYPE(charptr), SPIF_NULL_TYPE(charptr));

    len = SPIF_CAST(uint32) strlen(str);

    if (idx < 0) {
        start_pos = len + idx;
    } else {
        start_pos = idx;
    }
    REQUIRE_RVAL(start_pos < len, SPIF_NULL_TYPE(charptr));

    if (cnt <= 0) {
        char_count = len - start_pos + cnt;
    } else {
        char_count = cnt;
    }
    UPPER_BOUND(char_count, len - start_pos);

    newstr = SPIF_CAST(charptr) MALLOC(char_count + 1);
    memcpy(newstr, str + start_pos, char_count);
    newstr[char_count] = 0;
    return newstr;
}

#if HAVE_REGEX_H
/**
 * Compare a string to a regular expression.
 */
spif_bool_t
spiftool_regexp_match(const spif_charptr_t str, const spif_charptr_t pattern)
{
    static regex_t *rexp = NULL;
    register int result;
    char errbuf[256];

    if (!str) {
        /* If we're passed a NULL str, we want to free our static storage. */
        FREE(rexp);
        return FALSE;
    } else if (!rexp) {
        /* If we don't have static storage yet, make some. */
        rexp = (regex_t *) MALLOC(sizeof(regex_t));
    }

    if (pattern) {
        /* We have a pattern, so we need to compile it. */
        if ((result = regcomp(rexp, pattern, REG_EXTENDED)) != 0) {
            regerror(result, rexp, errbuf, 256);
            libast_print_error("Unable to compile regexp %s -- %s.\n", pattern, errbuf);
            return (FALSE);
        }
    }

    /* Compare the string to the compiled pattern. */
    if (((result = regexec(rexp, str, (size_t) 0, (regmatch_t *) NULL, 0)) != 0)
        && (result != REG_NOMATCH)) {
        regerror(result, rexp, errbuf, 256);
        libast_print_error("Error testing input string %s -- %s.\n", str, errbuf);
        return (FALSE);
    }
    return ((result == REG_NOMATCH) ? (FALSE) : (TRUE));
}

/**
 * Thread-safe way to compare a string to a regular expression.
 */
spif_bool_t
spiftool_regexp_match_r(register const char *str, register const char *pattern, register regex_t **rexp)
{
    register int result;
    char errbuf[256];

    ASSERT_RVAL(rexp != NULL, FALSE);
    if (*rexp == NULL) {
        *rexp = (regex_t *) MALLOC(sizeof(regex_t));
    }

    if (pattern) {
        if ((result = regcomp(*rexp, pattern, REG_EXTENDED)) != 0) {
            regerror(result, *rexp, errbuf, 256);
            libast_print_error("Unable to compile regexp %s -- %s.\n", pattern, errbuf);
            FREE(*rexp);
            return (FALSE);
        }
    }

    if (((result = regexec(*rexp, str, (size_t) 0, (regmatch_t *) NULL, 0))
         != 0) && (result != REG_NOMATCH)) {
        regerror(result, *rexp, errbuf, 256);
        libast_print_error("Error testing input string %s -- %s.\n", str, errbuf);
        return (FALSE);
    }
    return ((result == REG_NOMATCH) ? (FALSE) : (TRUE));
}
#endif

#define IS_DELIM(c)  ((delim != NULL) ? (strchr(delim, (c)) != NULL) : (isspace(c)))
#define IS_QUOTE(c)  (quote && quote == (c))

char **
spiftool_split(const char *delim, const char *str)
{
    char **slist;
    register const char *pstr;
    register char *pdest;
    char quote = 0;
    unsigned short cnt = 0;
    unsigned long len;

    REQUIRE_RVAL(str != NULL, (char **) NULL);

    if ((slist = (char **) MALLOC(sizeof(char *))) == NULL) {
        libast_print_error("split():  Unable to allocate memory -- %s\n", strerror(errno));
        return ((char **) NULL);
    }

    /* Before we do anything, skip leading "whitespace." */
    for (pstr = str; *pstr && IS_DELIM(*pstr); pstr++);

    /* The outermost for loop is where we traverse the string.  Each new
       word brings us back to the top where we resize our string list. */
    for (; *pstr; cnt++) {
        /* First, resize the list to two bigger than our count.  Why two?
           One for the string we're about to do, and one for a trailing NULL. */
        if ((slist = (char **) REALLOC(slist, sizeof(char *) * (cnt + 2))) == NULL) {
            libast_print_error("split():  Unable to allocate memory -- %s\n", strerror(errno));
            return ((char **) NULL);
        }

        /* The string we're about to create can't possibly be larger than the remainder
           of the string we have yet to parse, so allocate that much space to start. */
        len = strlen(pstr) + 1;
        if ((slist[cnt] = (char *) MALLOC(len)) == NULL) {
            libast_print_error("split():  Unable to allocate memory -- %s.\n", strerror(errno));
            return ((char **) NULL);
        }
        pdest = slist[cnt];

        /* This for loop is where we process each character. */
        for (; *pstr && (quote || !IS_DELIM(*pstr));) {
            if (*pstr == '\"' || *pstr == '\'') {
                /* It's a quote character, so set or reset the quote variable. */
                if (quote) {
                    if (quote == *pstr) {
                        quote = 0;
                    } else {
                        /* It's a single quote inside double quotes, or vice versa.  Leave it alone. */
                        *pdest++ = *pstr++;
                    }
                } else {
                    quote = *pstr;
                }
                pstr++;
            } else {
                /* Handle any backslashes that are escaping delimiters or quotes. */
                if ((*pstr == '\\') && (IS_DELIM(*(pstr + 1)) || IS_QUOTE(*(pstr + 1)))) {
                    /* Incrementing pstr here moves us past the backslash so that the line
                       below will copy the next character to the new token, no questions asked. */
                    pstr++;
                }
                *pdest++ = *pstr++;
            }
        }
        /* Add the trailing \0 to terminate the new string. */
        *pdest = 0;

        /* Reallocate the new string to be just the right size. */
        len = strlen(slist[cnt]) + 1;
        slist[cnt] = (char *) REALLOC(slist[cnt], len);

        /* Move past any trailing "whitespace." */
        for (; *pstr && IS_DELIM(*pstr); pstr++);
    }
    if (cnt == 0) {
        return NULL;
    } else {
        /* The last element of slist[] should be NULL. */
        slist[cnt] = 0;
        return slist;
    }
}

char **
spiftool_split_regexp(const char *regexp, const char *str)
{
    USE_VAR(regexp);
    USE_VAR(str);
    return (NULL);
}

char *
spiftool_join(const char *sep, char **slist)
{
    register unsigned long i;
    size_t len, slen;
    char *new_str;

    if (sep == NULL) {
        sep = "";
    }
    slen = strlen(sep);
    for (i = len = 0; slist[i]; i++) {
        len += strlen(slist[i]);
    }
    len += slen * (i - 1);
    new_str = (char *) MALLOC(len);
    strcpy(new_str, slist[0]);
    for (i = 1; slist[i]; i++) {
        if (slen) {
            strcat(new_str, sep);
        }
        strcat(new_str, slist[i]);
    }
    return new_str;
}

/* Return malloc'd pointer to index-th word in str.  "..." counts as 1 word. */
#undef IS_DELIM
#define IS_DELIM(c)  (delim ? ((c) == delim) : isspace(c))

char *
spiftool_get_word(unsigned long index, const char *str)
{
    char *tmpstr;
    char delim = 0;
    register unsigned long i, j, k;

    k = strlen(str) + 1;
    if ((tmpstr = (char *) MALLOC(k)) == NULL) {
        libast_print_error("get_word(%lu, %s):  Unable to allocate memory -- %s.\n", index, str, strerror(errno));
        return ((char *) NULL);
    }
    *tmpstr = 0;
    for (i = 0, j = 0; j < index && str[i]; j++) {
        for (; isspace(str[i]); i++);
        switch (str[i]) {
          case '\"':
              delim = '\"';
              i++;
              break;
          case '\'':
              delim = '\'';
              i++;
              break;
          default:
              delim = 0;
        }
        for (k = 0; str[i] && !IS_DELIM(str[i]);) {
            if (str[i] == '\\') {
                if (str[i + 1] == '\'' || str[i + 1] == '\"') {
                    i++;
                }
            }
            tmpstr[k++] = str[i++];
        }
        switch (str[i]) {
          case '\"':
          case '\'':
              i++;
              break;
        }
        tmpstr[k] = 0;
    }

    if (j != index) {
        FREE(tmpstr);
        D_STRINGS(("get_word(%lu, %s) returning NULL.\n", index, str));
        return ((char *) NULL);
    } else {
        tmpstr = (char *) REALLOC(tmpstr, strlen(tmpstr) + 1);
        D_STRINGS(("get_word(%lu, %s) returning \"%s\".\n", index, str, tmpstr));
        return (tmpstr);
    }
}

/* Return pointer into str to index-th word in str.  "..." counts as 1 word. */
char *
spiftool_get_pword(unsigned long index, const char *str)
{
    register const char *tmpstr = str;
    register unsigned long j;

    if (!str)
        return ((char *) NULL);
    for (; isspace(*tmpstr) && *tmpstr; tmpstr++);
    for (j = 1; j < index && *tmpstr; j++) {
        for (; !isspace(*tmpstr) && *tmpstr; tmpstr++);
        for (; isspace(*tmpstr) && *tmpstr; tmpstr++);
    }

    if (*tmpstr == '\"' || *tmpstr == '\'') {
        tmpstr++;
    }
    if (*tmpstr == '\0') {
        D_STRINGS(("get_pword(%lu, %s) returning NULL.\n", index, str));
        return ((char *) NULL);
    } else {
        D_STRINGS(("get_pword(%lu, %s) returning \"%s\"\n", index, str, tmpstr));
        return (char *) tmpstr;
    }
}

/* Returns the number of words in str, for use with get_word() and get_pword().  "..." counts as 1 word. */
unsigned long
spiftool_num_words(const char *str)
{
    register unsigned long cnt = 0;
    char delim = 0;
    register unsigned long i;

    for (i = 0; str[i] && IS_DELIM(str[i]); i++);
    for (; str[i]; cnt++) {
        switch (str[i]) {
          case '\"':
              delim = '\"';
              i++;
              break;
          case '\'':
              delim = '\'';
              i++;
              break;
          default:
              delim = 0;
        }
        for (; str[i] && !IS_DELIM(str[i]); i++);
        switch (str[i]) {
          case '\"':
          case '\'':
              i++;
              break;
        }
        for (; str[i] && isspace(str[i]); i++);
    }

    D_STRINGS(("num_words() returning %lu\n", cnt));
    return (cnt);
}

/* chomp() removes leading and trailing whitespace from a string */
char *
spiftool_chomp(char *s)
{

    register char *front, *back;

    ASSERT_RVAL(s != NULL, NULL);
    for (front = s; *front && isspace(*front); front++);
    for (back = s + strlen(s) - 1; *back && isspace(*back) && back > front; back--);

    *(++back) = 0;
    if (front != s) {
        memmove(s, front, back - front + 1);
    }
    return (s);
}

char *
spiftool_strip_whitespace(register char *str)
{
    register unsigned long i, j;

    ASSERT_RVAL(str != NULL, NULL);
    if ((j = strlen(str))) {
        for (i = j - 1; isspace(*(str + i)); i--);
        str[j = i + 1] = 0;
        for (i = 0; isspace(*(str + i)); i++);
        j -= i;
        memmove(str, str + i, j + 1);
    }
    return (str);
}

char *
spiftool_downcase_str(char *str)
{
    register char *tmp;

    for (tmp = str; *tmp; tmp++) {
        *tmp = tolower(*tmp);
    }
    D_STRINGS(("downcase_str() returning %s\n", str));
    return (str);
}

char *
spiftool_upcase_str(char *str)
{
    register char *tmp;

    for (tmp = str; *tmp; tmp++) {
        *tmp = toupper(*tmp);
    }
    D_STRINGS(("upcase_str() returning %s\n", str));
    return (str);
}

char *
spiftool_condense_whitespace(char *s)
{

    register unsigned char gotspc = 0;
    register char *pbuff = s, *pbuff2 = s;

    D_STRINGS(("condense_whitespace(%s) called.\n", s));
    for (; *pbuff2; pbuff2++) {
        if (isspace(*pbuff2)) {
            if (!gotspc) {
                *pbuff = ' ';
                gotspc = 1;
                pbuff++;
            }
        } else {
            *pbuff = *pbuff2;
            gotspc = 0;
            pbuff++;
        }
    }
    if ((pbuff >= s) && (isspace(*(pbuff - 1))))
        pbuff--;
    *pbuff = 0;
    D_STRINGS(("condense_whitespace() returning \"%s\".\n", s));
    return ((char *) REALLOC(s, strlen(s) + 1));
}

char *
spiftool_safe_str(register char *str, unsigned short len)
{
    register unsigned short i;

    for (i = 0; i < len; i++) {
        if (iscntrl(str[i])) {
            str[i] = '.';
        }
    }

    return (str);
}

void
spiftool_hex_dump(void *buff, register size_t count)
{

    register unsigned long j, k, l;
    register unsigned char *ptr;
    unsigned char buffr[9];

    fprintf(stderr, "  Address  |  Size  | Offset  | 00 01 02 03 04 05 06 07 |  ASCII  \n");
    fprintf(stderr, "-----------+--------+---------+-------------------------+---------\n");
    for (ptr = (unsigned char *) buff, j = 0; j < count; j += 8) {
        fprintf(stderr, " %10p | %06lu | %07x | ", buff, (unsigned long) count, (unsigned int) j);
        l = ((count - j < 8) ? (count - j) : (8));
        memcpy(buffr, ptr + j, l);
        memset(buffr + l, 0, 9 - l);
        for (k = 0; k < l; k++) {
            fprintf(stderr, "%02x ", buffr[k]);
        }
        for (; k < 8; k++) {
            fprintf(stderr, "   ");
        }
        fprintf(stderr, "| %-8s\n", spiftool_safe_str((char *) buffr, l));
    }
}

#define CHAR_CLASS_MATCH(a, b)      ((isalpha(a) && isalpha(b)) \
                                     || (isdigit(a) && isdigit(b)) \
                                     || (!isalnum(a) && !isalnum(b)))
spif_cmp_t
spiftool_version_compare(const char *v1, const char *v2)
{
    char buff1[128], buff2[128];

    for (; *v1 && *v2; ) {
        if (isalpha(*v1) && isalpha(*v2)) {
            char *p1 = buff1, *p2 = buff2;
            spif_int8_t ival1 = 6, ival2 = 6;

            /* Compare words.  First, copy each word into buffers. */
            for (; isalpha(*v1); v1++, p1++) *p1 = *v1;
            for (; isalpha(*v2); v2++, p2++) *p2 = *v2;
            *p1 = *p2 = 0;

            /* Change the buffered strings to lowercase for easier comparison. */
            spiftool_downcase_str(buff1);
            spiftool_downcase_str(buff2);

            /* Some strings require special handling. */
            if (!strcmp(buff1, "snap")) {
                ival1 = 1;
            } else if (!strcmp(buff1, "pre")) {
                ival1 = 2;
            } else if (!strcmp(buff1, "alpha")) {
                ival1 = 3;
            } else if (!strcmp(buff1, "beta")) {
                ival1 = 4;
            } else if (!strcmp(buff1, "rc")) {
                ival1 = 5;
            }
            if (!strcmp(buff2, "snap")) {
                ival2 = 1;
            } else if (!strcmp(buff2, "pre")) {
                ival2 = 2;
            } else if (!strcmp(buff2, "alpha")) {
                ival2 = 3;
            } else if (!strcmp(buff2, "beta")) {
                ival2 = 4;
            } else if (!strcmp(buff2, "rc")) {
                ival2 = 5;
            }
            if (ival1 != ival2) {
                /* If the values are different, compare them. */
                return SPIF_CMP_FROM_INT(ival1 - ival2);
            } else if (ival1 == 6) {
                int c;

                /* Two arbitrary strings.  Compare them too. */
                if ((c = strcmp(buff1, buff2)) != 0) {
                    return SPIF_CMP_FROM_INT(c);
                }
            }
        } else if (isdigit(*v1) && isdigit(*v2)) {
            char *p1 = buff1, *p2 = buff2;
            spif_int32_t ival1, ival2;
            spif_cmp_t c;

            /* Compare numbers.  First, copy each number into buffers. */
            for (; isdigit(*v1); v1++, p1++) *p1 = *v1;
            for (; isdigit(*v2); v2++, p2++) *p2 = *v2;
            *p1 = *p2 = 0;

            /* Convert the strings into actual integers. */
            ival1 = SPIF_CAST(int32) strtol(buff1, (char **) NULL, 10);
            ival2 = SPIF_CAST(int32) strtol(buff2, (char **) NULL, 10);

            /* Compare the integers and return if not equal. */
            c = SPIF_CMP_FROM_INT(ival1 - ival2);
            if (!SPIF_CMP_IS_EQUAL(c)) {
                return c;
            }
        } else if (!isalnum(*v1) && !isalnum(*v2)) {
            char *p1 = buff1, *p2 = buff2;
            spif_cmp_t c;

            /* Compare non-alphanumeric strings. */
            for (; !isalnum(*v1); v1++, p1++) *p1 = *v1;
            for (; !isalnum(*v2); v2++, p2++) *p2 = *v2;
            *p1 = *p2 = 0;

            c = SPIF_CMP_FROM_INT(strcasecmp(buff1, buff2));
            if (!SPIF_CMP_IS_EQUAL(c)) {
                return c;
            }
        } else {
            return SPIF_CMP_FROM_INT(strcasecmp(buff1, buff2));
        }
    }

    /* We've reached the end of one of the strings. */
    if (*v1) {
        if (!BEG_STRCASECMP(v1, "snap") || !BEG_STRCASECMP(v1, "pre")
            || !BEG_STRCASECMP(v1, "alpha") || !BEG_STRCASECMP(v1, "beta")) {
            return SPIF_CMP_LESS;
        } else {
            return SPIF_CMP_GREATER;
        }
    } else if (*v2) {
        if (!BEG_STRCASECMP(v2, "snap") || !BEG_STRCASECMP(v2, "pre")
            || !BEG_STRCASECMP(v2, "alpha") || !BEG_STRCASECMP(v2, "beta")) {
            return SPIF_CMP_GREATER;
        } else {
            return SPIF_CMP_LESS;
        }
    }
    return SPIF_CMP_EQUAL;
}

/**
 * @defgroup DOXGRP_STRINGS String Utility Routines
 *
 * This group of functions/defines/macros provides oft-needed string
 * manipulation functionality which is not found (at least not
 * portably) in libc.
 *
 * Over the past several years, I have had to implement some simple
 * string routines which, for whatever reason, are not part of the
 * Standard C Library.  I have gathered these macros, functions,
 * etc. into LibAST in the hopes that others might not have to
 * reimplement them themselves.  Unlike the memory/debugging/object
 * stuff, there really isn't a well-defined architecture surrounding
 * these.  They're just utility functions and implementations for
 * non-portable functionality.
 *
 * A small sample program demonstrating some of these routines can be
 * found @link strings_example.c here @endlink.
 */

/**
 * @example strings_example.c
 * Example code for using the string routines.
 *
 */
