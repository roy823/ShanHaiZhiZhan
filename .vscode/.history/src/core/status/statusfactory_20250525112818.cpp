// src/core/status/statusfactory.cpp
#include "statusfactory.h"
#include "poisonstatus.h"
#include "burnstatus.h"
#include "frozenstatus.h"
#include "bleedingstatus.h"
#include "confusionstatus.h"
#include "paralysisstatus.h"
#include "fatiguestatus.h" // Corrected filename
#include "fearstatus.h"
#include "sleepstatus.h"
#include "unbreakablestatus.h" // Corrected filename

std::unique_ptr<Status> StatusFactory::createStatus(StatusType type, int duration) {
    switch (type) {
        case StatusType::Poison:
            return std::make_unique<PoisonStatus>(duration);
        case StatusType::Burn:
            return std::make_unique<BurnStatus>(duration);
        case StatusType::Frozen:
            return std::make_unique<FrozenStatus>(duration);
        case StatusType::Bleeding:
            return std::make_unique<BleedingStatus>(duration);
        case StatusType::Confusion:
            return std::make_unique<ConfusionStatus>(duration); // Duration typically 2-5 turns
        case StatusType::Paralysis:
            return std::make_unique<ParalysisStatus>(duration); // Duration typically until cured
        case StatusType::Fatigue:
            return std::make_unique<FatigueStatus>(duration);   // Duration e.g. 1-2 turns
        case StatusType::Fear:
            return std::make_unique<FearStatus>(duration);     // Duration e.g. 1 turn
        case StatusType::Sleep:
            return std::make_unique<SleepStatus>(duration);    // Duration e.g. 1-3 turns
        case StatusType::Unbreakable:
            return std::make_unique<UnbreakableStatus>(duration); // Duration usually managed by trait logic
        default:
            return nullptr;
    }
}