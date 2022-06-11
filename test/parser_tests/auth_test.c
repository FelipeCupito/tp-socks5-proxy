#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/auth.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_auth_normal)
{
    struct auth_parser parser;
    auth_parser_init(&parser);
    uint8_t data[] = {
        0x01, // auth version
        0x04, // user len
        0x46, 0x41, 0x55, 0x53, // faus
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum auth_state st = auth_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_str_eq("FAUS", (char*) parser.user.username);
    ck_assert_str_eq("FOO", (char*) parser.pass.passwd);
    ck_assert_uint_eq(auth_done, st);
}
END_TEST

START_TEST(test_auth_unsupported_version)
{
    struct auth_parser parser;
    auth_parser_init(&parser);
    uint8_t data[] = {
        0x02, // auth version
        0x04, // user len
        0x46, 0x41, 0x55, 0x53, // faus
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum auth_state st = auth_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(auth_error_unsupported_version, st);
}
END_TEST

START_TEST(test_auth_userlen_invalid)
{
    struct auth_parser parser;
    auth_parser_init(&parser);
    uint8_t data[] = {
        0x01, // auth version
        0x00, // user len
        0x46, 0x41, 0x55, 0x53, // faus
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum auth_state st = auth_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(auth_error_user_len, st);
}
END_TEST

START_TEST(test_auth_passlen_invalid)
{
    struct auth_parser parser;
    auth_parser_init(&parser);
    uint8_t data[] = {
        0x01, // auth version
        0x04, // user len
        0x46, 0x41, 0x55, 0x53, // faus
        0x00, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum auth_state st = auth_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(auth_error_pass_len, st);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("auth");

	TCase* testcase_auth = tcase_create("auth_normal");
	tcase_add_test(testcase_auth, test_auth_normal);
	suite_add_tcase(s, testcase_auth);

	TCase *testcase_userlen_invalid = tcase_create("auth_userlen_invalid");
	tcase_add_test(testcase_userlen_invalid, test_auth_userlen_invalid);
	suite_add_tcase(s, testcase_userlen_invalid);

  TCase *testcase_passlen_invalid = tcase_create("auth_passlen_invalid");
	tcase_add_test(testcase_passlen_invalid, test_auth_passlen_invalid);
	suite_add_tcase(s, testcase_passlen_invalid);

	TCase *testcase_unsupported_version = tcase_create("auth_unsupported_version");
	tcase_add_test(testcase_unsupported_version, test_auth_unsupported_version);
	suite_add_tcase(s, testcase_unsupported_version);

	// TCase *testcase_multiple_requests = tcase_create("auth_multiple_requests");
	// tcase_add_test(testcase_multiple_requests, test_auth_multiple_requests);
	// suite_add_tcase(s, testcase_multiple_requests);

	return s;
}

int main(void) {
	SRunner* sr = srunner_create(suite());
	int number_failed;

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}