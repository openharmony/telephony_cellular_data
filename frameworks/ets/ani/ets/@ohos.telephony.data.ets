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
        callback(new BusinessError(), (content as int) as DataConnectState);
    }, (err: BusinessError): void => {
        callback(err, DataConnectState.DATA_STATE_UNKNOWN);
    });
  }

  export function getCellularDataState(): Promise<DataConnectState> {
    return new Promise<DataConnectState>((resolve, reject) => {
      let p = taskpool.execute(nativeGetCellularDataState)
      p.then((e: NullishType) => {
          let r = (e as int) as DataConnectState
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
    }, (err: BusinessError): void => {
        callback(err, false);
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
    }, (err: BusinessError): void => {
        callback(err, undefined);
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
    }, (err: BusinessError): void => {
        callback(err, undefined);
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
}
