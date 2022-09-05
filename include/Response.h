#pragma once

#include <Arduino.h>

struct Response
{
    char payload[200] {};
    int code {};

    Response(int code, const char*);
};
