//
// Created by maxbr on 22.12.2022.
// Longest edge triangle bisection algorithm based on 
// https://math.aalto.fi/reports/a521.pdf and https://onrendering.com/data/papers/cbt/ConcurrentBinaryTrees.pdf
//

#pragma once

#include "Engine/ConcurrentBinaryTree.h"
#include <omp.h>

typedef struct {
    cbt_Node base, top;
} diamondParent;
diamondParent decodeDiamondParent       (const cbt_Node node);
diamondParent decodeDiamondParent_Square(const cbt_Node node);

// Custom 3x3 matrix type to allow for bigger transform matrices later
typedef float Matrix3x3[3][3];

class LongestEdgeBisection
{
public:
    LongestEdgeBisection();

    void decodeNodeAttributeArray(const cbt_Node node, int64_t attributeArraySize, float attributeArray[][3]);

private:
    float dotProduct(int64_t argSize, const float* x, const float* y);
    void decodeTransformationMatrix(const cbt_Node node, Matrix3x3 m);
    void identityMatrix3x3(Matrix3x3 m);
    void windingMatrix(Matrix3x3 matrix, uint64_t bitValue);
    void matrix3x3Product(const Matrix3x3 m1, const Matrix3x3 m2, Matrix3x3 out);
    void transposeMatrix3x3(const Matrix3x3 m, Matrix3x3 out);
    void splittingMatrix(Matrix3x3 matrix, uint64_t bitValue);
    inline uint64_t getBitValue(const uint64_t bitField, int64_t bitID);
};