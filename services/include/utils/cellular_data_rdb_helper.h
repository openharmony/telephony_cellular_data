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
#include <regex>
#include <singleton.h>
#include <utility>

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "iservice_registry.h"
#include "pdp_profile_data.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "uri.h"

namespace OHOS {
namespace Telephony {
class CellularDataRdbHelper : public DelayedSingleton<CellularDataRdbHelper> {
    DECLARE_DELAYED_SINGLETON(CellularDataRdbHelper);

public:
    bool QueryApns(const std::string &mcc, const std::string &mnc, std::vector<PdpProfile> &apnVec, int32_t slotId);
    bool QueryMvnoApnsByType(const std::string &mcc, const std::string &mnc, const std::string &mvnoType,
        const std::string &mvnoDataFromSim, std::vector<PdpProfile> &mvnoApnVec, int32_t slotId);
    bool QueryPreferApn(int32_t slotId, std::vector<PdpProfile> &apnVec);
    void RegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    void UnRegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    bool ResetApns(int32_t slotId);

private:
    std::shared_ptr<DataShare::DataShareHelper> CreateDataAbilityHelper();
    int Update(const DataShare::DataShareValuesBucket &value, const DataShare::DataSharePredicates &predicates);
    int Insert(const DataShare::DataShareValuesBucket &values);
    void ReadApnResult(const std::shared_ptr<DataShare::DataShareResultSet> &result, std::vector<PdpProfile> &apnVec);
    void ReadMvnoApnResult(const std::shared_ptr<DataShare::DataShareResultSet> &result,
        const std::string &mvnoDataFromSim, std::vector<PdpProfile> &apnVec);
    bool IsMvnoDataMatched(const std::string &mvnoDataFromSim, const PdpProfile &apnBean);
    void MakePdpProfile(const std::shared_ptr<DataShare::DataShareResultSet> &result, int i, PdpProfile &apnBean);

private:
    Uri cellularDataUri_;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_RDB_HELPER_H
