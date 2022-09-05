#pragma once

#include <Arduino.h>

struct Response
{
    String payload {};
    int code {};

    Response(int code, String payload);
};
