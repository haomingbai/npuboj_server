#ifndef SIMPLE_COMPILE_STRATEGY_H
#define SIMPLE_COMPILE_STRATEGY_H

#include "compile_strategy.h"
#include <mutex>

namespace npuboj
{
class SimpleCompileStrategy : public CompileStrategy
{
  protected:
    const std::string path;
    std::mutex mtx;

  public:
    SimpleCompileStrategy(const std::string &path) : path(path) {};

    virtual ~SimpleCompileStrategy() = default;

    /// @brief Verify whether the source code is valid
    /// @return The validatity of the source code
    virtual bool validate() = 0;

    virtual std::vector<Result> &&compile(const std::string &src, std::vector<std::string> &input, const Limitation &limitation) = 0;
    virtual std::vector<int> &&compile(const std::string &src, const std::vector<std::string> &input,
                                        const std::vector<std::string> &output, const Limitation &limitation) = 0;
};

} // namespace npuboj

#endif