#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/request.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_request_connect_ipv4) {
    struct request_parser parser;
    request_parser_init(&parser);
    uint8_t data[] = {
        0x05, // version
        0x01, // connect
        0x00, // reserved
        0x01, // atyp: ipv4
        0x7F, 0x00, 0x00, 0x01, // dst addr: 127.0.0.1
        0x23, 0x82, // dst port: 9090
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum request_state st = request_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(request_done, st);
}
END_TEST

Suite*
suite(void) {
	Suite* s = suite_create("request");

	TCase* testcase_request_connect_ipv4 = tcase_create("request_connect_ipv4");
	tcase_add_test(testcase_request_connect_ipv4, test_request_connect_ipv4);
	suite_add_tcase(s, testcase_request_connect_ipv4);

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