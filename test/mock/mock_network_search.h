/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_NETWORK_SEARCH_H
#define MOCK_NETWORK_SEARCH_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "i_network_search.h"

namespace OHOS {
namespace Telephony {

class MockNetworkSearchManager : public INetworkSearch {
public:
    MockNetworkSearchManager() = default;
    virtual ~MockNetworkSearchManager() = default;

    MOCK_METHOD0(OnInit, bool());
    MOCK_METHOD1(InitTelExtraModule, int32_t(int32_t slotId));
    MOCK_METHOD2(GetPsRadioTech, int32_t(int32_t slotId, int32_t &psRadioTech));
    MOCK_METHOD2(GetCsRadioTech, int32_t(int32_t slotId, int32_t &csRadioTech));
    MOCK_METHOD1(GetOperatorNumeric, std::u16string(int32_t slotId));
    MOCK_METHOD1(GetResidentNetworkNumeric, std::string(int32_t slotId));
    MOCK_METHOD2(GetOperatorName, int32_t(int32_t slotId, std::u16string &operatorName));
    MOCK_METHOD2(GetNetworkStatus, int32_t(int32_t slotId, sptr<NetworkState> &networkState));
    MOCK_METHOD1(GetRadioState, int32_t(int32_t slotId));
    MOCK_METHOD2(GetRadioState, int32_t(int32_t slotId, NSCALLBACK &callback));
    MOCK_METHOD3(SetRadioState, void(int32_t slotId, bool isOn, int32_t rst));
    MOCK_METHOD4(SetRadioState, int32_t(int32_t slotId, bool isOn, int32_t rst, NSCALLBACK &callback));
    MOCK_METHOD2(GetSignalInfoList, int32_t(int32_t slotId, std::vector<sptr<SignalInformation>> &signals));
    MOCK_METHOD3(RegisterCoreNotify, void(int32_t slotId, HANDLE &handler, int32_t what));
    MOCK_METHOD3(UnRegisterCoreNotify, void(int32_t slotId, HANDLE &handler, int32_t what));
    MOCK_METHOD1(RegisterCellularDataObject, void(const sptr<NetworkSearchCallBackBase> &callback));
    MOCK_METHOD1(UnRegisterCellularDataObject, void(const sptr<NetworkSearchCallBackBase> &callback));
    MOCK_METHOD1(RegisterCellularCallObject, void(const sptr<NetworkSearchCallBackBase> &callback));
    MOCK_METHOD1(UnRegisterCellularCallObject, void(const sptr<NetworkSearchCallBackBase> &callback));
    MOCK_METHOD2(GetNetworkSearchInformation, int32_t(int32_t slotId, NSCALLBACK &callback));
    MOCK_METHOD2(GetNetworkSelectionMode, int32_t(int32_t slotId, NSCALLBACK &callback));
    MOCK_METHOD5(SetNetworkSelectionMode, int32_t(int32_t slotId, int32_t selectMode,
        const sptr<NetworkInformation> &networkInformation, bool resumeSelection, NSCALLBACK &callback));
    MOCK_METHOD2(GetIsoCountryCodeForNetwork, int32_t(int32_t slotId, std::u16string &countryCode));
    MOCK_METHOD2(GetPreferredNetwork, int32_t(int32_t slotId, NSCALLBACK &callback));
    MOCK_METHOD3(SetPreferredNetwork, int32_t(int32_t slotId, int32_t networkMode, NSCALLBACK &callback));
    MOCK_METHOD1(GetPsRegState, int32_t(int32_t slotId));
    MOCK_METHOD1(GetCsRegState, int32_t(int32_t slotId));
    MOCK_METHOD1(GetPsRoamingState, int32_t(int32_t slotId));
    MOCK_METHOD2(GetImei, int32_t(int32_t slotId, std::u16string &imei));
    MOCK_METHOD2(GetImeiSv, int32_t(int32_t slotId, std::u16string &imeiSv));
    MOCK_METHOD3(GetImsRegStatus, int32_t(int32_t slotId, ImsServiceType imsSrvType, ImsRegInfo &info));
    MOCK_METHOD2(GetCellInfoList, int32_t(int32_t slotId, std::vector<sptr<CellInformation>> &cellInfo));
    MOCK_METHOD1(SendUpdateCellLocationRequest, int32_t(int32_t slotId));
    MOCK_METHOD1(GetCellLocation, sptr<CellLocation>(int32_t slotId));
    MOCK_METHOD2(GetMeid, int32_t(int32_t slotId, std::u16string &meid));
    MOCK_METHOD2(GetUniqueDeviceId, int32_t(int32_t slotId, std::u16string &deviceId));
    MOCK_METHOD1(GetPhoneType, PhoneType(int32_t slotId));
    MOCK_METHOD3(SetNrOptionMode, int32_t(int32_t slotId, int32_t mode, NSCALLBACK &callback));
    MOCK_METHOD2(GetNrOptionMode, int32_t(int32_t slotId, NrMode &mode));
    MOCK_METHOD2(GetNrOptionMode, int32_t(int32_t slotId, NSCALLBACK &callback));
    MOCK_METHOD1(GetFrequencyType, FrequencyType(int32_t slotId));
    MOCK_METHOD1(GetNrState, NrState(int32_t slotId));
    MOCK_METHOD4(RegisterImsRegInfoCallback, int32_t(int32_t slotId, ImsServiceType imsSrvType, const int32_t tokenId,
        const sptr<ImsRegInfoCallback> &callback));
    MOCK_METHOD3(UnregisterImsRegInfoCallback, int32_t(int32_t slotId, ImsServiceType imsSrvType,
        const int32_t tokenId));
    MOCK_METHOD2(GetBasebandVersion, int32_t(int32_t slotId, std::string &version));
    MOCK_METHOD1(InitAirplaneMode, void(int32_t slotId));
    MOCK_METHOD1(GetAirplaneMode, int32_t(bool &airplaneMode));
    MOCK_METHOD3(GetNetworkCapability, int32_t(int32_t slotId, int32_t networkCapabilityType,
        int32_t &networkCapabilityState));
    MOCK_METHOD3(SetNetworkCapability, int32_t(int32_t slotId, int32_t networkCapabilityType,
        int32_t networkCapabilityState));
    MOCK_METHOD1(UpdateRadioOn, int32_t(int32_t slotId));
    MOCK_METHOD2(GetRrcConnectionState, int32_t(int32_t slotId, int32_t &status));
    MOCK_METHOD1(FactoryReset, int32_t(int32_t slotId));
    MOCK_METHOD2(GetNrSsbId, int32_t(int32_t slotId, const std::shared_ptr<NrSsbInformation> &nrSsbInformation));
    MOCK_METHOD1(IsNrSupported, bool(int32_t slotId));
    MOCK_METHOD0(IsSatelliteEnabled, bool());
    MOCK_METHOD2(DcPhysicalLinkActiveUpdate, void(int32_t slotId, bool isActive));
    MOCK_METHOD2(NotifyCallStatusToNetworkSearch, int32_t(int32_t slotId, int32_t callStatus));
    MOCK_METHOD2(HandleNotifyStateChangeWithDelay, int32_t(int32_t slotId, bool isNeedDelay));
    MOCK_METHOD1(StartRadioOnState, int32_t(int32_t slotId));
    MOCK_METHOD1(StartGetRilSignalIntensity, int32_t(int32_t slotId));
    MOCK_METHOD2(ProcessSignalIntensity, int32_t(int32_t slotId, const struct Rssi &signalIntensity));
    MOCK_METHOD2(IsGsm, int32_t(int32_t slotId, bool &isGsm));
    MOCK_METHOD2(IsCdma, int32_t(int32_t slotId, bool &isCdma));
};
}  // namespace Telephony
}  // namespace OHOS
#endif  // MOCK_NETWORK_SEARCH_H