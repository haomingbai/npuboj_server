#ifndef NPUBOJ_UTILS_H
#define NPUBOJ_UTILS_H

#include <string>

namespace npuboj
{
struct Limitation
{
    int time;   // in seconds
    int memory; // in MB
};

/// @brief Error code: -1 means compile error, 0 means normal exit, other values are the status of the program, when 4 means timeout, 2 means memory limit exceeded, 3 means runtime error.
struct Result
{
    int code; // The exit code of the program, if the value is -1, it means compile error.
    std::string std_out;
    std::string std_err;
};

} // namespace npuboj

#endif