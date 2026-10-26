#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define MAX_FIELDS 8

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

static uint32_t rotate_left(uint32_t value, unsigned count) {
    return (value << count) | (value >> (32U - count));
}

static uint32_t rotate_right(uint32_t value, unsigned count) {
    return (value >> count) | (value << (32U - count));
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t step1 = (high + 8U * middle) ^ UINT32_C(0x2770);
    uint32_t step0 = (3U * low) ^ (high + UINT32_C(0xd44e));
    values[0] = step0 ^ (step1 + UINT32_C(0x9808));
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = 15U + 2U * ((uint32_t)(challenge >> 8) & 0xfU);
    for (size_t i = 1; i < count; ++i) {
        unsigned shift = 19U + 2U * (unsigned)(i - 1);
        uint32_t digit = (uint32_t)(challenge >> shift) & 0xfU;
        values[i] = 3U * (values[i - 1] + (uint32_t)i) - digit;
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t operation = (uint32_t)challenge & 7U;
    uint32_t value = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = operation;
    switch (operation) {
        case 0: values[1] = 5U * value + UINT32_C(0x36d12); break;
        case 1: values[1] = (4U * value) ^ UINT32_C(0x7da1); break;
        case 2: values[1] = 5U * value + UINT32_C(0x8035); break;
        case 3: values[1] = (value ^ UINT32_C(0xb57a)) + (value >> 2); break;
        case 4: values[1] = 5U * value + UINT32_C(0xb59b); break;
        case 5: values[1] = (value ^ UINT32_C(0xf378)) + (value >> 1); break;
        case 6: values[1] = (2U * value) ^ UINT32_C(0x5d15); break;
        case 7: values[1] = 5U * value + UINT32_C(0x4236); break;
    }
    return write_answer(out, cap, values, count);
}

static uint32_t phase_4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t salt) {
    uint32_t middle = low + ((high - low) >> 1);
    if (target == middle) return (middle + UINT32_C(0x87)) ^ salt;

    uint32_t folded = middle + salt;
    if (target < middle) {
        return (5U * phase_4_walk(target, low, middle - 1U, salt) ^ folded) ^ 0xcU;
    }
    return (5U * phase_4_walk(target, middle + 1U, high, salt) ^ folded) ^ 0xaU;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t target = (uint32_t)challenge & 0x7fU;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase_4_walk(target, 0, 0x7fU, salt);
    return write_answer(out, cap, values, count);
}

struct phase_5_node {
    int next;
    uint32_t tag;
    uint32_t key;
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    static const struct phase_5_node nodes[] = {
        {-1, 0x0a21, 0xcfbf}, {3, 0x045f, 0xca89},
        {7, 0x0981, 0xd45d},  {10, 0x0569, 0x3adb},
        {1, 0x0204, 0xa7ba},  {6, 0x060e, 0x5da8},
        {8, 0x034d, 0xb8d9},  {9, 0x013d, 0xc7c1},
        {2, 0x086d, 0xddd7},  {0, 0x0039, 0xb76e},
        {5, 0x0b59, 0xd33b},  {4, 0x0758, 0x5272},
    };
    static const unsigned roots[] = {11, 4, 1, 3, 10, 5, 6, 8};
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    unsigned current = roots[challenge & 7U];
    unsigned steps = (unsigned)(challenge >> 3) & 3U;
    uint32_t folded = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);

    folded = ((folded ^ UINT32_C(0x55a1)) * 3U + nodes[current].key) ^ nodes[current].tag;
    for (unsigned i = 0; i < steps; ++i) {
        current = (unsigned)nodes[current].next;
        folded = (folded * 3U + nodes[current].key) ^ nodes[current].tag;
    }
    values[0] = folded;
    values[1] = nodes[current].tag;
    return write_answer(out, cap, values, count);
}

static uint32_t phase_6_operation(unsigned operation, uint32_t value) {
    switch (operation) {
        case 0:
        case 1: return rotate_right(value, 2);
        case 2:
        case 3: return (8U * value + UINT32_C(0xde4c)) ^ (7U * value);
        case 4: return (value ^ (value >> 2)) + UINT32_C(0xfc28);
        case 5: return (value ^ UINT32_C(0x996d)) + 8U * value;
        case 6: return rotate_left(value, 5) ^ UINT32_C(0x6e4e);
        default: return (value + UINT32_C(0x61b5)) ^ (value >> 5);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t value = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    value = phase_6_operation((unsigned)challenge & 7U, value) ^ UINT32_C(0x4a22);
    values[0] = phase_6_operation(((unsigned)challenge >> 3) & 7U, value) + UINT32_C(0x6c43);
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t challenge_fold = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_fold = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t selector = (uint32_t)context & 3U;
    uint32_t salt = (selector * UINT32_C(0xd0d1)) ^ (uint32_t)(context >> 16);
    uint32_t mixed = rotate_left(challenge_fold ^ context_fold, 1);
    uint32_t second;

    mixed = (mixed + UINT32_C(0xd0d1)) ^ UINT32_C(0x2b62);
    second = mixed ^ salt;
    if (cap <= 36) return 1;
    memcpy(out, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN", 36);
    int size = snprintf(out + 36, cap - 36, " %" PRIu32 " %" PRIu32 " %" PRIu32,
                        selector, second,
                        second ^ (context_fold + selector + UINT32_C(0x2b62) + (challenge_fold >> 1)));
    return size < 0 || (size_t)size >= cap - 36;
}