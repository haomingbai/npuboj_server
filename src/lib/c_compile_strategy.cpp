#include "c_compile_strategy.h"
#include <absl/log/check.h>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <fstream>
#include <istream>
#include <mutex>
#include <ostream>
#include <ranges>
#include <sandboxed_api/sandbox2/executor.h>
#include <sandboxed_api/sandbox2/policy.h>
#include <sandboxed_api/sandbox2/policybuilder.h>
#include <sandboxed_api/sandbox2/sandbox2.h>
#include <unistd.h>
#include <exception>

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

std::vector<Result> CCompilerStrategy::compile(const std::string &src, const std::vector<std::string> &input,
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
        return (results);
    }

    // Run the program
    try{
        for (size_t i = 0; i < input.size(); i++)
        {
            // Create the executor
            vector<string> args;
            args.push_back(this->path + "/a.out");
            auto executor = absl::make_unique<sandbox2::Executor>(this->path + "/a.out", args);
            executor->limits()
                ->set_rlimit_as(((unsigned long long)limitation.memory) << 20)
                .set_rlimit_cpu(limitation.time)
                .set_walltime_limit(absl::Seconds(limitation.time));

            // The input of the program
            int pipe_stdin_fd[2];
            if (pipe(pipe_stdin_fd) == -1)
            {
                std::runtime_error err("Unable to create a new pipe!");
            }
            write(pipe_stdin_fd[1], input.at(i).c_str(), input.at(i).length());
            close(pipe_stdin_fd[1]);
            executor->ipc()->MapFd(pipe_stdin_fd[0], STDIN_FILENO);

            // The output of the program
            int pipe_stdout_fd[2];
            if (pipe(pipe_stdout_fd) == -1)
            {
                std::runtime_error err("Unable to create a new pipe!");
            }
            executor->ipc()->MapFd(pipe_stdout_fd[1], STDOUT_FILENO);

            // The error of the program
            int pipe_stderr_fd[2];
            if (pipe(pipe_stderr_fd) == -1)
            {
                std::runtime_error err("Unable to create a new pipe!");
            }
            executor->ipc()->MapFd(pipe_stderr_fd[1], STDERR_FILENO);
            
            // Default policy
            auto policy = sandbox2::PolicyBuilder().BuildOrDie();

            // Create the sandbox
            sandbox2::Sandbox2 sandbox(move(executor), move(policy));

            // Run the sandbox
            auto run_result = sandbox.Run();

            // Get the result
            string std_out, std_err;
            char buffer[4096];
            ssize_t n;
            while ((n = read(pipe_stdout_fd[0], buffer, sizeof(buffer))) > 0)
            {
                std_out.append(buffer, n);
            }
            while ((n = read(pipe_stderr_fd[0], buffer, sizeof(buffer))) > 0)
            {
                std_err.append(buffer, n);
            }

            // Close the file descriptor
            close(pipe_stdout_fd[0]);
            close(pipe_stderr_fd[0]);

            auto err_code = run_result.final_status();
            if (err_code != sandbox2::Result::OK)
            {
                results.emplace_back(0, std_out, std_err);
            }
            else
            {
                if (err_code == sandbox2::Result::TIMEOUT)
                {
                    results.emplace_back(1, std_out, std_err);
                }
                else
                {
                    int reason_code = run_result.reason_code();
                    if (reason_code == sandbox2::Result::FAILED_LIMITS)
                    {
                        results.emplace_back(2, std_out, std_err);
                    }
                    else
                    {
                        results.emplace_back(3, std_out, std_err);
                    }
                }
            }
        }
    } catch(std::exception &e)
    {
        string err = e.what() + '\n';
        clog << err;
        results.clear();
        results.emplace_back(-2, "", err);
    }

    return results;
};

vector<int> CCompilerStrategy::compile(const string &src, const vector<string> &input, const vector<string> &output,
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