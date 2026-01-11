/*
Pars SARICA <pars@parssarica.com>
*/

#define PCRE2_CODE_UNIT_WIDTH 8
#include "testify.h"
#include <pcre2.h>

int regex_pass(char *regexcode, char *s, int slength)
{
    int errorcode;
    int rc;
    int result;
    PCRE2_SIZE erroroffset;
    pcre2_code *re = pcre2_compile((PCRE2_SPTR)regexcode, PCRE2_ZERO_TERMINATED,
                                   0, &errorcode, &erroroffset, NULL);

    if (!re)
    {
        return -1;
    }

    pcre2_match_data *match = pcre2_match_data_create_from_pattern(re, NULL);
    if (!match)
    {
        pcre2_code_free(re);
        return -1;
    }

    rc = pcre2_match(re, (PCRE2_SPTR)s, slength, 0, 0, match, NULL);

    if (rc >= 0)
        result = 1;
    else
        result = 0;

    pcre2_match_data_free(match);
    pcre2_code_free(re);

    return result;
}
