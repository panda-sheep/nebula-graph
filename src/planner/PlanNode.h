/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef PLANNER_PLANNODE_H_
#define PLANNER_PLANNODE_H_

#include "common/base/Base.h"
#include "common/expression/Expression.h"

namespace nebula {
namespace graph {

namespace cpp2 {
class PlanNodeDescription;
}   // namespace cpp2

/**
 * PlanNode is an abstraction of nodes in an execution plan which
 * is a kind of directed cyclic graph.
 */
class PlanNode {
public:
    enum class Kind : uint8_t {
        kUnknown = 0,
        kStart,
        kGetNeighbors,
        kGetVertices,
        kGetEdges,
        kIndexScan,
        kFilter,
        kUnion,
        kIntersect,
        kMinus,
        kProject,
        kSort,
        kLimit,
        kAggregate,
        kSelect,
        kLoop,
        kSwitchSpace,
        kDedup,
        kPassThrough,
        kCreateSpace,
        kCreateTag,
        kCreateEdge,
        kDescSpace,
        kShowCreateSpace,
        kDescTag,
        kDescEdge,
        kAlterTag,
        kAlterEdge,
        kShowSpaces,
        kShowTags,
        kShowEdges,
        kShowCreateTag,
        kShowCreateEdge,
        kDropSpace,
        kDropTag,
        kDropEdge,
        kInsertVertices,
        kInsertEdges,
        kBalanceLeaders,
        kBalance,
        kStopBalance,
        kShowBalance,
        kSubmitJob,
        kShowHosts,
        kDataCollect,
        // user related
        kCreateUser,
        kDropUser,
        kUpdateUser,
        kGrantRole,
        kRevokeRole,
        kChangePassword,
        kListUserRoles,
        kListUsers,
        kListRoles,
        kCreateSnapshot,
        kDropSnapshot,
        kShowSnapshots,
        kDataJoin,
        kDeleteVertices,
        kDeleteEdges,
        kUpdateVertex,
        kUpdateEdge,
        kShowParts,
        kShowCharset,
        kShowCollation,
        kShowConfigs,
        kSetConfig,
        kGetConfig,
    };

    PlanNode(int64_t id, Kind kind);

    virtual ~PlanNode() = default;

    // Describe plan node
    virtual std::unique_ptr<cpp2::PlanNodeDescription> explain() const;

    Kind kind() const {
        return kind_;
    }

    int64_t id() const {
        return id_;
    }

    void setOutputVar(std::string var) {
        outputVar_ = std::move(var);
    }

    std::string varName() const {
        return outputVar_;
    }

    std::vector<std::string> colNames() const {
        return colNames_;
    }

    const std::vector<std::string>& colNamesRef() const {
        return colNames_;
    }

    void setId(int64_t id) {
        id_ = id;
    }

    void setColNames(std::vector<std::string>&& cols) {
        colNames_ = std::move(cols);
    }

    void setColNames(const std::vector<std::string>& cols) {
        colNames_ = cols;
    }

    const PlanNode* dep(size_t index = 0) const {
        DCHECK_LT(index, dependencies_.size());
        return dependencies_.at(index);
    }

    void setDep(size_t index, const PlanNode* dep) {
        DCHECK_LT(index, dependencies_.size());
        dependencies_[index] = DCHECK_NOTNULL(dep);
    }

    const std::vector<const PlanNode*>& dependencies() const {
        return dependencies_;
    }

    static const char* toString(Kind kind);

protected:
    static void addDescription(std::string key, std::string value, cpp2::PlanNodeDescription* desc);

    Kind                                     kind_{Kind::kUnknown};
    int64_t                                  id_{-1};
    std::string                              outputVar_;
    std::vector<std::string>                 colNames_;
    std::vector<const PlanNode*>             dependencies_;
};

std::ostream& operator<<(std::ostream& os, PlanNode::Kind kind);

// Dependencies will cover the inputs, For example bi input require bi dependencies as least,
// but single dependencies may don't need any inputs (I.E admin plan node)
// Single dependecy without input
// It's useful for admin plan node
class SingleDependencyNode : public PlanNode {
public:
    void dependsOn(const PlanNode* dep) {
        setDep(0, dep);
    }

protected:
    SingleDependencyNode(int64_t id, Kind kind, const PlanNode* dep)
        : PlanNode(id, kind) {
        dependencies_.emplace_back(dep);
    }

    std::unique_ptr<cpp2::PlanNodeDescription> explain() const override;
};

class SingleInputNode : public SingleDependencyNode {
public:
    void setInputVar(std::string inputVar) {
        inputVar_ = std::move(inputVar);
    }

    const std::string& inputVar() const {
        return inputVar_;
    }

    std::unique_ptr<cpp2::PlanNodeDescription> explain() const override;

protected:
    SingleInputNode(int64_t id, Kind kind, const PlanNode* dep)
        : SingleDependencyNode(id, kind, dep) {
    }

    // Datasource for this node.
    std::string inputVar_;
};

class BiInputNode : public PlanNode {
public:
    void setLeft(const PlanNode* left) {
        setDep(0, left);
    }

    void setRight(const PlanNode* right) {
        setDep(1, right);
    }

    void setLeftVar(std::string leftVar) {
        leftVar_ = std::move(leftVar);
    }

    void setRightVar(std::string rightVar) {
        rightVar_ = std::move(rightVar);
    }

    const PlanNode* left() const {
        return dep(0);
    }

    const PlanNode* right() const {
        return dep(1);
    }

    const std::string& leftInputVar() const {
        return leftVar_;
    }

    const std::string& rightInputVar() const {
        return rightVar_;
    }

    std::unique_ptr<cpp2::PlanNodeDescription> explain() const override;

protected:
    BiInputNode(int64_t id, Kind kind, PlanNode* left, PlanNode* right)
        : PlanNode(id, kind) {
        dependencies_.emplace_back(DCHECK_NOTNULL(left));
        dependencies_.emplace_back(DCHECK_NOTNULL(right));
    }

    // Datasource for this node.
    std::string leftVar_;
    std::string rightVar_;
};

}  // namespace graph
}  // namespace nebula

#endif  // PLANNER_PLANNODE_H_
