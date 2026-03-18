#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Model.hpp"

class Importer {
public:
    Importer() {};
    void LoadJSON(const char *filename, Model &model);
};
