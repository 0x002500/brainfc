#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <cstdio>

using namespace llvm;
using namespace std;

void translateIncrementPtr(IRBuilder<>& builder, Value* dataPointer) {
    Value* ptrValue = builder.CreateLoad(dataPointer);
    Value* newPtrValue = builder.CreateAdd(ptrValue, builder.getInt8(1));
    builder.CreateStore(newPtrValue, dataPointer);
}

void translateDecrementPtr(IRBuilder<>& builder, Value* dataPointer) {
    Value* ptrValue = builder.CreateLoad(dataPointer);
    Value* newPtrValue = builder.CreateSub(ptrValue, builder.getInt8(1));
    builder.CreateStore(newPtrValue, dataPointer);
}

void translateIncrementData(IRBuilder<>& builder, Value* dataPointer, Value* memory) {
    Value* ptr = builder.CreateLoad(dataPointer);
    Value* cellValue = builder.CreateLoad(builder.CreateGEP(memory, {ptr}));
    Value* newCellValue = builder.CreateAdd(cellValue, builder.getInt8(1));
    builder.CreateStore(newCellValue, builder.CreateGEP(memory, {ptr}));
}

void translateDecrementData(IRBuilder<>& builder, Value* dataPointer, Value* memory) {
    Value* ptr = builder.CreateLoad(dataPointer);
    Value* cellValue = builder.CreateLoad(builder.CreateGEP(memory, {ptr}));
    Value* newCellValue = builder.CreateSub(cellValue, builder.getInt8(1));
    builder.CreateStore(newCellValue, builder.CreateGEP(memory, {ptr}));
}

void translateStartLoop(IRBuilder<>& builder, Value* dataPointer, Value* memory, BasicBlock* loopStart, BasicBlock* loopEnd) {
    builder.CreateBr(loopStart);
    builder.SetInsertPoint(loopStart);

    Value* ptr = builder.CreateLoad(dataPointer);
    Value* cellValue = builder.CreateLoad(builder.CreateGEP(memory, {ptr}));
    Value* loopCondition = builder.CreateICmpNE(cellValue, builder.getInt8(0));
    builder.CreateCondBr(loopCondition, loopStart, loopEnd);

    builder.SetInsertPoint(loopEnd);
}

void translateEndLoop(IRBuilder<>& builder, Value* dataPointer, Value* memory, BasicBlock* loopStart, BasicBlock* loopEnd) {
    builder.CreateBr(loopStart);
    builder.SetInsertPoint(loopEnd);

    Value* ptr = builder.CreateLoad(dataPointer);
    Value* cellValue = builder.CreateLoad(builder.CreateGEP(memory, {ptr}));
    Value* loopCondition = builder.CreateICmpNE(cellValue, builder.getInt8(0));
    builder.CreateCondBr(loopCondition, loopStart, loopEnd);
}

void translateOutput(IRBuilder<>& builder, Value* dataPointer, Value* memory, Function* putcharFunction) {
    Value* ptr = builder.CreateLoad(dataPointer);
    Value* cellValue = builder.CreateLoad(builder.CreateGEP(memory, {ptr}));
    builder.CreateCall(putcharFunction, {cellValue});
}

void translate(const string& brainfuckCode, const string& outputFileName) {
    LLVMContext context;
    Module module("BrainfuckModule", context);
    IRBuilder<> builder(context);

    // Declare putchar function
    FunctionType* putcharFuncType = FunctionType::get(builder.getVoidTy(), {builder.getInt8Ty()});
    Function* putcharFunction = Function::Create(putcharFuncType, Function::ExternalLinkage, "putchar", &module);

    // Initialize memory array (30000 cells)
    Value* memory = builder.CreateAlloca(builder.getInt8Ty(), builder.getInt32(30000), "memory");
    Value* dataPointer = builder.CreateAlloca(builder.getInt8Ty(), nullptr, "dataPointer");

    // Create main function
    FunctionType* mainFuncType = FunctionType::get(builder.getVoidTy(), false);
    Function* mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage, "main", &module);
    BasicBlock* entryBlock = BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entryBlock);

    // Initialize data pointer
    builder.CreateStore(builder.getInt8(0), dataPointer);

    // Translate Brainfuck code
    for (char instruction : brainfuckCode) {
        switch (instruction) {
            case '>':
                translateIncrementPtr(builder, dataPointer);
                break;
            case '<':
                translateDecrementPtr(builder, dataPointer);
                break;
            case '+':
                translateIncrementData(builder, dataPointer, memory);
                break;
            case '-':
                translateDecrementData(builder, dataPointer, memory);
                break;
            case '[': {
                BasicBlock* loopStart = BasicBlock::Create(context, "loop_start", mainFunc);
                BasicBlock* loopEnd = BasicBlock::Create(context, "loop_end", mainFunc);
                translateStartLoop(builder, dataPointer, memory, loopStart, loopEnd);
                break;
            }
            case ']': {
                BasicBlock* loopStart = builder.GetInsertBlock();
                BasicBlock* loopEnd = BasicBlock::Create(context, "loop_end", mainFunc);
                translateEndLoop(builder, dataPointer, memory, loopStart, loopEnd);
                break;
            }
            case '.':
                translateOutput(builder, dataPointer, memory, putcharFunction);
                break;
            // Ignore other characters
        }
    }

    // Add return statement
    builder.CreateRetVoid();

    // 输出到文件或stdout
    if (!outputFileName.empty()) {
        std::error_code EC;
        raw_fd_ostream file(outputFileName, EC, sys::fs::F_None);
        module.print(file, nullptr);
        file.close();
    } else {
        module.print(outs(), nullptr);
    }
}

int main() {
    std::string brainfuckCode = "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>+++.>+.+++++++..+++.>++.<<+++++++++++++.>.";

    // 将Brainfuck代码转换为LLVM IR并输出到文件
    translate(brainfuckCode, "output.ll");

    return 0;
}
