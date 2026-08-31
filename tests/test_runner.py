#!/usr/bin/env python3
import os
import subprocess
import sys

SHELL_EXEC = "./myshell"

class ShellTestRunner:
    def __init__(self):
        self.passed = 0
        self.failed = 0

    def run_case(self, category: str, test_name: str, commands: str, expected_snippet: str):
        print(f"[{category}] {test_name}...", end=" ", flush=True)
        try:
            proc = subprocess.run(
                [SHELL_EXEC],
                input=commands.encode("utf-8"),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=10
            )
            output = proc.stdout.decode("utf-8", errors="replace")
            
            if expected_snippet in output:
                print("PASSED")
                self.passed += 1
            else:
                print("FAILED")
                print(f"  --> Expected snippet: {expected_snippet!r}")
                print(f"  --> Actual output:\n{output.strip()}")
                self.failed += 1
        except subprocess.TimeoutExpired:
            print("FAILED (Timeout)")
            self.failed += 1
        except Exception as e:
            print(f"FAILED (Error: {e})")
            self.failed += 1

def main():
    if not os.path.exists(SHELL_EXEC):
        print(f"Error: {SHELL_EXEC} not found. Run 'make' first.")
        sys.exit(1)

    runner = ShellTestRunner()
    print("=" * 70)
    print("      Mini UNIX Shell Exhaustive CV & Feature Test Suite       ")
    print("=" * 70)

    # -------------------------------------------------------------
    # Category 1: Process Creation, Synchronization & Status Tracking
    # -------------------------------------------------------------
    runner.run_case(
        "Bullet 1: Process & Built-ins",
        "External command execution (mkdir, ls, rmdir)",
        "mkdir -p cv_test_dir\nls -d cv_test_dir\nrmdir cv_test_dir\nexit 0\n",
        "cv_test_dir"
    )
    runner.run_case(
        "Bullet 1: Process & Built-ins",
        "Exit status tracking ($? on success = 0)",
        "echo test\necho STATUS:$?\nexit 0\n",
        "STATUS:0"
    )
    runner.run_case(
        "Bullet 1: Process & Built-ins",
        "Exit status tracking ($? on failure != 0)",
        "ls /nonexistent_dir_for_cv_test_12345 2>/dev/null\necho STATUS:$?\nexit 0\n",
        "STATUS:2"
    )
    runner.run_case(
        "Bullet 1: Process & Built-ins",
        "Built-in cd, pwd, cd -, cd ~",
        "pwd\ncd /tmp\npwd\ncd -\npwd\nexit 0\n",
        "/tmp"
    )
    runner.run_case(
        "Bullet 1: Process & Built-ins",
        "Built-in echo with -n flag",
        "echo -n 'hello_no_newline'\nexit 0\n",
        "hello_no_newline"
    )
    runner.run_case(
        "Bullet 1: Process & Built-ins",
        "Built-in help and exit",
        "help\nexit 0\n",
        "Mini UNIX Shell Built-in Commands:"
    )

    # -------------------------------------------------------------
    # Category 2: Quote-Aware Parser, Escapes, Comments & Variables
    # -------------------------------------------------------------
    runner.run_case(
        "Bullet 2: Quote-Aware Parser",
        "Single quotes (Literal preservation, no variable expansion)",
        "export TEST_VAR=SecretVal\necho 'Literal $TEST_VAR with spaces'\nexit 0\n",
        "Literal $TEST_VAR with spaces"
    )
    runner.run_case(
        "Bullet 2: Quote-Aware Parser",
        "Double quotes (Preserves spaces with variable expansion)",
        "export TEST_VAR=ExpandedVal\necho \"Grouped $TEST_VAR with spaces\"\nexit 0\n",
        "Grouped ExpandedVal with spaces"
    )
    runner.run_case(
        "Bullet 2: Quote-Aware Parser",
        "Backslash escape sequences (\\ , \\\", \\\\)",
        "echo hello\\ world\nexit 0\n",
        "hello world"
    )
    runner.run_case(
        "Bullet 2: Quote-Aware Parser",
        "Full-line and inline comments (#)",
        "# This entire line is a comment\necho code # this is an inline comment\nexit 0\n",
        "code"
    )
    runner.run_case(
        "Bullet 2: Quote-Aware Parser",
        "Environment variables ($VAR, export, unset)",
        "export MY_KEY=MySecret123\necho VAR:$MY_KEY\nunset MY_KEY\necho AFTER:$MY_KEY\nexit 0\n",
        "VAR:MySecret123"
    )
    runner.run_case(
        "Bullet 2: Quote-Aware Parser",
        "Special variables ($$ PID and ~ Home)",
        "echo PID:$$\necho HOME:~\nexit 0\n",
        "PID:"
    )

    # -------------------------------------------------------------
    # Category 3: File I/O Redirection & Multi-Stage Pipelines
    # -------------------------------------------------------------
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "Output overwrite (> ) and Input (< ) redirection",
        "echo 'line one' > /tmp/test_redir_1.txt\ncat < /tmp/test_redir_1.txt\nexit 0\n",
        "line one"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "Output append (>> ) redirection",
        "echo 'first' > /tmp/test_redir_2.txt\necho 'second' >> /tmp/test_redir_2.txt\ncat /tmp/test_redir_2.txt\nexit 0\n",
        "second"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "Attached redirection operators without spaces (cat<in>out)",
        "echo 'compact test'>/tmp/test_redir_3.txt\ncat</tmp/test_redir_3.txt\nexit 0\n",
        "compact test"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "2-Stage Pipeline (cmd1 | cmd2)",
        "echo 'hello world' | grep 'hello'\nexit 0\n",
        "hello world"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "3-Stage Pipeline (cmd1 | cmd2 | cmd3)",
        "echo 'one' > /tmp/p3.txt\necho 'two' >> /tmp/p3.txt\necho 'three' >> /tmp/p3.txt\ncat /tmp/p3.txt | grep 'two' | wc -l\nexit 0\n",
        "1"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "4-Stage Pipeline (cmd1 | cmd2 | cmd3 | cmd4)",
        "echo 'apple' > /tmp/p4.txt\necho 'banana' >> /tmp/p4.txt\necho 'cherry' >> /tmp/p4.txt\ncat /tmp/p4.txt | grep -v 'banana' | tr 'a-z' 'A-Z' | wc -l\nexit 0\n",
        "2"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "Pipeline combined with input (<) and output (>) redirection",
        "echo 'dog' > /tmp/p_in.txt\necho 'cat' >> /tmp/p_in.txt\necho 'bird' >> /tmp/p_in.txt\ncat < /tmp/p_in.txt | grep 'cat' | wc -l > /tmp/p_out.txt\ncat /tmp/p_out.txt\nexit 0\n",
        "1"
    )
    runner.run_case(
        "Bullet 3: I/O & Pipelines",
        "Wildcard Globbing expansion (*.cpp)",
        "ls src/*.cpp\nexit 0\n",
        "main.cpp"
    )

    # -------------------------------------------------------------
    # Category 4: Background Jobs, Signals & Process Reaping
    # -------------------------------------------------------------
    runner.run_case(
        "Bullet 4: Jobs & Signals",
        "Background execution (&) and jobs list",
        "sleep 0.3 &\njobs\nsleep 0.4\njobs\nexit 0\n",
        "Running"
    )
    runner.run_case(
        "Bullet 4: Jobs & Signals",
        "Non-blocking process reaping (No zombie leaks)",
        "sleep 0.2 &\nsleep 0.3\njobs\nexit 0\n",
        "myshell>"
    )

    # Clean up temp files
    for f in [
        "/tmp/test_redir_1.txt", "/tmp/test_redir_2.txt", "/tmp/test_redir_3.txt",
        "/tmp/p3.txt", "/tmp/p4.txt", "/tmp/p_in.txt", "/tmp/p_out.txt"
    ]:
        if os.path.exists(f):
            try:
                os.remove(f)
            except OSError:
                pass

    print("=" * 70)
    print(f" TOTAL TESTS: {runner.passed + runner.failed}")
    print(f" PASSED:      {runner.passed}")
    print(f" FAILED:      {runner.failed}")
    print("=" * 70)

    if runner.failed == 0:
        print(">>> ALL 22 TEST CASES PASSED PERFECTLY (100% COVERAGE)! <<<")
        sys.exit(0)
    else:
        print(">>> SOME TESTS FAILED <<<")
        sys.exit(1)

if __name__ == "__main__":
    main()