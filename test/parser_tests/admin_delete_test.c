#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_delete.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_delete_user)
{
    struct admin_delete_parser parser;
    admin_delete_parser_init(&parser);
    uint8_t data[] = {
        0x05, // Action: DELETE
        0x00, // Field: USERS
        0x04, // Keylen
        0x46, 0x41, 0x55, 0x53, // FAUS
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_delete_state st = admin_delete_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_delete_done, st);
    ck_assert_str_eq("FAUS", (char*) parser.key);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_delete_invalid_action)
{
    struct admin_delete_parser parser;
    admin_delete_parser_init(&parser);
    uint8_t data[] = {
        0x03, // Action: INVALID
        0x00, // Field: USERS
        0x04, // Keylen
        0x46, 0x41, 0x55, 0x53, // FAUS
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_delete_state st = admin_delete_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_delete_error_action, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_delete_invalid_field)
{
    struct admin_delete_parser parser;
    admin_delete_parser_init(&parser);
    uint8_t data[] = {
        0x05, // Action: DELETE
        0x01, // Field: INVALID
        0x04, // Keylen
        0x46, 0x41, 0x55, 0x53, // FAUS
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_delete_state st = admin_delete_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_delete_error_field, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_delete_invalid_keylen)
{
    struct admin_delete_parser parser;
    admin_delete_parser_init(&parser);
    uint8_t data[] = {
        0x05, // Action: DELETE
        0x00, // Field: USERS
        0x00, // Keylen: 0, INVALID
        0x46, 0x41, 0x55, 0x53, // FAUS
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_delete_state st = admin_delete_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_delete_error_keylen, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_delete");

	TCase* testcase_admin_delete_user = tcase_create("admin_delete_user");
	tcase_add_test(testcase_admin_delete_user, test_admin_delete_user);
	suite_add_tcase(s, testcase_admin_delete_user);

  TCase* testcase_admin_delete_invalid_action = tcase_create("admin_delete_invalid_action");
	tcase_add_test(testcase_admin_delete_invalid_action, test_admin_delete_invalid_action);
	suite_add_tcase(s, testcase_admin_delete_invalid_action);

  TCase* testcase_admin_delete_invalid_field = tcase_create("admin_delete_invalid_field");
	tcase_add_test(testcase_admin_delete_invalid_field, test_admin_delete_invalid_field);
	suite_add_tcase(s, testcase_admin_delete_invalid_field);

  TCase* testcase_admin_delete_invalid_keylen = tcase_create("admin_delete_invalid_keylen");
	tcase_add_test(testcase_admin_delete_invalid_keylen, test_admin_delete_invalid_keylen);
	suite_add_tcase(s, testcase_admin_delete_invalid_keylen);

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
