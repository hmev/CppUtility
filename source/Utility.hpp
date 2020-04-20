#pragma once

#include <memory>
#include "NonCopyable.hpp"
#include "Nullable.hpp"

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Observer = T * ;

template <typename T>
using Shared = std::shared_ptr<T>;