#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_connect.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_connect_normal)
{
    struct admin_connect_parser parser;
    admin_connect_parser_init(&parser);
    uint8_t data[] = {
        0x00, // version
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_connect_state st = admin_connect_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
		ck_assert_uint_eq(0, parser.version);
    ck_assert_str_eq("FOO", (char*) parser.password.passwd);
    ck_assert_uint_eq(admin_connect_done, st);
}
END_TEST

START_TEST(test_admin_connect_unsupported_version)
{
    struct admin_connect_parser parser;
    admin_connect_parser_init(&parser);
    uint8_t data[] = {
        0xFF, // version
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_connect_state st = admin_connect_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(admin_connect_error_version, st);
}
END_TEST

START_TEST(test_admin_connect_passlen_zero)
{
    struct admin_connect_parser parser;
    admin_connect_parser_init(&parser);
    uint8_t data[] = {
        0x00, // version
        0x00, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_connect_state st = admin_connect_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(admin_connect_error_passlen, st);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_connect");

	TCase* testcase_admin_connect = tcase_create("admin_connect_normal");
	tcase_add_test(testcase_admin_connect, test_admin_connect_normal);
	suite_add_tcase(s, testcase_admin_connect);

	TCase* testcase_admin_connect_unsupported_version = tcase_create("admin_connect_unsupported_version");
	tcase_add_test(testcase_admin_connect_unsupported_version, test_admin_connect_unsupported_version);
	suite_add_tcase(s, testcase_admin_connect_unsupported_version);

  TCase* testcase_admin_connect_passlen_zero = tcase_create("admin_connect_passlen_zero");
	tcase_add_test(testcase_admin_connect_passlen_zero, test_admin_connect_passlen_zero);
	suite_add_tcase(s, testcase_admin_connect_passlen_zero);

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