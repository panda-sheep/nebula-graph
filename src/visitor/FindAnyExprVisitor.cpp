/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "visitor/FindAnyExprVisitor.h"

namespace nebula {
namespace graph {

FindAnyExprVisitor::FindAnyExprVisitor(const std::unordered_set<Expression::Kind> &kinds)
    : kinds_(kinds) {
    DCHECK(!kinds.empty());
}

void FindAnyExprVisitor::visit(TypeCastingExpression *expr) {
    findExpr(expr);
    if (found_) return;
    expr->operand()->accept(this);
}

void FindAnyExprVisitor::visit(UnaryExpression *expr) {
    findExpr(expr);
    if (found_) return;
    expr->operand()->accept(this);
}

void FindAnyExprVisitor::visit(FunctionCallExpression *expr) {
    findExpr(expr);
    if (found_) return;
    for (const auto &arg : expr->args()->args()) {
        arg->accept(this);
        if (found_) return;
    }
}

void FindAnyExprVisitor::visit(ListExpression *expr) {
    findExpr(expr);
    if (found_) return;
    for (const auto &item : expr->items()) {
        const_cast<Expression *>(item)->accept(this);
        if (found_) return;
    }
}

void FindAnyExprVisitor::visit(SetExpression *expr) {
    findExpr(expr);
    if (found_) return;
    for (const auto &item : expr->items()) {
        const_cast<Expression *>(item)->accept(this);
        if (found_) return;
    }
}

void FindAnyExprVisitor::visit(MapExpression *expr) {
    findExpr(expr);
    if (found_) return;
    for (const auto &pair : expr->items()) {
        const_cast<Expression *>(pair.second)->accept(this);
        if (found_) return;
    }
}

void FindAnyExprVisitor::visit(ConstantExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(EdgePropertyExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(TagPropertyExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(InputPropertyExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(VariablePropertyExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(SourcePropertyExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(DestPropertyExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(EdgeSrcIdExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(EdgeTypeExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(EdgeRankExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(EdgeDstIdExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(UUIDExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(VariableExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(VersionedVariableExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(LabelExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(VertexExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visit(EdgeExpression *expr) {
    findExpr(expr);
}

void FindAnyExprVisitor::visitBinaryExpr(BinaryExpression *expr) {
    findExpr(expr);
    if (found_) return;
    expr->left()->accept(this);
    if (found_) return;
    expr->right()->accept(this);
}

void FindAnyExprVisitor::findExpr(const Expression *expr) {
    found_ = kinds_.find(expr->kind()) != kinds_.cend();
    if (found_) {
        expr_ = expr;
    }
}

}   // namespace graph
}   // namespace nebula
