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

TEST(ppkt, prefix)
{
    struct ppkt_t *p = ppkt_alloc(10);
    struct ppkt_t *hdr = ppkt_alloc(5);

    struct ppkt_t *new_chain = ppkt_prefix_header(hdr, p);

    CHECK(new_chain);
    CHECK(15 == ppkt_chain_size(new_chain));
    CHECK(new_chain == hdr);
    CHECK(5 == ppkt_size(new_chain));
    CHECK(2 == ppkt_chain_count(new_chain));
    CHECK(1 == ppkt_chain_count(p));
    CHECK(p == ppkt_next(new_chain));

    ppkt_free(new_chain);
}

TEST(ppkt, append)
{
    struct ppkt_t *p = ppkt_alloc(10);
    struct ppkt_t *footer = ppkt_alloc(5);

    struct ppkt_t *new_chain = ppkt_append_footer(footer, p);

    CHECK(new_chain);
    CHECK(15 == ppkt_chain_size(new_chain));
    CHECK(new_chain == p);
    CHECK(10 == ppkt_size(new_chain));
    CHECK(2 == ppkt_chain_count(new_chain));
    CHECK(2 == ppkt_chain_count(p));
    CHECK(1 == ppkt_chain_count(footer));
    CHECK(footer == ppkt_next(new_chain));

    ppkt_free(new_chain);
}

TEST(ppkt, coalesce)
{
    struct ppkt_t *one = ppkt_alloc(5);
    struct ppkt_t *two = ppkt_alloc(5);

    for (int i = 0; i < 5; i++)
    {
        ppkt_payload(one)[i] = i;
        ppkt_payload(two)[i] = 5 + i;
    }

    struct ppkt_t *chain = ppkt_prefix_header(one, two);
    struct ppkt_t *out = ppkt_coalesce(chain, 10);

    CHECK(out);
    CHECK(10 == ppkt_size(out));
    CHECK(1 == ppkt_chain_count(out));

    for (int i = 0; i < 10; i++)
        CHECK(i == ppkt_payload(out)[i]);

    ppkt_free(out);
}

TEST(ppkt, coalesce_two)
{
    struct ppkt_t *one = ppkt_alloc(5);
    struct ppkt_t *two = ppkt_alloc(5);

    for (int i = 0; i < 5; i++)
    {
        ppkt_payload(one)[i] = i;
        ppkt_payload(two)[i] = 5 + i;
    }

    struct ppkt_t *chain = ppkt_prefix_header(one, two);
    struct ppkt_t *out = ppkt_coalesce(chain, 7);

    CHECK(out);
    CHECK(7 == ppkt_size(out));
    CHECK(2 == ppkt_chain_count(out));

    for (int i = 0; i < 7; i++)
        CHECK(i == ppkt_payload(out)[i]);

    ppkt_free(out);
}

TEST(ppkt, split_single)
{
    struct ppkt_t *p = ppkt_alloc(5);

    for (int i = 0; i < 5; i++)
    {
        ppkt_payload(p)[i] = i;
    }

    struct ppkt_t *tail;

    ppkt_split(p, &tail, 3);
    CHECK(tail);
    CHECK(1 == ppkt_chain_count(p));
    CHECK(1 == ppkt_chain_count(tail));
    CHECK(3 == ppkt_size(p));
    CHECK(2 == ppkt_size(tail));

    for (int i = 0; i < 3; i++)
    {
        CHECK(i == ppkt_payload(p)[i]);
    }
    for (int i = 0; i < 2; i++)
    {
        CHECK(i + 3 == ppkt_payload(tail)[i]);
    }
}

TEST(ppkt, split_in_second)
{
    struct ppkt_t *one = ppkt_alloc(5);
    struct ppkt_t *two = ppkt_alloc(5);

    for (int i = 0; i < 5; i++)
    {
        ppkt_payload(one)[i] = i;
        ppkt_payload(two)[i] = 5 + i;
    }
    struct ppkt_t *chain = ppkt_prefix_header(one, two);

    struct ppkt_t *tail;
    ppkt_split(chain, &tail, 7);

    CHECK(tail);
    CHECK(2 == ppkt_chain_count(chain));
    CHECK(1 == ppkt_chain_count(tail));
    CHECK(7 == ppkt_chain_size(chain));
    CHECK(3 == ppkt_chain_size(tail));

    CHECK(6 == ppkt_payload(ppkt_next(chain))[1]);
    CHECK(7 == ppkt_payload(tail)[0]);
}

TEST(ppkt, split_between_packets)
{
    struct ppkt_t *one = ppkt_alloc(5);
    struct ppkt_t *two = ppkt_alloc(5);

    for (int i = 0; i < 5; i++)
    {
        ppkt_payload(one)[i] = i;
        ppkt_payload(two)[i] = 5 + i;
    }
    struct ppkt_t *chain = ppkt_prefix_header(one, two);

    struct ppkt_t *tail;
    ppkt_split(chain, &tail, 5);

    CHECK(tail);
    CHECK(5 == ppkt_size(chain));
    CHECK(5 == ppkt_size(tail));
    CHECK(1 == ppkt_chain_count(chain));
    CHECK(1 == ppkt_chain_count(tail));

    for (int i = 0; i < 5; i++)
    {
        CHECK(i == ppkt_payload(chain)[i]);
        CHECK(i + 5 == ppkt_payload(tail)[i]);
    }
}
