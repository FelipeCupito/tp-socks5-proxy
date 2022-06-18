#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_put.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_put_normal)
{
    struct admin_put_parser parser;
    admin_put_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Action: PUT
        0x00, // Field: USERS
        0x04, // Namelen
        0x46, 0x41, 0x55, 0x53, // faus
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_put_state st = admin_put_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_put_done, st);
    ck_assert_str_eq("FAUS", (char*) parser.user.username);
    ck_assert_str_eq("FOO", (char*) parser.pass.passwd);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_put_invalid_namelen)
{
    struct admin_put_parser parser;
    admin_put_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Action: PUT
        0x00, // Field: USERS
        0x00, // Namelen: 0
        0x46, 0x41, 0x55, 0x53, // faus
        0x03, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_put_state st = admin_put_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_put_error_namelen, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_put_invalid_passlen)
{
    struct admin_put_parser parser;
    admin_put_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Action: PUT
        0x00, // Field: USERS
        0x04, // Namelen: 0
        0x46, 0x41, 0x55, 0x53, // faus
        0x00, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_put_state st = admin_put_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_put_error_passlen, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_put_invalid_action)
{
    struct admin_put_parser parser;
    admin_put_parser_init(&parser);
    uint8_t data[] = {
        0x04, // Action: PUT
        0x00, // Field: USERS
        0x04, // Namelen: 0
        0x46, 0x41, 0x55, 0x53, // faus
        0x00, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_put_state st = admin_put_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_put_error_action, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_put_invalid_field)
{
    struct admin_put_parser parser;
    admin_put_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Action: PUT
        0x08, // Field: USERS
        0x04, // Namelen: 0
        0x46, 0x41, 0x55, 0x53, // faus
        0x00, // pass len
        0x46, 0x4F, 0x4F, // foo
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_put_state st = admin_put_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_put_error_field, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_put");

	TCase* testcase_admin_put_normal = tcase_create("admin_put_normal");
	tcase_add_test(testcase_admin_put_normal, test_admin_put_normal);
	suite_add_tcase(s, testcase_admin_put_normal);

  TCase* testcase_admin_put_invalid_passlen = tcase_create("admin_put_invalid_passlen");
	tcase_add_test(testcase_admin_put_invalid_passlen, test_admin_put_invalid_passlen);
	suite_add_tcase(s, testcase_admin_put_invalid_passlen);

  TCase* testcase_admin_put_invalid_namelen = tcase_create("admin_put_invalid_namelen");
	tcase_add_test(testcase_admin_put_invalid_namelen, test_admin_put_invalid_namelen);
	suite_add_tcase(s, testcase_admin_put_invalid_namelen);

  TCase* testcase_admin_put_invalid_field = tcase_create("admin_put_invalid_field");
	tcase_add_test(testcase_admin_put_invalid_field, test_admin_put_invalid_field);
	suite_add_tcase(s, testcase_admin_put_invalid_field);

  TCase* testcase_admin_put_invalid_action = tcase_create("admin_put_invalid_action");
	tcase_add_test(testcase_admin_put_invalid_action, test_admin_put_invalid_action);
	suite_add_tcase(s, testcase_admin_put_invalid_action);

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