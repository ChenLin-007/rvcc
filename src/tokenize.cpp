#include "tokenize.h"
#include "rvcc.h"
#include <iostream>
#include <memory>
#include <stack>
#include <cstdarg>
#include <string>
#include <cassert>

static Token *TokenHdl = nullptr;
Token &Token::instance() {
  if (TokenHdl == nullptr) {
    TokenHdl = new Token();
  }
  return *TokenHdl;
}


Token *Token::createToken(Token::TKind kind,
                          std::string::iterator Start,
                          std::string::iterator End) {
  auto *Tok = new Token{kind, nullptr, 0, Start,
                        static_cast<size_t>(std::distance(Start, End))};
  return Tok;
}

std::string::iterator Token::getNumberPos(std::string &input,
                                          std::string::iterator Start) {
  auto End = input.end();
  for (; Start != End; ++Start) {
    char achar = *Start;
    if (!std::isdigit(achar))
      return Start;
  }
  return Start;
}

// EOF parser.
Token *Token::tokenize(std::string &input) {
  Token Head{};
  Token *Cur = &Head;
  auto It = input.begin();

  for (auto End = input.end(); It != End;) {
    char achar = *It;

    // skip spaces
    if (std::isspace(achar)) {
      ++It;
      continue;
    }

    // parse number.
    if (std::isdigit(achar)) {
      Cur->Next = TokenHdl->createToken(Token::TKind::TK_NUM, It, It);
      Cur = Cur->Next;

      auto NumEndPos = TokenHdl->getNumberPos(input, It);
      Cur->Val = std::stoi(input.substr(std::distance(input.begin(), It),
                                        std::distance(It, NumEndPos)));
      Cur->Len = std::distance(It, NumEndPos);
      It = NumEndPos;
      continue;
    }

    // parse operators.
    int PunctLen = readPunct(input, std::distance(input.begin(), It));
    if (PunctLen) {
      Cur->Next = TokenHdl->createToken(Token::TKind::TK_PUNCT, It, It + PunctLen);
      Cur = Cur->Next;
      // move string iterator PunctLen length.
      It += PunctLen;
      continue;
    }

    // unknown character.
    error("invalid token: %c", achar);
  }

  // Add EOF operator.
  Cur->Next = TokenHdl->createToken(Token::TKind::TK_EOF, It, It);

  return Head.Next;
}


