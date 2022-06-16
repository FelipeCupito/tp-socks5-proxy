#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_edit.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))


START_TEST(test_admin_edit_username)
{
    struct admin_edit_parser parser;
    admin_edit_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Action: EDIT
        0x00, // Field: USERS
        0x04, // Keylen
        0x46, 0x41, 0x55, 0x53, // Key: FAUS
        0x00, // Attribute: username
        0x04, // Valuelen
        0x46, 0x41, 0x55, 0x5A, // Value: FAUZ
    };
    buffer b;
    bool errored = false;
    FIXBUF(b, data);
    enum admin_edit_state st = admin_edit_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_edit_done, st);
    ck_assert_str_eq("FAUS", (char*) parser.key);
    ck_assert_uint_eq(0, parser.attr);
    ck_assert_str_eq("FAUZ", (char*) parser.value);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_edit_password)
{
    struct admin_edit_parser parser;
    admin_edit_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Action: EDIT
        0x00, // Field: USERS
        0x04, // Keylen
        0x46, 0x41, 0x55, 0x53, // Key: FAUS
        0x01, // Attribute: password
        0x04, // Valuelen
        0x46, 0x41, 0x55, 0x5A, // Value: FAUZ
    };
    buffer b;
    bool errored = false;
    FIXBUF(b, data);
    enum admin_edit_state st = admin_edit_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_edit_done, st);
    ck_assert_str_eq("FAUS", (char*) parser.key);
    ck_assert_uint_eq(1, parser.attr);
    ck_assert_str_eq("FAUZ", (char*) parser.value);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_edit_invalid_keylen)
{
    struct admin_edit_parser parser;
    admin_edit_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Action: EDIT
        0x00, // Field: USERS
        0x00, // Keylen
        0x46, 0x41, 0x55, 0x53, // Key: FAUS
        0x01, // Attribute: password
        0x04, // Valuelen
        0x46, 0x41, 0x55, 0x5A, // Value: FAUZ
    };
    buffer b;
    bool errored = false;
    FIXBUF(b, data);
    enum admin_edit_state st = admin_edit_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_edit_error_keylen, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_edit_invalid_action)
{
    struct admin_edit_parser parser;
    admin_edit_parser_init(&parser);
    uint8_t data[] = {
        0x09, // Action: INVALI
        0x00, // Field: USERS
        0x00, // Keylen
        0x46, 0x41, 0x55, 0x53, // Key: FAUS
        0x01, // Attribute: password
        0x04, // Valuelen
        0x46, 0x41, 0x55, 0x5A, // Value: FAUZ
    };
    buffer b;
    bool errored = false;
    FIXBUF(b, data);
    enum admin_edit_state st = admin_edit_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_edit_error_action, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_edit_invalid_field)
{
    struct admin_edit_parser parser;
    admin_edit_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Action: EDIT
        0x01, // Field: INVALID
        0x00, // Keylen
        0x46, 0x41, 0x55, 0x53, // Key: FAUS
        0x01, // Attribute: password
        0x04, // Valuelen
        0x46, 0x41, 0x55, 0x5A, // Value: FAUZ
    };
    buffer b;
    bool errored = false;
    FIXBUF(b, data);
    enum admin_edit_state st = admin_edit_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_edit_error_field, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_edit_invalid_attr)
{
    struct admin_edit_parser parser;
    admin_edit_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Action: EDIT
        0x00, // Field: USERS
        0x04, // Keylen
        0x46, 0x41, 0x55, 0x53, // Key: FAUS
        0x09, // Attribute: INVALID
        0x04, // Valuelen
        0x46, 0x41, 0x55, 0x5A, // Value: FAUZ
    };
    buffer b;
    bool errored = false;
    FIXBUF(b, data);
    enum admin_edit_state st = admin_edit_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_edit_error_attribute, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_edit");

	TCase* testcase_admin_edit_username = tcase_create("admin_edit_username");
	tcase_add_test(testcase_admin_edit_username, test_admin_edit_username);
	suite_add_tcase(s, testcase_admin_edit_username);

    TCase* testcase_admin_edit_password = tcase_create("admin_edit_password");
	tcase_add_test(testcase_admin_edit_password, test_admin_edit_password);
	suite_add_tcase(s, testcase_admin_edit_password);

    TCase* testcase_admin_edit_invalid_action = tcase_create("admin_edit_invalid_action");
	tcase_add_test(testcase_admin_edit_invalid_action, test_admin_edit_invalid_action);
	suite_add_tcase(s, testcase_admin_edit_invalid_action);

    TCase* testcase_admin_edit_invalid_attr = tcase_create("admin_edit_invalid_attr");
	tcase_add_test(testcase_admin_edit_invalid_attr, test_admin_edit_invalid_attr);
	suite_add_tcase(s, testcase_admin_edit_invalid_attr);

    TCase* testcase_admin_edit_invalid_field = tcase_create("admin_edit_invalid_field");
	tcase_add_test(testcase_admin_edit_invalid_field, test_admin_edit_invalid_field);
	suite_add_tcase(s, testcase_admin_edit_invalid_field);

    TCase* testcase_admin_edit_invalid_keylen = tcase_create("admin_edit_invalid_keylen");
	tcase_add_test(testcase_admin_edit_invalid_keylen, test_admin_edit_invalid_keylen);
	suite_add_tcase(s, testcase_admin_edit_invalid_action);

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