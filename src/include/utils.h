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

/// @brief Error code: -2 means the error of the sandbox, -1 means compile error, 0 means normal exit, other values are the status of the program. 
struct Result
{
    int code; // The exit code of the program, if the value is -1, it means compile error, if the value is -2, it means the error of the sandbox, if the value is 1, it means TLE, if the value is 2, it means MLE, 3 means RE.
    std::string std_out;
    std::string std_err;
};

} // namespace npuboj

#endif