 
//
// Created by maxbr on 22.12.2022.
//

#include "Engine/LongestEdgeBisection.h"

LongestEdgeBisection::LongestEdgeBisection() 
{
    //
}

// Decodes node attributes
void LongestEdgeBisection::decodeNodeAttributeArray(const cbt_Node node, int64_t attributeArraySize, float attributeArray[][3])
{
    if(!attributeArraySize > 0);
        LOG_WARNING("Attribute array size is 0. Nothing to decode.");

    Matrix3x3 m;
    float attributeVector[3];

    decodeTransformationMatrix(node, m);

    for (int64_t i = 0; i < attributeArraySize; ++i) {
        memcpy(attributeVector, attributeArray[i], sizeof(attributeVector));
        attributeArray[i][0] = dotProduct(3, m[0], attributeVector);
        attributeArray[i][1] = dotProduct(3, m[1], attributeVector);
        attributeArray[i][2] = dotProduct(3, m[2], attributeVector);
    }
}

// Computes longest edge bisection matrix for a given node
void LongestEdgeBisection::decodeTransformationMatrix(const cbt_Node node, Matrix3x3 matrix) {
    identityMatrix3x3(matrix);

    for (int64_t bitID = node.depth - 1; bitID >= 0; --bitID) {
        splittingMatrix(matrix, getBitValue(node.id, bitID));
    }

    windingMatrix(matrix, node.depth & 1);
}

// Computes dot product of two vectors
float LongestEdgeBisection::dotProduct(int64_t argSize, const float *x, const float *y)
{
    float dp = 0.0f;

    for (int64_t i = 0; i < argSize; ++i)
        dp+= x[i] * y[i];

    return dp;
}

// Return identity matrix
void LongestEdgeBisection::identityMatrix3x3(Matrix3x3 m)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

// Compute mirror matrix
void LongestEdgeBisection::windingMatrix(Matrix3x3 matrix, uint64_t bitValue)
{
    float b = (float)bitValue;
    float c = 1.0f - b;
    Matrix3x3 windingMatrix = {
        {c, 0.0f, b},
        {0, 1.0f, 0},
        {b, 0.0f, c}
    };
    Matrix3x3 tmp;

    memcpy(tmp, matrix, sizeof(tmp));
    matrix3x3Product(windingMatrix, tmp, matrix);
}

// Compute product of two 3x3 matrices
void LongestEdgeBisection::matrix3x3Product(const Matrix3x3 m1, const Matrix3x3 m2, Matrix3x3 out)
{
    Matrix3x3 tra;

    transposeMatrix3x3(m2, tra);

    for (int64_t j = 0; j < 3; ++j)
    for (int64_t i = 0; i < 3; ++i)
        out[j][i] = dotProduct(3, m1[j], tra[i]);
}

void LongestEdgeBisection::transposeMatrix3x3(const Matrix3x3 m, Matrix3x3 out)
{
    for (int64_t i = 0; i < 3; ++i)
    for (int64_t j = 0; j < 3; ++j)
        out[i][j] = m[j][i];
}

// Compute splitting matrix for longest edge bisection
void LongestEdgeBisection::splittingMatrix(Matrix3x3 matrix, uint64_t bitValue)
{
    float b = (float)bitValue;
    float c = 1.0f - b;
    Matrix3x3 splitMatrix = {
        {c   , b   , 0.0f},
        {0.5f, 0.0f, 0.5f},
        {0.0f,    c,    b}
    };
    Matrix3x3 tmp;

    memcpy(tmp, matrix, sizeof(tmp));
    matrix3x3Product(splitMatrix, tmp, matrix);
}

// Returns the value of a bit stored in 64 bit word for concurrent binary tree decoding
inline uint64_t LongestEdgeBisection::getBitValue(const uint64_t bitField, int64_t bitID)
{
    return ((bitField >> bitID) & 1u);
}