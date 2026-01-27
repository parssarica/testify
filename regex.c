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

sds regex_extract(char *regexcode, char *s, int slength, int groupnumber)
{
    pcre2_code *re;
    pcre2_match_data *match_data;
    int errornumber;
    int rc;
    PCRE2_SIZE erroroffset;
    sds result_sds = NULL;
    char *result = NULL;
    PCRE2_UCHAR *buffer = NULL;
    PCRE2_SIZE buffer_len = 0;

    re = pcre2_compile((PCRE2_SPTR)regexcode, PCRE2_ZERO_TERMINATED, 0,
                       &errornumber, &erroroffset, NULL);

    if (re == NULL)
        return NULL;

    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    rc = pcre2_match(re, (PCRE2_SPTR)s, (PCRE2_SIZE)slength, 0, 0, match_data,
                     NULL);
    if (rc > groupnumber)
    {
        if (pcre2_substring_get_bynumber(match_data, groupnumber, &buffer,
                                         &buffer_len) == 0)
        {
            result = (char *)buffer;
            result_sds = sdsnew(result);
            pcre2_substring_free((PCRE2_UCHAR *)result);
        }
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return result_sds;
}

int regex_count(char *regexcode, char *s, size_t slen)
{
    pcre2_code *re;
    pcre2_match_data *match_data;
    int errornumber;
    PCRE2_SIZE erroroffset;
    int count = 0;
    PCRE2_SIZE subject_length = slen;
    PCRE2_SIZE start_offset = 0;
    PCRE2_SIZE *ovector;

    re = pcre2_compile((PCRE2_SPTR)regexcode, PCRE2_ZERO_TERMINATED, 0,
                       &errornumber, &erroroffset, NULL);
    if (re == NULL)
        return -1;

    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    while (pcre2_match(re, (PCRE2_SPTR)s, subject_length, start_offset, 0,
                       match_data, NULL) >= 0)
    {
        count++;
        ovector = pcre2_get_ovector_pointer(match_data);
        start_offset = ovector[1];
        if (ovector[0] == ovector[1])
        {
            start_offset++;
            if (start_offset > subject_length)
                break;
        }
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return count;
}
