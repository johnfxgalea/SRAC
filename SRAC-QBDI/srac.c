#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <QBDI.h>

#define FAKE_RET_ADDR 42
#define VSTACK_SIZE 0x600000
#define RSTACK_SIZE 40

typedef struct{

    rword ret_addrs[RSTACK_SIZE]; 
    rword *base; // Points to base of stack
    rword *top; // Points to top of stack
}ret_stack_t;


void copy_and_print(char *str){

    char str_cpy[20];

    // Perform no boundary checks.
    strcpy(str_cpy, str);

    printf("The copied string is %s\n", str_cpy);
}

int prog_main(int argc, char **argv){

    if (argc == 2){
      // Call buggy function
      copy_and_print((char *) argv[1]);
    }else{
        printf("No param was specified\n");
    }
}

VMAction handle_call(VMInstanceRef vm, GPRState *gprState, FPRState *fprState, void *data) {

    // Obtain an analysis of the instruction from the VM
    const InstAnalysis* instAnalysis = qbdi_getInstAnalysis(vm, QBDI_ANALYSIS_INSTRUCTION | QBDI_ANALYSIS_DISASSEMBLY);
    
    // Assert that it is a call instr
    assert(instAnalysis->isCall);

    // Calculate the fall instr addr right after the call instr. This is where the return jumps back to
    rword fall_instr_addr = instAnalysis->address + instAnalysis->instSize;

    // Push the calculate addr to the shadow stack
    ret_stack_t *ret_stack = (ret_stack_t *) data;
    *(ret_stack->top) = fall_instr_addr;
    ret_stack->top++;

    // Perform a boundary check for the shadow stack. If we exceed the limit, the program exists
    if (ret_stack->top >= (ret_stack->base + RSTACK_SIZE)){
        printf("Stopping execution. Shadow Stack has reached its size limit.\n");
        return QBDI_STOP;
    }

    // Print some information about call
    printf("CALL Detected! Addr: 0x%" PRIRWORD " Fall Addr: 0x%" PRIRWORD "\n", instAnalysis->address, fall_instr_addr);

    return QBDI_CONTINUE;
}

VMAction handle_ret(VMInstanceRef vm, GPRState *gprState, FPRState *fprState, void *data) {

    // Obtain an analysis of the instruction from the VM
    const InstAnalysis* instAnalysis = qbdi_getInstAnalysis(vm, QBDI_ANALYSIS_INSTRUCTION | QBDI_ANALYSIS_DISASSEMBLY);

    // Assert that it is a return instr
    assert(instAnalysis->isReturn);
    
    ret_stack_t *ret_stack = (ret_stack_t *) data;

    // This ret could be the last ret of our simulated call. We determine this by checking if the stack is empty
    if (ret_stack->top == ret_stack->base)
        return QBDI_CONTINUE;

    // Get the saved IP on the stack
    GPRState *gpr = qbdi_getGPRState(vm);
    rword *stk_ptr = (rword *) gpr->rsp;
    rword saved_ip = *stk_ptr;

    //Print some information about ret
    printf("RET Detected! IP on Stack: 0x%" PRIRWORD "\n", saved_ip);


    // Check the validity of the saved IP
    bool is_valid = false;

    while (ret_stack->top > ret_stack->base){
        ret_stack->top--;
                
        if (saved_ip == *(ret_stack->top)){
            is_valid = true;
            break;  
        }
    }

    // If it is not valid, we exit the program
    if (!is_valid){
        printf("Saved IP is not valid. Exiting program\n");
        return QBDI_STOP;
    }else
        return QBDI_CONTINUE;
}

int main(int argc, char** argv) {
    VMInstanceRef vm = NULL;
    uint8_t *fakestack = NULL;

    printf("string : %s\n", argv[1]);

    // init VM
    qbdi_initVM(&vm, NULL, NULL);

    // Get a pointer to the GPR state of the VM
    GPRState *state = qbdi_getGPRState(vm);
    assert(state != NULL);

    // Setup initial GPR state, this fakestack will produce a ret NULL at the end of the execution
    bool res = qbdi_allocateVirtualStack(state, VSTACK_SIZE, &fakestack);
    assert(res == true);

    // Initialise ret stack
    ret_stack_t ret_stack;
    ret_stack.base = ret_stack.ret_addrs;
    ret_stack.top = ret_stack.ret_addrs;  

    // Add callback to call (POST) and ret (PRE) instructions
    qbdi_addMnemonicCB(vm, "CALL64pcrel32", QBDI_POSTINST, handle_call, &ret_stack);    
    qbdi_addMnemonicCB(vm, "RETQ", QBDI_PREINST, handle_ret, &ret_stack);

    res = qbdi_addInstrumentedModuleFromAddr(vm, (rword) &main);
    assert(res == true);

    // Simulate a call in stack
    qbdi_simulateCall(state, FAKE_RET_ADDR, 2, (rword) argc, (rword) argv);

    // call prog_main using VM, custom state and fake stack
    res = qbdi_run(vm, (rword) prog_main, (rword) FAKE_RET_ADDR);
    assert(res == true);

    // free everything
    qbdi_alignedFree(fakestack);
    qbdi_terminateVM(vm);

    return 0;
}
