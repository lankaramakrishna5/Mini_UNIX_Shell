#!/usr/bin/env bash
set -e

echo "=== Running Mini UNIX Shell Automated Tests ==="

echo "[Test 1] Quotes and Echo"
printf 'echo "hello world"\necho '\''single quotes work'\''\necho no_quotes\nexit 0\n' | ./myshell

echo "[Test 2] File Redirection"
printf 'echo "first line" > /tmp/shell_test_out.txt\necho "second line" >> /tmp/shell_test_out.txt\ncat < /tmp/shell_test_out.txt\nexit 0\n' | ./myshell

echo "[Test 3] Pipelines"
printf 'cat /tmp/shell_test_out.txt | grep line | wc -l\nexit 0\n' | ./myshell

echo "[Test 4] Env vars and Exit code"
printf 'export TEST_VAR=MiniShell123\necho "TEST_VAR: $TEST_VAR"\nls /nonexistent_dir_mini_shell\necho "Last status: $?"\nexit 0\n' | ./myshell

echo "[Test 5] Builtins cd, pwd, help"
printf 'pwd\ncd /tmp\npwd\ncd -\npwd\nhelp\nexit 0\n' | ./myshell

echo "[Test 6] Background execution and jobs"
printf 'sleep 0.3 &\njobs\nsleep 0.4\njobs\nexit 0\n' | ./myshell

rm -f /tmp/shell_test_out.txt
echo "=== All Tests Passed Successfully! ==="