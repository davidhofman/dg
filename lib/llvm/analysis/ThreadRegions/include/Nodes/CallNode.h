#ifndef CALLNODE_H
#define CALLNODE_H

#include "Node.h"

class CallNode : public Node
{
public:
    CallNode(const llvm::Instruction * instruction = nullptr);
};

#endif // CALLNODE_H
