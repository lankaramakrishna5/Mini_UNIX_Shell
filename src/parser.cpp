#include "parser.h"

#include <cstdlib>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace
{
    enum class TokenType
    {
        WORD,
        PIPE,
        REDIRECT_IN,
        REDIRECT_OUT,
        REDIRECT_APPEND,
        BACKGROUND
    };

    struct Token
    {
        TokenType type;
        string value;
    };

    string expandVariable(const string &input, size_t &i, int lastExitStatus)
    {
        // i points to '$'
        ++i;
        if (i >= input.size())
            return "$";

        if (input[i] == '?')
        {
            ++i;
            return to_string(lastExitStatus);
        }

        if (input[i] == '$')
        {
            ++i;
            return to_string(getpid());
        }

        if (input[i] == '{')
        {
            ++i;
            string varName;
            while (i < input.size() && input[i] != '}')
            {
                varName += input[i];
                ++i;
            }
            if (i < input.size() && input[i] == '}')
                ++i;

            const char *val = getenv(varName.c_str());
            return val ? string(val) : "";
        }

        string varName;
        while (i < input.size() && (isalnum(static_cast<unsigned char>(input[i])) || input[i] == '_'))
        {
            varName += input[i];
            ++i;
        }

        if (varName.empty())
            return "$";

        const char *val = getenv(varName.c_str());
        return val ? string(val) : "";
    }

    vector<Token> tokenize(const string &input, int lastExitStatus)
    {
        vector<Token> tokens;
        size_t i = 0;
        const size_t n = input.size();

        while (i < n)
        {
            // Skip whitespace
            while (i < n && isspace(static_cast<unsigned char>(input[i])))
            {
                ++i;
            }

            if (i >= n)
                break;

            // Check comments (#)
            if (input[i] == '#')
            {
                break; // Ignore rest of line
            }

            // Check append redirection >>
            if (input[i] == '>' && i + 1 < n && input[i + 1] == '>')
            {
                tokens.push_back(Token{TokenType::REDIRECT_APPEND, ">>"});
                i += 2;
                continue;
            }

            // Check single character operators
            if (input[i] == '>')
            {
                tokens.push_back(Token{TokenType::REDIRECT_OUT, ">"});
                ++i;
                continue;
            }

            if (input[i] == '<')
            {
                tokens.push_back(Token{TokenType::REDIRECT_IN, "<"});
                ++i;
                continue;
            }

            if (input[i] == '|')
            {
                tokens.push_back(Token{TokenType::PIPE, "|"});
                ++i;
                continue;
            }

            if (input[i] == '&')
            {
                tokens.push_back(Token{TokenType::BACKGROUND, "&"});
                ++i;
                continue;
            }

            // Parse word / argument
            string currentWord;
            bool hasContent = false;

            while (i < n)
            {
                char c = input[i];

                if (isspace(static_cast<unsigned char>(c)) || c == '|' || c == '<' || c == '>' || c == '&' || c == '#')
                {
                    break;
                }

                if (c == '\'')
                {
                    hasContent = true;
                    ++i; // skip opening single quote
                    while (i < n && input[i] != '\'')
                    {
                        currentWord += input[i];
                        ++i;
                    }
                    if (i < n && input[i] == '\'')
                    {
                        ++i; // skip closing single quote
                    }
                }
                else if (c == '"')
                {
                    hasContent = true;
                    ++i; // skip opening double quote
                    while (i < n && input[i] != '"')
                    {
                        if (input[i] == '\\' && i + 1 < n)
                        {
                            char next = input[i + 1];
                            if (next == '"' || next == '\\' || next == '$' || next == '`')
                            {
                                currentWord += next;
                                i += 2;
                                continue;
                            }
                        }
                        else if (input[i] == '$')
                        {
                            currentWord += expandVariable(input, i, lastExitStatus);
                            continue;
                        }
                        currentWord += input[i];
                        ++i;
                    }
                    if (i < n && input[i] == '"')
                    {
                        ++i; // skip closing double quote
                    }
                }
                else if (c == '\\')
                {
                    hasContent = true;
                    ++i;
                    if (i < n)
                    {
                        currentWord += input[i];
                        ++i;
                    }
                }
                else if (c == '$')
                {
                    hasContent = true;
                    currentWord += expandVariable(input, i, lastExitStatus);
                }
                else if (c == '~' && currentWord.empty() && !hasContent)
                {
                    hasContent = true;
                    if (i + 1 >= n || input[i + 1] == '/' || isspace(static_cast<unsigned char>(input[i + 1])))
                    {
                        const char *home = getenv("HOME");
                        if (home)
                            currentWord += home;
                        else
                            currentWord += "~";
                        ++i;
                    }
                    else
                    {
                        currentWord += c;
                        ++i;
                    }
                }
                else
                {
                    hasContent = true;
                    currentWord += c;
                    ++i;
                }
            }

            if (hasContent)
            {
                tokens.push_back(Token{TokenType::WORD, currentWord});
            }
        }

        return tokens;
    }
} // namespace

ParsedLine parseCommand(const string &input, int lastExitStatus)
{
    ParsedLine result;
    result.background = false;

    vector<Token> tokens = tokenize(input, lastExitStatus);

    Command currentCommand;
    currentCommand.append = false;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const Token &tok = tokens[i];

        if (tok.type == TokenType::BACKGROUND)
        {
            result.background = true;
        }
        else if (tok.type == TokenType::PIPE)
        {
            if (!currentCommand.args.empty() || !currentCommand.inputFile.empty() || !currentCommand.outputFile.empty())
            {
                result.commands.push_back(currentCommand);
                currentCommand = Command();
                currentCommand.append = false;
            }
        }
        else if (tok.type == TokenType::REDIRECT_IN)
        {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::WORD)
            {
                currentCommand.inputFile = tokens[++i].value;
            }
        }
        else if (tok.type == TokenType::REDIRECT_OUT)
        {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::WORD)
            {
                currentCommand.outputFile = tokens[++i].value;
                currentCommand.append = false;
            }
        }
        else if (tok.type == TokenType::REDIRECT_APPEND)
        {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::WORD)
            {
                currentCommand.outputFile = tokens[++i].value;
                currentCommand.append = true;
            }
        }
        else if (tok.type == TokenType::WORD)
        {
            currentCommand.args.push_back(tok.value);
        }
    }

    if (!currentCommand.args.empty() || !currentCommand.inputFile.empty() || !currentCommand.outputFile.empty())
    {
        result.commands.push_back(currentCommand);
    }

    return result;
}

