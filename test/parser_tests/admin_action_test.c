#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_action.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_action_get)
{
    struct admin_action_parser parser;
    admin_action_parser_init(&parser);
    uint8_t data[] = {
        0x00 // GET
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_action_state st = admin_action_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(0,parser.action);
    ck_assert_uint_eq(admin_action_done, st);
}
END_TEST

START_TEST(test_admin_action_set)
{
    struct admin_action_parser parser;
    admin_action_parser_init(&parser);
    uint8_t data[] = {
        0x01 // SET
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_action_state st = admin_action_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(1,parser.action);
    ck_assert_uint_eq(admin_action_done, st);
}
END_TEST

START_TEST(test_admin_action_invalid)
{
    struct admin_action_parser parser;
    admin_action_parser_init(&parser);
    uint8_t data[] = {
        0xE6 // SET
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_action_state st = admin_action_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(admin_action_error_invalid_action, st);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_action");

	TCase* testcase_admin_action_get = tcase_create("admin_action_get");
	tcase_add_test(testcase_admin_action_get, test_admin_action_get);
	suite_add_tcase(s, testcase_admin_action_get);

  TCase* testcase_admin_action_set = tcase_create("admin_action_set");
	tcase_add_test(testcase_admin_action_set, test_admin_action_set);
	suite_add_tcase(s, testcase_admin_action_set);

  TCase* testcase_admin_action_invalid = tcase_create("admin_action_invalid");
	tcase_add_test(testcase_admin_action_invalid, test_admin_action_invalid);
	suite_add_tcase(s, testcase_admin_action_invalid);

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