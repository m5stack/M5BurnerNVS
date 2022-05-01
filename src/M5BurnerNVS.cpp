// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include <Arduino.h>

#include <Preferences.h>
#include "M5BurnerNVS.h"

std::vector<String> subscribeKeys;

void M5BurnerNVS::init() {
    Preferences pref;
    if (!pref.begin(BURNER_NVS_NAMESPACE, true)) {
        Serial.println(ERR_NVS_NOT_FOUND);
        return;
    }

    if (!Serial || Serial.baudRate() != 115200) {
        Serial.begin(115200);
    }

    initialized = true;
}

void M5BurnerNVS::_listen(void* pvParameters) {
    PTASK_CONTEXT* context = (PTASK_CONTEXT*)pvParameters;
    Preferences pref;
    bool exist = pref.begin(BURNER_NVS_NAMESPACE, true);

    String comData = "";
    while (true) {
        if (!(*context->isListening)) {
            vTaskDelete(NULL);
            break;
        }

        if (Serial.available()) {
            int b = Serial.read();
            comData += (char)b;

            switch (*context->commandMatchState) {
                case COMMAND_MATCH_STATE_IDLE:

                {
                    if (!comData.startsWith("C")) {
                        comData = "";
                        break;
                    }

                    while (true) {
                        int next = Serial.peek();
                        if (next > -1) {
                            if ((comData + (char)next)
                                    .endsWith(COMMAND_MATCH_STR_CMD_END)) {
                                if (comData.startsWith(
                                        COMMAND_MATCH_STR_LIST)) {
                                    *context->commandMatchState =
                                        COMMAND_MATCH_STATE_LIST;
                                    break;
                                } else if (comData.startsWith(
                                               COMMAND_MATCH_STR_SET)) {
                                    *context->commandMatchState =
                                        COMMAND_MATCH_STATE_SET;
                                    break;
                                } else if (comData.startsWith(
                                               COMMAND_MATCH_STR_GET)) {
                                    *context->commandMatchState =
                                        COMMAND_MATCH_STATE_GET;
                                    break;
                                } else if (comData.startsWith(
                                               COMMAND_MATCH_STR_INIT)) {
                                    *context->commandMatchState =
                                        COMMAND_MATCH_STATE_INIT;
                                    break;
                                } else if (comData.startsWith(
                                               COMMAND_MATCH_STR_SUB)) {
                                    *context->commandMatchState =
                                        COMMAND_MATCH_STATE_SUB;
                                    break;
                                } else if (comData.startsWith(
                                               COMMAND_MATCH_STR_UNSUB)) {
                                    *context->commandMatchState =
                                        COMMAND_MATCH_STATE_UNSUB;
                                    break;
                                } else {
                                    Serial.read();
                                    comData = "";
                                    break;
                                }
                            } else {
                                int n = Serial.read();
                                comData += (char)n;
                            }
                        }
                        vTaskDelay(1 / portTICK_RATE_MS);
                    }

                    break;

                    break;
                }

                case COMMAND_MATCH_STATE_INIT:

                {
                    if (!exist) {
                        Serial.println(ERR_NVS_NOT_FOUND);
                    } else {
                        Serial.println(INF_NVS_EXIST);
                        subscribeKeys.clear();
                    }

                    comData                     = "";
                    *context->commandMatchState = COMMAND_MATCH_STATE_IDLE;

                    break;
                }

                case COMMAND_MATCH_STATE_LIST:

                {
                    String list = get(BURNER_NVS_KEYS_FIELD);
                    Serial.println(list);

                    comData                     = "";
                    *context->commandMatchState = COMMAND_MATCH_STATE_IDLE;

                    break;
                }

                case COMMAND_MATCH_STATE_SET:

                {
                    comData.replace(COMMAND_MATCH_STR_SET, "");
                    comData.trim();

                    int separatorIdx = comData.indexOf('=');
                    if (separatorIdx >= 0) {
                        String key = comData.substring(0, separatorIdx);
                        String val = comData.substring(separatorIdx + 1);
                        set(key, val);

                        Serial.println("OK");
                    }

                    comData                     = "";
                    *context->commandMatchState = COMMAND_MATCH_STATE_IDLE;

                    break;
                }

                case COMMAND_MATCH_STATE_GET:

                {
                    comData.replace(COMMAND_MATCH_STR_GET, "");
                    comData.trim();

                    String val = get(comData.c_str());
                    Serial.println(val);

                    comData                     = "";
                    *context->commandMatchState = COMMAND_MATCH_STATE_IDLE;

                    break;
                }

                case COMMAND_MATCH_STATE_SUB:

                {
                    comData.replace(COMMAND_MATCH_STR_SUB, "");
                    comData.trim();

                    bool hasSub = false;
                    for (int i = 0; i < subscribeKeys.size(); i++) {
                        if (subscribeKeys[i] == comData) {
                            hasSub = true;
                            break;
                        }
                    }

                    if (!hasSub) {
                        subscribeKeys.push_back(comData);
                    }

                    comData                     = "";
                    *context->commandMatchState = COMMAND_MATCH_STATE_IDLE;

                    break;
                }

                case COMMAND_MATCH_STATE_UNSUB:

                {
                    comData.replace(COMMAND_MATCH_STR_UNSUB, "");
                    comData.trim();

                    for (int i = 0; i < subscribeKeys.size(); i++) {
                        if (subscribeKeys[i] == comData) {
                            subscribeKeys.erase(subscribeKeys.begin() + i);
                            break;
                        }
                    }

                    comData                     = "";
                    *context->commandMatchState = COMMAND_MATCH_STATE_IDLE;

                    break;
                }

                default:
                    break;
            }
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void M5BurnerNVS::listen() {
    if (!initialized) return;

    if (isListening) return;

    Serial.flush();
    isListening                       = true;
    static PTASK_CONTEXT pTaskContext = {
        .commandMatchState = &commandMatchState,
        .isListening       = &isListening,
        .isConnected       = &isConnected};
    xTaskCreatePinnedToCore(_listen, "listen", 2048, &pTaskContext, 12, NULL,
                            0);
}

void M5BurnerNVS::end() {
    if (!isListening) return;
    isListening = false;
    isConnected = false;
}

String M5BurnerNVS::get(String key) {
    Preferences pref;
    pref.begin(BURNER_NVS_NAMESPACE, true);
    return pref.getString(key.c_str(), "");
}

void M5BurnerNVS::set(String key, String val) {
    Preferences pref;
    pref.begin(BURNER_NVS_NAMESPACE, false);
    pref.putString(key.c_str(), val.c_str());

    bool keyExist = false;
    String keys   = get(BURNER_NVS_KEYS_FIELD);
    while (keys.indexOf('/') > -1) {
        int idx      = keys.indexOf('/');
        String field = keys.substring(0, idx);
        if (field == key) {
            keyExist = true;
            break;
        }
        keys = keys.substring(idx + 1);
    }
    if (!keyExist && keys != key) {
        pref.putString(BURNER_NVS_KEYS_FIELD,
                       pref.getString(BURNER_NVS_KEYS_FIELD, "") + "/" + key);
    }

    for (int i = 0; i < subscribeKeys.size(); i++) {
        if (subscribeKeys[i] == key) {
            Serial.println("CMD::PUB:" + key + "=" + val);
            break;
        }
    }
}
