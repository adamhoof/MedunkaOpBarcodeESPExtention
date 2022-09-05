#pragma once

struct ProductData
{
    const char* name {};
    float price {};
    const char* stock {};
    const char* unitOfMeasure {};
    float unitOfMeasureKoef {};

    ProductData(const char* name, float price, const char* stock, const char* unitOfMeasure,
                float unitOfMeasureKoef);
};
