#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "../../include/parsers/request.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

START_TEST(test_request_connect_ipv4) {
    struct request req; 
    struct request_parser parser = {
        .request = &req
    };
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
    ck_assert_uint_eq(socks_req_cmd_connect, req.cmd);
    ck_assert_uint_eq(socks_req_addrtype_ipv4, req.dst_addr_type);
    ck_assert_str_eq("127.0.0.1", inet_ntoa(req.dst_addr.ipv4.sin_addr));
    ck_assert_uint_eq(htons(9090), req.dst_port);
    ck_assert_uint_eq(request_done, st);
}
END_TEST

START_TEST(test_request_connect_invalid_atyp) {
    struct request req; 
    struct request_parser parser = {
        .request = &req
    };
    request_parser_init(&parser);
    uint8_t data[] = {
        0x05, // version
        0x01, // connect
        0x00, // reserved
        0x0F, // atyp: ipv4
        0x7F, 0x00, 0x00, 0x01, // dst addr: 127.0.0.1
        0x23, 0x82, // dst port: 9090
    };
    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum request_state st = request_consume(&b, &parser, &errored);
    ck_assert_uint_eq(true, errored);
    // ck_assert_uint_eq(socks_req_cmd_connect, req.cmd);
    // ck_assert_uint_eq(socks_req_addrtype_ipv4, req.dst_addr_type);
    // ck_assert_uint_eq(htons(9090), req.dst_port);
    ck_assert_uint_eq(request_error_unsupported_addresstype, st);
}
END_TEST

START_TEST(test_request_connect_ipv6) {
    struct request req; 
    struct request_parser parser = {
        .request = &req
    };
    request_parser_init(&parser);
    uint8_t data[] = {
        0x05, // version
        0x01, // connect
        0x00, // reserved
        0x04, // atyp: ipv6
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // dst addr: ::1
        0x23, 0x82, // dst port: 9090
    };

    buffer b;
    FIXBUF(b, data);
    bool errored = false;
    enum request_state st = request_consume(&b, &parser, &errored);
    ck_assert_uint_eq(false, errored);
    ck_assert_uint_eq(socks_req_cmd_connect, req.cmd);
    ck_assert_uint_eq(socks_req_addrtype_ipv6, req.dst_addr_type);
    char dst[INET6_ADDRSTRLEN];
    ck_assert_str_eq("::1", inet_ntop(AF_INET6,&req.dst_addr.ipv6.sin6_addr,dst,INET6_ADDRSTRLEN));
    ck_assert_uint_eq(htons(9090), req.dst_port);
    ck_assert_uint_eq(request_done, st);
}
END_TEST

// START_TEST(test_request_unsopported_version) {
//     struct request_parser parser;
//     request_parser_init(&parser);
// }

Suite*
suite(void) {
    Suite* s = suite_create("request");

    TCase* testcase_request_connect_ipv4 = tcase_create("request_connect_ipv4");
    tcase_add_test(testcase_request_connect_ipv4, test_request_connect_ipv4);
    suite_add_tcase(s, testcase_request_connect_ipv4);

    TCase* testcase_request_connect_invalid_atyp = tcase_create("request_connect_invalid_atyp");
    tcase_add_test(testcase_request_connect_invalid_atyp, test_request_connect_invalid_atyp);
    suite_add_tcase(s, testcase_request_connect_invalid_atyp);

    TCase* testcase_request_connect_ipv6 = tcase_create("request_connect_ipv6");
    tcase_add_test(testcase_request_connect_ipv6, test_request_connect_ipv6);
    suite_add_tcase(s, testcase_request_connect_ipv6);
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