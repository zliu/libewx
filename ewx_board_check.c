#include "cvmx.h"
#include "cvmx-twsi.h"
static CVMX_SHARED uint8_t board_valid= 0;

#define KEY_LEN 24
static const unsigned char key[KEY_LEN]="EmbedWay Technology Inc.";

int ewx_board_check() {
    uint16_t i, len;
    uint64_t data;
    uint8_t buffer[512]={0};
    uint8_t *p;

    for (i = 0; i < 256; i++) {
        if (cvmx_twsix_read_ia(1, 0x50, (uint16_t)i, 1, 2, &data) == -1) {
            printf("ERR: Board hardware config error!\n");
            return 0;
        }
        buffer[i] = data & 0xFF;
    }
    p = buffer + buffer[3] * 8; p += 6;
    len = *p++; len &= 0x3f;
    if (len != KEY_LEN) {
        board_valid = 0;
        return 0;
    }
    for (i = 0; i < len; i++) {
        if (p[i] != key[i]) {
            board_valid = 0;
            return 0;
        }
    }

    p = buffer + buffer[4] * 8; p += 3;
    len = *p++; len &= 0x3f;
    if (len != KEY_LEN) {
        board_valid = 0;
        return 0;
    }
    for (i = 0; i < len; i++) {
        if (p[i] != key[i]) {
            board_valid = 0;
            return 0;
        }
    }
    board_valid = 1;
    return 0;
}

int ewx_board_valid() {
    return board_valid;
}
