/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CELLULAR_DATA_RDB_HELPER_H
#define CELLULAR_DATA_RDB_HELPER_H

#include <memory>
#include <singleton.h>
#include <utility>

#include "abs_shared_result_set.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "uri.h"
#include "values_bucket.h"

#include "pdp_profile_data.h"

namespace OHOS {
namespace Telephony {
class CellularDataRdbHelper : public DelayedSingleton<CellularDataRdbHelper> {
    DECLARE_DELAYED_SINGLETON(CellularDataRdbHelper);

public:
    bool QueryApns(const std::string &mcc, const std::string &mnc, std::vector<PdpProfile> &apnVec);
    void RegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    void UnRegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

private:
    std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateDataAbilityHelper();
    int Update(const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates);
    int Insert(const NativeRdb::ValuesBucket &values);
    void ReadApnResult(const std::shared_ptr<NativeRdb::AbsSharedResultSet> &result, std::vector<PdpProfile> &apnVec);

private:
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper_;
    Uri cellularDataUri_;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_RDB_HELPER_H
