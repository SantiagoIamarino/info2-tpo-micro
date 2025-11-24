#include "caidas.h"

enum StateCaida : uint8_t {
    C_WAIT_LT = 0,       // esperando '<'
    C_MATCHING,          // comparando "CAIDA_OK"
    C_WAIT_GT,           // esperando '>'
    C_DONE,
    C_ERROR
};

static StateCaida c_state = C_WAIT_LT;
static uint8_t c_index = 0;

static const char* C_STR = "CAIDA_OK";


void resp_caida_maq_estados(int32_t b, bool* caida_ok_detected)
{
    if (b < 0) return;
    log_debug((uint8_t*)"+", 0);
    log_debug((uint8_t*)b, 0);

    *caida_ok_detected = false;

    switch (c_state)
    {
        case C_WAIT_LT:
            if (b == '<') {
                c_state = C_MATCHING;
                c_index = 0;
            }
            break;

        case C_MATCHING:
            if (b == C_STR[c_index]) {
                c_index++;
                if (C_STR[c_index] == '\0') {
                    c_state = C_WAIT_GT;
                }
            } else {
                c_state = (b == '<') ? C_MATCHING : C_WAIT_LT;
                c_index = 0;
            }
            break;

        case C_WAIT_GT:
            if (b == '>') {
                *caida_ok_detected = true;
                c_state = C_DONE;
            } else {
                c_state = C_ERROR;
            }
            break;

        case C_DONE:
            c_state = C_WAIT_LT;
            c_index = 0;
            break;

        case C_ERROR:
        default:
            c_state = C_WAIT_LT;
            c_index = 0;
            break;
    }
}
