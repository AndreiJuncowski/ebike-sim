#ifndef FITNES_MACHINE_CONTROL_POINT
#define FITNES_MACHINE_CONTROL_POINT
#include <NimBLEDevice.h>
#include <Arduino.h>
#include "utils.h"


// ─────────────────────────────────────────────────────────────────────────────
//  Op Codes (Client → Server, written to 0x2AD9)
// ─────────────────────────────────────────────────────────────────────────────
enum class FMCPOpCode : uint8_t {
    REQUEST_CONTROL                     = 0x00,
    RESET                               = 0x01,
    SET_TARGET_SPEED                    = 0x02,  // km/h × 100  → uint16
    SET_TARGET_INCLINATION              = 0x03,  // % × 10      → int16
    SET_TARGET_RESISTANCE_LEVEL         = 0x04,  // unitless × 10 → uint16 (0–100)
    SET_TARGET_POWER                    = 0x05,  // Watts       → int16
    SET_TARGET_HEART_RATE               = 0x06,  // bpm         → uint8
    START_RESUME                        = 0x07,
    STOP_PAUSE                          = 0x08,  // 1 = stop, 2 = pause
    SET_TARGETED_EXP_ENERGY             = 0x09,  // kcal        → uint16
    SET_TARGETED_NB_OF_STEPS            = 0x0A,  // steps       → uint16
    SET_TARGETED_NB_OF_STRIDES          = 0x0B,  // strides     → uint16
    SET_TARGETED_DISTANCE               = 0x0C,  // metres      → uint24 (3 bytes)
    SET_TARGETED_TRAINING_TIME          = 0x0D,  // seconds     → uint16
    SET_TARGETED_TIME_IN_HR_ZONE_2      = 0x0E,  // seconds     → uint16
    SET_TARGETED_TIME_IN_HR_ZONE_3      = 0x0F,  // seconds     → uint16
    SET_TARGETED_TIME_IN_HR_ZONE_5      = 0x10,  // seconds     → uint16
    SET_INDOOR_BIKE_SIMULATION_PARAMS   = 0x11,  // see struct below
    SET_WHEEL_CIRCUMFERENCE             = 0x12,  // mm × 10     → uint16
    SPIN_DOWN_CONTROL                   = 0x13,  // 1 = start, 2 = ignore
    SET_TARGETED_CADENCE                = 0x14,  // rpm × 2     → uint16
    RESPONSE_CODE                       = 0x80   // Server → Client response
};

// ─────────────────────────────────────────────────────────────────────────────
//  Result Codes (inside a 0x80 Response indication)
// ─────────────────────────────────────────────────────────────────────────────
enum class FMCPResultCode : uint8_t {
    SUCCESS                 = 0x01,
    OP_CODE_NOT_SUPPORTED   = 0x02,
    INVALID_PARAMETER       = 0x03,
    OPERATION_FAILED        = 0x04,
    CONTROL_NOT_PERMITTED   = 0x05
};

// ─────────────────────────────────────────────────────────────────────────────
//  Stop/Pause parameter values (used with STOP_PAUSE)
// ─────────────────────────────────────────────────────────────────────────────
enum class FMCPStopPauseValue : uint8_t {
    STOP  = 0x01,
    PAUSE = 0x02
};

// ─────────────────────────────────────────────────────────────────────────────
//  Spin-Down control values
// ─────────────────────────────────────────────────────────────────────────────
enum class FMCPSpinDownValue : uint8_t {
    START  = 0x01,
    IGNORE = 0x02
};


class FitnessMachineControlPoint : public NimBLECharacteristicCallbacks {
    private:
    // ── Per-opcode payload fields (only the relevant one is set) ─────────────
    uint16_t  targetSpeed_raw       = 0;  ///< km/h × 100
    int16_t   targetInclination_raw = 0;  ///< % × 10
    uint16_t  targetResistance_raw  = 0;  ///< unitless × 10
    int16_t   targetPower_W         = 0;  ///< Watts
    uint8_t   targetHeartRate_bpm   = 0;  ///< bpm
    FMCPStopPauseValue stopPauseVal = FMCPStopPauseValue::STOP;
    uint16_t  targetEnergy_kcal     = 0;
    uint16_t  targetSteps           = 0;
    uint16_t  targetStrides         = 0;
    uint32_t  targetDistance_m      = 0;  ///< uint24 packed into uint32
    uint16_t  targetTime_s          = 0;
    uint16_t  targetTimeHRZ2_s      = 0;
    uint16_t  targetTimeHRZ3_s      = 0;
    uint16_t  targetTimeHRZ5_s      = 0;
    uint16_t  wheelCircumference_raw = 0; ///< mm × 10
    FMCPSpinDownValue spinDownVal   = FMCPSpinDownValue::START;
    uint16_t  targetCadence_raw     = 0;  ///< rpm × 2

    int16_t  wind_speed_raw = 0;    ///< m/s × 1000
    int16_t  grade_raw      = 0;    ///< % × 100
    uint8_t  crr_raw        = 0;    ///< rolling resistance × 10000
    uint8_t  cw_raw         = 0;    ///< wind resistance kg/m × 100

    

    public: 

    // ── Convenience unit conversions ─────────────────────────────────────────
    float targetSpeedKmh()        const { return targetSpeed_raw        / 100.0f;  }
    float targetInclinationPct()  const { return targetInclination_raw  / 10.0f;   }
    float targetResistance()      const { return targetResistance_raw   / 10.0f;   }
    float wheelCircumferenceMm()  const { return wheelCircumference_raw / 10.0f;   }
    float targetCadenceRpm()      const { return targetCadence_raw      / 2.0f;    }
    float windSpeedMps()  const { return wind_speed_raw / 1000.0f; }
    float gradePct()      const { return grade_raw      / 100.0f;  }
    float rollingCoeff()  const { return crr_raw        / 10000.0f;}
    float windCoeffKgM()  const { return cw_raw         / 100.0f;  }
    float targetPowerWatts()  const { return targetPower_W;  }

    


    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        // Serial.printf("%s : FitnessMachineControlPoint.onRead(), value: %s\n",
        //               pCharacteristic->getUUID().toString().c_str(),
        //               pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        // Serial.printf("%s : FitnessMachineControlPoint.onWrite(), value: %s\n",
        //               pCharacteristic->getUUID().toString().c_str(),
        //               pCharacteristic->getValue().c_str());
        boolean valid = parseCommand(pCharacteristic->getValue().data(), pCharacteristic->getValue().length());
    }

    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
    }

    /** Peer subscribed to notifications/indications */
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        std::string str  = "Client ID: ";
        str             += connInfo.getConnHandle();
        str             += " Address: ";
        str             += connInfo.getAddress().toString();
        if (subValue == 0) {
            str += " Unsubscribed to ";
        } else if (subValue == 1) {
            str += " Subscribed to notifications for ";
        } else if (subValue == 2) {
            str += " Subscribed to indications for ";
        } else if (subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID());

        Serial.printf("%s\n", str.c_str());
    }

    /**
     * Build a server indication response packet.
     *
     * @param out       Output buffer (at least 3 bytes).
     * @param outLen    Set to the number of bytes written.
     * @param reqOpCode The Op Code that is being responded to.
     * @param result    The result code.
     */
    void buildResponse(uint8_t* out, size_t& outLen,
                       FMCPOpCode reqOpCode,
                       FMCPResultCode result = FMCPResultCode::SUCCESS) {
        out[0] = static_cast<uint8_t>(FMCPOpCode::RESPONSE_CODE);
        out[1] = static_cast<uint8_t>(reqOpCode);
        out[2] = static_cast<uint8_t>(result);
        outLen = 3;
    }

    void print() {
        Serial.printf("Simulation | Wind: %.3f m/s | Grade: %.2f % | Crr: %.5f | Cw: %.4f kg/m | Target Inclination: %.2f | Target Resistence: %.2f  | Target Power: %d w\n",
            windSpeedMps(),
            gradePct(),
            rollingCoeff(),
            windCoeffKgM(),
            targetInclinationPct(),
            targetResistance(),
            targetPowerWatts()
            );
    }

    boolean parseCommand(const uint8_t* data, size_t length) {
        if (!data || length < 1) return false;

        FMCPOpCode opCode = static_cast<FMCPOpCode>(data[0]);

        switch (opCode) {

            // ── No-parameter commands ────────────────────────────────────────
            case FMCPOpCode::REQUEST_CONTROL:
            case FMCPOpCode::RESET:
            case FMCPOpCode::START_RESUME:
                return (length >= 1);
                break;

            // ── uint16 parameter (2 bytes LE) ────────────────────────────────
            case FMCPOpCode::SET_TARGET_SPEED:
                if (length >= 3) {
                    targetSpeed_raw = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGET_RESISTANCE_LEVEL:
                if (length >= 3) {
                    targetResistance_raw = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_EXP_ENERGY:
                if (length >= 3) {
                    targetEnergy_kcal = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_NB_OF_STEPS:
                if (length >= 3) {
                    targetSteps = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_NB_OF_STRIDES:
                if (length >= 3) {
                    targetStrides = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_TRAINING_TIME:
                if (length >= 3) {
                    targetTime_s = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_TIME_IN_HR_ZONE_2:
                if (length >= 3) {
                    targetTimeHRZ2_s = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_TIME_IN_HR_ZONE_3:
                if (length >= 3) {
                    targetTimeHRZ3_s = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_TIME_IN_HR_ZONE_5:
                if (length >= 3) {
                    targetTimeHRZ5_s = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_WHEEL_CIRCUMFERENCE:
                if (length >= 3) {
                    wheelCircumference_raw = readUint16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGETED_CADENCE:
                if (length >= 3) {
                    targetCadence_raw = readUint16LE(data, 1);
                    return true;
                }
                break;

            // ── int16 parameter ──────────────────────────────────────────────
            case FMCPOpCode::SET_TARGET_INCLINATION:
                if (length >= 3) {
                    targetInclination_raw = readInt16LE(data, 1);
                    return true;
                }
                break;

            case FMCPOpCode::SET_TARGET_POWER:
                if (length >= 3) {
                    targetPower_W = readInt16LE(data, 1);
                    return true;
                }
                break;

            // ── uint8 parameter ──────────────────────────────────────────────
            case FMCPOpCode::SET_TARGET_HEART_RATE:
                if (length >= 2) {
                    targetHeartRate_bpm = data[1];
                    return true;
                }
                break;

            case FMCPOpCode::STOP_PAUSE:
                if (length >= 2) {
                    stopPauseVal = static_cast<FMCPStopPauseValue>(data[1]);
                    return true;
                }
                break;

            case FMCPOpCode::SPIN_DOWN_CONTROL:
                if (length >= 2) {
                    spinDownVal = static_cast<FMCPSpinDownValue>(data[1]);
                    return true;
                }
                break;

            // ── uint24 parameter (3 bytes LE) ────────────────────────────────
            case FMCPOpCode::SET_TARGETED_DISTANCE:
                if (length >= 4) {
                    targetDistance_m = readUint24LE(data, 1);
                    return true;
                }
                break;

            // ── Indoor Bike Simulation (6 bytes of params) ───────────────────
            case FMCPOpCode::SET_INDOOR_BIKE_SIMULATION_PARAMS:
                if (length >= 7) {
                    wind_speed_raw = readInt16LE(data, 1);
                    grade_raw      = readInt16LE(data, 3);
                    crr_raw        = data[5];
                    cw_raw         = data[6];
                    return true;
                }
                break;

            default:
                // Unknown op code – mark invalid
                break;
        }
        return false;
    }
};
#endif