//
// Created by CL on 2022/8/14.
//

#ifndef RVCC_ASTCONTEXT_H
#define RVCC_ASTCONTEXT_H
#include "ASTBaseNode.h"
#include <map>

class Token;
class VarObj;
class Function;

// Generate AST
class ASTContext {
public:
  ASTContext() = default;

  static ASTContext &instance();

  // BNF:
  //    这样来构建，可以保证优先级没有问题, 越往下，优先级越高
  //    program = "{" compoundStmt // 表示程序是由多个statements(语句)来构成的
  //    compoundStmt = stmt* "}"
  //    stmt = "return" expr ";" | exprStmt  // 语句是由表达式语句构成,
  //    stmt = "return" expr ";"
  //           | "if" "(" expr ")" stmt ("else" stmt)?
  //           | "{" compoundStmt
  //           | exprStmt
  Function *create(Token *Tok);

private:
  Node *compoundStmt(Token **Rest, Token *Tok);
  Node *createStmt(Token **Rest, Token *Tok);
  Node *createExprStmt(Token **Rest, Token *Tok);
  Node *createExpr(Token **Rest, Token *Tok);

  Node *createAssignExpr(Token **Rest, Token *Tok);
  Node *createEqualityExpr(Token **Rest, Token *Tok);
  Node *createRelationalExpr(Token **Rest, Token *Tok);

  Node *createAddExpr(Token **Rest, Token *Tok);
  Node *createMulExpr(Token **Rest, Token *Tok);
  Node *createUnaryExpr(Token **Rest, Token *Tok);
  Node *createPrimaryExpr(Token **Rest, Token *Tok);

  friend class Node;

private:
  Token *CurTok;
};

#endif // RVCC_ASTCONTEXT_H
