#include "Response.h"

Response::Response(int code, String payload)
{
    this->code = code;
    this->payload = std::move(payload);
}
