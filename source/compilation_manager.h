#pragma once
#include "/pck/sys/include/fs.h"
#include "/pck/sys/include/mem.h"
#include "/pck/sys/include/strings.h"
#include "/pck/sys/include/dbg.h"
#include "argv_parser.h"
#include "parser.h"
#include "info.h"
#include <stdlib.h>

#define Help                                                                                               \
  "Usage: zpp [task] +[flags..] [input-file]\n"                                                            \
  "\n"                                                                                                     \
  "Tasks:\n"                                                                                               \
  "  + astgen ------------------------ generate ast json from 'input-file'\n"                              \
  "  + build ------------------------- generate some specifiable ouput (default: exe) from 'input-file'\n" \
  "  + run --------------------------- generate executable in /tmp/ and run it\n"                          \
  "  + test -------------------------- generate executable in /tmp/ and run its tests\n"                   \
  "\n"                                                                                                     \
  "  + version ----------------------- print out the compiler version\n"                                   \
  "  + help -------------------------- print out this message\n"                                           \
  "\n"                                                                                                     \
  "Flags:\n"                                                                                               \
  "  + opt:n ------------------------- set the optimization level (1, 2, 3, default: 0) to n\n"            \
  "  + out:path ---------------------- set the output path\n"                                              \
  "\n"                                                                                                     \
  "Examples:\n"                                                                                            \
  "  + `zpp build +opt:2 main.zpp` --- generate optimized executable from source\n"

// ! create a parser instance
// ! and collects all global nodes into a buffer
error AstGen(ArgvTable const* self);

// ! performs the specified task as the first parameter in the command line
// ! for example `zpp help`, `zpp version` `zpp build +opt:1 main.zpp`
error CompilationTaskRun(ArgvTable const* self);

// ! take the index and the reference text and calculate
// ! at which line and column the index is
inline void CalculateLineAndColumn(SourceLocation const* location, u32* line_out, u32* column_out) {
  *line_out = 1;
  *column_out = 1;

  for (u64 i = 0; i < location->index; i++) {
    auto c = location->source_reference->buffer[i];

    if (c == '\n') {
      *line_out += 1;
      *column_out = 1;
      continue;
    }

    *column_out += 1;
  }
}

// ! print the first part of the error
inline void PrintErrorFirstPart(SourceLocation const* location) {
  u32 line, column;
  CalculateLineAndColumn(location, &line, &column);

  printf("%s:%u:%hu: ", location->source_reference->Filename, line, column);
}

inline void ReportUnexpectedToken(Token const* token) {
  PrintErrorFirstPart(&token->location);
  printf("unexpected token: %.*s\n", token->length, GetTokenValue(token));
  exit(1);
}

inline void ReportUnexpectedTokenInGlobalContext(Token const* token) {
  PrintErrorFirstPart(&token->location);
  printf("unexpected token in global context: `%.*s`\n", token->length, GetTokenValue(token));
  exit(1);
}

inline void ReportExpectedAnotherToken(Token const* found_token, u8 expected_token_tag) {
  PrintErrorFirstPart(&found_token->location);
  if (TokenTagIsSym(expected_token_tag))
    printf("expected token `%c`, found `%.*s`\n", expected_token_tag, found_token->length, GetTokenValue(found_token));
  else
    printf("expected token `%s`, found `%.*s`\n", TokenTagToString(expected_token_tag), found_token->length, GetTokenValue(found_token));
  exit(1);
}