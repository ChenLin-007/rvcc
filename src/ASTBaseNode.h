#ifndef PRASE_H
#define PRASE_H
#include "../include/c_syntax.h"
#include "logs.h"
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <stack>
#include <string>

class VarObj;
class Function;
class Type;

class Node {
public:
  // AST node type.
  enum class NKind {
#define NODE_INFO(Type, Expr, Desc) ND_##Type,
#include "node_type.def"
  };

  enum class TypeSystemKind : unsigned {
    TY_INT, // int
    TY_PTR, // pointer
  };

  class Type {
  public:
    explicit Type(TypeSystemKind TyKind = TypeSystemKind::TY_INT,
                  Type *Base = nullptr)
        : Kind_(TyKind), Base_(Base) {}

    [[nodiscard]] bool isInteger() const {
      return Kind_ == TypeSystemKind::TY_INT;
    }
    [[nodiscard]] bool isPtr() const { return Kind_ == TypeSystemKind::TY_PTR; }
    [[nodiscard]] Type *getBase() { return Base_; }
    [[nodiscard]] Type *getBase() const { return Base_; }

  public:
    static Type *getIntTy() { return TyInt_; }

  private:
    TypeSystemKind Kind_; // Kind.
    Type *Base_;          // the original type which pointer points to.
    static Type *TyInt_;
  };

public:
	[[nodiscard]] Node::NKind getKind() const { return Kind_; }

 	void dump(unsigned Depth = 0);

	virtual void print(std::ostream &os) const {}

public:
  Node() = delete;	// 不能使用这个ctor
  explicit Node(Node::NKind Kind, std::string_view Name,
                Node *Next = nullptr)
      : Kind_(Kind), Name_(Name), Next_(Next), Ty_(Type::getIntTy()) {}

  [[nodiscard]] Node *getNext() { return Next_; }
  void setNext(Node *Next) { Next_ = Next; }

  // node type related.
  [[nodiscard]] bool isIntegerTy() const { return Ty_->isInteger(); }
  [[nodiscard]] bool isPtrTy() const { return Ty_->isPtr(); }
  [[nodiscard]] Type *getPtrTyBase() { return Ty_->getBase(); }
  [[nodiscard]] Type *getTy() const { return Ty_; }

  void setTy(Type *Ty) { Ty_ = Ty; }

  static Node *createUnaryNode(Node::NKind Kind, Node *Nd);
  static Node *createKeywordNode(c_syntax::CKType, Node *N1, Node *N2 = nullptr,
                                 Node *N3 = nullptr, Node *N4 = nullptr);
  static Node *createBinaryNode(Node::NKind Kind, Node *LHS, Node *RHS);
  static Node *createNumNode(int Val);
  static Node *createVarNode(VarObj *Var);
  static std::string getTypeName(Node::NKind Kind) {
#define NODE_INFO(Type, Expr, Desc)                                            \
  case Node::NKind::ND_##Type:                                                 \
    return Expr;

    switch (Kind) {
#include "node_type.def"
    default:
      logging::unreachable("unknown node type:", static_cast<uint8_t>(Kind));
      break;
    }
  }
  friend class ASTContext;

protected:
  Node *Next_ = nullptr;

private:
  Node::NKind Kind_;      // node type.
  std::string_view Name_; // variable name.
  Type *Ty_;
};

// ########################## type information #################################
Node::Type *pointerTo(Node::Type *Base);
void addType(Node *Nd);

class BinaryNode : public Node {
public:
  explicit BinaryNode(Node::NKind Kind, std::string_view Name, Node *LHS,
                      Node *RHS)
      : Node(Kind, Name), LHS_(LHS), RHS_(RHS) {}

  [[nodiscard]] Node *getLHS() { return LHS_; }
  [[nodiscard]] Node *getRHS() { return RHS_; }
  [[nodiscard]] Node::Type *getLhsTy() const { return LHS_->getTy(); }
  [[nodiscard]] Node::Type *getRhsTy() const { return RHS_->getTy(); }
  void setLTy(Node::Type *Ty) { LHS_->setTy(Ty); }
  void setRTy(Node::Type *Ty) { RHS_->setTy(Ty); }

  void print(std::ostream &os) const override {
    os << Node::getTypeName(getKind());
  }

public:
  static bool isa(const Node *N) {
#define BINARY_NODE_INFO(Type, Expr, Desc)                                     \
  case Node::NKind::ND_##Type:                                                 \
    return true;

    switch (N->getKind()) {
#include "node_type.def"
    default:
      break;
    }
    return false;
  }

private:
  Node *LHS_;
  Node *RHS_;
};

class UnaryNode : public Node {
public:
  explicit UnaryNode(Node::NKind Kind, Node *Rhs = nullptr)
      : Node(Kind, Node::getTypeName(Kind)), Rhs_(Rhs) {}

  [[nodiscard]] Node *getRhs() const { return Rhs_; }

  void print(std::ostream &os) const override {
    os << Node::getTypeName(getKind());
  }

public:
  static bool isa(const Node *N) {
#define UNARY_NODE_INFO(Keyword, Expr, Desc)                                   \
  case Node::NKind::ND_##Keyword:                                              \
    return true;

    switch (N->getKind()) {
#include "node_type.def"
    default:
      break;
    }
    return false;
  }

private:
  Node *Rhs_;
};

// 数字
class NumNode : public Node {
public:
  explicit NumNode(const std::string_view Name, int Value)
      : Node(Node::NKind::ND_NUM, Name), Value_(Value) {}

  [[nodiscard]] int getValue() const { return Value_; }

  void print(std::ostream &os) const override {
    os << Node::getTypeName(getKind());
  }

public:
  static bool isa(const Node *N) { return N->getKind() == Node::NKind::ND_NUM; }

private:
  int Value_;
};

class KeywordNode : public Node {
public:
  explicit KeywordNode(c_syntax::CKType KeywordType, Node *L);

  [[nodiscard]] std::string_view getKeywordName() const { return KeywordName_; }
  [[nodiscard]] Node *getBody() { return BodyNode_; }
  [[nodiscard]] c_syntax::CKType getKeywordType() const { return KeywordType_; }
  void setBody(Node *Body) { BodyNode_ = Body; }
  void print(std::ostream &os) const override {
    os << getKeywordName() << " " << Node::getTypeName(getKind());
  }

public:
  static bool isa(const Node *N) {
    return N->getKind() == Node::NKind::ND_KEYROWD;
  }

private:
  Node *BodyNode_;
  c_syntax::CKType KeywordType_;
  std::string_view KeywordName_;
};

/*
 * @brief
 *        if (cond) {
 *          // BodyNode
 *        } else {
 *          // ElseNode
 *        }
 *
 *   This if-statement source code will generate following code:
 *      cond code
 *      beqz cond else  //  if condition is false, then goto else block.
 *      body code
 *      jump to end
 *   .L.else
 *      Else code
 *   .L.end
 */
class IfCondNode : public KeywordNode {
public:
  explicit IfCondNode(c_syntax::CKType KeywordType, Node *Cond, Node *Body,
                      Node *ElseN)
      : KeywordNode(KeywordType, Body), CondNode_(Cond), ElseNode_(ElseN) {}
  [[nodiscard]] Node *getCond() const { return CondNode_; }
  [[nodiscard]] Node *getElse() const { return ElseNode_; }

  void setCond(Node *CondNode) { CondNode_ = CondNode; }
  void setElseN(Node *ElseNode) { ElseNode_ = ElseNode; }

public:
  static bool isa(const Node *N) {
    if (KeywordNode::isa(N))
      return false;

    switch (dynamic_cast<const KeywordNode *>(N)->getKeywordType()) {
    case c_syntax::CKType::CK_IF:
    case c_syntax::CKType::CK_ELSE:
      return true;
    default:
      break;
    }
    return false;
  }

  /* there may be has many if else */
  static unsigned getCount() {
    static unsigned Count = 1;
    return Count++;
  }

private:
  Node *CondNode_;
  Node *ElseNode_;
};

class ForLoopNode : public KeywordNode {
public:
  // explicit ForLoopNode(c_syntax::CKType KeywordType, Node *Cond, Node *Init,
  // Node *Body, Node *Inc)
  explicit ForLoopNode(c_syntax::CKType KeywordType, Node *Latch, Node *Header,
                       Node *Body, Node *Exiting)
      : KeywordNode(KeywordType, Body), Latch_(Latch), Header_(Header),
        Exiting_(Exiting) {}

  [[nodiscard]] Node *getLatch() const { return Latch_; }
  [[nodiscard]] Node *getHeader() const { return Header_; }
  [[nodiscard]] Node *getExiting() const { return Exiting_; }

public:
  static unsigned getCount() {
    static unsigned Count = 1;
    return Count++;
  }

private:
  Node *Latch_;
  Node *Header_;
  Node *Exiting_;
};

class VarObj; // old variable info class.

class VariableNode : public Node {
public:
  /*
   * @brief 由于变量需要在 stack
   * 上面申请空间，而且变量的顺序也需要固定，方便后面去获取，
   *        因此也需要使用链表将变量关联在一起.
   *        TODO: 是否可以将变量之间的链表关系放在外面，通过链表数据结构来保证?
   */
  struct VarInfo {
    VarInfo *Next = nullptr;
    std::string_view Name;
    unsigned FrameIdx = 0;
  };

public:
  [[deprecated]] VariableNode(VarInfo *VI)
      : Node(Node::NKind::ND_VAR, Node::getTypeName(Node::NKind::ND_VAR)),
        Obj_(VI) {}

  explicit VariableNode(VarObj *ObjOld)
      : Node(Node::NKind::ND_VAR, Node::getTypeName(Node::NKind::ND_VAR)),
        Old_Obj_(ObjOld) {}

  [[nodiscard]] VarObj &getObj() const { return *Old_Obj_; }
  void print(std::ostream &os) const override {
    os << Node::getTypeName(getKind());
  }

public:
  static bool isa(const Node *N) { return N->getKind() == Node::NKind::ND_VAR; }

private:
  VarInfo *Obj_ = nullptr;
  VarObj *Old_Obj_ = nullptr;
};

template <typename ET> bool isa(const Node *N) { return ET::isa(N); }

#endif