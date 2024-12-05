#ifndef C_COMPILE_STRATEGY_H
#define C_COMPILE_STRATEGY_H

#include "simple_compile_strategy.h"

namespace npuboj
{
class CCompilerStrategy : public SimpleCompileStrategy
{
  private:
    std::string cc = "/bin/gcc";

  public:
    /// @brief The constructor of the class, with the path of the working directory.
    /// @param path The path of the working directory.
    CCompilerStrategy(std::string &path);

    /// @brief The constructor of the class, with the path of the working directory and the compiler.
    /// @param path The path of the working directory.
    /// @param cc The compiler to be used in this instance.
    CCompilerStrategy(std::string &path, std::string &cc);
    virtual ~CCompilerStrategy() = default;

    virtual std::vector<Result> &&compile(const std::string &src, const std::vector<std::string> &input, const Limitation &limitation) override;
    virtual std::vector<int> &&compile(const std::string &src, const std::vector<std::string> &input,
                                        const std::vector<std::string> &output, const Limitation &limitation) override;
};
} // namespace npuboj

#endif