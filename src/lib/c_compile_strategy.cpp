#include "c_compile_strategy.h"
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <fmt/ranges.h>
#include <fstream>
#include <istream>
#include <mutex>
#include <ostream>
#include <ranges>
#include <sandboxed_api/sandbox2/executor.h>
#include <sandboxed_api/sandbox2/policy.h>
#include <sandboxed_api/sandbox2/policybuilder.h>
#include <sandboxed_api/sandbox2/sandbox2.h>

using namespace npuboj;
using namespace std;
namespace bp = ::boost::process;

CCompilerStrategy::CCompilerStrategy(string &path) : SimpleCompileStrategy(path)
{
    // Verify the path of the working directory.
    if (!(boost::filesystem::exists(path) && boost::filesystem::is_directory(path)))
    {
        throw invalid_argument("The path is invalid.");
    }

    // Verify the compiler.
    bp::child c(this->cc + " --version", bp::std_out > bp::null, bp::std_err > bp::null);
    c.wait();

    if (c.exit_code() != 0)
    {
        throw invalid_argument("The compiler is invalid.");
    }
}

CCompilerStrategy::CCompilerStrategy(string &path, string &cc) : SimpleCompileStrategy(path), cc(cc)
{
    // Verify the path of the working directory.
    if (!(boost::filesystem::exists(path) && boost::filesystem::is_directory(path)))
    {
        throw invalid_argument("The path is invalid.");
    }

    // Verify the compiler.
    bp::child c(this->cc + " --version", bp::std_out > bp::null, bp::std_err > bp::null);
    c.wait();

    if (c.exit_code() != 0)
    {
        throw invalid_argument("The compiler is invalid.");
    }
}

std::vector<Result> &&CCompilerStrategy::compile(const std::string &src, const std::vector<std::string> &input,
                                                 const Limitation &limitation)
{
    // Lock the mutex
    lock_guard<mutex> lock(this->mtx);

    // Write the source code to the file
    boost::filesystem::path src_path = boost::filesystem::temp_directory_path() / "src.c";
    ofstream src_file(src_path.string());
    src_file << src;
    src_file.close();

    // Compile the source code
    bp::ipstream compile_out;
    bp::child compileCommand(this->cc + " -o " + this->path + "/a.out " + src_path.string(), bp::std_out > bp::null,
                             bp::std_err > compile_out);

    compileCommand.wait();
    vector<Result> results;
    if (compileCommand.exit_code() != 0)
    {
        string err, tmp;
        getline(compile_out, tmp);
        err += tmp;
        while (getline(compile_out, tmp))
        {
            err.push_back('\n');
            err += tmp;
        }
        results.emplace_back(-1, "", err);
        return move(results);
    }

    // Run the program
    for (auto &&in : input)
    {
        auto executor = absl::make_unique<sandbox2::Executor>(this->path + "/a.out");
        executor->limits()->set_rlimit_as(((unsigned long long)limitation.memory) << 20);
        executor->limits()->set_rlimit_cpu(limitation.time);
        executor->limits()->set_walltime_limit(absl::Seconds(limitation.time));
        auto policy = sandbox2::PolicyBuilder().BuildOrDie();

        sandbox2::Sandbox2 sandbox(move(executor), move(policy));

        if (sandbox.RunAsync())
        {
            sandbox.comms()->SendString(in);

            auto result = sandbox.AwaitResult();
            string out;
            sandbox.comms()->RecvString(&out);
            results.emplace_back(result.final_status() - 1, out, result.ToString());
        }
        else
        {
            results.emplace_back(3, "", "The program is killed.");
        }
    }

    return move(results);
};

vector<int> &&CCompilerStrategy::compile(const string &src, const vector<string> &input, const vector<string> &output,
                                         const Limitation &limitation)
{
    auto res = this->compile(src, input, limitation);
    int status = 0;

    vector<int> results;
    results.reserve(res.size());

    for (auto &&i : res)
    {
        if (i.code == -1)
        {
            results.push_back(-1);
            break;
        }
    }

    for (auto &&i : res)
    {
        if (i.code != 0)
        {
            results.push_back(i.code);
            continue;
        }
        else
        {
            auto v = i.std_out | std::views::split('\n') |
                     std::views::transform([](auto word) { return std::string(word.begin(), word.end()); });
            
            vector<string> output_vec(v.begin(), v.end());

            for (auto &str : output_vec)
            {
                while (str.back() == '\r' || str.back() == '\n' || str.back() == ' ' || str.back() == '\t')
                {
                    str.pop_back();
                }
            }
            
        }
    }
}