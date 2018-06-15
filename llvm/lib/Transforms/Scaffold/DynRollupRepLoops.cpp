//===----------------------- DynRollupRepLoops.cpp --------------------------===///
// This file implements the Scaffold pass of traversing basic blocks, finding
// purely quantum loops, and transforming them so they get called only once.


//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "DynRollupRepLoops"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Intrinsics.h"
#include <sstream>
#include <climits>

using namespace llvm;
using namespace std;


bool debugDynRollupRepLoops = false;


STATISTIC(NumLoopsRolled, "Number of loops rolled up");
STATISTIC(NumDynLoopsRolled, "Number of dynamic loops rolled up");
STATISTIC(NumTotalLoops, "Number of total loops");

// An anonymous namespace for the pass. Things declared inside it are
// only visible to the current file.
namespace {

    // Derived from FunctionPass to count qbits in functions
  struct DynRollupRepLoops : public ModulePass {
    static char ID; // Pass identification
    
    //map<BasicBlock*, bool> blockCollapse; //roll this loop
    vector<BasicBlock*> blockCollapse; //roll this loop
    map<BasicBlock*, string> blockRepCond; //repeat condition
    map<BasicBlock*, BasicBlock*> blockBodyInc; //map for.body and for.inc
    vector<BasicBlock*> blockForAll; //list of forall basicblocks
    map<BasicBlock*, int> bbTripCount; //non zero trip count of loops

    map<BasicBlock*, Instruction*> bbFirstInst;
    map<BasicBlock*, BinaryOperator*> bbIncrInst;
    map<BasicBlock*, ICmpInst*> bbICmpInst;
    map<BasicBlock*, PHINode*> bbPhiInst;
    map<BasicBlock*, Value*> bbTripCountVal;

    Function* dummyStartLoop32; //dummyFunc to mark start of loop      
    Function* dummyStartLoop64; //dummyFunc to mark start of loop      
    Function* dummyEndLoop; //dummyFunc to mark start of loop
    
    DynRollupRepLoops() : ModulePass(ID) {}
    //AnalysisUsage AU;
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addPreserved<LoopInfo>();
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();            
    }
    

    /*void print_blockForAll(){
      errs() << "Printing blockForAll: \n";
      for(vector<BasicBlock*>::iterator vit = blockForAll.begin(); vit!=blockForAll.end();++vit)
	errs() << (*vit) << ":" << (*vit)->getName() << " ";
      errs() << "\n";
     
      }*/
    
    bool checkIfQuantumType(Type* t) {
      bool isQtmTy = true;
      
      if(t->isIntegerTy(16) || t->isIntegerTy(1))
	isQtmTy &= true;
      else if(ArrayType *arrayType = dyn_cast<ArrayType>(t))
	{
	  Type *elementType = arrayType->getElementType();
	  if(elementType->isIntegerTy(16) || elementType->isIntegerTy(1))
	    isQtmTy &= true;
	  else isQtmTy &= false;
	}	      
      else if(t->isPointerTy()){
	Type* elementType = t->getPointerElementType();
	if(elementType->isIntegerTy(16) || elementType->isIntegerTy(1))
	  isQtmTy &= true;
      }
      else{
	isQtmTy = false;
	
      }
      
      return isQtmTy;
    }    
    
    bool checkIfQuantum(BasicBlock* BB) {

      //does not allow non-constant integer/double arguments to calls
      //must contain atleast one call instruction


      bool isQtm = true;

      string indVarStr = "";

      //errs() << "BB name: " << BB->getName() << "\n";

      bool hasOtherThanPhiAndBrInst = false;      
      //bool bodyHasIndVarComputation = false;
      bool hasCallInst = false; //must have atleast one call inst
      
      for(BasicBlock::iterator bbi = BB->begin(); bbi != BB->end(); ++bbi)
	{
	  if(debugDynRollupRepLoops)
	    errs() << *bbi << "\n";

	    	    	  
	  if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(&*bbi)){
	    if(debugDynRollupRepLoops)
	      errs() << "Is a GEPI \n";

	    hasOtherThanPhiAndBrInst = true;
	  }
	  /***
	    //check type of GEPI inst first
	    Type* gepiType = GEPI->getType();
	    
	    if(!checkIfQuantumType(gepiType)){
	      isQtm = false;
	      if(debugDynRollupRepLoops)
		errs() << "Unrecognized Type of GEPI Inst. \n";
	      break;
	    }


	    //check indices of GEPI inst next: should be constant or indvars
	    if(isQtm && GEPI->hasIndices())
	      {
		unsigned numOps = GEPI->getNumOperands();
		for(unsigned opIter=1; opIter < numOps; opIter++)
		  {
		    Value* opInst = GEPI->getOperand(opIter);

		    if(opInst->getName().str()==indVarStr){
		      //bodyHasIndVarComputation = true;
		      errs() << "IndVar is present in block \n";
		      return false;
		      //blockForAll.push_back(BB);
		      //print_blockForAll();
		    }      
		    /*
		    if(isa<ConstantInt>(opInst)){
		      isQtm &= true;
		    }
		    //else if(opInst->getName().find("indvars.iv")!=string::npos)
		    else if(opInst->getName().str()==indVarStr)
		      isQtm &= true;
		    else{
		      isQtm = false;
		      if(debugDynRollupRepLoops)
			errs() << "GEPI indices do not satisfy qtm block criteria \n";
		      return isQtm;
		      //break;
		      }*/
	    /**		    
		  }
	      }
	      }**/

	  else if(isa<LoadInst>(*bbi)){
	    if(debugDynRollupRepLoops)
	      errs() << "Is a Load \n";
	  }
	  

	  else if(isa<TruncInst>(*bbi)){
	    if(debugDynRollupRepLoops)
	      errs() << "Is Trunc \n";
	  }

	  else if(isa<PHINode>(*bbi)){
	    if(debugDynRollupRepLoops)
	      errs() << "Is a PHI Node \n";
	      //errs() << *bbi << "\n";
	    //indVarStr = (*bbi).getName();
	      //errs() << "Phi Node Var = " << (*bbi).getName() << "\n";
	  }
	  
		
	  else if(CallInst *CI = dyn_cast<CallInst>(&*bbi))
	    {
	      hasOtherThanPhiAndBrInst = true;
	      hasCallInst = true;

		//check if all operands of call inst are i16 or i16*
		//if not i16 or i16*, they should be constant integers i64 or i32 or floats
		for(unsigned iop=0;iop<CI->getNumArgOperands();iop++){
		  Type* argType = CI->getArgOperand(iop)->getType();
		  
		  //errs() << "Called Func = " << CI->getCalledFunction()->getName() << "\n";
		  //errs() << "Call inst = " << *CI << "\n";

		  if(argType->isIntegerTy(32) || argType->isIntegerTy(64)){
		    //must be constant int
		    if(!isa<ConstantInt>(CI->getArgOperand(iop))){
		      if(debugDynRollupRepLoops)
		      errs() << "Not a constant int \n";
		      return false;
		    }
		  }
		  else if(argType->isDoubleTy() || argType->isFloatTy()){
		    //must be constant
		    if(!isa<ConstantFP>(CI->getArgOperand(iop))){
		      if(debugDynRollupRepLoops)
		      errs() << "Not a constant double/float \n";
		      return false;
		    }
		  }
		  else if(!checkIfQuantumType(argType)){
		    if(debugDynRollupRepLoops)
		      errs() << "Operand Type of Call inst is non-quantum \n";
		    //isQtm = false;
		    return false; 
		  }
		  //else errs() << "Is qtm \n";
		}
	    }

	  else if(isa<BranchInst>(*bbi)){
	    if(debugDynRollupRepLoops)
	      errs() << "Is a Branch Inst \n";
	  }
	  
	  else{
	    if(debugDynRollupRepLoops)
	      errs() << "Unrecognized in quantum module. \n";
	    //isQtm = false;
	    return false;
	    //break;
	  }
	  
	}

      if(hasOtherThanPhiAndBrInst)
	return isQtm;
      else{
	//errs() << "Only contains Phi and Br \n";
	return false;

      }
    }
    
    
    
    void getIncAndCmpConditions(Loop *L, BasicBlock *BB, BasicBlock* Header){
      if(debugDynRollupRepLoops){
	errs() << "Basic Block: " << BB->getName() << "\n";
	errs() << "Get Inc and Cmp conditions \n";
      }
	    stringstream loopRepStr;
	    stringstream incVal;
	    stringstream cmpVal;
	    stringstream initVal;

	    stringstream incVal2;
	    stringstream cmpVal2;
	    stringstream initVal2;


	    int initAt = 0;
	    int incBy = 0;
	    int endAt = 0;

	    bool initConst = false;
	    bool endConst = false;

	    //PHINode* PN;
	    //unsigned IncomingEdge;
	    //unsigned BackEdge;


	    Value* inVal;
	    Value* outVal;

	    BasicBlock::iterator firstIter = Header->begin();
	    while(isa<PHINode>(&*firstIter)) ++firstIter;
	    Instruction* FirstInst = &*(firstIter);

	    //errs() << "First Inst of BB body = " << *FirstInst << "\n";

	    //get initVal str
	    for(BasicBlock::iterator hIter = Header->begin(); hIter!=Header->end(); ++hIter)
	      {
		//find phinode - process only the first phinode found
		if(PHINode *PN = dyn_cast<PHINode>(&*hIter)){ //assuming we will get one PHI node at the beginning of the block. Will get this if -indvars has been run on the code before.
		  //errs() << "Found PHI Node \n";
		  //errs() << *PN << "\n";

		  bbPhiInst[Header] = PN;

		  unsigned IncomingEdge = L->contains(PN->getIncomingBlock(0));	       
		  unsigned BackEdge     = IncomingEdge^1;
		  if(ConstantInt *CInt = dyn_cast<ConstantInt>(PN->getIncomingValue(IncomingEdge))){
		    int val = CInt->getZExtValue();
		    if(debugDynRollupRepLoops)
		      errs() << "Incoming Value = "<< val << "\n";
		    initVal << "i="<<val;
		    initVal2 << val;
		    initAt = val;
		    initConst = true;
		    
		  }
		  else{
		    //Incoming Value will be set at runtime
		    inVal = PN->getIncomingValue(IncomingEdge);
		    //if(debugDynRollupRepLoops)
		      errs() << "Found dyn Incoming Value = "<< inVal->getName() << "\n";
		      ++NumDynLoopsRolled;

		  }

		  if(BinaryOperator *Incr = dyn_cast<BinaryOperator>(PN->getIncomingValue(BackEdge))){

		    bbIncrInst[BB] = Incr;

		    //Instruction* bInst = &*bbi;
		    unsigned instOp = Incr->getOpcode();
		    
		    //for(BasicBlock::iterator bbi = BB->begin(); bbi != BB->end(); ++bbi)
		    //{
		    if(debugDynRollupRepLoops)
		    errs() << *Incr << "\n";
		
		    //Instruction* bInst = &*bbi;
		    //unsigned instOp = bInst->getOpcode();
		    if(instOp == Instruction::Add || instOp == Instruction::Sub){
		      
		      unsigned numOps = Incr->getNumOperands();
		      for(unsigned opIter=0; opIter < numOps; opIter++)
			{
			  Value* opInst = Incr->getOperand(opIter);
			  
			  if(ConstantInt *CI = dyn_cast<ConstantInt>(opInst)){
			    int val = CI->getZExtValue();
			    if(debugDynRollupRepLoops)
			      errs() << "Arith value = "<< val << "\n";
			    
			    if(instOp == Instruction::Add){
			      if(val == 1) { incVal << "i++"; incBy = -1; incVal2<<1; }
			      else if(val == -1) { incVal << "i--"; incBy = 1; incVal2<< -1; }
			      else if(val>0) {incVal << "i+=" << val; incBy = -1*val; incVal2<<val; }
			      else if(val<0) {incVal << "i-=" << val; incBy = val; incVal2 << val; }		  
			    }
			    
			    if(instOp == Instruction::Sub){
			      if(val == 1) { incVal << "i--"; incBy = 1; incVal2 << -1; }
			      else if(val == -1) { incVal << "i++"; incBy = -1; incVal2<< 1; }
			      else if(val>0) { incVal << "i-=" << val; incBy = val; incVal2 << val; }		  
				else if(val<0) { incVal << "i+=" << val; incBy = -1*val; incVal2<<val; }		  
			    }
			   
			    assert((incBy==1 || incBy==-1) && "Loop Step Value NOT EQ 1");
 
			  }
			  //else if(Incr->getName().find("indvars.iv")==string::npos)
			  //errs() << "WARNING: Not an inc instruction. \n";
			}
		      
		    } //end of inst::Add or Inst::Sub		    
		  } //end of Incr
		  break; // process only one Phi Node
		} //end of PHI Node
	      } //end of BB iterator

	    bool addOne = false;
	    bool biggerEnd = false;
	    bool subOne = false;
	    		    
	    for(BasicBlock::iterator bbi = BB->begin(); bbi != BB->end(); ++bbi)
	      {
		if(debugDynRollupRepLoops)
		  errs() << "Inst : " << *bbi << "\n";
		
		Instruction* bInst = &*bbi;
		
		if(ICmpInst *IC = dyn_cast<ICmpInst>(bInst))
		  {
		    if(debugDynRollupRepLoops)
		      errs() << "Compare Inst = "<<*IC<<"\n";

		    bbICmpInst[BB] = IC;

		    int val = INT_MAX;
		    
		    unsigned numOps = IC->getNumOperands();
		    //for(unsigned opIter=0; opIter < numOps; opIter++)
		    unsigned opIter = 1;//check only second operand
		      {
			Value* opInst = IC->getOperand(opIter);
			
			if(ConstantInt *CI = dyn_cast<ConstantInt>(opInst)){
			  val = CI->getZExtValue();
			  if(debugDynRollupRepLoops)
			    errs() << "Arith value = "<< val << "\n";
			  
			}
			
			else{

			  //trace back once to check if result of trunc inst 
			  //if so, use the operand of trunc inst as outVal
			  if(isa<TruncInst>(opInst)){
			    Instruction* TI = dyn_cast<Instruction>(opInst);
			    //errs() << "Is from a trunc inst \n";
			    opInst = TI->getOperand(0);
			  }

			  outVal = opInst;
			  //if(outVal->getName() != "")
			  //errs() << "Dyn Cmp Value = "<< outVal->getName() << " Type = " << outVal->getType()->getTypeID() << "\n";			  
			  ++NumDynLoopsRolled;
			}

			//else if(opInst->getName().find("indvars.iv")==string::npos)
			//errs() << "WARNING: Not an inc instruction. \n";
		      }
		    

		      if(val!= INT_MAX)
			endConst = true;

		      //{
		      unsigned icmp_cond = IC->getUnsignedPredicate();
		      switch(icmp_cond){
		      case CmpInst::ICMP_EQ:
			if(endConst){
			  cmpVal << "i=" << val;
			  cmpVal2 << val;
			  endAt = val; }
			//errs() << "WARNING: equal SHOULD NOT OCCUR \n";
			break;
		      case CmpInst::ICMP_NE:
			if(endConst){
			  cmpVal << "i!=" << val;
			  cmpVal2 << val+incBy;
			  endAt = val+incBy; }
			else { addOne = false; subOne = false; }
			biggerEnd = true; 
			
			//errs() << "WARNING: not equal SHOULD NOT OCCUR \n";
			break;
		      case CmpInst::ICMP_UGT:
		      case CmpInst::ICMP_SGT:
			if(endConst){
			  cmpVal << "i>" << val;
			  cmpVal2 << val+incBy;
			  endAt = val; //+incBy; 
			}
			else { addOne = false; subOne = false; }
			biggerEnd = false; 
			//errs() << "greater \n";
			break;
		      case CmpInst::ICMP_UGE:
		      case CmpInst::ICMP_SGE:
			if(endConst){
			  cmpVal << "i>=" << val;
			  cmpVal2 << val;
			  endAt = val+incBy; }
			else { addOne = false; subOne = true; }
			biggerEnd = false; 
			//errs() << "greater or equal \n";
			break;
			case CmpInst::ICMP_ULT:
		      case CmpInst::ICMP_SLT:
			if(endConst){
			  cmpVal << "i<" << val;
			  cmpVal2 << val+incBy;
			  endAt = val; //+incBy; }
			}			 
			else { addOne =  false; subOne = false; }
			biggerEnd = true; 
			//errs() << "less \n";
			break;
		      case CmpInst::ICMP_ULE:
		      case CmpInst::ICMP_SLE:
			if(endConst){
			  cmpVal << "i<=" << val;
			  cmpVal2 << val;
			  endAt = val+incBy; }			
			else { addOne=true; subOne = false; }
			biggerEnd = true;
			//errs() << "less equal \n";
			break;		  
		      } //end of switch
		      //}	       	      
		      


		  } //end of Icmp Inst	
	      } //end of BB iterator       		      


	    BinaryOperator* Bout = NULL;
	    bool useStoredVal = false;
	    Value* storedVal = NULL;
	    { //hello
	    if(addOne && biggerEnd) //end - start
	      {		
		if(initConst && !endConst) //initAt = constant, endAt not constant
		  {
		    BinaryOperator* BO = BinaryOperator::Create(Instruction::Add, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);

		    if(initAt!=0)
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)BO, ConstantInt::get(outVal->getType(),initAt),"", FirstInst);		      		    
		    else{
		      useStoredVal = true;
		      storedVal = outVal;
		    }
		  }
		else if(!initConst && endConst) //initAt NOT const, endAt const
		  {
		    Bout = BinaryOperator::Create(Instruction::Sub, ConstantInt::get(inVal->getType(),endAt), inVal,"", FirstInst);
		  }
		else if(!initConst && !endConst) //both NOT const
		  {		    		   		    
		    unsigned b1 = inVal->getType()->getIntegerBitWidth();
		    unsigned b2 = outVal->getType()->getIntegerBitWidth();
		    
		    if(b1>b2){
		      CastInst* TI = CastInst::CreateTruncOrBitCast(inVal,outVal->getType(),"",FirstInst);
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Add, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)BO, (Value*)TI,"", FirstInst);		      
		    }
		    else{
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Add, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      CastInst* TI = CastInst::CreateTruncOrBitCast((Value*)BO,inVal->getType(),"",FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)TI, inVal,"", FirstInst);		      
		    }
		  }
		else errs() << "WARNING: This case should not occur. TripCount should have been known by here.\n";                
	      } //end addOne and biggerEnd
         
            else if(addOne && !biggerEnd) //start - end
	      {
		
		if(initConst && !endConst) //initAt = constant, endAt not constant
		  {
		    BinaryOperator* BO = BinaryOperator::Create(Instruction::Add, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);		    
		    Bout = BinaryOperator::Create(Instruction::Sub, ConstantInt::get(outVal->getType(),initAt),(Value*)BO, "", FirstInst);
		  }
		else if(!initConst && endConst) //initAt NOT const, endAt const
		  {

		      Bout = BinaryOperator::Create(Instruction::Sub, inVal, ConstantInt::get(inVal->getType(),endAt), "", FirstInst);
		  }
		else if(!initConst && !endConst) //both NOT const
		  {                             
		    unsigned b1 = inVal->getType()->getIntegerBitWidth();
		    unsigned b2 = outVal->getType()->getIntegerBitWidth();
		    

		    
		    if(b1>b2){
		      CastInst* TI = CastInst::CreateTruncOrBitCast(inVal,outVal->getType(),"",FirstInst);
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Add, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)TI, (Value*)BO, "", FirstInst);
		    }
		    else{
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Add, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      CastInst* TI = CastInst::CreateTruncOrBitCast((Value*)BO,inVal->getType(),"",FirstInst);		      
		      Bout = BinaryOperator::Create(Instruction::Sub, inVal, (Value*)TI, "", FirstInst);

		    }
		  }
		else errs() << "WARNING: This case should not occur. TripCount should have been known by here.\n";                
	      } //end addOne and not biggerEnd

	    else if(subOne && biggerEnd) //end - start
	      {
		
		if(initConst && !endConst) //initAt = constant, endAt not constant
		  {

		    BinaryOperator* BO = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		    
		    if(initAt!=0)		      
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)BO, ConstantInt::get(outVal->getType(),initAt),"", FirstInst);
		    else{
		      useStoredVal = true;
		      storedVal = (Value*)BO;
		    }
		  }
		else if(!initConst && endConst) //initAt NOT const, endAt const
		  {

		    Bout = BinaryOperator::Create(Instruction::Sub, ConstantInt::get(inVal->getType(),endAt), inVal,"", FirstInst);
		  }
		else if(!initConst && !endConst) //both NOT const
		  {		    		              
		    unsigned b1 = inVal->getType()->getIntegerBitWidth();
		    unsigned b2 = outVal->getType()->getIntegerBitWidth();



		    if(b1>b2){
		      CastInst* TI = CastInst::CreateTruncOrBitCast(inVal,outVal->getType(),"",FirstInst);
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)BO, (Value*)TI,"", FirstInst);
		    }
		    else{
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      CastInst* TI = CastInst::CreateTruncOrBitCast((Value*)BO,inVal->getType(),"",FirstInst);		      
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)TI, inVal,"", FirstInst);		      
		    }
		  }
		else errs() << "WARNING: This case should not occur. TripCount should have been known by here.\n";                
	      } //end of subOne and biggerEnd
                    
            else if(subOne && !biggerEnd) //start - end
	      {
		
		if(initConst && !endConst) //initAt = constant, endAt not constant
		  {
		    BinaryOperator* BO = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);		    
		    Bout = BinaryOperator::Create(Instruction::Sub, ConstantInt::get(outVal->getType(),initAt),(Value*)BO, "", FirstInst);
		  }
		else if(!initConst && endConst) //initAt NOT const, endAt const
		  {
		    Bout = BinaryOperator::Create(Instruction::Sub, inVal, ConstantInt::get(inVal->getType(),endAt), "", FirstInst);
		  }
		else if(!initConst && !endConst) //both NOT const
		  {		    		    		    
		    unsigned b1 = inVal->getType()->getIntegerBitWidth();
		    unsigned b2 = outVal->getType()->getIntegerBitWidth();
		    
		    if(b1>b2){
		      CastInst* TI = CastInst::CreateTruncOrBitCast(inVal,outVal->getType(),"",FirstInst);
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)TI, (Value*)BO, "", FirstInst);
		    }
		    else{
		      BinaryOperator* BO = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),1),"", FirstInst);
		      CastInst* TI = CastInst::CreateTruncOrBitCast((Value*)BO,inVal->getType(),"",FirstInst);		      
		      Bout = BinaryOperator::Create(Instruction::Sub, inVal, (Value*)TI, "", FirstInst);
		    }
		  }
		else errs() << "WARNING: This case should not occur. TripCount should have been known by here.\n";        
	      } //end of subOne and NOT biggerEnd
	    
	    else if(!addOne && !subOne && biggerEnd) //end - start
	      {
		
		if(initConst && !endConst) //initAt = constant, endAt not constant
		  {
		    //errs() << "insert 101 initVal = " << initAt << " endAt Ptr = " << outVal->getName()<< "\n";
		    
		    if(initAt!=0)		      
		      Bout = BinaryOperator::Create(Instruction::Sub, outVal, ConstantInt::get(outVal->getType(),initAt),"", FirstInst);
		    else{
		      useStoredVal = true;
		      storedVal = outVal;
		      //errs() << "useStoredValue set = " <<useStoredVal << "\n";
		    }
		  }
		else if(!initConst && endConst) //initAt NOT const, endAt const
		  {
		    Bout = BinaryOperator::Create(Instruction::Sub, ConstantInt::get(inVal->getType(),endAt), inVal,"", FirstInst);
		  }
		else if(!initConst && !endConst) //both NOT const
		  {		    		              
		    unsigned b1 = inVal->getType()->getIntegerBitWidth();
		    unsigned b2 = outVal->getType()->getIntegerBitWidth();

		    if(b1>b2){
		      CastInst* TI = CastInst::CreateTruncOrBitCast(inVal,outVal->getType(),"",FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, outVal, (Value*)TI,"", FirstInst);
		    }
		    else{
		      CastInst* TI = CastInst::CreateTruncOrBitCast(outVal,inVal->getType(),"",FirstInst);		      
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)TI, inVal,"", FirstInst);		      
		    }
		  }
		else errs() << "WARNING: This case should not occur. TripCount should have been known by here.\n";                
		//errs() << "useStoredValue 1 = " <<useStoredVal << "\n";
	      } //end of subOne and biggerEnd
                    
            else if(!addOne && !subOne && !biggerEnd) //start - end
	      {
		
		if(initConst && !endConst) //initAt = constant, endAt not constant
		  {
		    Bout = BinaryOperator::Create(Instruction::Sub, ConstantInt::get(outVal->getType(),initAt),outVal, "", FirstInst);
		  }
		else if(!initConst && endConst) //initAt NOT const, endAt const
		  {
		    Bout = BinaryOperator::Create(Instruction::Sub, inVal, ConstantInt::get(inVal->getType(),endAt), "", FirstInst);
		  }
		else if(!initConst && !endConst) //both NOT const
		  {		    		    		    
		    unsigned b1 = inVal->getType()->getIntegerBitWidth();
		    unsigned b2 = outVal->getType()->getIntegerBitWidth();
		    
		    
		    if(b1>b2){
		      CastInst* TI = CastInst::CreateTruncOrBitCast(inVal,outVal->getType(),"",FirstInst);
		      Bout = BinaryOperator::Create(Instruction::Sub, (Value*)TI, outVal, "", FirstInst);
		    }
		    else{		      
		      CastInst* TI = CastInst::CreateTruncOrBitCast(outVal,inVal->getType(),"",FirstInst);		      
		      Bout = BinaryOperator::Create(Instruction::Sub, inVal, (Value*)TI, "", FirstInst);
		    }
		  }
		else errs() << "WARNING: This case should not occur. TripCount should have been known by here.\n";        
	      } //end of NOT biggerEnd
	    }                    
	    //if(useStoredVal) errs() << "StoredVal: " << storedVal->getName() << "\n";
	    //errs() << " 15e \n";

	    if(useStoredVal) bbTripCountVal[Header] = storedVal;
	    else 
	      if(Bout!=NULL)
		bbTripCountVal[Header] = (Value*)Bout;
	      else errs() << "WARNING: NO TRIP COUNT FOUND \n";



	    blockBodyInc[Header] = BB;
	    bbFirstInst[Header] = FirstInst;


      /*

	    if(initVal.str()!="" && cmpVal.str()!="" && incVal.str()!=""){
	      //errs() << "Searching blockForAll for " << Header << ":" << Header->getName() << "\n";
	      vector<BasicBlock*>::iterator isForAll = find(blockForAll.begin(), blockForAll.end(),Header);
	      if(isForAll!=blockForAll.end())		
		//loopRepStr << "forall "<< initVal.str() << "; " << cmpVal.str() << "; " << incVal.str();
		loopRepStr << "forall ["<< initVal2.str() << ":" << cmpVal2.str() << ":" << incVal2.str() << "]";
	     
	      else{
		//loopRepStr << "repeat "<< initVal.str() << "; " << cmpVal.str() << "; " << incVal.str();
		int numRepeats = endAt - initAt + 1;
		if(numRepeats>0)
		  loopRepStr << "repeat "<< (endAt-initAt+1) << " times ";
	      }
	    }
	    if(debugDynRollupRepLoops)
	      errs() << "Rep String1 = " << loopRepStr.str() << "\n";
	    
	      blockRepCond[Header] = loopRepStr.str();*/
	    //}
      
    }


    void getBBInsts(Loop *L,BasicBlock* BB,BasicBlock* Header)
    {

      //errs() << "getBBInsts: BB = " << BB->getName() << "\n";
      blockBodyInc[Header] = BB;

      BasicBlock::iterator firstIter = Header->begin();
      //if(isa<PHINode>(*firstIter)){
      if(PHINode *PN = dyn_cast<PHINode>(&*firstIter)){ //assuming we will get one PHI node at the beginning of the block. Will get this if -indvars has been run on the code before.       
	bbPhiInst[Header] = PN;
	//errs() << "Found PHI \n";

	unsigned IncomingEdge = L->contains(PN->getIncomingBlock(0));	       
	unsigned BackEdge     = IncomingEdge^1;

	if(BinaryOperator *Incr = dyn_cast<BinaryOperator>(PN->getIncomingValue(BackEdge))){
	  bbIncrInst[BB] = Incr;
	  //errs() << "Found Incr : " << *Incr << "\n";
	}
	else errs() << "WARNING: Incr Inst Not Found \n";
      }

      while(isa<PHINode>(&*firstIter)) ++firstIter;
      Instruction* FirstInst = &*(firstIter);
      bbFirstInst[Header] = FirstInst;



      for(BasicBlock::iterator bbi = BB->begin(); bbi != BB->end(); ++bbi)
	{
	  if(debugDynRollupRepLoops)
	    errs() << "Inst : " << *bbi << "\n";
	  
	  Instruction* bInst = &*bbi;
	  
	  if(ICmpInst *IC = dyn_cast<ICmpInst>(bInst))
	    {
	      if(debugDynRollupRepLoops)
		errs() << "Found Compare Inst = "<<*IC<<"\n";
	      
	      bbICmpInst[BB] = IC;
	      break;
	    }

	}

    }
    
    void getLoopInfo(Function& F) {
      LoopInfo *LI = &getAnalysis<LoopInfo> ( F );
      ScalarEvolution *SE = &getAnalysis<ScalarEvolution>( F );

      for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
	{           
	  
	  string BBname = BB->getName();
	  //errs() << "Basic Block Name = "<<BBname << "\n";	    
	  
	  
	  Loop* L = LI->getLoopFor(&*BB);
	  if (L != NULL)
	    {    
	      
	      if(BBname.find("for.body")!=string::npos){
		++NumTotalLoops;
		if(debugDynRollupRepLoops)
		  errs() << "Counting this: "<<BBname<<"\n";

		BasicBlock* Latch = L->getLoopLatch();
                if(Latch){

		  int tripCount = SE->getSmallConstantTripCount(L, Latch);

		  if(tripCount > 1)
		    bbTripCount[BB] = tripCount;


		  if(Latch!=&*BB){
		    bool isCollapsable = checkIfQuantum(&*BB);
		    if(debugDynRollupRepLoops)
		      errs() << "isCollapsable = " << isCollapsable << "\n";

		    if(isCollapsable && tripCount!=1){		      
		      //blockCollapse[&*BB] = isCollapsable;		    
		      blockCollapse.push_back(&*BB);

		      if(debugDynRollupRepLoops)
			errs() << "Trip Count of " << BBname << " is " << tripCount <<"\n";
		  
		      //errs() << "Latch is: " << Latch->getName() << "\n";
		   
		      if(tripCount == 0)
			getIncAndCmpConditions(L, Latch, &*BB);
		      else
			getBBInsts(L,Latch,&*BB);		      
		    }
		  }
		  else{ //Latch and BB are the same basic blocks
		    errs() << "WARNING: Latch and BB are the same basic blocks. This condition should never occur here\n.";
		   
		    }

		}

	      }
	    }
	  
	  //errs() << "\n";
	}
    }
    
   
    void addDummyStart(BasicBlock* BB){
      
      //BasicBlock::iterator BBiter = BB->getFirstNonPHI();

      //while(isa<AllocaInst>(BBiter))
      //++BBiter;

      //errs() << "addDummyStart: " << BB->getName() << "\n";

      map<BasicBlock*, Instruction*>::iterator fi_iter = bbFirstInst.find(BB);
      assert(fi_iter!=bbFirstInst.end() && "First Inst not found for this BB");

      Instruction* BBiter = (*fi_iter).second;

      map<BasicBlock*,int>::iterator tcIter = bbTripCount.find(BB);
      if(tcIter!=bbTripCount.end()){
	int tripCount = (*tcIter).second;
	//errs() << "Adding TripCount of " << BB->getName() << " is "<<tripCount<<"\n";	      	
	
	//add dummy function with trip count as argument

	Value* intArg = ConstantInt::get(Type::getInt32Ty(BB->getContext()),tripCount);
	
	CallInst::Create(dummyStartLoop32, intArg, "",BBiter);      	
      }
      else{
	map<BasicBlock*,Value*>::iterator tcvalIter = bbTripCountVal.find(BB);
	if(tcvalIter!=bbTripCountVal.end()){
	  Value* tripCountVal = (*tcvalIter).second;

	  //errs() << "tripcountVal name = " << tripCountVal->getName() << "\n";
	  //add dummy function with trip count as argument

	  
	  //Value* intArg = ConstantInt::get(Type::getInt32Ty(BB->getContext()),tripCount);
	  
	  if(tripCountVal->getType()->isIntegerTy(32))
	    CallInst::Create(dummyStartLoop32, tripCountVal, "",BBiter);      	
	  if(tripCountVal->getType()->isIntegerTy(64))
	    CallInst::Create(dummyStartLoop64, tripCountVal, "",BBiter);      	
	  
	}
	else errs() << "WARNING: Trip Count Value not found for this BB \n";	
      }            
    }


    void addDummyEnd(BasicBlock* BB){

     TerminatorInst *BBTerm = BB->getTerminator();

     //while(isa<AllocaInst>(BBiter))
     //++BBiter;
            
      CallInst::Create(dummyEndLoop, "",(Instruction*)BBTerm);      

      return;

    }
    void modifyIndVars(Loop* L, BasicBlock* BB){
      
      string indVarStr = "";

      //errs() << "modifyIndVars: " << BB->getName() << "\n";

      map<BasicBlock*, PHINode*>::iterator piter = bbPhiInst.find(BB);
      assert(piter != bbPhiInst.end() && "Phi Inst for this BB not found");


      

      PHINode *PN = (*piter).second;
      //if(PHINode *PN = dyn_cast<PHINode>(&*bbi)){
      
	//if(debugDynRollupRepLoops)
	//    errs() << "Is a PHI Node: " << *bbi << "\n";                                               
	//  indVarStr = (*bbi).getName();
	    //errs() << "Phi Node Var = " << (*bbi).getName() << "\n";    	 

	indVarStr = PN->getName();

	    //rewrite Init Value
	    unsigned IncomingEdge = L->contains(PN->getIncomingBlock(0));
	    //unsigned BackEdge     = IncomingEdge^1;

	    Value* currVal = PN->getIncomingValue(IncomingEdge);
	    	    
	    PN->setIncomingValue(IncomingEdge,Constant::getNullValue(currVal->getType()));	    

	    // }
	    //else
	    /*

	      for(BasicBlock::iterator bbi = BB->begin(); bbi!=BB->end(); ++bbi)
	      {
		//if(debugDynRollupRepLoops)
		//    errs()<<"\t" << *bbi<<"\n";

	    if(GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(&*bbi)){
	    //check indices of GEPI inst next: should be constant or indvars          
            if(GEPI->hasIndices())
              {
		unsigned numOps = GEPI->getNumOperands();
                for(unsigned opIter=1; opIter < numOps; opIter++)
                  {
                    Value* opInst = GEPI->getOperand(opIter);               
                    if(opInst->getName().str()==indVarStr){
		      //GEPI->setOperand(opIter, Constant::getNullValue(Type::getInt16Ty(BB->getContext())));
		      GEPI->setOperand(opIter, ConstantInt::get(Type::getInt16Ty(BB->getContext()),0));
		    }           
                  }
              }
	    } //end of GEPI 
	    } //end of basic block iterator           */
    }

    void transformInc(Loop* L, BasicBlock* BB){
      //errs() << "In Transform Inc\n";


      //errs() << "transformInc: " << BB->getName() << "\n";

      map<BasicBlock*, BinaryOperator*>::iterator incriter = bbIncrInst.find(BB);
      assert(incriter != bbIncrInst.end() && "Incr Inst for this BB not found");          
      
      BinaryOperator* Incr = (*incriter).second;

      //Instruction* IncrInst;

      //for(BasicBlock::iterator bbi = BB->begin(); bbi != BB->end(); ++bbi)
      //{
      //  if(debugDynRollupRepLoops)
      //    errs() << *bbi << "\n";
	    
      //  Instruction* bInst = &*bbi;	  

      //  if(BinaryOperator *Incr = dyn_cast<BinaryOperator>(bInst)){

	  unsigned instOp = Incr->getOpcode();
	  if(instOp == Instruction::Add) // || instOp == Instruction::Sub){
	    {

	      //IncrInst = Incr;

	      //errs() << "Inc inst name = "<<bInst->getName() << "\n";
	      //assert(Incr->getName().find("indvars.iv")!=string::npos && "Not an inc instr in for.inc");

	      unsigned numOps = Incr->getNumOperands();
	      for(unsigned opIter=0; opIter < numOps; opIter++)
		{
		  Value* opInst = Incr->getOperand(opIter);

		  if(isa<ConstantInt>(opInst)){
		    Incr->setOperand(opIter, ConstantInt::get(opInst->getType(),1));
		    break;		    
		  }

		}
	    } //end of Inst::Add
	  else if(instOp == Instruction::Sub){ //should not contain a sub inst
	    unsigned numOps = Incr->getNumOperands();
	    for(unsigned opIter=0; opIter < numOps; opIter++)
	      {
		Value* opInst = Incr->getOperand(opIter);

		if(isa<ConstantInt>(opInst)){
		  Incr->setOperand(opIter, ConstantInt::get(opInst->getType(),-1));
		  break;
		}

	      }
	  } //end of Inst::Sub     
	  //} //end of if Binary Operator
	  
      //Rewrite Exit Value and Condition
      map<BasicBlock*, ICmpInst*>::iterator icmpiter = bbICmpInst.find(BB);
      assert(icmpiter != bbICmpInst.end() && "ICmp Inst for this BB not found");                
      ICmpInst* IC = (*icmpiter).second;

      //if(ICmpInst *IC = dyn_cast<ICmpInst>(bInst))
      //    {

	      CmpInst::Predicate NewPred = CmpInst::ICMP_SLT;	     
	      
	      //hack for bwt loop rotate problem
	      //get IC pred, if it is eq, keep eq.
	      unsigned currPred = IC->getUnsignedPredicate();

	      if(currPred == CmpInst::ICMP_EQ)
		NewPred = CmpInst::ICMP_EQ;
		
	      if(currPred == CmpInst::ICMP_NE)
		NewPred = CmpInst::ICMP_NE;

	      BranchInst *Br = cast<BranchInst>(IC->use_back());
	      assert(Br->isConditional() && "Did not find a branch");

	      /*
	      if(ConstantInt *Val1 = dyn_cast<ConstantInt>(IC->getOperand(1))){
		errs() << "Opd1: Val: "<< Val1->getZExtValue() << "\n";		
	      }
	      else{

		errs() << "Opd1: Name: "<< IC->getOperand(1)->getName() << "\n";		

	      }

	      if(ConstantInt *Val0 = dyn_cast<ConstantInt>(IC->getOperand(0))){
		errs() << "Opd0: Val: "<< Val0->getZExtValue() << "\n";		
	      }
	      else{

		errs() << "Opd0: Name: "<< IC->getOperand(0)->getName() << "\n";		

		}	*/      

	      Value* ICop = IC->getOperand(0);

	      ICmpInst *NewCmp = new ICmpInst(Br,NewPred,ICop,ConstantInt::get(ICop->getType(),1),IC->getName());

	      NewCmp->takeName(IC);
	      IC->replaceAllUsesWith(NewCmp);
	      RecursivelyDeleteTriviallyDeadInstructions(IC);

	      //break;
	      //    }

	  //} //end of BB iterator      

      return;
    }


    void getLoopInfoAndModify(Function &F,BasicBlock* thisBB,BasicBlock* thisInc){


    //errs() << "BB = " << thisBB->getName() << " Func=" << F->getName() <<"\n";                   
              LoopInfo *LI = &getAnalysis<LoopInfo> ( F );                                                                Loop* L1 = LI->getLoopFor(&*thisBB);                                                           
              //assert(L1 != NULL); //must be a loop by this point                                           
              modifyIndVars(L1, thisBB);                                                                                                                                                                                
              Loop* L2 = LI->getLoopFor(&*thisInc);                                                          
              transformInc(L2, thisInc);                                                                   
    }

    void processLoopsToRoll(Module &M)
    {
      //for(map<BasicBlock*, bool>::iterator bbc = blockCollapse.begin(); bbc!=blockCollapse.end(); ++bbc){
      for(vector<BasicBlock*>::iterator bbc = blockCollapse.begin(); bbc!=blockCollapse.end(); ++bbc){

	//if((*bbc).second == true){

	  //map<BasicBlock*, string>::iterator bbrep = blockRepCond.find((*bbc).first);
	  //if(bbrep!=blockRepCond.end())
	  //if((*bbrep).second != ""){
	      

	//BasicBlock* thisBB = (*bbc).first;
	BasicBlock* thisBB = (*bbc);

	      ++NumLoopsRolled;

	      
	      map<BasicBlock*, BasicBlock*>::iterator bbInc = blockBodyInc.find(thisBB);
	      assert(bbInc!=blockBodyInc.end());
	      BasicBlock* thisInc = (*bbInc).second;
	      if(debugDynRollupRepLoops){
		errs() << "Found basicblock to collapse : " << thisBB->getName() <<"\n";
		for(BasicBlock::iterator bbi = thisBB->begin(); bbi!=thisBB->end(); ++bbi)
		  {
		    errs()<<"\t" << *bbi<<"\n";		  		
		  }
	      }
  
	      //addDummyStart(M,thisBB,(*bbrep).second);
	      //addDummyEnd(M,thisBB,(*bbrep).second);

	      addDummyStart(thisBB);
	      addDummyEnd(thisBB);

	      Function* F = thisBB->getParent();
	      getLoopInfoAndModify(*F,thisBB,thisInc);

	      //} //end of found basic block to collapse

	//}




      } //blockCollapse iterator

    }



    void processFunctions(Function &F) {
      if(F.isDeclaration())
	errs() << "Fn is declaration\n";      
      
      //blockCollapse.clear();
      //blockRepCond.clear();
      
      //errs() << "==========Function: " << F.getName() << '\n';
      
      getLoopInfo(F);
      //errs() << "\n";
                 
    } // End runOnFunction
    

    bool runOnModule(Module &M){
      for(Module::iterator mIter = M.begin(); mIter != M.end(); ++mIter) {
	Function* F = &(*mIter);

	if(F && !F->isDeclaration()){
	  processFunctions((*F));
	}	
      }

      string dummyFnNameSt = "qasm_print_RepLoopStart";
      //dummyFnName.append(repStr);
      //if(debugDynRollupRepLoops)
      //errs() << "Dummy Fn Name = " << dummyFnName << "\n";


      dummyStartLoop32 = cast<Function>(M.getOrInsertFunction("qasm_print_RepLoopStart32", Type::getVoidTy(M.getContext()), Type::getInt32Ty(M.getContext()), (Type*)0));
      dummyStartLoop64 = cast<Function>(M.getOrInsertFunction("qasm_print_RepLoopStart64", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext()), (Type*)0));

      string dummyFnNameE = "qasm_print_RepLoopEnd";
      //dummyFnName.append(repStr);
      //if(debugDynRollupRepLoops)
      //	errs() << "Dummy Fn Name = " << dummyFnName << "\n";

      //addDummyFunc

      dummyEndLoop = cast<Function>(M.getOrInsertFunction(dummyFnNameE, Type::getVoidTy(M.getContext()), (Type*)0));


      //process loops identified as candidates
      //errs() << "Processing Loops " << blockCollapse.size() << "\n";
      processLoopsToRoll(M);

      return true;
      }
   

  }; // End of struct FunctionDynRollupRepLoops
} // End of anonymous namespace

char DynRollupRepLoops::ID = 0;
static RegisterPass<DynRollupRepLoops> X("dyn-rollup-rep-loops", "Do not unroll repeat loops if possible.");
