/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base'
import type { AsyncCallback } from '@ohos.base';

export default namespace data {
  loadLibrary("cellular_data_ani")

  export native function nativeIsCellularDataEnabled(): boolean;

  export native function nativeEnableCellularData(): void;

  export native function nativeDisableCellularData(): void;

  export native function nativeGetDefaultCellularDataSlotIdSync(): int;

  export native function nativeGetCellularDataState(): DataConnectState;

  export function getDefaultCellularDataSlotIdSync(): int {
    return nativeGetDefaultCellularDataSlotIdSync()
  }

  export function getCellularDataState(callback: AsyncCallback<DataConnectState>): void {
    let p1 = taskpool.execute(nativeGetCellularDataState)
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as DataConnectState);
    }, (err: BusinessError): void => {
        callback(err, DataConnectState.DATA_STATE_UNKNOWN);
    });
  }

  export function getCellularDataState(): Promise<DataConnectState> {
    return new Promise<DataConnectState>((resolve, reject) => {
      let p = taskpool.execute(nativeGetCellularDataState)
      p.then((e: NullishType) => {
          let r = e as DataConnectState
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export function isCellularDataEnabled(callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute(nativeIsCellularDataEnabled)
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
        callback(err as BusinessError, false);
    });
  }

  export function isCellularDataEnabled(): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      let p = taskpool.execute(nativeIsCellularDataEnabled)
      p.then((e: NullishType) => {
          let r = e as boolean
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export function enableCellularData(callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute(nativeEnableCellularData)
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }

  export function enableCellularData(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let p = taskpool.execute(nativeEnableCellularData)
      p.then(() => {
          resolve(undefined)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export function disableCellularData(callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute(nativeDisableCellularData)
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }


  export function disableCellularData(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let p = taskpool.execute(nativeDisableCellularData)
      p.then(() => {
          resolve(undefined)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }


  export enum DataFlowType {
    DATA_FLOW_TYPE_NONE = 0,

    DATA_FLOW_TYPE_DOWN = 1,

    DATA_FLOW_TYPE_UP = 2,

    DATA_FLOW_TYPE_UP_DOWN = 3,

    DATA_FLOW_TYPE_DORMANT = 4
  }

  export enum DataConnectState {
    DATA_STATE_UNKNOWN = -1,

    DATA_STATE_DISCONNECTED = 0,

    DATA_STATE_CONNECTING = 1,

    DATA_STATE_CONNECTED = 2,

    DATA_STATE_SUSPENDED = 3
  }

  export native function nativeDisableCellularDataRoaming(slotId: int): void;

  export function disableCellularDataRoaming(slotId: int, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
      return nativeDisableCellularDataRoaming(slotId);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }


  export function disableCellularDataRoaming(slotId: int): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let p1 = taskpool.execute((): undefined => {
        return nativeDisableCellularDataRoaming(slotId);
      })
      p1.then(() => {
          resolve(undefined)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export native function nativeEnableCellularDataRoaming(slotId: int): void;

  export function enableCellularDataRoaming(slotId: int, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
      return nativeEnableCellularDataRoaming(slotId);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }


  export function enableCellularDataRoaming(slotId: int): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let p1 = taskpool.execute((): undefined => {
        return nativeEnableCellularDataRoaming(slotId);
      })
      p1.then(() => {
          resolve(undefined)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export native function nativeIsCellularDataRoamingEnabled(slotId: int): boolean;

  export function isCellularDataRoamingEnabled(slotId: int, callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
      return nativeIsCellularDataRoamingEnabled(slotId);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
        callback(err as BusinessError, false);
    });
  }

  export function isCellularDataRoamingEnabled(slotId: int): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      let p1 = taskpool.execute((): boolean => {
        return nativeIsCellularDataRoamingEnabled(slotId);
      })
      p1.then((e: NullishType) => {
          let r = e as boolean
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export native function nativeSetDefaultCellularDataSlotId(slotId: int): void;

  export function setDefaultCellularDataSlotId(slotId: int, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
      return nativeSetDefaultCellularDataSlotId(slotId);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }

  export function setDefaultCellularDataSlotId(slotId: int): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let p1 = taskpool.execute((): undefined => {
        return nativeSetDefaultCellularDataSlotId(slotId);
      })
      p1.then(() => {
          resolve(undefined)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export native function nativeGetCellularDataFlowType(): DataFlowType;

  export function getCellularDataFlowType(callback: AsyncCallback<DataFlowType>): void {
    let p1 = taskpool.execute((): DataFlowType => {
      return nativeGetCellularDataFlowType();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as DataFlowType);
    }, (err: BusinessError): void => {
        callback(err, DataFlowType.DATA_FLOW_TYPE_NONE);
    });
  }
  export function getCellularDataFlowType(): Promise<DataFlowType> {
    return new Promise<DataFlowType>((resolve, reject) => {
      let p = taskpool.execute((): DataFlowType => {
        return nativeGetCellularDataFlowType();
      })
      p.then((e: NullishType) => {
          let r = e as DataFlowType
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }


  export function getDefaultCellularDataSlotId(callback: AsyncCallback<int>): void {
    let p1 = taskpool.execute((): int => {
      return nativeGetDefaultCellularDataSlotIdSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as int);
    }, (err: Error): void => {
        callback(err as BusinessError, 0);
    });
  }
  export function getDefaultCellularDataSlotId(): Promise<int> {
    return new Promise<int>((resolve, reject) => {
        taskpool.execute((): int => {
        return nativeGetDefaultCellularDataSlotIdSync();
      }).then((content: NullishType) => {
        resolve(content as int);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function nativeSetPreferredApn(apnId: int): boolean;

  export function setPreferredApn(apnId: int): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      let p1 = taskpool.execute((): boolean => {
        return nativeSetPreferredApn(apnId);
      })
      p1.then((e: NullishType) => {
          let r = e as boolean
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  interface ApnInfo {
    apnName: string;
    apn: string;
    mcc: string;
    mnc: string;
    user?: string;
    type?: string;
    proxy?: string;
    mmsproxy?: string;
  }

  export native function nativeQueryApnIds(apnInfo: ApnInfo): Array<int>;
  export function queryApnIds(apnInfo: ApnInfo): Promise<Array<int>> {
    return new Promise<Array<int>>((resolve, reject) => {
      let p1 = taskpool.execute((): Array<int> => {
        return nativeQueryApnIds(apnInfo);
      })
      p1.then((e: NullishType) => {
          let r = e as Array<int>
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export native function nativeQueryAllApns(): Array<ApnInfo>;
  export function queryAllApns(): Promise<Array<ApnInfo>> {
    return new Promise<Array<ApnInfo>>((resolve, reject) => {
      let p1 = taskpool.execute((): Array<ApnInfo> => {
        return nativeQueryAllApns();
      })
      p1.then((e: NullishType) => {
          let r = e as Array<ApnInfo>
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export native function nativeGetActiveApnName(): string;

  export function getActiveApnName(): Promise<string> {
    return new Promise<string>((resolve, reject) => {
      let p1 = taskpool.execute((): string => {
        return nativeGetActiveApnName();
      })
      p1.then((e: NullishType) => {
          let r = e as string
          resolve(r)
      }).catch((e: Error): void => {
          reject(e)
      })
    })
  }

  export function isCellularDataRoamingEnabledSync(slotId: int): boolean {
    return nativeIsCellularDataRoamingEnabled(slotId);
  }


  export function isCellularDataEnabledSync(): boolean {
    return nativeIsCellularDataEnabled();
  }

  export native function nativeGetDefaultCellularDataSimId(): int;
  export function getDefaultCellularDataSimId(): int {
    return nativeGetDefaultCellularDataSimId();
  }
}
