/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <typeinfo>

#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "codegen.h"
#include "errors.h"
#include "tac.h"

using std::cout;
using std::endl;
        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (this->id=n)->SetParent(this); 
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (this->type=t)->SetParent(this);
}
 
bool VarDecl::HasSameTypeSig(VarDecl* vd){
    if(this->type)
        return this->type->HasSameType(vd->GetType());
    else
        return false;
} 

void VarDecl::CheckStatements(){
    //A variable has no inherent statements so
    //nothing to do.
} 

void VarDecl::CheckDeclError(){
    if(this->type)
        this->type->CheckTypeError();
    /*Node* parent = this->GetParent(), *cur = this;
    while(parent){
      if(typeid(*parent) == typeid(WhileStmt) ||
         typeid(*parent) == typeid(ForStmt)   ||
         typeid(*parent) == typeid(IfStmt)    ||
         typeid(*parent) == typeid(ClassDecl)  ||
         typeid(*parent) == typeid(InterfaceDecl))
        break;
      cur = parent;
      parent = parent->GetParent();
    }
    if(parent == NULL)
      Program::sym_table->Enter(this->GetID()->GetName(), this);*/
}

Location* VarDecl::Emit(){
    /*******************************************
     * Sets locations for local and global     *
     * variables declared in the scope.        *
     *******************************************/
    FnDecl* fndecl = this->GetEnclosFunc(this); //Get Enclosing function
    ClassDecl* classdecl = this->GetEnclosClass(this); //Get enclosing class
    int localOffset = 0;
    const char* name = this->GetID()->GetName();

    if(fndecl){
        //If we are in a local function scope
        localOffset = fndecl->UpdateFrame();
        //Get curr location and update it
        this->id->SetMemLoc(Program::cg->GenVar(fpRelative, localOffset, name));
        //Set the retreived loc to this variable
    }
    else{
        //Global scope
        //Maybe add class too?
        this->id->SetMemLoc(Program::cg->GenVar(gpRelative, Program::offset, name));
        //gpRelative since its global.
        Program::offset = Program::offset + CodeGenerator::VarSize;
    }

    return NULL;

}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    this->extends = ex;
    if (this->extends) this->extends->SetParent(this);
    (this->implements=imp)->SetParentAll(this);
    (this->members=m)->SetParentAll(this);
    this->sym_table = new Hashtable<Decl*>;

    this->methodlabels = new List<const char*>;
    this->fieldlabels = new List<const char*>;
    this->instanceOffset = CodeGenerator::VarSize;
}

void ClassDecl::CheckStatements(){
    if(this->members){
        //If the class decl has checkable stuff
        //then check each such member.
        for(int i = 0; i < this->members->NumElements(); i++)
            this->members->Nth(i)->CheckStatements();
    }
}

void ClassDecl::CheckDeclError(){
    /**************************************
     * Checks for possible errors in the  *
     * class definition and reports them. *
     **************************************/
    this->sym_table->Enter(this->GetID()->GetName(), this);
    // First enter the class identifier into the symtable
    if(this->members){
        for(int i = 0; i < this->members->NumElements(); i++){
            Decl* cur = this->members->Nth(i);
            Decl* prev;
            const char* name = cur->GetID()->GetName();
            if(name){
                //If an entry has already been made with same if
                //then report error. Else continue.
                if((prev = this->sym_table->Lookup(name)) != NULL)
                    ReportError::DeclConflict(cur, prev);
                else 
                    this->sym_table->Enter(name, cur);
            }
        }
    }

    NamedType* ex = this->extends;
    while(ex){
        const char* classname = ex->GetID()->GetName();
        if(classname){
            Node* node = Program::sym_table->Lookup(classname);
            if(node == NULL){
                //i.e in the case: A extends B; ==> B must exist
                //if B doesn not exist then report error.
                ReportError::IdentifierNotDeclared(ex->GetID(), LookingForClass);
                break;
            }
            else if(typeid(*node) == typeid(ClassDecl)){
                //If B is a custom declared class
                ClassDecl* base = dynamic_cast<ClassDecl*>(node);
                List<Decl*> *base_members = base->members;
                List<Decl*> *inherited = new List<Decl*>;
                if(base_members){
                    for(int i = 0; i < base_members->NumElements(); i++){
                        Decl* cur = base_members->Nth(i);
                        Decl* prev;
                        const char* name = cur->GetID()->GetName(); 
                        //check if a particular object is defined in dervd class.
                        if((prev = this->sym_table->Lookup(name)) != NULL){
                            //Variables cant be overriden
                            //Overriding cannot be done if type isnt same.
                            if(typeid(*cur) == typeid(VarDecl) || typeid(*cur) != typeid(*prev))
                                ReportError::DeclConflict(prev, cur);
                            else if(typeid(*cur) == typeid(FnDecl) && typeid(*cur) == typeid(*prev)){
                                FnDecl* fdcur = dynamic_cast<FnDecl*>(cur);
                                FnDecl* fdprev = dynamic_cast<FnDecl*>(prev);
                                //If the two functions dont have same type signatures
                                if(!fdcur->HasSameTypeSig(fdprev))
                                    ReportError::OverrideMismatch(fdprev);
                            }
                        }
                        else{
                            //Its a new member. Append.
                            inherited->Append(cur);
                        }
                    }
                    for(int i = 0; i < inherited->NumElements(); i++){
                        //Enter each of the members into the symbol table.
                        Decl* decl = inherited->Nth(i);
                        this->sym_table->Enter(decl->GetID()->GetName(), decl);
                    }
                }
                ex = base->GetExtends(); // move on to next base class if present
            }
        }
    }

    if(this->implements){
        for(int i = 0; i < this->implements->NumElements(); i++){
            NamedType* implement = this->implements->Nth(i);
            Identifier* id = implement->GetID();
            if(id){
                Node* node = Program::sym_table->Lookup(id->GetName());
                if(node == NULL || (typeid(*node) != typeid(InterfaceDecl))){
                    //If the implemented object is not declared or is not
                    //an interface then report error.
                    ReportError::IdentifierNotDeclared(id, LookingForInterface);
                }
                else if(typeid(*node) == typeid(InterfaceDecl)){
                    InterfaceDecl* ifd = dynamic_cast<InterfaceDecl*> (node);
                    List<Decl*> *members = ifd->GetMembers();
                    for(int j = 0; j < members->NumElements(); j++){
                        FnDecl* cur = dynamic_cast<FnDecl*>(members->Nth(j));
                        //cycle through the functions (virtual as well as not)
                        //in the interface decl.
                        Decl* prev;
                        const char* name = cur->GetID()->GetName();
                        if((prev = this->sym_table->Lookup(name)) != NULL){
                            //If the virtual function isnt implemented as a function
                            //or if it does not have same signature,
                            if(typeid(*prev) != typeid(FnDecl))
                                ReportError::DeclConflict(cur, prev);
                            else if(!cur->HasSameTypeSig(dynamic_cast<FnDecl*>(prev)))
                                ReportError::OverrideMismatch(prev);
                        }
                        else
                            ReportError::InterfaceNotImplemented(this, implement); //Not all virtual fns implemented.
                    }
                }
            }
        }
    }

    if(this->members){
        for(int i = 0; i < this->members->NumElements(); i++){
            this->members->Nth(i)->CheckDeclError();
        }
    }
}

bool ClassDecl::IsCompatibleWith(Decl* decl){
    /***************************************
     * Checks if "this" is compatible with *
     * the object represented by decl.     *
     ***************************************/
    NamedType* extends = this->GetExtends();
    List<NamedType*> *implements = this->GetImplements();

    if(typeid(*decl) == typeid(ClassDecl)){
        //if decl is a class
        ClassDecl *cldecl = dynamic_cast<ClassDecl*>(decl);
        if(extends){
            const char* name = extends->GetTypeName();
            if(!strcmp(cldecl->GetID()->GetName(), name))
                return true;
            else{
                if(name){
                    Decl* exdecl = Program::sym_table->Lookup(name);
                    //If our class extends another class then recursively check for compatibility
                    if(exdecl && typeid(*exdecl) == typeid(ClassDecl))
                        return dynamic_cast<ClassDecl*>(exdecl)->IsCompatibleWith(decl);
                }
            }
        }
    }
    else if(typeid(*decl) == typeid(InterfaceDecl)){
        //If decl is an interface
        InterfaceDecl* itfdecl = dynamic_cast<InterfaceDecl*>(decl);
        if(implements){
            for(int i = 0; i < implements->NumElements(); i++){
                NamedType *implement = implements->Nth(i);
                //If decl is one of the implemented interfaces by this, then compatible.
                if(implement && !strcmp(itfdecl->GetID()->GetName(), implement->GetTypeName()))
                    return true;
            }
        }
        if(extends){
            //Recursively, for each extended interface check if compatible.
            const char* name = extends->GetTypeName();
            if(name){
                Decl* exdecl = Program::sym_table->Lookup(name);
                if(exdecl && typeid(*exdecl) == typeid(ClassDecl))
                    return dynamic_cast<ClassDecl*>(exdecl)->IsCompatibleWith(decl);
            }
        }
    }
    return false;
}

void ClassDecl::SetLabels(){
    /***********************************
     * Fills in methodlabels and       *
     * fieldlabels for the class decl. *
     ***********************************/
    if(this->sym_table){
        Iterator<Decl*> iter = this->sym_table->GetIterator(); //iterate over entries in sym_table
        Decl* decl;

        while((decl = iter.GetNextValue()) != NULL){
            if(typeid(*decl) == typeid(FnDecl)){
                //Every defined function needs to be assigned 
                //a method label
                ClassDecl* classdecl = decl->GetEnclosClass(decl);
                string name = Program::GetClassLabel(classdecl->GetID()->GetName(), decl->GetID()->GetName());
                //Get the associated label and append it to our list
                this->methodlabels->Append(strdup(name.c_str()));
            }
            else if(typeid(*decl) == typeid(VarDecl))
                this->fieldlabels->Append(decl->GetID()->GetName()); //Do the same for vars
        }
    }
}

Location* ClassDecl::Emit(){
    if(this->members){
        for(int i = 0; i < this->members->NumElements(); i++)
            this->members->Nth(i)->SetLabels();

        for(int i = 0; i < this->members->NumElements(); i++)
            this->members->Nth(i)->Emit();
    }
    Program::cg->GenVTable(this->id->GetName(), this->methodlabels);

    return NULL;
}

int ClassDecl::UpdateInstanceOffset(){
    //Returns current offset and updates it
    //for next use.
    int offset = this->instanceOffset;
    this->instanceOffset += CodeGenerator::VarSize;
    return offset;
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
    sym_table = new Hashtable<Decl*>;
}

void InterfaceDecl::CheckDeclError(){
    /**************************************
     * Checks for possible errors in the  *
     * interface definition and           *
     * reports them.                      *
     **************************************/
    if(this->members){
        for(int i = 0; i < this->members->NumElements(); i++){
            Decl* cur = members->Nth(i);
            Decl* prev;
            const char* name = cur->GetID()->GetName();
            if(name){
                //Ensure that there are no conflicting declarations
                if((prev = this->sym_table->Lookup(name)) != NULL)
                    ReportError::DeclConflict(cur, prev);
                else
                    sym_table->Enter(name, cur);
            }
        }
    }
}
	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (this->returnType=r)->SetParent(this);
    (this->formals=d)->SetParentAll(this);
    this->body = NULL;
    this->sym_table = new Hashtable<Decl*>;

    this->beginFunc = NULL;
    this->frameSize = 0;
    this->localOffset = CodeGenerator::OffsetToFirstLocal;
    this->paramOffset = CodeGenerator::OffsetToFirstParam;
}

bool FnDecl::HasSameTypeSig(FnDecl* fd){
    /********************************
     * Checks if "this" and fd      *
     * have the same type signature *
     * i.e. they are compatible.    *
     ********************************/
    if(!strcmp(this->id->GetName(), fd->GetID()->GetName()))
        if(this->returnType->HasSameType(fd->GetType())){
            //Return value must be of same type.
            List<VarDecl*> *f1 = formals;
            List<VarDecl*> *f2 = fd->GetFormals();

            if(f1 && f2){
                if(f1->NumElements() == f2->NumElements()){
                    for(int i = 0; i < f1->NumElements(); i++){
                        //Each formal must be compatible
                        //IN ORDER.
                        VarDecl* vd1 = f1->Nth(i);
                        VarDecl* vd2 = f2->Nth(i);
                        if(!vd1->HasSameTypeSig(vd2))
                            return false;
                    }
                    return true;
                }
            }
        }

    return false; //If the functions have the same name return false
}

void FnDecl::CheckStatements(){
    //Checks body statements for errors.
    if(this->body)
        this->body->CheckStatements();
}

void FnDecl::CheckDeclError(){
    /************************************
     * Checks for errors in declaration *
     * of the function.                 *
     ************************************/
    if(this->formals){
        //If there are formals
        for(int i = 0; i < this->formals->NumElements(); i++){
            VarDecl* cur = this->formals->Nth(i);
            Decl* prev;
            const char* name = cur->GetID()->GetName();
            if(name){
                //We cant have repeated formals
                if((prev = this->sym_table->Lookup(name)) != NULL){
                    ReportError::DeclConflict(cur, prev);
                }
                else{
                    //Enter the symbol in table and check that its type is
                    //valid, etc.
                    sym_table->Enter(name, cur);
                    cur->CheckDeclError();
                }
            }
        }
    }
    //If there is body, check that the decls in the body are OK.
    if(this->body)
        this->body->CheckDeclError();
}

Location* FnDecl::Emit(){
    /************************************
     * Emits bytecode for this function.*
     ************************************/
    const char* name = this->GetID()->GetName();
    ClassDecl* classdecl = this->GetEnclosClass(this); //Get enclosing class

    if(!strcmp(name, "main") && classdecl == NULL) //We are in the main function
        Program::cg->GenLabel("main");
    else
        Program::cg->GenLabel((this->label).c_str());

    this->beginFunc = Program::cg->GenBeginFunc(); 
    //Emit bytecode representing "Begin Func of this label"

    if(this->formals){
        List<VarDecl*> *newformals = new List<VarDecl*>;
        if(classdecl){
            VarDecl* this_var = new VarDecl(new Identifier(*this->GetLocation(), "this"), Type::nullType);
            //Add a variable corresponding to the "this" ptr, i.e. to the enclosing class
            this_var->GetID()->SetMemLoc(Program::cg->GenVar(fpRelative, this->paramOffset, "this"));
            this->paramOffset += CodeGenerator::VarSize;
            newformals->Append(this_var);
        }
        for(int i = 0; i < this->formals->NumElements(); i++){
            VarDecl* vardecl = this->formals->Nth(i);
            //For each formal add an entry to the vardecls
            vardecl->GetID()->SetMemLoc(Program::cg->GenVar(fpRelative, this->paramOffset, vardecl->GetID()->GetName()));
            this->paramOffset += CodeGenerator::VarSize;
            newformals->Append(vardecl);
        }
        if(classdecl){
            this->formals = newformals; //replace formals by the newer version
            this->formals->SetParentAll(this);
        }
        else
            delete newformals; //If we arent enclosed by any class then dont need
    }
    if(this->body)
        this->body->Emit(); //Emit the body

    this->beginFunc->SetFrameSize(this->GetFrameSize()); //calculate frame size in bytes and set

    Program::cg->GenEndFunc();
    //Emit bytecode representing end of function

    return NULL;
}

void FnDecl::SetFunctionBody(StmtBlock *b) { 
    (body=b)->SetParent(this);
}

int FnDecl::UpdateFrame(){
    this->frameSize +=  CodeGenerator::VarSize;
    //Increase frame size by size of variable
    int offset = this->localOffset;
    //offset to return
    this->localOffset -= CodeGenerator::VarSize;
    //Increase local offset (note that stack grows)
    //downward so '-' means increasing the offset.

    return offset;
}

void FnDecl::SetLabels(){
    //Sets the labels
    ClassDecl* classdecl = this->GetEnclosClass(this);
    const char* name = this->GetID()->GetName();
    if(strcmp(name, "main") || classdecl){
        //If we are in the "main" func or a class
        //then assign label
        string label;
        if(classdecl)
            label = Program::GetClassLabel(classdecl->GetID()->GetName(), name);
        else
            label = Program::GetFuncLabel(name);

        this->label = label;
    }
}



