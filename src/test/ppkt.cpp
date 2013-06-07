#include <CppUTest/TestHarness.h>
extern "C" {
#include "ppkt.h"
};

TEST_GROUP(ppkt)
{
};

TEST(ppkt, alloc)
{
    struct ppkt_t *p = ppkt_alloc(15);
    CHECK(p);
    CHECK(15 == ppkt_size(p));
    ppkt_free(p);
}

TEST(ppkt, create)
{
    uint8_t buffer[10] = { 0 };

    struct ppkt_t *p = ppkt_create(buffer, sizeof(buffer));
    CHECK(p);
    CHECK(10 == ppkt_size(p));

    ppkt_free(p);

    CHECK(buffer[5] == 0);
}
