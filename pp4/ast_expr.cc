/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>

#include <iostream>
#include <typeinfo>

#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "codegen.h"
#include "errors.h"
#include "tac.h"

using namespace std;

using std::cout;
using std::endl;


bool Expr::HasBase(){
    if(this->GetBase())
        return true;
    else{
        Decl* decl = this->GetField()->CheckIdDecl();
        if(decl){
            if(decl->GetEnclosFunc(decl) == NULL && decl->GetEnclosClass(decl) != NULL)
                return true;
        }
    }
    return false;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    this->value = val;
    Expr::type = Type::intType;
}

Location* IntConstant::Emit(){
    //Emits bytecode corresponding to this intconstant
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        //If we are inside a function then
        //Update frame and generate bytecode corresponding
        //to loading a constant into the loc
        int localOffset = fndecl->UpdateFrame();
        return Program::cg->GenLoadConstant(this->value, localOffset);
    }
    return NULL;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    this->value = val;
    Expr::type = Type::doubleType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    this->value = val;
    Expr::type = Type::boolType;
}

Location* BoolConstant::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        int localOffset = fndecl->UpdateFrame();
        return Program::cg->GenLoadConstant(static_cast<int>(this->value), localOffset);
    }
    return NULL;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    this->value = strdup(val);
    Expr::type = Type::stringType;
}

Location* StringConstant::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        int localOffset = fndecl->UpdateFrame();
        return Program::cg->GenLoadConstant(this->value, localOffset);
    }
    return NULL;
}

NullConstant::NullConstant(yyltype loc)
    : Expr(loc) {
    Expr::type = Type::nullType;
}

Location* NullConstant::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    int localOffset = 0;
    if(fndecl){
        localOffset = fndecl->UpdateFrame();
        return Program::cg->GenLoadConstant(0, localOffset);
    }
    return NULL;
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(this->tokenString, tok, sizeof(this->tokenString));
}

void Operator::SetToken(const char* tok){
    Assert(tok != NULL);
    strncpy(this->tokenString, tok, sizeof(this->tokenString));
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (this->op=o)->SetParent(this);
    (this->left=l)->SetParent(this); 
    (this->right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    this->left = NULL; 
    (this->op=o)->SetParent(this);
    (this->right=r)->SetParent(this);
}

void ArithmeticExpr::CheckStatements(){
    /**********************************
     * Checks if the arithmetic expr  *
     * is valid.                      *
     **********************************/
    Type* lt = NULL, *rt = NULL;
    if(this->left){
        //Check if left operand is well formed
        this->left->CheckStatements();
        lt = this->left->GetType();
    }
    //Check if right operand is well formed
    this->right->CheckStatements();
    rt = this->right->GetType();
    if(lt && rt){
        //Binary expr
        //For arithmetic ops, both lhs and rhs must be
        //either int or double, and both must be the same
        //type.
        if((lt != Type::intType && lt != Type::doubleType) ||
           (rt != Type::intType && rt != Type::doubleType) ||
           (lt != rt))
            ReportError::IncompatibleOperands(this->op, lt, rt);
    }
    else if(rt){
        //Unary expr
        //It must be either int or double
        if(rt != Type::intType && rt != Type::doubleType)
            ReportError::IncompatibleOperand(this->op, rt);
    }
}  

Location* ArithmeticExpr::Emit(){
    if(this->right){
        FnDecl* fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            const char* token = this->GetOp()->GetToken();
            if(this->left){
                //Generate the binary operation opcode
                //for the arithmetic operation
                int localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp(token, this->left->Emit(), this->right->Emit(), localOffset);
            }
            else{
                //same thing, but in the global scope instead of
                //a local one
                int localOffset = fndecl->UpdateFrame();
                Location* zero = Program::cg->GenLoadConstant(0, localOffset);
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp(token, zero, this->right->Emit(), localOffset);
            }
        }
    }
    return NULL;
}

void RelationalExpr::CheckStatements(){
    /********************************
     * Checks if the statements in  *
     * the relational expr are well *
     * formed.                      *
     ********************************/
    this->left->CheckStatements();
    Type* lt = this->left->GetType();

    this->right->CheckStatements();
    Type* rt = this->right->GetType();
    if(lt && rt){
        //Relational exprs are always binary
        //Both ops must be wither int or double
        //and both must be the same type
        if((lt != Type::intType && lt != Type::doubleType) ||
           (rt != Type::intType && rt != Type::doubleType) ||
           (lt != rt))
            ReportError::IncompatibleOperands(this->op, lt, rt);
    }
}

Location* RelationalExpr::Emit(){
    if(this->left && this->right){
        FnDecl* fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            const char* token = this->GetOp()->GetToken();
            Location* left_loc = this->left->Emit();
            Location* right_loc = this->right->Emit();
            if(!strcmp(token, "<")){
                int localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp(token, left_loc, right_loc, localOffset);
            }
            else if(!strcmp(token, ">")){
                int localOffset = fndecl->UpdateFrame();
                Location* new_left = Program::cg->GenBinaryOp("<", right_loc, left_loc, localOffset);
            }
            else if(!strcmp(token, "<=")){
                int localOffset = fndecl->UpdateFrame();
                Location* new_left = Program::cg->GenBinaryOp("<", left_loc, right_loc, localOffset);
                localOffset = fndecl->UpdateFrame();
                Location* new_right = Program::cg->GenBinaryOp("==", left_loc, right_loc, localOffset);
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp("||", new_left, new_right, localOffset);
            }
            else if(!strcmp(token, ">=")){
                int localOffset = fndecl->UpdateFrame();
                Location* new_left = Program::cg->GenBinaryOp("<", right_loc, left_loc, localOffset);
                localOffset = fndecl->UpdateFrame();
                Location* new_right = Program::cg->GenBinaryOp("==", left_loc, right_loc, localOffset);
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp("||", new_left, new_right, localOffset);
            }
        }
    }

    return NULL;
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

Location* EqualityExpr::Emit(){
    if(this->left && this->right){
        int localOffset = 0;
        FnDecl* fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            if(this->left->GetType() == Type::stringType && this->right->GetType() == Type::stringType){
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBuiltInCall(StringEqual, this->left->Emit(), this->right->Emit(), localOffset);
            }
            else{
                const char* token = this->GetOp()->GetToken();
                Location* left_loc = this->left->Emit();
                Location* right_loc = this->right->Emit();
                if(!strcmp(token, "!=")){
                    int localOffset = fndecl->UpdateFrame();
                    Location* less = Program::cg->GenBinaryOp("<", left_loc, right_loc, localOffset);

                    localOffset = fndecl->UpdateFrame();
                    Location* greater = Program::cg->GenBinaryOp("<", right_loc, left_loc, localOffset);

                    localOffset = fndecl->UpdateFrame();
                    return Program::cg->GenBinaryOp("||", less, greater, localOffset);
                }
                else{
                    int localOffset = fndecl->UpdateFrame();
                    return Program::cg->GenBinaryOp(token, left_loc, right_loc, localOffset);
                }
            }
        }
    }
    return NULL;
}

void LogicalExpr::CheckStatements(){
    /*********************************
     * Checks if the logical expr is *
     * well formed.                  *
     *********************************/
    Type* lt = NULL, *rt = NULL;
    if(this->left){
        //Check if left op is well formed
        this->left->CheckStatements();
        lt = this->left->GetType();
    }
    //Check if right op is well formed
    this->right->CheckStatements();
    rt = this->right->GetType();
    if(lt && rt){
        //Both must be bool in a logical stmt. [binary]
        if((lt != Type::boolType) || (rt != Type::boolType))
            ReportError::IncompatibleOperands(this->op, lt, rt);
    }
    else if(rt){
        //In a unary logical stmt, op must be a bool.
        if(rt != Type::boolType)
            ReportError::IncompatibleOperand(this->op, rt);
    }
} 

Location* LogicalExpr::Emit(){
    if(this->right){
        FnDecl* fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            //local
            if(this->left){
                //binary
                int localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp(this->GetOp()->GetToken(), this->left->Emit(), this->right->Emit(), localOffset);
            }
            else{
                //unary => NOT
                int localOffset = fndecl->UpdateFrame();
                Location* one = Program::cg->GenLoadConstant(1, localOffset);
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp("-", one, this->right->Emit(), localOffset);
            }
        }
    }
    return NULL;
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

Location* AssignExpr::Emit(){
    int localOffset = 0;
    if(this->left && this->right){
        if(typeid(*this->right) == typeid(ReadLineExpr)){

        }
        Location* right_loc = this->right->Emit();
        if(this->left->HasBase()){
            //If this is a member variable of an object
            //then gotta store the value in the object
            //using a STORE instruction
            Location* left_loc = this->left->StoreEmit();
            Program::cg->GenStore(left_loc, right_loc);
            return left_loc;
        }
        else{
            //Otherwise simple assignment will do
            Location* left_loc = this->left->Emit();
            Program::cg->GenAssign(left_loc, right_loc);
            return left_loc;
        }
    }
    return NULL;
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

Location* This::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        //0-th (first) formal of a function is itself
        //GetMemLoc returns its location in memory
        return fndecl->GetFormals()->Nth(0)->GetID()->GetMemLoc();
    }
    return NULL;
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (this->base=b)->SetParent(this); 
    (this->subscript=s)->SetParent(this);
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

Location* ArrayAccess::Emit(){
    Location* address = this->StoreEmit();
    int localOffset;
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        //Exec allowed only inside a function
        //Generate a LOAD instruction to load
        //value at specified index into address
        localOffset = fndecl->UpdateFrame();
        return Program::cg->GenLoad(address, localOffset);
    }
    return NULL;
}

Location* ArrayAccess::StoreEmit(){
    /******************************************
     * Emits bytecode to perform checking on  *
     * array indices and returns address of   *
     * specified index                        *
     ******************************************/
    if(this->base && this->subscript){
        //Unless we have both a base and a subscript
        //Its not an array access
        int localOffset = 0;
        FnDecl* fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            //local
            Location* base_loc = this->base->Emit();
            localOffset = fndecl->UpdateFrame();
            Location* size = Program::cg->GenLoad(base_loc, localOffset);
            //Emit bytecode that LOADs address of base address into base_loc

            localOffset = fndecl->UpdateFrame();

            Location* subs = this->subscript->Emit();
            Location* test_max = Program::cg->GenBinaryOp("<", subs, size, localOffset);
            //check that index is < number of elements in array
            localOffset = fndecl->UpdateFrame();
            Location* minus = Program::cg->GenLoadConstant(-1, localOffset);
            //Generate bytecode to LOAD the constant -1 into minus

            localOffset = fndecl->UpdateFrame();
            Location* test_min = Program::cg->GenBinaryOp("<", minus, subs, localOffset);
            //Array index must be >= 0

            //Labels to be used for goto statements
            char* label_0 = Program::cg->NewLabel();
            char* label_1 = Program::cg->NewLabel();

            localOffset = fndecl->UpdateFrame();
            Location* test = Program::cg->GenBinaryOp("&&", test_min, test_max, localOffset);
            //Bytecode to check legality of access
            
            Program::cg->GenIfZ(test, label_0);
            //If illegal access goto label_0

            localOffset = fndecl->UpdateFrame();
            Location* varsize_loc = Program::cg->GenLoadConstant(CodeGenerator::VarSize, localOffset);
            localOffset = fndecl->UpdateFrame();
            Location* tmp = Program::cg->GenBinaryOp("*", subs, varsize_loc, localOffset);
            localOffset = fndecl->UpdateFrame();
            Location* offset_loc = Program::cg->GenBinaryOp("+", tmp, varsize_loc, localOffset);
            Location* address = Program::cg->GenBinaryOp("+", base_loc, offset_loc, localOffset);
            //Basically, address =  base_address + sizeof(variable)*index.

            Program::cg->GenGoto(label_1);
            //Since access was legal we skip ahead to label_1

            Program::cg->GenLabel(label_0);
            //label_0 responsible for printing error message
            const char* error_msg = "Decaf runtime error: Array script out of bounds";
            Program::PrintError(error_msg, fndecl);

            Program::cg->GenLabel(label_1);

            return address;            
        }
    }

    return NULL;
}

FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    this->base = b; 
    if (this->base) this->base->SetParent(this); 
    (this->field=f)->SetParent(this);
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

Location* FieldAccess::Emit(){
    //Emit bytecode for field access
    FnDecl* fndecl = this->GetEnclosFunc(this);
    int localOffset = 0;
    if(fndecl){
        if(this->base){
            Location* base_loc = this->base->Emit();
            ClassDecl* classdecl = NULL;
            if(typeid(*this->base) == typeid(This)) //If we were called via "this", get the class we're in
                classdecl = this->GetEnclosClass(this);
            else{
                Decl* decl = Program::sym_table->Lookup(this->base->GetTypeName());
                if(decl && typeid(*decl) == typeid(ClassDecl)) //check that base is a class
                    classdecl = dynamic_cast<ClassDecl*>(decl);
            }
            if(classdecl)
                return this->LoadField(base_loc, classdecl, fndecl); //emit the load field bytecode
        }
        else{
            Decl* decl = this->field->CheckIdDecl();
            if(decl){
                ClassDecl* classdecl = decl->GetEnclosClass(decl);
                if((decl->GetEnclosFunc(decl) != NULL) ||
                    classdecl == NULL)
                    return decl->GetID()->GetMemLoc();
                else{
                    Location* base_loc = fndecl->GetFormals()->Nth(0)->GetID()->GetMemLoc();
                    return this->LoadField(base_loc, classdecl, fndecl);
                }
            }
        }
    }
    return NULL;
}

Location* FieldAccess::StoreEmit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    int localOffset = 0;

    if(fndecl){
        if(this->base){
            Location* base_loc = this->base->Emit();
            ClassDecl* classdecl = NULL;
            if(typeid(*this->base) == typeid(This))
                classdecl = this->GetEnclosClass(this);
            else{
                Decl* decl = Program::sym_table->Lookup(this->base->GetTypeName());
                if(decl && typeid(*decl) == typeid(ClassDecl))
                    classdecl = dynamic_cast<ClassDecl*>(decl);
            }
            if(classdecl)
                return this->StoreField(base_loc, classdecl, fndecl);
        }
        else{
            Decl* decl = this->field->CheckIdDecl();
            //must be a standard field line .length() etc
            if(decl){
                ClassDecl* classdecl = decl->GetEnclosClass(decl);
                if(decl->GetEnclosFunc(decl) != NULL ||
                    classdecl == NULL)
                    return decl->GetID()->GetMemLoc(); //For .length() etc
                else{
                    Location* base_loc = fndecl->GetFormals()->Nth(0)->GetID()->GetMemLoc();
                    return this->StoreField(base_loc, classdecl, fndecl);
                }
            }
        }
    }
    return NULL;
}

Location* FieldAccess::LoadField(Location* base_loc, ClassDecl* classdecl, FnDecl* fndecl){
    if(classdecl){
        int localOffset = 0;
        List<const char*> *fieldlabels = classdecl->GetFieldLabels();
        for(int i = 0; i < fieldlabels->NumElements(); i++){
            if(!strcmp(fieldlabels->Nth(i), this->field->GetName())){
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenLoad(base_loc, localOffset, (i+1)*CodeGenerator::VarSize);
            }
        }
    }
    return NULL;
}

Location* FieldAccess::StoreField(Location* base_loc, ClassDecl* classdecl, FnDecl* fndecl){
    if(classdecl){
        int localOffset = 0;
        List<const char*> *fieldlabels = classdecl->GetFieldLabels();
        for(int i = 0; i < fieldlabels->NumElements(); i++){
            if(!strcmp(fieldlabels->Nth(i), this->field->GetName())){
                localOffset = fndecl->UpdateFrame();
                Location* offset_loc = Program::cg->GenLoadConstant((i+1)*CodeGenerator::VarSize, localOffset);
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenBinaryOp("+", base_loc, offset_loc, localOffset);
            }
        }
    }
    return NULL;
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    this->base = b;
    if (this->base) base->SetParent(this);
    (this->field=f)->SetParent(this);
    (this->actuals=a)->SetParentAll(this);
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

Location* Call::Emit(){
    //Emits bytecode for a call
    Location* rtvalue = NULL;
    int localOffset = 0;
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        if(this->base){
            if(typeid(*this->base->GetType()) == typeid(ArrayType) && !strcmp(this->field->GetName(), "length")){
                //array.length()
                localOffset = fndecl->UpdateFrame();
                return Program::cg->GenLoad(this->base->Emit(), localOffset);
            }
            Location* base_loc = this->base->Emit();
            Decl* decl = NULL;

            const char* classname = this->base->GetTypeName();
            if((decl = Program::sym_table->Lookup(classname)) != NULL)
                rtvalue = RuntimeCall(base_loc, dynamic_cast<ClassDecl*>(decl), fndecl);

            return rtvalue;
        }
        else{
            Decl* decl = this->field->CheckIdDecl();
            if(decl && typeid(*decl) == typeid(FnDecl)){
                ClassDecl* classdecl = decl->GetEnclosClass(decl);
                if(classdecl){
                    Location* base_loc = fndecl->GetFormals()->Nth(0)->GetID()->GetMemLoc();
                    rtvalue = RuntimeCall(base_loc, this->GetEnclosClass(this), fndecl);
                }
                else{
                    int args_num = PushArguments(this->actuals);
                    FnDecl* call = dynamic_cast<FnDecl*>(decl);
                    if(decl->GetType() == Type::voidType){
                        rtvalue = Program::cg->GenLCall(call->GetLabel(), false);
                        //generate an LCALL opcode
                    }
                    else{
                        localOffset = fndecl->UpdateFrame();
                        rtvalue = Program::cg->GenLCall(call->GetLabel(), true, localOffset);
                    }
                    Program::cg->GenPopParams(args_num * CodeGenerator::VarSize);
                }
                return rtvalue;
            }
        }
    }
    return NULL;
}

int Call::PushArguments(List<Expr*> *args){
    int args_num = 0;
    if(args){
        args_num = args->NumElements();
        for(int i = args_num - 1; i >= 0; i--){
            //Push from right to left
            Expr* arg = args->Nth(i);
            Program::cg->GenPushParam(arg->Emit());
            //emit the push bytecode
        }
    }
    return args_num;
}

Location* Call::RuntimeCall(Location* base_loc, ClassDecl* classdecl, FnDecl* fndecl){
    Location* func = NULL, *rtvalue = NULL;
    int localOffset;
    localOffset = fndecl->UpdateFrame();
    Location* vtable = Program::cg->GenLoad(base_loc, localOffset);
    //generate a LOAD instruction to load virtual function table

    List<const char*> *methodlabels = classdecl->GetMethodLabels();
    for(int i = 0; i < methodlabels->NumElements(); i++){
        const char* methodname = strchr(methodlabels->Nth(i), '.') + 1;
        if(!strcmp(methodname, this->field->GetName())){
            localOffset = fndecl->UpdateFrame();
            func = Program::cg->GenLoad(vtable, localOffset, i * CodeGenerator::VarSize);
            //If we found which function was called, then load its address into func
            break;
        }
    }

    int args_num = PushArguments(this->actuals);
    Program::cg->GenPushParam(base_loc);//First arg is always "this"

    if(func){
        if(this->GetType() == Type::voidType)
            rtvalue = Program::cg->GenACall(func, false);
        else{
            localOffset = fndecl->UpdateFrame();
            //If it has a return type then gotta update frame
            rtvalue = Program::cg->GenACall(func, true, localOffset);
        }
    }
    Program::cg->GenPopParams((args_num + 1)*CodeGenerator::VarSize);
    //Pop params
    return rtvalue;
}

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (this->cType=c)->SetParent(this);
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

Location* NewExpr::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    int localOffset = 0;

    if(fndecl){
        const char* name = this->cType->GetTypeName();
        Decl* decl = Program::sym_table->Lookup(name);
        if(decl && typeid(*decl) == typeid(ClassDecl)){
            ClassDecl* classdecl = dynamic_cast<ClassDecl*>(decl);
            List<const char*> *fieldlabels = classdecl->GetFieldLabels();
            int field_num = fieldlabels->NumElements();
            localOffset = fndecl->UpdateFrame();
            Location* var_size = Program::cg->GenLoadConstant(CodeGenerator::VarSize * (field_num + 1), localOffset);
            localOffset = fndecl->UpdateFrame();
            Location* dst = Program::cg->GenBuiltInCall(Alloc, var_size, NULL, localOffset);
            localOffset = fndecl->UpdateFrame();
            Location* src = Program::cg->GenLoadLabel(this->cType->GetTypeName(), localOffset);
            Program::cg->GenStore(dst, src);
            return dst;
        }
    }
    return NULL;
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (this->size=sz)->SetParent(this); 
    (this->elemType=et)->SetParent(this);
}

Location* NewArrayExpr::Emit(){
    if(this->size && this->elemType){
        FnDecl* fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            char* label_0 = Program::cg->NewLabel();
            Location* size_loc = this->size->Emit();

            int localOffset = fndecl->UpdateFrame();
            Location* zero = Program::cg->GenLoadConstant(0, localOffset);
            //load 0 into zero

            localOffset = fndecl->UpdateFrame();
            Location* less = Program::cg->GenBinaryOp("<", size_loc, zero, localOffset);
            //is size < 0?

            localOffset = fndecl->UpdateFrame();
            Location* equal = Program::cg->GenBinaryOp("==", size_loc, zero, localOffset);
            //is size == 0?

            localOffset = fndecl->UpdateFrame();
            Location* test = Program::cg->GenBinaryOp("||", less, equal, localOffset);
            Program::cg->GenIfZ(test, label_0);
            //If neither then acceptable

            const char* error_msg = "Decaf runtime error: Array size is <= 0";
            Program::PrintError(error_msg, fndecl);

            Program::cg->GenLabel(label_0);

            localOffset = fndecl->UpdateFrame();
            Location* var_size = Program::cg->GenLoadConstant(CodeGenerator::VarSize, localOffset);
            localOffset = fndecl->UpdateFrame();
            Location* bytes = Program::cg->GenBinaryOp("*", size_loc, var_size, localOffset);
            localOffset = fndecl->UpdateFrame();
            bytes = Program::cg->GenBinaryOp("+", bytes, var_size, localOffset);
            //numbytes to alloc = num_elements * sizeof(one element)

            localOffset = fndecl->UpdateFrame();
            Location* address = Program::cg->GenBuiltInCall(Alloc, bytes, NULL, localOffset); //call to malloc etc
            Program::cg->GenStore(address, size_loc);
            return address;
        }
    }
    return NULL;
}

const char* NewArrayExpr::GetTypeName(){
    /******************************
     * Returns name of the type   * 
     * of the declared array.     *
     ******************************/
    if (this->elemType)
    {
      string delim = "[]";
      string str = this->elemType->GetTypeName() + delim;
      return str.c_str();
    }
    else
        return NULL;
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

Location* ReadLineExpr::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        int localOffset = fndecl->UpdateFrame();
        return Program::cg->GenBuiltInCall(ReadLine, NULL, NULL, localOffset);
    }
    return NULL;
}
       
ReadIntegerExpr::ReadIntegerExpr(yyltype loc)
    : Expr(loc) {
    Expr::type = Type::intType;
}

Location* ReadIntegerExpr::Emit(){
    FnDecl* fndecl = this->GetEnclosFunc(this);
    if(fndecl){
        int localOffset = fndecl->UpdateFrame();
        return Program::cg->GenBuiltInCall(ReadInteger, NULL, NULL, localOffset);
    }
    return NULL;
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

Location* PostfixExpr::Emit(){
    if(this->lvalue){
        //Below line says (new expression) = lvalue + 1
        Expr* expr = new AssignExpr(this->lvalue, new Operator(*this->GetLocation(), "="), new ArithmeticExpr(this->lvalue, new Operator(*this->GetLocation(), "+"), new IntConstant(*this->GetLocation(), 1)));
        //Isnt the above line wrong for -- though?
        expr->SetParent(this);
        FnDecl *fndecl = this->GetEnclosFunc(this);
        if(fndecl){
            Location* value = this->lvalue->Emit();
            int localOffset = fndecl->UpdateFrame();
            Location* one = Program::cg->GenLoadConstant(1, localOffset);

            localOffset = fndecl->UpdateFrame();
            if(!strcmp(this->optr->GetToken(), "++")){
                Location* plus = Program::cg->GenBinaryOp("+", value, one, localOffset);
                Program::cg->GenAssign(value, plus);
            }
            else{
                Location* minus = Program::cg->GenBinaryOp("-", value, one, localOffset);
                Program::cg->GenAssign(value, minus);
            }
            return value;
        }
    }
    return NULL;
}
