/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement 
 * semantic processing including detection of declaration conflicts 
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "ast_type.h"
#include "hashtable.h"
#include "list.h"

class Type;
class NamedType;
class Identifier;
class Stmt;
class StmtBlock;

class Decl : public Node
{
  protected:
    Identifier *id;
  
  public:
    Decl(Identifier *name);
    Identifier* GetID(){
      return id;
    }
    virtual const char* GetTypeName(){
      return NULL;
    }
    virtual Type* GetType(){
      return NULL;
    }
    friend ostream& operator<<(ostream& out, Decl *d) { return out << d->id; }
};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    
  public:
    VarDecl(Identifier *name, Type *type);
    Type* GetType(){
      return type;
    }
    const char* GetTypeName(){
      if(type)
        return type->GetTypeName();
      else return NULL;
    }
    bool HasSameTypeSig(VarDecl* vd);
    void CheckStatements();
    void CheckDeclError();
};

class ClassDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;
    Hashtable<Decl*> *sym_table;

  public:
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);
    List<Decl*> *GetMembers(){
      return members;
    }
    NamedType* GetExtends(){
      return extends;
    }
    List<NamedType*> *GetImplements(){
      return implements;
    }
    Hashtable<Decl*> *GetSymTable(){
      return sym_table;
    }
    void CheckStatements();
    void CheckDeclError();
    bool IsCompatibleWith(Decl* decl);
};

class InterfaceDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    Hashtable<Decl*> *sym_table;
    
  public:
    InterfaceDecl(Identifier *name, List<Decl*> *members);
    void CheckDeclError();
    List<Decl*> *GetMembers(){
      return members;
    }
    Hashtable<Decl*> *GetSymTable(){
      return sym_table;
    }
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    Hashtable<Decl*> *sym_table;
    
  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(StmtBlock *b);
    void CheckStatements();
    void CheckDeclError();
    Type* GetType(){
      return returnType;
    }
    List<VarDecl*> *GetFormals(){
      return formals;
    }
    const char* GetTypeName(){
      if(returnType)
        return returnType->GetTypeName();
      else return NULL;
    }
    bool HasSameTypeSig(FnDecl* fd);
    Hashtable<Decl*> *GetSymTable(){
      return sym_table;
    }
};

#endif
