#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/admin_configbuffsize.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_admin_configbuff_normal)
{
    struct admin_configbuff_parser parser;
    admin_configbuff_parser_init(&parser);
    uint8_t data[] = {
        0x03, // Action: BUFFSIZE
        0x00, 0x00, 0x0A, 0x0F // Size: 
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configbuff_state st = admin_configbuff_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_configbuff_done, st);
    // TODO size
    uint32_t size = parser.size[3] | (parser.size[2] << 8) | (parser.size[1] << 16) | (parser.size[0] << 24);
    ck_assert_uint_eq(size, 2575);
    ck_assert_uint_eq(false, errored);
}
END_TEST

START_TEST(test_admin_configbuff_invalid_action)
{
    struct admin_configbuff_parser parser;
    admin_configbuff_parser_init(&parser);
    uint8_t data[] = {
        0x01, // Action: INVALID
        0x00, 0x00, 0x0A, 0x0F // Size: 
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configbuff_state st = admin_configbuff_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_configbuff_error_action, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

START_TEST(test_admin_configbuff_invalid_size)
{
    struct admin_configbuff_parser parser;
    admin_configbuff_parser_init(&parser);
    uint8_t data[] = {
        0x03, // Action: BUFFSIZE
        0x00, 0x00, 0x00, 0x00 // Size: 
    };
    buffer b;   
    bool errored = false;
    FIXBUF(b, data);
    enum admin_configbuff_state st = admin_configbuff_consume(&b, &parser, &errored);
    ck_assert_uint_eq(admin_configbuff_error_size, st);
    ck_assert_uint_eq(true, errored);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("admin_configbuff");

	TCase* testcase_admin_configbuff_normal = tcase_create("admin_configbuff_normal");
	tcase_add_test(testcase_admin_configbuff_normal, test_admin_configbuff_normal);
	suite_add_tcase(s, testcase_admin_configbuff_normal);

    TCase* testcase_admin_configbuff_invalid_size = tcase_create("admin_configbuff_invalid_size");
	tcase_add_test(testcase_admin_configbuff_invalid_size, test_admin_configbuff_invalid_size);
	suite_add_tcase(s, testcase_admin_configbuff_invalid_size);

    TCase* testcase_admin_configbuff_invalid_action = tcase_create("admin_configbuff_invalid_action");
	tcase_add_test(testcase_admin_configbuff_invalid_action, test_admin_configbuff_invalid_action);
	suite_add_tcase(s, testcase_admin_configbuff_invalid_action);

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