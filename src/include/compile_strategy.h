#ifndef COMPILE_STRATEGY_H
#define COMPILE_STRATEGY_H

#include <string>
#include <vector>
#include "utils.h"

namespace npuboj
{
/// @brief The abstract class of the compile strategy
class CompileStrategy
{
  public:
    virtual ~CompileStrategy() = default;
    /// @brief The function used to compile the source code and run the code with input to stdin
    /// @param src The source code of the program.
    /// @param input The input to the program, each element in the vector is a string that will be written to the stdin
    /// of the program in one execution.
    /// @return The stdout of the program, with each input in the vector input, there will be a corresponding output in
    /// the return value
    virtual std::vector<Result> &&compile(const std::string &src, const std::vector<std::string> &input, const Limitation &limitation) = 0;
    /// @brief Compile, run and judge the output of the program
    /// @param src The source code of the program.
    /// @param input The standard input of the program, in one time of execution, the program will read one string from
    /// the input vector.
    /// @param output The standard output expected from the program, in one time of execution, the program will be
    /// judged by the corresponding string in the output vector.
    /// @return The result of the judgement, each element in the vector is a boolean value, true means the output is
    /// correct, false means the output is wrong.
    virtual std::vector<int> &&compile(const std::string &src, const std::vector<std::string> &input,
                                        const std::vector<std::string> &output, const Limitation &limitation) = 0;
};
} // namespace npuboj

#endif