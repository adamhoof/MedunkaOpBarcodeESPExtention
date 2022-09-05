#pragma once

struct ProductData
{
    const char* name {};
    float price {};
    const char* stock {};
    const char* unitOfMeasure {};
    float unitOfMeasureKoef {};

    ProductData(const char* name, float price, const char* stock, const char* unitOfMeasure,
                const float unitOfMeasureKoef)
    {
        this->name = name;
        this->price = price;
        this->stock = stock;
        this->unitOfMeasure = unitOfMeasure;
        this->unitOfMeasureKoef = unitOfMeasureKoef;
    }
};
