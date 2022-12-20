//
// Created by maxbr on 19.12.2022.
// Concurrent binary tree encoding based on https://onrendering.com/data/papers/cbt/ConcurrentBinaryTrees.pdf
// for parallelizing subdivision
//

#pragma once

#include <stdint.h>
#include <cassert>
#include <atomic>
#include "Defs.h"
#include "ParallelFor.h"


typedef struct cbt_Tree cbt_Tree;
typedef struct {
    uint64_t id   : 58; // heapID
    uint64_t depth:  6; // log2(heapID)
} cbt_Node;

// Parallel binary tree
struct cbt_Tree {
    uint64_t *heap;
};

// Args for reading and writing to the two 64 bit words that bound the queries range
typedef struct {
    uint64_t *bitFieldLSB, *bitFieldMSB;
    int64_t bitOffsetLSB;
    int64_t bitCountLSB, bitCountMSB;
} cbt_HeapArgs;

class ConcurrentBinaryTree
{
public:
    // Create tree
    ConcurrentBinaryTree(int64_t maxDepth);
    // Release tree
    ~ConcurrentBinaryTree();

    // create / destroy tree
    void createAtDepth(int64_t maxDepth, int64_t depth);

    // loader
    void resetToDepth(int64_t depth);

    // manipulation
    void splitNode     (const cbt_Node node);
    void mergeNode     (const cbt_Node node);
    typedef void (*cbt_UpdateCallback)(const cbt_Node node,
                                       const void *userData);
    void update(cbt_UpdateCallback updater,
                    const void *userData);

    // O(1) queries
    int64_t getMaxDepth();
    int64_t getNodeCount();
    uint64_t getHeapRead(const cbt_Node node);
    bool isLeafNode(const cbt_Node node);
    bool isCeilNode(const cbt_Node node);
    bool isRootNode(const cbt_Node node);
    bool isNullNode(const cbt_Node node);

    // node constructors
    cbt_Node createNode           (uint64_t id, int64_t depth);
    cbt_Node parentNode           (const cbt_Node node);
    cbt_Node siblingNode          (const cbt_Node node);
    cbt_Node leftSiblingNode      (const cbt_Node node);
    cbt_Node rightSiblingNode     (const cbt_Node node);
    cbt_Node leftChildNode        (const cbt_Node node);
    cbt_Node rightChildNode       (const cbt_Node node);

    // O(depth) queries
    cbt_Node decodeNode(int64_t leafID);
    int64_t encodeNode(const cbt_Node node);

    // serialization
    int64_t heapByteSize();
    const char *getHeap();


private:
    // Helpers for heap management
    void clearBitField();
    uint64_t heapReadExplicit(const cbt_Node node, int64_t bitCount);
    cbt_HeapArgs createHeapArgs(const cbt_Node node, int64_t bitCount);
    inline int64_t heapUint64Size(int64_t treeMaxDepth);
    inline int64_t getLSB(uint64_t x);
    inline int64_t cbt_GetMSB(uint64_t x);
    inline uint64_t minValue(uint64_t a, uint64_t b);
    void heapWrite(const cbt_Node node, uint64_t bitData);
    void heapWriteExplicit(const cbt_Node node, int64_t bitCount, uint64_t bitData);
    void heapWrite_BitField(const cbt_Node node, const uint64_t bitValue);

    // Helper for reading and writing to bitfield
    void writeToBitField(const cbt_Node node, const uint64_t bitValue);
    int64_t nodeBitID_BitField(const cbt_Node node);
    inline int64_t nodeBitID(const cbt_Node node);
    inline int64_t nodeBitSize(const cbt_Node node);
    void setBitValue(uint64_t *bitField, int64_t bitID, uint64_t bitValue);
    inline uint64_t bitFieldExtract(const uint64_t bitField, int64_t bitOffset, int64_t bitCount);
    inline void bitFieldInsert( uint64_t *bitField, int64_t  bitOffset, int64_t  bitCount, uint64_t bitData);
    cbt_Node ceilNode(const cbt_Node node);

    void computeSumReduction();

    // Tree handle 
    cbt_Tree *m_tree;

};
