#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_configstatus.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_configstatus_auth)
{
    struct admin_configstatus_parser parser;
    admin_configstatus_parser_init(&parser);
    uint8_t data[] = {
        0x04, // Action: CONFIGSTATUS
        0x03, // Field: AUTH
        0x00, // Status: ON 
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configstatus_state st = admin_configstatus_consume(&b, &parser, &errored);
    ck_assert_uint_eq(ON,parser.status);
    ck_assert_uint_eq(admin_configstatus_done, st);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_configstatus_spoof)
{
    struct admin_configstatus_parser parser;
    admin_configstatus_parser_init(&parser);
    uint8_t data[] = {
        0x04, // Action: CONFIGSTATUS
        0x04, // Field: SPOOF
        0x01, // Status: OFF
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configstatus_state st = admin_configstatus_consume(&b, &parser, &errored);
    ck_assert_uint_eq(OFF,parser.status);
    ck_assert_uint_eq(admin_configstatus_done, st);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_configstatus_invalid_action)
{
    struct admin_configstatus_parser parser;
    admin_configstatus_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Action: INVALID
        0x04, // Field: SPOOF
        0x01, // Status: OFF
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configstatus_state st = admin_configstatus_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_configstatus_error_action, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_configstatus_invalid_field)
{
    struct admin_configstatus_parser parser;
    admin_configstatus_parser_init(&parser);
    uint8_t data[] = {
        0x04, // Action: CONFIGSTATUS
        0x00, // Field: INVALID
        0x01, // Status: OFF
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configstatus_state st = admin_configstatus_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_configstatus_error_field, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_configstatus_invalid_status)
{
    struct admin_configstatus_parser parser;
    admin_configstatus_parser_init(&parser);
    uint8_t data[] = {
        0x04, // Action: CONFIGSTATUS
        0x04, // Field: SPOOF
        0x02, // Status: INVALID
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configstatus_state st = admin_configstatus_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_configstatus_error_status, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_configstatus");

	TCase* testcase_admin_configstatus_auth = tcase_create("admin_configstatus_auth");
	tcase_add_test(testcase_admin_configstatus_auth, test_admin_configstatus_auth);
	suite_add_tcase(s, testcase_admin_configstatus_auth);

    TCase* testcase_admin_configstatus_spoof = tcase_create("admin_configstatus_spoof");
	tcase_add_test(testcase_admin_configstatus_spoof, test_admin_configstatus_spoof);
	suite_add_tcase(s, testcase_admin_configstatus_spoof);

    TCase* testcase_admin_configstatus_invalid_field = tcase_create("admin_configstatus_invalid_field");
	tcase_add_test(testcase_admin_configstatus_invalid_field, test_admin_configstatus_invalid_field);
	suite_add_tcase(s, testcase_admin_configstatus_invalid_field);

    TCase* testcase_admin_configstatus_invalid_status = tcase_create("admin_configstatus_invalid_status");
	tcase_add_test(testcase_admin_configstatus_invalid_status, test_admin_configstatus_invalid_status);
	suite_add_tcase(s, testcase_admin_configstatus_invalid_status);

    TCase* testcase_admin_configstatus_invalid_action = tcase_create("admin_configstatus_invalid_action");
	tcase_add_test(testcase_admin_configstatus_invalid_action, test_admin_configstatus_invalid_action);
	suite_add_tcase(s, testcase_admin_configstatus_invalid_action);

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
