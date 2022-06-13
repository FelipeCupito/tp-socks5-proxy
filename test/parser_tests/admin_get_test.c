#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_get.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_get_n_users)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x00, // Field: USERS   
        0X00, // Option: NONE
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(0, parser.field);
    ck_assert_uint_eq(0, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_passwords)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Field: PASSWORDS
        0X00, // Option: NONE
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(1, parser.field);
    ck_assert_uint_eq(0, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_buffsize)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x04, // Field: BUFFSIZE
        0X00, // Option: NONE
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(4, parser.field);
    ck_assert_uint_eq(0, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_authstatus)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x05, // Field: AUTH STATUS
        0X00, // Option: NONE
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(5, parser.field);
    ck_assert_uint_eq(0, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_spoofstatus)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x06, // Field: AUTH STATUS
        0X00, // Option: NONE
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(6, parser.field);
    ck_assert_uint_eq(0, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_sentbytes)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x03, // Field: BYTES
        0X01, // Option: SENT BYTES
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(3, parser.field);
    ck_assert_uint_eq(1, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_rcvbytes)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x03, // Field: BYTES
        0X02, // Option: RECEIVED BYTES
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(3, parser.field);
    ck_assert_uint_eq(2, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_connections_h)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Field: CONNECTIONS
        0X03, // Option: HISTORIC CONNECTIONS
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(2, parser.field);
    ck_assert_uint_eq(3, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_connections_c)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x02, // Field: CONNECTIONS
        0X04, // Option: CURRENT CONNECTIONS
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(2, parser.field);
    ck_assert_uint_eq(4, parser.option);
    ck_assert_uint_eq(admin_get_done, st);
}
END_TEST

START_TEST(test_admin_get_invalid_field)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x42, // Field: INEXISTENT
        0X04, // Option: CURRENT CONNECTIONS
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(admin_get_error_field, st);
}
END_TEST

START_TEST(test_admin_get_invalid_option)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x00, // Field: USERS
        0XA4, // Option: INEXISTENT
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(admin_get_error_option, st);
}
END_TEST

START_TEST(test_admin_get_invalid_option_for_field)
{
    struct admin_get_parser parser;
    admin_get_parser_init(&parser);
    uint8_t data[] = {
        0x00, // Field: USERS
        0X03, // Option: HISTORIC CONNECTIONS (Not valid for field USERS)
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum admin_get_state st = admin_get_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    ck_assert_uint_eq(admin_get_error_option, st);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_get");

	TCase* testcase_admin_get_n_users = tcase_create("admin_get_n_users");
	tcase_add_test(testcase_admin_get_n_users, test_admin_get_n_users);
	suite_add_tcase(s, testcase_admin_get_n_users);

    TCase* testcase_admin_get_passwords = tcase_create("admin_get_passwords");
	tcase_add_test(testcase_admin_get_passwords, test_admin_get_passwords);
	suite_add_tcase(s, testcase_admin_get_passwords);

    TCase* testcase_admin_get_buffsize = tcase_create("admin_get_buffsize");
	tcase_add_test(testcase_admin_get_buffsize, test_admin_get_buffsize);
	suite_add_tcase(s, testcase_admin_get_buffsize);

    TCase* testcase_admin_get_authstatus = tcase_create("admin_get_authstatus");
	tcase_add_test(testcase_admin_get_authstatus, test_admin_get_authstatus);
	suite_add_tcase(s, testcase_admin_get_authstatus);

    TCase* testcase_admin_get_spoofstatus = tcase_create("admin_get_spooftatus");
	tcase_add_test(testcase_admin_get_spoofstatus, test_admin_get_spoofstatus);
	suite_add_tcase(s, testcase_admin_get_spoofstatus);

    TCase* testcase_admin_get_sentbytes = tcase_create("admin_get_sentbytes");
	tcase_add_test(testcase_admin_get_sentbytes, test_admin_get_sentbytes);
	suite_add_tcase(s, testcase_admin_get_sentbytes);

    TCase* testcase_admin_get_rcvbytes = tcase_create("admin_get_rcvbytes");
	tcase_add_test(testcase_admin_get_rcvbytes, test_admin_get_rcvbytes);
	suite_add_tcase(s, testcase_admin_get_rcvbytes);

    TCase* testcase_admin_get_connections_h = tcase_create("admin_get_connections_h");
	tcase_add_test(testcase_admin_get_connections_h, test_admin_get_connections_h);
	suite_add_tcase(s, testcase_admin_get_connections_h);

    TCase* testcase_admin_get_invalid_field = tcase_create("admin_get_invalid_field");
	tcase_add_test(testcase_admin_get_invalid_field, test_admin_get_invalid_field);
	suite_add_tcase(s, testcase_admin_get_invalid_field);

    TCase* testcase_admin_get_invalid_option = tcase_create("admin_get_invalid_option");
	tcase_add_test(testcase_admin_get_invalid_option, test_admin_get_invalid_option);
	suite_add_tcase(s, testcase_admin_get_invalid_option);
    
    TCase* testcase_admin_get_invalid_option_for_field = tcase_create("admin_get_invalid_option_for_field");
	tcase_add_test(testcase_admin_get_invalid_option_for_field, test_admin_get_invalid_option_for_field);
	suite_add_tcase(s, testcase_admin_get_invalid_option_for_field);

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