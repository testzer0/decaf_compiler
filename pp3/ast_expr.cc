/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include <typeinfo>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "errors.h"
#include <iostream>
using namespace std;


IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
    Expr::type = Type::intType;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
    Expr::type = Type::doubleType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
    Expr::type = Type::boolType;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
    Expr::type = Type::stringType;
}

NullConstant::NullConstant(yyltype loc)
    : Expr(loc) {
    Expr::type = Type::nullType;
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

void ArithmeticExpr::CheckStatements(){
    /**********************************
     * Checks if the arithmetic expr  *
     * is valid.                      *
     **********************************/
    const char* lt = NULL, *rt = NULL;
    if(this->left){
        //Check if left operand is well formed
        this->left->CheckStatements();
        lt = this->left->GetTypeName();
    }
    //Check if right operand is well formed
    this->right->CheckStatements();
    rt = this->right->GetTypeName();
    if(lt && rt){
        //Binary expr
        //For arithmetic ops, both lhs and rhs must be
        //either int or double, and both must be the same
        //type.
        if((strcmp(lt, "int") && strcmp(lt, "double")) ||
           (strcmp(rt, "int") && strcmp(rt, "double")) ||
           (strcmp(lt,rt)))
            ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
    }
    else if(rt){
        //Unary expr
        //It must be either int or double
        if(strcmp(rt, "int") && strcmp(rt, "double"))
            ReportError::IncompatibleOperand(this->op, new Type(rt));
    }
}  

void RelationalExpr::CheckStatements(){
    /********************************
     * Checks if the statements in  *
     * the relational expr are well *
     * formed.                      *
     ********************************/
    this->left->CheckStatements();
    const char* lt = this->left->GetTypeName();

    this->right->CheckStatements();
    const char* rt = this->right->GetTypeName();
    if(lt && rt){
        //Relational exprs are always binary
        //Both ops must be wither int or double
        //and both must be the same type
        if((strcmp(lt, "int") && strcmp(lt, "double")) ||
           (strcmp(rt, "int") && strcmp(rt, "double")) ||
           (strcmp(lt, rt)))
            ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
    }
}

void EqualityExpr::CheckStatements(){
    /*******************************
     * Checks if the equality expr *
     * is well formed.             *
     *******************************/
    this->left->CheckStatements();
    this->right->CheckStatements();
    //Check if both operands are well formed
    const char* lt = this->left->GetTypeName();
    const char* rt = this->right->GetTypeName();
    if(lt && rt){
        //Binary equality expr
        Decl* ldecl = Program::sym_table->Lookup(lt);
        Decl* rdecl = Program::sym_table->Lookup(rt);
    
        if(ldecl != NULL && rdecl != NULL){
            if(!strcmp(lt, rt))
                return;
            else if(typeid(*ldecl) == typeid(ClassDecl)){
                ClassDecl* lcldecl = dynamic_cast<ClassDecl*>(ldecl);
                //Check if the right op
                //can be cast into a compatible
                //class.
                if(lcldecl->IsCompatibleWith(rdecl))
                    return;
            }
            else if(typeid(*rdecl) == typeid(ClassDecl)){
                ClassDecl* rcldecl = dynamic_cast<ClassDecl*>(rdecl);
                if(rcldecl->IsCompatibleWith(ldecl))
                    return;
            }
        }
        else if(ldecl && !strcmp(rt, "null"))
            return;
        else if(!strcmp(lt, rt))
            return;
    }
    ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
}
 
void LogicalExpr::CheckStatements(){
    /*********************************
     * Checks if the logical expr is *
     * well formed.                  *
     *********************************/
    const char* lt = NULL, *rt = NULL;
    if(this->left){
        //Check if left op is well formed
        this->left->CheckStatements();
        lt = this->left->GetTypeName();
    }
    //Check if right op is well formed
    this->right->CheckStatements();
    rt = this->right->GetTypeName();
    if(lt && rt){
        //Both must be bool in a logical stmt. [binary]
        if(strcmp(lt, "bool") || strcmp(rt, "bool"))
            ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
    }
    else if(rt){
        //In a unary logical stmt, op must be a bool.
        if(strcmp(rt, "bool"))
            ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
    }
} 

void AssignExpr::CheckStatements(){
    /*********************************
     * Checks if the assignment expr *
     * is well formed.               *
     *********************************/
    this->left->CheckStatements();
    this->right->CheckStatements();
    const char* lt = this->left->GetTypeName();
    const char* rt = this->right->GetTypeName();

    if(lt && rt){
        Decl* ldecl = Program::sym_table->Lookup(lt);
        Decl* rdecl = Program::sym_table->Lookup(rt);
        
        if(ldecl && rdecl){
            //Both are objects
            if(strcmp(lt, rt) == 0) //both are of same type
                return;
            else if(typeid(*rdecl) == typeid(ClassDecl)){
                ClassDecl* rcldecl = dynamic_cast<ClassDecl*>(rdecl);
                if(rcldecl->IsCompatibleWith(ldecl)) //r is compatible with l.
                    return;
            }
        }
        else if(ldecl == NULL && rdecl == NULL)
          return;
        else if(ldecl && !strcmp(rt, "null"))
            return;
        else if(!strcmp(lt, rt))
            return;
        ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
    }
}

void This::CheckStatements(){
    Node* parent = this->GetParent();
    while(parent){ //walk up the ast tree
        if(typeid(*parent) == typeid(ClassDecl)){
            //If we are any descendant of a class then "this" is valid.
            this->type = new NamedType(dynamic_cast<ClassDecl*>(parent)->GetID());
            return;
        }
        parent = parent->GetParent();
    }
    //We reach here only when at any level we arent in a class.
    ReportError::ThisOutsideClassScope(this);
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}
    
Type* ArrayAccess::GetType(){
    Type* type = base->GetType();
    if(type)
        return type->GetElemType();
    else 
        return NULL;
}

const char* ArrayAccess::GetTypeName(){
    Type* type = this->base->GetType();
    if(type)
        return type->GetElemType()->GetTypeName();
    else
        return NULL;
}

void ArrayAccess::CheckStatements(){
    /***************************
     * Checks if array access  *
     * is legal.               *
     ***************************/
    this->base->CheckStatements(); //Ensure base is well formed
    if(typeid(*this->base->GetType()) != typeid(ArrayType)) //Base must be of type ArrayType
        ReportError::BracketsOnNonArray(this->base);
    this->subscript->CheckStatements(); //Ensure subscript is well formed
    if(strcmp(this->subscript->GetTypeName(), "int")) //Subscript must be an int
        ReportError::SubscriptNotInteger(this->subscript);
}

FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}

void FieldAccess::CheckStatements(){
    /**********************************
     * Checks if the field access is  *
     * valid.                         *
     **********************************/
    Decl* decl = NULL;
    if(this->base){
        //Scoped field access
        this->base->CheckStatements();  //Check that base is well formed
        const char* name = this->base->GetTypeName();

        if(name){
            Node* parent = this->GetParent();
            Decl* cldecl = NULL;
            while(parent){ //recursively walk up the AST
                Hashtable<Decl*> *sym_table = parent->GetSymTable();
                if(sym_table){
                    if((cldecl = sym_table->Lookup(name)) != NULL){
                        decl = this->field->CheckIdDecl(cldecl->GetSymTable(), this->field->GetName());
                        //Verify that decl is declared in this scope and is a variable.
                        if((decl == NULL) || (typeid(*decl) != typeid(VarDecl))) 
                            ReportError::FieldNotFoundInBase(this->field, new Type(name));
                    }
                }
                parent = parent->GetParent();
            }
            if(cldecl == NULL){
                //Class not found, now find out which error
                if((cldecl = Program::sym_table->Lookup(name)) != NULL){
                    decl = this->field->CheckIdDecl(cldecl->GetSymTable(), this->field->GetName());
                    //If there is such a field then it must be out of scope
                    //Else the field is not found.
                    if((decl != NULL) && typeid(*decl) == typeid(VarDecl))
                        ReportError::InaccessibleField(this->field, new Type(name));
                    else
                        ReportError::FieldNotFoundInBase(this->field, new Type(name));
                }
                else
                    ReportError::FieldNotFoundInBase(this->field, new Type(name));
            }
        }
    }
    else{
        decl = this->field->CheckIdDecl(); //Check that the identifier has been declared.
        if(decl == NULL || typeid(*decl) != typeid(VarDecl)){
            ReportError::IdentifierNotDeclared(this->field, LookingForVariable);
            decl = NULL;
        }
    }
    if(decl != NULL)
        this->type = decl->GetType();
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}
 
void Call::CheckArguments(FnDecl* fndecl){
    /**********************************
     * Checks that a correct or comp- *
     * atible set of argmuments is    *
     * provided to the fn call.       *
     **********************************/
    List<VarDecl*> *formals = fndecl->GetFormals();
    int formals_num = formals->NumElements(); //Number of formals
    int args_num = this->actuals->NumElements(); //Number of actuals
    if(formals_num != args_num){
        //Either fewer or extra args passed
        ReportError::NumArgsMismatch(this->field, formals_num, args_num);
        return;
    }
    else{
        for(int i = 0; i < formals_num; i++){
            VarDecl* vardecl = formals->Nth(i);
            const char* expected = vardecl->GetTypeName();
            Expr* expr = this->actuals->Nth(i);
            const char* given = expr->GetTypeName();

            if(given && expected){
                Decl* gdecl = Program::sym_table->Lookup(given);
                Decl* edecl = Program::sym_table->Lookup(expected);

                if(gdecl && edecl){
                    //both are objects
                    if(strcmp(given, expected)) //if both arent of same type
                        if(typeid(*gdecl) == typeid(ClassDecl)){
                            //Check if the class is compatible with the formal's type
                            ClassDecl* gcldecl = dynamic_cast<ClassDecl*>(gdecl);
                            if(!gcldecl->IsCompatibleWith(edecl))
                                ReportError::ArgMismatch(expr, (i+1), new Type(given), new Type(expected));
                        }
                }
                else if(edecl && strcmp(given, "null")) //Expected arg and not given the right arg.
                    ReportError::ArgMismatch(expr, (i+1), new Type(given), new Type(expected));
                else if(gdecl == NULL && edecl == NULL && strcmp(given, expected))
                    ReportError::ArgMismatch(expr, (i+1), new Type(given), new Type(expected));
            }
        }
    }
}

void Call::CheckStatements(){
    /**********************************
     * Checks if the stmts of the call*
     * are okay.                      *
     **********************************/
    if(this->actuals){
        //Check that each of the actuals is well formed.
        for(int i = 0; i < actuals->NumElements(); i++)
            this->actuals->Nth(i)->CheckStatements();
    }
    Decl* decl = NULL;
    if(this->base){
        //If it is a member function of a scope(nonglobal)
        this->base->CheckStatements(); //Check that the base is well formed
        const char* name = this->base->GetTypeName();
        if(name){
            if((decl = Program::sym_table->Lookup(name)) != NULL){
                //There exists the type if the base
                decl = this->field->CheckIdDecl(decl->GetSymTable(), this->field->GetName());
                if((decl == NULL) || (typeid(*decl) != typeid(FnDecl))) //No such func in base
                    ReportError::FieldNotFoundInBase(this->field, new Type(name));
                else
                    CheckArguments(dynamic_cast<FnDecl*>(decl)); //Check that the args are well formed
            }
            else if((typeid(*this->base->GetType()) == typeid(ArrayType))  //.length is a valid field
                && !strcmp(this->field->GetName(), "length")){
                this->type = Type::intType;
            }
            else{ //No such field in base
                ReportError::FieldNotFoundInBase(this->field, new Type(name));
            }
        }
    }
    else{
        //Global func.
        decl = this->field->CheckIdDecl();
        if((decl == NULL) || (typeid(*decl) != typeid(FnDecl))){ //No such function declared
            ReportError::IdentifierNotDeclared(this->field, LookingForFunction);
            decl = NULL;
        }
        else
            CheckArguments(dynamic_cast<FnDecl*>(decl)); //Check that args are well formed.
    }
    if(decl != NULL)
        this->type = decl->GetType();
}

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

void NewExpr::CheckStatements(){
    /******************************
     * Checks that the statement  *
     * is well formed.            *
     ******************************/
    if(this->cType){
        const char* name = this->cType->GetTypeName();
        if(name){
            Decl* decl = Program::sym_table->Lookup(name);
            if((decl == NULL) || (typeid(*decl) != typeid(ClassDecl))) //type not declared
                ReportError::IdentifierNotDeclared(new Identifier(*this->cType->GetLocation(), name), LookingForClass);
        }
    }
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
    if(this->elemType){
        string delim = "[]";
        string str = this->elemType->GetTypeName() + delim;
        type = new Type(str.c_str());
    }
}

const char* NewArrayExpr::GetTypeName(){
    /******************************
     * Returns name of the type   * 
     * of the declared array.     *
     ******************************/
    if(type)
      return type->GetTypeName();
    else return NULL;
}

void NewArrayExpr::CheckStatements(){
    //Checks if the new array stmt is well formed
    this->size->CheckStatements(); //Check that size is well formed
    if(strcmp(this->size->GetTypeName(), "int")) //size must be int
        ReportError::NewArraySizeNotInteger(this->size);
    this->elemType->CheckTypeError(); //check the elemType is a proper type
}

ReadLineExpr::ReadLineExpr(yyltype loc)
    : Expr(loc) {
    Expr::type = Type::stringType;
}
       
ReadIntegerExpr::ReadIntegerExpr(yyltype loc)
    : Expr(loc) {
    Expr::type = Type::intType;
}

PostfixExpr::PostfixExpr(yyltype loc, LValue *lv, Operator *op)
    : Expr(loc) {
    Assert(lv != NULL && op != NULL);
    (this->lvalue = lv)->SetParent(this);
    (this->optr = op)->SetParent(this);
}

void PostfixExpr::CheckStatements(){
    //Checks if postfix expr is well formed
    if(this->lvalue){
        this->lvalue->CheckStatements(); //Verify that operand is well formed
        const char* name = this->lvalue->GetTypeName();
        if(strcmp(name, "int") && strcmp(name, "double")) //Operand must be int or double for ++, --
            ReportError::IncompatibleOperand(this->optr, this->lvalue->GetType());
    }
}
