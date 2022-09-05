#pragma once

struct Response
{
    String payload {};
    int code {};

    Response(int code, String payload)
    {
        this->code = code;
        this->payload = std::move(payload);
    }
};
