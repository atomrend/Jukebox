#include "Filter.h"
#include "JukeboxPlaylist.h"
#include <ctype.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STACK_SIZE 100

typedef struct Token
{
    // Valid left side values with type.
    // 0 = invalid token
    // 1 = title (string)
    // 2 = album (string)
    // 3 = artist (string)
    // 4 = genre (string)
    // 5 = comment (string)
    // 6 = year (integer)
    // 7 = favorite (boolean)
    unsigned char left;

    // Valid operation values.
    // 0 = invalid operator
    // 1 = ==
    // 2 = !=
    // 3 = <
    // 4 = >
    // 5 = <=
    // 6 = >=
    // 7 = =~ (regular expression)
    unsigned char tokenOperator;

    // Right hand values.
    char right[256];
} Token;

void
toLowerCase(char output[256], const char* const input, int length)
{
	int index = 0;
	
	while (input[index] && index < length) {
		output[index] = tolower(input[index]);
		++index;
	}
    output[index] = '\0';
}

void
push(unsigned char value, unsigned char stack[STACK_SIZE], int* position)
{
    if (*position < STACK_SIZE) {
        *position += 1;
        stack[*position] = value;
    }
}

unsigned char
pop(unsigned char stack[STACK_SIZE], int* position)
{
    unsigned char result = 0xff;

    if (*position < STACK_SIZE && *position >= 0) {
        result = stack[*position];
        *position -= 1;
    }

    return result;
}

unsigned int
tokenize(Token** output, const char* const input)
{
    unsigned int result = 0;
    int operatorIndex[STACK_SIZE];
    char lowerCase[256];
    Token *token = 0;
    unsigned int index = 0;
    int start = 0;
    int end = 0;

    index = 0;
    while (index < STACK_SIZE) {
        operatorIndex[index] = 0;
        ++index;
    }

    // Determine number of tokens.
    index = 0;
    while (index < strlen(input)) {
        if (input[index] == '"') {
            // Iterate till the closing quotation mark.
            ++index;
            while (input[index] != '"' && index < strlen(input)) {
                ++index;
            }
            ++index;
        } else if (strncmp(input + index, "==", 2) == 0) {
            operatorIndex[result] = index;
            ++result;
        } else if (strncmp(input + index, "!=", 2) == 0) {
            operatorIndex[result] = index;
            ++result;
        } else if (strncmp(input + index, "<=", 2) == 0) {
            operatorIndex[result] = index;
            ++result;
        } else if (strncmp(input + index, ">=", 2) == 0) {
            operatorIndex[result] = index;
            ++result;
        } else if (strncmp(input + index, "=~", 2) == 0) {
            operatorIndex[result] = index;
            ++result;
        } else if (input[index] == '<') {
            operatorIndex[result] = index;
            ++result;
        } else if (input[index] == '>') {
            operatorIndex[result] = index;
            ++result;
        }

        ++index;
    }

    // Allocate the tokens and initialize to invalid state.
    *output = (Token*) malloc(result * sizeof(Token));
    token = *output;
    index = 0;
    while (index < result) {
        token->left = 0;
        token->tokenOperator = 0;
        token->right[0] = '\0';

        ++token;
        ++index;
    }

    // Determine the left value of the token.
    token = *output;
    index = 0;
    while (index < result) {
        end = operatorIndex[index] - 1;
        while (end >= 0) {
            if (input[end] != ' ') {
                break;
            }
            --end;
        }
        if (end == 0) {
            continue;
        }

        start = end - 1;
        while (start >= 0) {
            if (isspace(input[start]) != 0 || input[start] == '(') {
                break;
            }
            --start;
        }
        ++start;

        toLowerCase(lowerCase, input + start, end - start + 1);
        if (strncmp(lowerCase, "title", 5) == 0) {
            token->left = 1;
        } else if (strncmp(lowerCase, "album", 5) == 0) {
            token->left = 2;
        } else if (strncmp(lowerCase, "artist", 6) == 0) {
            token->left = 3;
        } else if (strncmp(lowerCase, "genre", 5) == 0) {
            token->left = 4;
        } else if (strncmp(lowerCase, "comment", 7) == 0) {
            token->left = 5;
        } else if (strncmp(lowerCase, "year", 4) == 0) {
            token->left = 6;
        } else if (strncmp(lowerCase, "favorite", 8) == 0) {
            token->left = 7;
        }

        ++token;
        ++index;
    }

    // Determine the token operator.
    token = *output;
    index = 0;
    while (index < result) {
        if (token->left > 0) {
            if (strncmp(input + operatorIndex[index], "==", 2) == 0) {
                token->tokenOperator = 1;
            } else if (strncmp(input + operatorIndex[index], "!=", 2) == 0) {
                token->tokenOperator = 2;
            } else if (strncmp(input + operatorIndex[index], "<=", 2) == 0) {
                token->tokenOperator = 5;
            } else if (strncmp(input + operatorIndex[index], ">=", 2) == 0) {
                token->tokenOperator = 6;
            } else if (strncmp(input + operatorIndex[index], "=~", 2) == 0) {
                token->tokenOperator = 7;
            } else if (input[operatorIndex[index]] == '<') {
                token->tokenOperator = 3;
            } else if (input[operatorIndex[index]] == '>') {
                token->tokenOperator = 4;
            }
        }

        ++token;
        ++index;
    }

    // Determine the right hand value.
    // If the value starts with a quotation mark, it must end with one.
    // If it does not use quotations, then it must be a single word.
    // Iterate until a whitespace, logical operator, or grouping.
    token = *output;
    index = 0;
    while (index < result) {
        if (token->left > 0) {
            start = operatorIndex[index] + 1;
            if (token->tokenOperator < 3 ||
                    token->tokenOperator > 4) {
                ++start;
            }

            // Iterate forward until a nonwhitespace is found.
            while (input[start] != 0 && isspace(input[start]) != 0) {
                ++start;
            }

            if (input[start] == '"') {
                // Iterate until the closing quotation is found.
                ++start;
                end = start + 1;
                while (input[end] != '"' && input[end] != 0) {
                    ++end;
                }
            } else {
                end = start + 1;
                while (input[end] != 0 && isspace(input[end]) == 0) {
                    ++end;
                }
            }

            strncpy(token->right, input + start, (unsigned int) end - start);
            token->right[end - start] = '\0';
        }

        ++token;
        ++index;
    }

    // Determine if any tokens are invalid.
    start = 0;
    token = *output;
    index = 0;
    while (index < result) {
        if (token->left == 0 || token->tokenOperator == 0 ||
                strlen(token->right) == 0) {
            start = 1;
            break;
        }

        ++token;
        ++index;
    }
    if (start == 1) {
        free(*output);
        result = 0;
    }

    return result;
}

void
postfixConversion(unsigned char output[256], const char* const filter)
{
    unsigned char stack[STACK_SIZE];
    int stackPosition = -1;
    unsigned int index = 0;
    unsigned char pos = 0;
    unsigned char tokenIndex = 0;
    unsigned char value = 0;

    // 0xff = Invalid, 0xfe = NOT, 0xfd = OR, 0xfc = AND, 0xfb = (

    memset(output, 0xff, 256);

    while (index < strlen(filter)) {
        if (filter[index] == '"') {
            // Go to the closing quotation mark.
            ++index;
            while (filter[index] != '"' && filter[index] != 0) {
                ++index;
            }
            ++index;
        } else if (strncmp(filter + index, "&&", 2) == 0) {
            push(0xfc, stack, &stackPosition);
            ++index;
        } else if (strncmp(filter + index, "||", 2) == 0) {
            push(0xfd, stack, &stackPosition);
            ++index;
        } else if (strncmp(filter + index, "==", 2) == 0) {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
            ++index;
        } else if (strncmp(filter + index, "!=", 2) == 0) {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
            ++index;
        } else if (strncmp(filter + index, "<=", 2) == 0) {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
            ++index;
        } else if (strncmp(filter + index, ">=", 2) == 0) {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
            ++index;
        } else if (strncmp(filter + index, "=~", 2) == 0) {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
            ++index;
        } else if (filter[index] == '<') {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
        } else if (filter[index] == '>') {
            output[pos] = tokenIndex;
            ++pos;
            ++tokenIndex;
        } else if (filter[index] == '!') {
            push(0xfe, stack, &stackPosition);
        } else if (filter[index] == '(') {
            push(0xfb, stack, &stackPosition);
        } else if (filter[index] == ')') {
            value = pop(stack, &stackPosition);
            while (value != 0xfb && value != 0xff) {
                output[pos] = value;
                ++pos;

                value = pop(stack, &stackPosition);
            }
        }

        ++index;
    }

    value = pop(stack, &stackPosition);
    while (value != 0xfb && value != 0xff) {
        output[pos] = value;
        ++pos;

        value = pop(stack, &stackPosition);
    }
}

unsigned char
evaluateToken(const FileInfo* const song, const Token* const token)
{
    unsigned char result = 0;
    regex_t regEx;
    char *leftString = 0;
    unsigned int leftInteger = 0;
    unsigned int rightInteger = 0;

    switch (token->left) {
    case 1:
        leftString = song->title;
        break;
    case 2:
        leftString = song->album;
        break;
    case 3:
        leftString = song->artist;
        break;
    case 4:
        leftString = song->genre;
        break;
    case 5:
        leftString = song->comment;
        break;
    case 6:
        leftInteger = song->year;
        break;
    case 7:
        leftInteger = (unsigned int) song->favorite;
        break;
    default:
        break;
    }

    if (token->left > 0 && token->left < 6) {
        // String comparison.
        switch (token->tokenOperator) {
        case 1:
            result = strcmp(leftString, token->right) == 0;
            break;
        case 2:
            result = strcmp(leftString, token->right) != 0;
            break;
        case 3:
            result = strcmp(leftString, token->right) < 0;
            break;
        case 4:
            result = strcmp(leftString, token->right) > 0;
            break;
        case 5:
            result = strcmp(leftString, token->right) <= 0;
            break;
        case 6:
            result = strcmp(leftString, token->right) >= 0;
            break;
        case 7:
            if (regcomp(&regEx, token->right, REG_EXTENDED | REG_NOSUB) == 0) {
                result = regexec(&regEx, leftString, 0, 0, 0) == 0;
                regfree(&regEx);
            }
            break;
        default:
            break;
        }
    } else if (token->left >= 6 && token->left <= 7) {
        // Integer comparison.
        rightInteger = (unsigned int) atoi(token->right);

        switch (token->tokenOperator) {
        case 1:
            result = leftInteger == rightInteger;
            break;
        case 2:
            result = leftInteger != rightInteger;
            break;
        case 3:
            result = leftInteger < rightInteger;
            break;
        case 4:
            result = leftInteger >  rightInteger;
            break;
        case 5:
            result = leftInteger <= rightInteger;
            break;
        case 6:
            result = leftInteger >= rightInteger;
            break;
        default:
            break;
        }
    }

    return result;
}

unsigned char
evaluateExpression(
    const FileInfo* const file,
    const unsigned char postfix[512],
    const Token* const tokens,
    unsigned int numTokens)
{
    unsigned char stack[STACK_SIZE];
    unsigned char *tokenResults = (unsigned char*) calloc(numTokens,
        sizeof(unsigned char));
    unsigned int index = 0;
    int stackPosition = -1;
    unsigned char left;
    unsigned char right;

    // Evaluate each token.
    while (index < numTokens) {
        tokenResults[index] = evaluateToken(file, tokens + index);
        ++index;
    }

    // Evaluate the postfix expression using the tokens.
    index = 0;
    while (postfix[index] != 0xff) {
        if (postfix[index] == 0xfe) {
            right = pop(stack, &stackPosition);
            push(!right, stack, &stackPosition);
        } else if (postfix[index] == 0xfd) {
            left = pop(stack, &stackPosition);
            right = pop(stack, &stackPosition);
            push(left || right, stack, &stackPosition);
        } else if (postfix[index] == 0xfc) {
            left = pop(stack, &stackPosition);
            right = pop(stack, &stackPosition);
            push(left && right, stack, &stackPosition);
        } else {
            push(tokenResults[postfix[index]], stack, &stackPosition);
        }

        ++index;
    }

    free(tokenResults);

    return pop(stack, &stackPosition);
}

char*
filterPlaylist(
    unsigned int* count,
    const char* const filter,
    const FileList* const fileList)
{
    char *result = 0;
    char *vector = (char*) calloc(fileList->count, sizeof(char));
    unsigned char postfix[256];
	Token *tokens = 0;
    FileInfo *file = fileList->list;
	unsigned int numTokens = tokenize(&tokens, filter);
	unsigned int index = 0;
    unsigned int resultIndex = 0;
    unsigned char evaluationResult = 0;

    *count = 0;

    if (numTokens > 0) {
        memset(vector, 0, fileList->count);

        postfixConversion(postfix, filter);

        // Evaluate each file against the expression.
        index = 0;
        while (index < fileList->count) {
            evaluationResult = evaluateExpression(file, postfix, tokens,
                numTokens);
            if (evaluationResult == 1) {
                vector[index] = 1;
                ++(*count);
            }
            ++file;
            ++index;
        }

        free(tokens);

        result = (char*) malloc(*count * 17 * sizeof(char));

        // Copy the filtered file IDs into the result list.
        file = fileList->list;
        index = 0;
        while (index < fileList->count) {
            if (vector[index] == 1) {
                strncpy(result + (resultIndex * 17), file->id, 17);
                ++resultIndex;
            }
            ++file;
            ++index;
        }
    }

    free(vector);

    return result;
}
