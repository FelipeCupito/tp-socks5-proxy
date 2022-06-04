#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/hello.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

static void on_hello_method(void *data, const uint8_t method)
{
    uint8_t *selected = data;
    if (method == METHOD_NO_AUTH_REQUIRED || method >= 0xFA)
    {
        *selected = method;
    }
}

START_TEST(test_hello_normal)
{
    uint8_t method = METHOD_NO_ACCEPTABLE_METHODS;
    struct hello_parser parser = {
        .data = &method,
        .on_auth_method = on_hello_method,
    };
    hello_parser_init(&parser);
    uint8_t data[] = {
        0x05, // socks version
        0x02, // nmethods
        0x00, // no authentication
        0x01, // gssapi
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum hello_state st = hello_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(METHOD_NO_AUTH_REQUIRED, method);
    ck_assert_uint_eq(hello_done, st);
}
END_TEST

START_TEST(test_hello_no_methods)
{
    uint8_t method = METHOD_NO_ACCEPTABLE_METHODS;
    struct hello_parser parser = {
        .data = &method,
        .on_auth_method = on_hello_method,
    };
    hello_parser_init(&parser);
    uint8_t data[] = {
        0x05, // socks version
        0x00, // nmethods
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum hello_state st = hello_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(METHOD_NO_ACCEPTABLE_METHODS, method);
    ck_assert_uint_eq(hello_done, st);
}
END_TEST


Suite *
suite(void)
{
    Suite *s = suite_create("hello");

    // Normal usage test case
    TCase *tc_normal = tcase_create("hello_normal");
    tcase_add_test(tc_normal, test_hello_normal);
    suite_add_tcase(s, tc_normal);

    // // No methods specified test case
    TCase *tc_no_methods = tcase_create("hello_no_methods");
    tcase_add_test(tc_no_methods, test_hello_no_methods);
    suite_add_tcase(s, tc_no_methods);

    // // Unsupported socks version test case
    // TCase *tc_unsupported_version = tcase_create("hello_unsupported_version");
    // tcase_add_test(tc_unsupported_version, test_hello_unsupported_socks_version);
    // suite_add_tcase(s, tc_unsupported_version);

    // // Multiple requests test case
    // TCase *tc_multiple_requests = tcase_create("hello_multiple_requests");
    // tcase_add_test(tc_multiple_requests, test_hello_multiple_requests);
    // suite_add_tcase(s, tc_multiple_requests);

    return s;
}

int main(void)
{
    SRunner *sr = srunner_create(suite());
    int number_failed;

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}