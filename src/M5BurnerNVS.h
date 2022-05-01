// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef ARDUINO_M5STACK_CORE_DEMO_M5BURNERNVS_H
#define ARDUINO_M5STACK_CORE_DEMO_M5BURNERNVS_H

#include <vector>

//
// NVS Information
//
#define BURNER_NVS_NAMESPACE  "__burner_nvs__"
#define BURNER_NVS_KEYS_FIELD "____keys____"

//
// Device states
//
#define COMMAND_MATCH_STATE_IDLE  (0)
#define COMMAND_MATCH_STATE_INIT  (1)
#define COMMAND_MATCH_STATE_LIST  (2)
#define COMMAND_MATCH_STATE_SET   (3)
#define COMMAND_MATCH_STATE_GET   (4)
#define COMMAND_MATCH_STATE_DEL   (5)
#define COMMAND_MATCH_STATE_SUB   (6)
#define COMMAND_MATCH_STATE_UNSUB (7)

// Command string
#define COMMAND_MATCH_STR_INIT    "CMD::INIT:"
#define COMMAND_MATCH_STR_LIST    "CMD::LIST:"
#define COMMAND_MATCH_STR_SET     "CMD::SET:"
#define COMMAND_MATCH_STR_GET     "CMD::GET:"
#define COMMAND_MATCH_STR_DEL     "CMD::DEL:"
#define COMMAND_MATCH_STR_SUB     "CMD::SUB:"
#define COMMAND_MATCH_STR_UNSUB   "CMD::UNSUB:"
#define COMMAND_MATCH_STR_CMD_END "\r\n\r\n"

#define INF_NVS_EXIST     "__NVS_EXIST__"
#define ERR_NVS_NOT_FOUND "__NVS_NOT_FOUND__"

typedef struct {
    int* commandMatchState;
    bool* isListening;
    bool* isConnected;
} PTASK_CONTEXT;

class M5BurnerNVS {
   public:
    void init();
    void listen();
    void end();
    static String get(String key);
    static void set(String key, String val);

   private:
    int commandMatchState = COMMAND_MATCH_STATE_IDLE;
    bool initialized      = false;
    bool isListening      = false;
    bool isConnected      = false;
    static void _listen(void* pvParameters);
};

// extern M5BurnerNVS M5BurnerNVS;

#endif  // ARDUINO_M5STACK_CORE_DEMO_M5BURNERNVS_H
