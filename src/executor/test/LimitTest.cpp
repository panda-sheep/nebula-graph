/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include <gtest/gtest.h>

#include "context/QueryContext.h"
#include "executor/query/LimitExecutor.h"
#include "executor/query/ProjectExecutor.h"
#include "executor/test/QueryTestBase.h"
#include "planner/Logic.h"
#include "planner/Query.h"
#include "util/ExpressionUtils.h"

namespace nebula {
namespace graph {
class LimitTest : public QueryTestBase {
};

#define LIMIT_RESUTL_CHECK(outputName, offset, count, expected)                                    \
    do {                                                                                           \
        auto start = StartNode::make(qctx_.get());                                                 \
        auto* limitNode = Limit::make(qctx_.get(), start, offset, count);                          \
        limitNode->setInputVar("input_neighbor");                                                  \
        limitNode->setOutputVar(outputName);                                                       \
        auto limitExec = Executor::create(limitNode, qctx_.get());                                 \
        EXPECT_TRUE(limitExec->execute().get().ok());                                              \
        auto& limitResult = qctx_->ectx()->getResult(limitNode->varName());                        \
        EXPECT_EQ(limitResult.state(), Result::State::kSuccess);                                   \
        auto yieldSentence =                                                                       \
            getYieldSentence("YIELD study._dst AS name, study.start_year AS start");               \
        auto columns = yieldSentence->columns();                                                   \
        for (auto& col : columns) {                                                                \
            if (col->expr()->kind() == Expression::Kind::kLabelAttribute) {                        \
                auto laExpr = static_cast<LabelAttributeExpression*>(col->expr());                 \
                col->setExpr(                                                                      \
                    ExpressionUtils::rewriteLabelAttribute<EdgePropertyExpression>(laExpr));       \
            } else {                                                                               \
                ExpressionUtils::rewriteLabelAttribute<EdgePropertyExpression>(col->expr());       \
            }                                                                                      \
        }                                                                                          \
        auto* project = Project::make(qctx_.get(), start, yieldSentence->yieldColumns());          \
        project->setInputVar(limitNode->varName());                                                \
        project->setColNames(std::vector<std::string>{"name", "start"});                           \
        auto proExe = Executor::create(project, qctx_.get());                                      \
        EXPECT_TRUE(proExe->execute().get().ok());                                                 \
        auto& proResult = qctx_->ectx()->getResult(project->varName());                            \
        EXPECT_EQ(proResult.value().getDataSet(), expected);                                       \
        EXPECT_EQ(proResult.state(), Result::State::kSuccess);                                     \
    } while (false)

TEST_F(LimitTest, getNeighborInRange1) {
    DataSet expected({"name", "start"});
    expected.emplace_back(Row({Value("School2"), Value(2014)}));
    expected.emplace_back(Row({Value("School1"), Value(2008)}));
    LIMIT_RESUTL_CHECK("limit_in_neighbor1", 1, 2, expected);
}

TEST_F(LimitTest, getNeighborInRange2) {
    DataSet expected({"name", "start"});
    expected.emplace_back(Row({Value("School1"), Value(2010)}));
    expected.emplace_back(Row({Value("School2"), Value(2014)}));
    expected.emplace_back(Row({Value("School1"), Value(2008)}));
    expected.emplace_back(Row({Value("School2"), Value(2012)}));
    LIMIT_RESUTL_CHECK("limit_in_neighbor2", 0, 4, expected);
}

TEST_F(LimitTest, getNeighborOutRange1) {
    DataSet expected({"name", "start"});
    expected.emplace_back(Row({Value("School1"), Value(2010)}));
    expected.emplace_back(Row({Value("School2"), Value(2014)}));
    expected.emplace_back(Row({Value("School1"), Value(2008)}));
    expected.emplace_back(Row({Value("School2"), Value(2012)}));
    LIMIT_RESUTL_CHECK("limit_out_neighbor1", 0, 5, expected);
}

TEST_F(LimitTest, getNeighborOutRange2) {
    DataSet expected({"name", "start"});
    LIMIT_RESUTL_CHECK("limit_out_neighbor1", 4, 2, expected);
}
}  // namespace graph
}  // namespace nebula
