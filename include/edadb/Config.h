/**
 * @file Config.h
 * @brief Config parameters
 */

#pragma once

#include "DbBackendType.h"

namespace edadb {

/**
 * @brief Config class provides the configuration parameters for the database.
 */
class Config {
public:
    static constexpr DbBackendType backend_type = DbBackendType::SQLITE; 
};

} // namespace edadb
