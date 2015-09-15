#ifndef _LLVM_ANALYSIS_GENERIC_H_
#define _LLVM_ANALYSIS_GENERIC_H_

#include <map>
#include <set>

namespace llvm {
    class ConstantExpr;
    class DataLayout;
};

namespace dg {

class LLVMDependenceGraph;
class LLVMNode;

namespace analysis {

#define UNKNOWN_OFFSET ~((uint64_t) 0)

// just a wrapper around uint64_t to
// handle UNKNOWN_OFFSET somehow easily
// maybe later we'll make it a range
struct Offset
{
    Offset(uint64_t o = UNKNOWN_OFFSET) : offset(o) {}
    Offset& operator+=(const Offset& o)
    {
        if (offset == UNKNOWN_OFFSET)
            return *this;

        if (o.offset == UNKNOWN_OFFSET)
            offset = UNKNOWN_OFFSET;
        else
            offset += o.offset;

        return *this;
    }

    Offset operator+(const Offset& o)
    {
        if (offset == UNKNOWN_OFFSET || o.offset == UNKNOWN_OFFSET)
            return UNKNOWN_OFFSET;

        return Offset(offset + o.offset);
    }

    bool operator<(const Offset& o) const
    {
        return offset < o.offset;
    }

    bool isUnknown() const { return offset == UNKNOWN_OFFSET; }

    uint64_t operator*() const { return offset; }
    const uint64_t *operator->() const { return &offset; }

    uint64_t offset;
};

struct MemoryObj;
struct Pointer
{
    Pointer(MemoryObj *m = nullptr, Offset off = 0) : obj(m), offset(off) {}

    MemoryObj *obj;
    Offset offset;

    bool operator<(const Pointer& oth) const
    {
        return obj == oth.obj ? offset < oth.offset : obj < oth.obj;
    }

    bool isNull() const { return obj == nullptr; }
};

typedef std::set<Pointer> PointsToSetT;
typedef std::set<LLVMNode *> ValuesSetT;
typedef std::map<Offset, PointsToSetT> PointsToMapT;
typedef std::map<Offset, ValuesSetT> ValuesMapT;

struct MemoryObj
{
    MemoryObj(LLVMNode *n, uint64_t s = 0) : node(n), size(s) {}
    LLVMNode *node;

    PointsToMapT pointsTo;

    bool addPointsTo(const Offset& off, const Pointer& ptr)
    {
        if (isUnknown())
            return false;

        return pointsTo[off].insert(ptr).second;
    }

    bool isUnknown() const { return node == nullptr; }
    bool setUnknown()
    {
        if (isUnknown())
            return false;

        pointsTo.clear();
        node = nullptr;
        return true;
    }

    // if the object is allocated via malloc or
    // similar, we can not infer the size from type,
    // because it is recast to (usually) i8*. Store the
    // size information here, if applicable and available
    uint64_t size;
    bool hasSize() const { return size != 0; }
#if 0
    ValuesMapT values;
    // actually we should take offsets into
    // account, since we can memset only part of the memory
    ValuesSetT memsetTo;
#endif
};

Pointer getConstantExprPointer(const llvm::ConstantExpr *CE,
                               LLVMDependenceGraph *dg,
                               const llvm::DataLayout *DL);

LLVMNode *getOperand(LLVMNode *node, const llvm::Value *val,
                     unsigned int idx, const llvm::DataLayout *DL);

} // namespace analysis
} // namespace dg

#endif // _LLVM_ANALYSIS_GENERIC_H_
