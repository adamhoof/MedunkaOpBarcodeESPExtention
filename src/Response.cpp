#include "Response.h"

Response::Response(int code, const char* payload)
{
    this->code = code;
    strcpy(this->payload, payload);
}
