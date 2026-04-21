#pragma once
#include <exception>

namespace sjtu {
class exception : public std::exception {};
class index_out_of_bound : public exception {};
class runtime_error : public exception {};
class invalid_iterator : public exception {};
class container_is_empty : public exception {};
}

