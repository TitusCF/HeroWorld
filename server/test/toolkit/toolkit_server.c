#include "../include/toolkit_server.h"

void clean_test_account_data() {
    unlink("/tmp/account/testaccount");
    rmdir("/tmp/account");
    unlink("/tmp/accounts");
}
