/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */
#ifndef PARSER_ADMINSENTENCES_H_
#define PARSER_ADMINSENTENCES_H_

#include "parser/Clauses.h"
#include "parser/Sentence.h"
#include "parser/MutateSentences.h"
#include "common/network/NetworkUtils.h"
#include "common/interface/gen-cpp2/meta_types.h"

namespace nebula {

using nebula::network::NetworkUtils;

class ConfigRowItem;

class ShowHostsSentence : public Sentence {
public:
    ShowHostsSentence() {
        kind_ = Kind::kShowHosts;
    }
    std::string toString() const override;
};

class ShowSpacesSentence : public Sentence {
public:
    ShowSpacesSentence() {
        kind_ = Kind::kShowSpaces;
    }
    std::string toString() const override;
};

class ShowCreateSpaceSentence : public Sentence {
public:
    explicit ShowCreateSpaceSentence(std::string *name) {
        name_.reset(name);
        kind_ = Kind::kShowCreateSpace;
    }
    std::string toString() const override;

    const std::string* spaceName() const {
        return name_.get();
    }
private:
    std::unique_ptr<std::string>                name_;
};

class ShowPartsSentence : public Sentence {
public:
    ShowPartsSentence() {
        kind_ = Kind::kShowParts;
    }

    explicit ShowPartsSentence(std::vector<int32_t>* list) {
        list_.reset(list);
        kind_ = Kind::kShowParts;
    }

    std::vector<int32_t>* getList() const {
        return list_.get();
    }

    std::string toString() const override;

private:
    std::unique_ptr<std::vector<int32_t>> list_;
};

class ShowUsersSentence : public Sentence {
public:
    ShowUsersSentence() {
        kind_ = Kind::kShowUsers;
    }
    std::string toString() const override;
};

class ShowRolesSentence : public Sentence {
public:
    explicit ShowRolesSentence(std::string *name) {
        name_.reset(name);
        kind_ = Kind::kShowRoles;
    }

    std::string toString() const override;

    const std::string *name() const {
        return name_.get();
    }

private:
    std::unique_ptr<std::string>          name_;
};

class ShowSnapshotsSentence : public Sentence {
public:
    ShowSnapshotsSentence() {
        kind_ = Kind::kShowSnapshots;
    }
    std::string toString() const override;
};

class ShowCharsetSentence final : public Sentence {
public:
    ShowCharsetSentence() {
        kind_ = Kind::kShowCharset;
    }
    std::string toString() const override;
};

class ShowCollationSentence final : public Sentence {
public:
    ShowCollationSentence() {
        kind_ = Kind::kShowCollation;
    }
    std::string toString() const override;
};

class SpaceOptItem final {
public:
    using Value = boost::variant<int64_t, std::string>;

    enum OptionType : uint8_t {
        PARTITION_NUM,
        REPLICA_FACTOR,
        VID_SIZE,
        CHARSET,
        COLLATE
    };

    SpaceOptItem(OptionType op, std::string val) {
        optType_ = op;
        optValue_ = std::move(val);
    }

    SpaceOptItem(OptionType op, int64_t val) {
        optType_ = op;
        optValue_ = val;
    }

    int64_t asInt() const {
        return boost::get<int64_t>(optValue_);
    }

    const std::string& asString() const {
        return boost::get<std::string>(optValue_);
    }

    bool isInt() const {
        return optValue_.which() == 0;
    }

    bool isString() const {
        return optValue_.which() == 1;
    }

    int64_t getPartitionNum() const {
        if (isInt()) {
            return asInt();
        } else {
            LOG(ERROR) << "partition_num value illegal.";
            return 0;
        }
    }

    int64_t getReplicaFactor() const {
        if (isInt()) {
            return asInt();
        } else {
            LOG(ERROR) << "replica_factor value illegal.";
            return 0;
        }
    }

    int32_t getVidSize() const {
        if (isInt()) {
            return asInt();
        } else {
            LOG(ERROR) << "vid size illegal.";
            return 0;
        }
    }

    std::string getCharset() const {
        if (isString()) {
            return asString();
        } else {
            LOG(ERROR) << "charset value illegal.";
            return 0;
        }
    }

    std::string getCollate() const {
        if (isString()) {
            return asString();
        } else {
            LOG(ERROR) << "collate value illage.";
            return 0;
        }
    }

    OptionType getOptType() const {
        return optType_;
    }

    std::string toString() const;

private:
    Value           optValue_;
    OptionType      optType_;
};


class SpaceOptList final {
public:
    void addOpt(SpaceOptItem *item) {
        items_.emplace_back(item);
    }

    std::vector<SpaceOptItem*> getOpts() const {
        std::vector<SpaceOptItem*> result;
        result.resize(items_.size());
        auto get = [] (auto &ptr) { return ptr.get(); };
        std::transform(items_.begin(), items_.end(), result.begin(), get);
        return result;
    }

    std::string toString() const;

private:
    std::vector<std::unique_ptr<SpaceOptItem>>    items_;
};


class CreateSpaceSentence final : public CreateSentence {
public:
    explicit CreateSpaceSentence(std::string* spaceName, bool ifNotExist)
        : CreateSentence(ifNotExist) {
        spaceName_.reset(spaceName);
        kind_ = Kind::kCreateSpace;
    }

    const std::string* spaceName() const {
        return spaceName_.get();
    }

    void setOpts(SpaceOptList* spaceOpts) {
        spaceOpts_.reset(spaceOpts);
    }

    std::vector<SpaceOptItem*> getOpts() {
        if (spaceOpts_ == nullptr) {
            return {};
        }
        return spaceOpts_->getOpts();
    }

    std::string toString() const override;

private:
    std::unique_ptr<std::string>     spaceName_;
    std::unique_ptr<SpaceOptList>    spaceOpts_;
};


class DropSpaceSentence final : public DropSentence {
public:
    explicit DropSpaceSentence(std::string *spaceName, bool ifExist) : DropSentence(ifExist) {
        spaceName_.reset(spaceName);
        kind_ = Kind::kDropSpace;
    }

    const std::string* spaceName() const {
        return spaceName_.get();
    }

    std::string toString() const override;

private:
    std::unique_ptr<std::string>     spaceName_;
};


class DescribeSpaceSentence final : public Sentence {
public:
    explicit DescribeSpaceSentence(std::string *spaceName) {
        spaceName_.reset(spaceName);
        kind_ = Kind::kDescribeSpace;
    }

    const std::string* spaceName() const {
        return spaceName_.get();
    }

    std::string toString() const override;

private:
    std::unique_ptr<std::string>     spaceName_;
};

class ConfigRowItem {
public:
    explicit ConfigRowItem(meta::cpp2::ConfigModule module) {
        module_ = module;
    }

    ConfigRowItem(meta::cpp2::ConfigModule module, std::string* name, Expression* value) {
        module_ = module;
        name_.reset(name);
        value_.reset(value);
    }

    ConfigRowItem(meta::cpp2::ConfigModule module, std::string* name) {
        module_ = module;
        name_.reset(name);
    }

    ConfigRowItem(meta::cpp2::ConfigModule module, std::string* name, UpdateList *items) {
        module_ = module;
        name_.reset(name);
        updateItems_.reset(items);
    }

    meta::cpp2::ConfigModule getModule() const {
        return module_;
    }

    const std::string* getName() const {
        return name_.get();
    }

    Expression* getValue() const {
        return value_.get();
    }

    const UpdateList* getUpdateItems() const {
        return updateItems_.get();
    }

    std::string toString() const;

private:
    meta::cpp2::ConfigModule        module_;
    std::unique_ptr<std::string>    name_;
    std::unique_ptr<Expression>     value_;
    std::unique_ptr<UpdateList>     updateItems_;
};

class ConfigBaseSentence : public Sentence {
public:
    explicit ConfigBaseSentence(Kind kind, ConfigRowItem* item) {
        kind_ = kind;
        configItem_.reset(item);
    }

    ConfigRowItem* configItem() {
        return configItem_.get();
    }

protected:
    std::unique_ptr<ConfigRowItem>  configItem_;
};

class ShowConfigsSentence final : public ConfigBaseSentence {
public:
    explicit ShowConfigsSentence(ConfigRowItem* item)
        : ConfigBaseSentence(Kind::kShowConfigs, item) {}

    std::string toString() const override;
};

class SetConfigSentence final : public ConfigBaseSentence {
public:
    explicit SetConfigSentence(ConfigRowItem* item)
        : ConfigBaseSentence(Kind::kSetConfig, item) {}

    std::string toString() const override;
};

class GetConfigSentence final : public ConfigBaseSentence {
public:
    explicit GetConfigSentence(ConfigRowItem* item)
        : ConfigBaseSentence(Kind::kGetConfig, item) {}

    std::string toString() const override;
};

class HostList final {
public:
    void addHost(HostAddr *addr) {
        hosts_.emplace_back(addr);
    }

    std::string toString() const;

    std::vector<HostAddr> hosts() const {
        std::vector<HostAddr> result;
        result.reserve(hosts_.size());
        for (auto &host : hosts_) {
            result.emplace_back(*host);
        }
        return result;
    }

private:
    std::vector<std::unique_ptr<HostAddr>>      hosts_;
};

class BalanceSentence final : public Sentence {
public:
    enum class SubType : uint32_t {
        kUnknown,
        kLeader,
        kData,
        kDataStop,
        kShowBalancePlan,
    };

    // TODO: add more subtype for balance
    explicit BalanceSentence(SubType subType) {
        kind_ = Kind::kBalance;
        subType_ = std::move(subType);
    }

    explicit BalanceSentence(int64_t id) {
        kind_ = Kind::kBalance;
        subType_ = SubType::kShowBalancePlan;
        balanceId_ = id;
    }

    BalanceSentence(SubType subType, HostList *hostDel) {
        kind_ = Kind::kBalance;
        subType_ = std::move(subType);
        hostDel_.reset(hostDel);
    }

    std::string toString() const override;

    SubType subType() const {
        return subType_;
    }

    int64_t balanceId() const {
        return balanceId_;
    }

    HostList* hostDel() const {
        return hostDel_.get();
    }

private:
    SubType                         subType_{SubType::kUnknown};
    int64_t                         balanceId_{0};
    std::unique_ptr<HostList>       hostDel_;
};

class CreateSnapshotSentence final : public Sentence {
public:
    CreateSnapshotSentence() {
        kind_ = Kind::kCreateSnapshot;
    }

    std::string toString() const override;
};

class DropSnapshotSentence final : public Sentence {
public:
    explicit DropSnapshotSentence(std::string *name) {
        kind_ = Kind::kDropSnapshot;
        name_.reset(name);
    }

    const std::string* name() {
        return name_.get();
    }

    std::string toString() const override;

private:
    std::unique_ptr<std::string>    name_;
};

class AdminJobSentence final : public Sentence {
public:
    explicit AdminJobSentence(meta::cpp2::AdminJobOp op) : op_(op) {
        kind_ = Kind::kAdminJob;
    }

    void addPara(const std::string& para);
    std::string toString() const override;
    meta::cpp2::AdminJobOp getType() const;
    const std::vector<std::string> &getParas() const;

private:
    meta::cpp2::AdminJobOp   op_;
    std::vector<std::string> paras_;
};


}   // namespace nebula

#endif  // PARSER_ADMINSENTENCES_H_
