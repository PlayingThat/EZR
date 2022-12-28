//
// Created by maxbr on 19.12.2022.
//

#include "Engine/ConcurrentBinaryTree.h"

ConcurrentBinaryTree::ConcurrentBinaryTree(int64_t maxDepth, int64_t depth)
{
    createAtDepth(maxDepth, depth);
}

ConcurrentBinaryTree::~ConcurrentBinaryTree()
{
    free(m_tree->heap);
    free(m_tree);
}

// Setup tree buffer
void ConcurrentBinaryTree::createAtDepth(int64_t maxDepth, int64_t depth)
{
    if(maxDepth < 6)
        LOG_WARNING("maxDepth must be at least 6");
    if(maxDepth > 58)
        LOG_WARNING("maxDepth must be at most 58");  // (dont try, it'll break)
    m_tree = (cbt_Tree *)malloc(sizeof(*m_tree));

    int64_t bitfieldSize = 1LL << (maxDepth - 1);
    m_tree->heap = (uint64_t *)malloc(bitfieldSize);
    m_tree->heap[0] = 1ULL << (maxDepth); // store max Depth

    resetToDepth(depth);
}

// Initialize tree to specific subdivision level
void ConcurrentBinaryTree::resetToDepth(int64_t depth)
{
    if(depth < 0)
        LOG_WARNING("depth must be at least equal to 0");
    if(depth > getMaxDepth())
        LOG_WARNING("depth must be at most equal to maxDepth");
    uint64_t minNodeID = 1ULL << depth;
    uint64_t maxNodeID = 2ULL << depth;

    clearBitField();

    parallel_for(maxNodeID, [&](int start, int end) {
        for (uint64_t nodeID = minNodeID; nodeID < maxNodeID; ++nodeID) {
            cbt_Node node = createNode(nodeID, depth);

            writeToBitField(node, 1u);
        }
    });

    computeSumReduction();
}

// Splits the node in two
void ConcurrentBinaryTree::splitNode(const cbt_Node node)
{
    if (!isCeilNode(node))
        heapWrite_BitField(rightChildNode(node), 1u);
}

// Merges the node with its neighbour
void ConcurrentBinaryTree::mergeNode(const cbt_Node node)
{
    if (!isRootNode(node))
        heapWrite_BitField(rightSiblingNode(node), 0u);
}

// Traverse tree and split or merge nodes according to user function
void ConcurrentBinaryTree::update(cbt_UpdateCallback updater, const void *userData)
{
    parallel_for(getNodeCount(), [&](int start, int end) {
        for (int64_t handle = 0; handle < getNodeCount(); ++handle) {
            updater(decodeNode(handle), userData);
        }
    });

    computeSumReduction();
}

// Construct the node data structure
cbt_Node ConcurrentBinaryTree::createNode(uint64_t id, int64_t depth)
{
    cbt_Node node;

    node.id = id;
    node.depth = depth;

    return node;
}

// Returns the parent of the node
cbt_Node ConcurrentBinaryTree::parentNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else
        return createNode(node.id >> 1, node.depth - 1);
}

// Returns the sibling of the node
cbt_Node ConcurrentBinaryTree::siblingNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else
        return createNode(node.id ^ 1u, node.depth);
}

// Returns the left sibling of the node
cbt_Node ConcurrentBinaryTree::leftSiblingNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else
        return createNode(node.id & (~1u), node.depth);
}

// Returns the right sibling of the node
cbt_Node ConcurrentBinaryTree::rightSiblingNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else
        return createNode(node.id | 1u, node.depth);
}

// Returns the left child node of the node
cbt_Node ConcurrentBinaryTree::leftChildNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else
        return createNode(node.id << 1u, node.depth + 1);
}

// Returns the right child node of the node
cbt_Node ConcurrentBinaryTree::rightChildNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else
        return createNode(node.id << 1u | 1u, node.depth + 1);
}

// Returns the leaf node associated to the index
cbt_Node ConcurrentBinaryTree::decodeNode(int64_t handle)
{
    if(handle >= getNodeCount())
        LOG_WARNING("handle > NodeCount");
    if(handle < 0)
        LOG_WARNING("handle < 0");

    cbt_Node node = createNode(1u, 0);

    while (getHeapRead(node) > 1u) {
        cbt_Node heapNode = createNode(node.id<<= 1u, ++node.depth);
        uint64_t cmp = getHeapRead(heapNode);
        uint64_t b = (uint64_t)handle < cmp ? 0u : 1u;

        node.id|= b;
        handle-= cmp * b;
    }

    return node;
}

// Returns the bit index of the associated node
int64_t ConcurrentBinaryTree::encodeNode(const cbt_Node node)
{
    if(!isLeafNode(node))
        LOG_WARNING("node is not a leaf");

    int64_t handle = 0u;
    cbt_Node nodeIterator = node;

    while (nodeIterator.id > 1u) {
        cbt_Node sibling = leftSiblingNode(nodeIterator);
        uint64_t nodeCount = getHeapRead(sibling);

        handle+= (nodeIterator.id & 1u) * nodeCount;
        nodeIterator = parentNode(nodeIterator);
    }

    return handle;
}

// Returns number of bytes necessary for the bitfield
int64_t ConcurrentBinaryTree::heapByteSize()
{
    int64_t maxDepth = getMaxDepth();   
    return 1LL << (maxDepth - 1);
}

// Return pointer to CBT heap
const char *ConcurrentBinaryTree::getHeap()
{
    return (const char *)m_tree->heap;
}

// Initializes bitfiled with zeros
void ConcurrentBinaryTree::clearBitField()
{
    int64_t maxDepth = getMaxDepth();
    int64_t bufferMinID = 1LL << (maxDepth - 5);
    int64_t bufferMaxID = heapUint64Size(maxDepth);

    parallel_for(bufferMaxID, [&](int start, int end) {
        for (int bufferID = bufferMinID; bufferID < bufferMaxID; ++bufferID) {
            m_tree->heap[bufferID] = 0;
        }
    });
}

inline int64_t ConcurrentBinaryTree::heapUint64Size(int64_t treeMaxDepth) 
{
    int64_t bitFieldSize = 1LL << (treeMaxDepth - 1);
    return bitFieldSize >> 3;
}

// return max CBT depth
int64_t ConcurrentBinaryTree::getMaxDepth()
{
    return getLSB(m_tree->heap[0]);
}

// Returns number of nodes
int64_t ConcurrentBinaryTree::getNodeCount()
{
    return getHeapRead(createNode(1u, 0));
}

// Get least signifiacnt bit
inline int64_t ConcurrentBinaryTree::getLSB(uint64_t x)
{
    int64_t lsb = 0;

    while (((x >> lsb) & 1u) == 0u) {
        ++lsb;
    }

    return lsb;
}

// Get most significant bit
inline int64_t ConcurrentBinaryTree::cbt_GetMSB(uint64_t x)
{
    int64_t msb = 0;

    while (x > 1u) {
        ++msb;
        x = x >> 1;
    }

    return msb;
}

// Returns the minimum value between two inputs
inline uint64_t ConcurrentBinaryTree::minValue(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

void ConcurrentBinaryTree::setBitValue(uint64_t *bitField, int64_t bitID, uint64_t bitValue)
{
    const uint64_t bitMask = ~(1ULL << bitID);

    // atomic
    m_lock.lock();
    (*bitField)&= bitMask;
    (*bitField)|= (bitValue << bitID);
    m_lock.unlock();
}

// Inserts data in range [offset, offset + count - 1]
inline void ConcurrentBinaryTree::bitFieldInsert( uint64_t *bitField, int64_t  bitOffset, int64_t  bitCount, uint64_t bitData) {
    if(bitOffset >= 64 && bitCount >= 64 && bitOffset + bitCount > 64)
        LOG_WARNING("Bitfield access using out of bounds bit offset or count (CBT heap)");
    uint64_t bitMask = ~(~(0xFFFFFFFFFFFFFFFFULL << bitCount) << bitOffset);

    // atomic
    m_lock.lock();
    (*bitField)&= bitMask;
    (*bitField)|= (bitData << bitOffset);
    m_lock.unlock();
}

// BitFieldExtract -- Extracts bits [bitOffset, bitOffset + bitCount - 1] from a bitfield
inline uint64_t ConcurrentBinaryTree::bitFieldExtract(const uint64_t bitField, 
                                                          int64_t bitOffset, 
                                                          int64_t bitCount) {
    if(bitOffset >= 64 && bitCount >= 64 && bitOffset + bitCount > 64)
        LOG_WARNING("Bitfield access using out of bounds bit offset or count (CBT heap)");
    uint64_t bitMask = ~(0xFFFFFFFFFFFFFFFFULL << bitCount);

    return (bitField >> bitOffset) & bitMask;
}

void ConcurrentBinaryTree::writeToBitField(const cbt_Node node, const uint64_t bitValue) {
    int64_t bitID = nodeBitID_BitField(node);

    setBitValue(&m_tree->heap[bitID >> 6], bitID & 63, bitValue);
}

int64_t ConcurrentBinaryTree::nodeBitID_BitField(const cbt_Node node)
{
    return nodeBitID(ceilNode(node));
}

// Returns the number of bits storing the input node value
inline int64_t ConcurrentBinaryTree::nodeBitSize(const cbt_Node node)
{
    return getMaxDepth() - node.depth + 1;
}

// Returns the bit index that stores data associated with a given node
inline int64_t ConcurrentBinaryTree::nodeBitID(const cbt_Node node)
{
    int64_t tmp1 = 2LL << node.depth;
    int64_t tmp2 = 1LL + getMaxDepth() - node.depth;

    return tmp1 + node.id * tmp2;
}


cbt_Node ConcurrentBinaryTree::ceilNode(const cbt_Node node)
{
    if(isNullNode(node))
        return node;
    else {
        int64_t maxDepth = getMaxDepth();

        return createNode(node.id << (maxDepth - node.depth), maxDepth);
    }
}

bool ConcurrentBinaryTree::isNullNode(const cbt_Node node) 
{
    return (node.id == 0u);
}

bool ConcurrentBinaryTree::isRootNode(const cbt_Node node)
{
    return (node.id == 1u);
}

bool ConcurrentBinaryTree::isCeilNode(const cbt_Node node)
{
    return (node.depth == getMaxDepth());
}

bool ConcurrentBinaryTree::isLeafNode(const cbt_Node node)
{
    return (getHeapRead(node) == 1u);
}

// Reads data at node
uint64_t ConcurrentBinaryTree::getHeapRead(const cbt_Node node)
{
    return heapReadExplicit(node, nodeBitSize(node));
}

// Returns bit count bits located at node id
uint64_t ConcurrentBinaryTree::heapReadExplicit(const cbt_Node node, int64_t bitCount) {
    cbt_HeapArgs args = createHeapArgs(node, bitCount);
    uint64_t lsb = bitFieldExtract(*args.bitFieldLSB,
                                        args.bitOffsetLSB,
                                        args.bitCountLSB);
    uint64_t msb = bitFieldExtract(*args.bitFieldMSB,
                                        0u,
                                        args.bitCountMSB);

    return (lsb | (msb << args.bitCountLSB));
}

// Writes bitData to the node
void ConcurrentBinaryTree::heapWrite(const cbt_Node node, uint64_t bitData)
{
    heapWriteExplicit(node, nodeBitSize(node), bitData);
}

// Sets the bit associated to a leaf node to bitValue
void ConcurrentBinaryTree::heapWrite_BitField(const cbt_Node node, const uint64_t bitValue)
{
    int64_t bitID = nodeBitID_BitField(node);

    setBitValue(&m_tree->heap[bitID >> 6], bitID & 63, bitValue);
}

// Sets bit count bits located at node id to bitData
void ConcurrentBinaryTree::heapWriteExplicit(const cbt_Node node, int64_t bitCount, uint64_t bitData) {
    cbt_HeapArgs args = createHeapArgs(node, bitCount);

    bitFieldInsert(args.bitFieldLSB,
                        args.bitOffsetLSB,
                        args.bitCountLSB,
                        bitData);
    bitFieldInsert(args.bitFieldMSB,
                        0u,
                        args.bitCountMSB,
                        bitData >> args.bitCountLSB);
}

// Create heap args necessary for reading and writing to the two 64-bit words that bound the queries range 
cbt_HeapArgs ConcurrentBinaryTree::createHeapArgs(const cbt_Node node, int64_t bitCount)
{
    int64_t alignedBitOffset = nodeBitID(node);
    int64_t maxBufferIndex = heapUint64Size(getMaxDepth()) - 1;
    int64_t bufferIndexLSB = (alignedBitOffset >> 6);
    int64_t bufferIndexMSB = minValue(bufferIndexLSB + 1, maxBufferIndex);
    cbt_HeapArgs args;

    args.bitOffsetLSB = alignedBitOffset & 63;
    args.bitCountLSB = minValue(64 - args.bitOffsetLSB, bitCount);
    args.bitCountMSB = bitCount - args.bitCountLSB;
    args.bitFieldLSB = &m_tree->heap[bufferIndexLSB];
    args.bitFieldMSB = &m_tree->heap[bufferIndexMSB];

    return args;
}

void ConcurrentBinaryTree::computeSumReduction()
{
    int64_t depth = getMaxDepth();
    uint64_t minNodeID = (1ULL << depth);
    uint64_t maxNodeID = (2ULL << depth);

    // prepass: processes deepest levels in parallel
    parallel_for(maxNodeID, [&](int start, int end) {
        for (uint64_t nodeID = minNodeID; nodeID < maxNodeID; nodeID+= 64u) {
            cbt_Node heapNode = createNode(nodeID, depth);
            int64_t alignedBitOffset = nodeBitID(heapNode);
            uint64_t bitField = m_tree->heap[alignedBitOffset >> 6];
            uint64_t bitData = 0u;

            // 2-bits
            bitField = (bitField & 0x5555555555555555ULL)
                    + ((bitField >>  1) & 0x5555555555555555ULL);
            bitData = bitField;
            auto test = (alignedBitOffset - minNodeID) >> 6;
            m_tree->heap[test] = bitData;

            // 3-bits
            bitField = (bitField & 0x3333333333333333ULL)
                    + ((bitField >>  2) & 0x3333333333333333ULL);
            bitData = ((bitField >>  0) & (7ULL <<  0))
                    | ((bitField >>  1) & (7ULL <<  3))
                    | ((bitField >>  2) & (7ULL <<  6))
                    | ((bitField >>  3) & (7ULL <<  9))
                    | ((bitField >>  4) & (7ULL << 12))
                    | ((bitField >>  5) & (7ULL << 15))
                    | ((bitField >>  6) & (7ULL << 18))
                    | ((bitField >>  7) & (7ULL << 21))
                    | ((bitField >>  8) & (7ULL << 24))
                    | ((bitField >>  9) & (7ULL << 27))
                    | ((bitField >> 10) & (7ULL << 30))
                    | ((bitField >> 11) & (7ULL << 33))
                    | ((bitField >> 12) & (7ULL << 36))
                    | ((bitField >> 13) & (7ULL << 39))
                    | ((bitField >> 14) & (7ULL << 42))
                    | ((bitField >> 15) & (7ULL << 45));
            heapWriteExplicit(createNode(nodeID >> 2, depth - 2), 48ULL, bitData);

            // 4-bits
            bitField = (bitField & 0x0F0F0F0F0F0F0F0FULL)
                    + ((bitField >>  4) & 0x0F0F0F0F0F0F0F0FULL);
            bitData = ((bitField >>  0) & (15ULL <<  0))
                    | ((bitField >>  4) & (15ULL <<  4))
                    | ((bitField >>  8) & (15ULL <<  8))
                    | ((bitField >> 12) & (15ULL << 12))
                    | ((bitField >> 16) & (15ULL << 16))
                    | ((bitField >> 20) & (15ULL << 20))
                    | ((bitField >> 24) & (15ULL << 24))
                    | ((bitField >> 28) & (15ULL << 28));
            heapWriteExplicit(createNode(nodeID >> 3, depth - 3), 32ULL, bitData);

            // 5-bits
            bitField = (bitField & 0x00FF00FF00FF00FFULL)
                    + ((bitField >>  8) & 0x00FF00FF00FF00FFULL);
            bitData = ((bitField >>  0) & (31ULL <<  0))
                    | ((bitField >> 11) & (31ULL <<  5))
                    | ((bitField >> 22) & (31ULL << 10))
                    | ((bitField >> 33) & (31ULL << 15));
            heapWriteExplicit(createNode(nodeID >> 4, depth - 4), 20ULL, bitData);

            // 6-bits
            bitField = (bitField & 0x0000FFFF0000FFFFULL)
                    + ((bitField >> 16) & 0x0000FFFF0000FFFFULL);
            bitData = ((bitField >>  0) & (63ULL << 0))
                    | ((bitField >> 26) & (63ULL << 6));
            heapWriteExplicit(createNode(nodeID >> 5, depth - 5), 12ULL, bitData);

            // 7-bits
            bitField = (bitField & 0x00000000FFFFFFFFULL)
                    + ((bitField >> 32) & 0x00000000FFFFFFFFULL);
            bitData = bitField;
            heapWriteExplicit(createNode(nodeID >> 6, depth - 6),  7ULL, bitData);
        }
    });
    depth-= 6;

    // iterate over elements atomically
    while (--depth >= 0) {
        uint64_t minNodeID = 1ULL << depth;
        uint64_t maxNodeID = 2ULL << depth;

        parallel_for(maxNodeID, [&](int start, int end) {
            for (uint64_t j = minNodeID; j < maxNodeID; ++j) {
                uint64_t x0 = getHeapRead(createNode(j << 1    , depth + 1));
                uint64_t x1 = getHeapRead(createNode(j << 1 | 1, depth + 1));

                heapWrite(createNode(j, depth), x0 + x1);
            }
        });
    }
}