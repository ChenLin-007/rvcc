#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <string>

class Token {
public:
  enum class TKind {
    TK_IDENT, // ident
    TK_PUNCT, // operator. + -
    TK_NUM,   // number
    TK_EOF,   // end of file
  };

  Token() = default;

  Token(TKind Kind_, Token *Next_, int Val_, std::string::iterator Loc_,
        size_t Len_)
    : Kind(Kind_), Next(Next_), Val(Val_), Loc(Loc_), Len(Len_) {}

public:
  [[nodiscard]] TKind getKind() const { return Kind; }
  [[nodiscard]] Token *next() const {return Next; }
  void setNext(Token *Tok_) { Next = Tok_; }
  [[nodiscard]] int getVal() const { return Val; }
  void setVal(int Val_) { Val = Val_; }
  [[nodiscard]] std::string::iterator getLocation() const { return Loc; }
  [[nodiscard]] std::string::size_type getLength() const { return Len; }
  void setLen(std::string::size_type Len_) { Len = Len_; }
  [[nodiscard]] std::string_view getTokenName() {
    std::string::size_type Start = std::distance(SourceCode.begin(), Loc);
    return std::string_view{SourceCode.data() + Start, Len};
  }

  void dump(unsigned StatementIndent = 0, unsigned Depth = 0);

private:
  std::string SourceCode;
  TKind Kind;                 // kind
  Token *Next;                // next token
  int Val;                    // value
  std::string::iterator Loc;  // location
  std::string::size_type Len; // length
};

class TokenContext {
public:
  TokenContext() = default;
  static TokenContext &instance();
  Token *tokenize(std::string &&input);

private:
  Token *create(Token::TKind kind, std::string::iterator start,
                std::string::iterator end);

private:
  std::string source_code_;
};





#endif