#include "./include/args_handler.h"

// Private functions
static void mng_usage(char* command);
static unsigned short port(const char* s);

void parse_args_handler(const int argc, char** argv, struct manage_args* mng_args) {
    memset(mng_args, 0, sizeof(struct manage_args));

    // Set default socks for our protocol
    mng_args->mng_addr = DEFAULT_MNG_ADDR_V4;
    mng_args->mng_port = DEFAULT_MNG_PORT;
    int c;
    char delimiter[] = ":";

    mng_args->get_flag = false;
    mng_args->add_flag = false;
    mng_args->toggle_flag = false;
    mng_args->delete_flag = false;
    mng_args->set_flag = false;

    while ((c = getopt(argc, argv, "hL:P:a:g:e:d:i:t:b:v")) != -1) {
        switch (c) {
            case 'h':
                // Help
                mng_usage(argv[0]);
                break;
            case 'L':
                mng_args->mng_addr = optarg;
                break;
            case 'P':
                mng_args->mng_port = port(optarg);
                break;
            case 'a':
                mng_args->try_password = optarg;
                break;
            case 'g':
                mng_args->get_flag = true;
                mng_args->get_option = optarg;
                break;
            case 'e':
                // edit user
                mng_args->edit_flag = true;
                mng_args->edit_username = strtok(optarg, delimiter);
                mng_args->edit_attribute = atoi(strtok(NULL, delimiter));
                mng_args->edit_value = strtok(NULL, delimiter);
                break;
            case 'd':
                // delete user (TODO como distingo que usuario borro?)
                mng_args->delete_flag = true;
                mng_args->delete_username = optarg;
                break;
            case 'i':
                // -i username:password
                mng_args->add_flag = true;
                mng_args->add_username = strtok(optarg, delimiter);
                mng_args->add_password = strtok(NULL, delimiter);
                // Chequear error desp
                break;
            case 't':
                // toggle
                // -t auth:on/off
                // -t spoof:on/off
                mng_args->toggle_flag = true;
                mng_args->toggle_option = strtok(optarg, delimiter);
                mng_args->toggle_status = strtok(NULL, delimiter);
                break;
            case 'b':
                // set buffsize
                mng_args->set_flag = true;
                mng_args->set_size = atoi(optarg);
                break;
            case 'v':
                // mensaje de la version
                // mng_args->version = atoi(optarg);
                printf("Wannapprove Protocol version 0\n");
                exit(0);
                break;
            default:
                break;
        }
    }


}

static void mng_usage(char* command) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   -h                  Prints help and end.\n"
        "   -L <GRUPO6 addr>    \n"
        "   -P <GRUPO6 port>    \n"
        "   -a <token>          Auth token to access the system.\n"
        "   -g <OPTION>         Shows info of the selected option.\n"
        "                       Where option can be: \n"
        "                            users, \n"
        "                            passwords, \n"
        "                            spoofstatus, \n"
        "                            authstatus, \n"
        "                            sentbytes, \n"
        "                            rrcvbytes, \n"
        "                            historic or, \n"
        "                            concurrent\n"
        "   -e user:attr:value  Edits the <attr> attribute of the user with the passed value \n"
        "   -d <username>       Deletes the user with the specified username\n"
        "   -i user:pass        Adds a user with <user> as username and <pass> as password\n"
        "   -t option:status    To the option (spoof or auth) sets the status (on or off)\n"
        "   -b <SIZE>           Sets the size of the system buffer to be <SIZE>\n"
        "   -v                  Gives information of the current version and ends.\n"
        "   ",
        command);
}

static unsigned short port(const char* s) {
    char* end = 0;
    const long sl = strtol(s, &end, 10);

    if (end == s || '\0' != *end ||
        ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) || sl < 0 ||
        sl > USHRT_MAX) {
        log_print(LOG_ERROR, "port should in in the range of 1-65536: %s\n", s);
        return 1;
    }
    return (unsigned short) sl;
}