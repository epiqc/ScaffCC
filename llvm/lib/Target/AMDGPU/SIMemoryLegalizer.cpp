//===- SIMemoryLegalizer.cpp ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Memory legalizer - implements memory model. More information can be
/// found here:
///   http://llvm.org/docs/AMDGPUUsage.html#memory-model
//
//===----------------------------------------------------------------------===//

#include "AMDGPU.h"
#include "AMDGPUMachineModuleInfo.h"
#include "AMDGPUSubtarget.h"
#include "SIDefines.h"
#include "SIInstrInfo.h"
#include "Utils/AMDGPUBaseInfo.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Pass.h"
#include "llvm/Support/AtomicOrdering.h"
#include <cassert>
#include <list>

using namespace llvm;
using namespace llvm::AMDGPU;

#define DEBUG_TYPE "si-memory-legalizer"
#define PASS_NAME "SI Memory Legalizer"

namespace {

class SIMemOpInfo final {
private:
  SyncScope::ID SSID = SyncScope::System;
  AtomicOrdering Ordering = AtomicOrdering::NotAtomic;
  AtomicOrdering FailureOrdering = AtomicOrdering::NotAtomic;
  bool IsNonTemporal = false;

  SIMemOpInfo(SyncScope::ID SSID, AtomicOrdering Ordering)
      : SSID(SSID), Ordering(Ordering) {}

  SIMemOpInfo(SyncScope::ID SSID, AtomicOrdering Ordering,
              AtomicOrdering FailureOrdering, bool IsNonTemporal = false)
      : SSID(SSID), Ordering(Ordering), FailureOrdering(FailureOrdering),
        IsNonTemporal(IsNonTemporal) {}

  /// \returns Info constructed from \p MI, which has at least machine memory
  /// operand.
  static Optional<SIMemOpInfo> constructFromMIWithMMO(
      const MachineBasicBlock::iterator &MI);

public:
  /// \returns Synchronization scope ID of the machine instruction used to
  /// create this SIMemOpInfo.
  SyncScope::ID getSSID() const {
    return SSID;
  }
  /// \returns Ordering constraint of the machine instruction used to
  /// create this SIMemOpInfo.
  AtomicOrdering getOrdering() const {
    return Ordering;
  }
  /// \returns Failure ordering constraint of the machine instruction used to
  /// create this SIMemOpInfo.
  AtomicOrdering getFailureOrdering() const {
    return FailureOrdering;
  }
  /// \returns True if memory access of the machine instruction used to
  /// create this SIMemOpInfo is non-temporal, false otherwise.
  bool isNonTemporal() const {
    return IsNonTemporal;
  }

  /// \returns True if ordering constraint of the machine instruction used to
  /// create this SIMemOpInfo is unordered or higher, false otherwise.
  bool isAtomic() const {
    return Ordering != AtomicOrdering::NotAtomic;
  }

  /// \returns Load info if \p MI is a load operation, "None" otherwise.
  static Optional<SIMemOpInfo> getLoadInfo(
      const MachineBasicBlock::iterator &MI);
  /// \returns Store info if \p MI is a store operation, "None" otherwise.
  static Optional<SIMemOpInfo> getStoreInfo(
      const MachineBasicBlock::iterator &MI);
  /// \returns Atomic fence info if \p MI is an atomic fence operation,
  /// "None" otherwise.
  static Optional<SIMemOpInfo> getAtomicFenceInfo(
      const MachineBasicBlock::iterator &MI);
  /// \returns Atomic cmpxchg info if \p MI is an atomic cmpxchg operation,
  /// "None" otherwise.
  static Optional<SIMemOpInfo> getAtomicCmpxchgInfo(
      const MachineBasicBlock::iterator &MI);
  /// \returns Atomic rmw info if \p MI is an atomic rmw operation,
  /// "None" otherwise.
  static Optional<SIMemOpInfo> getAtomicRmwInfo(
      const MachineBasicBlock::iterator &MI);

  /// \brief Reports unknown synchronization scope used in \p MI to LLVM
  /// context.
  static void reportUnknownSyncScope(
      const MachineBasicBlock::iterator &MI);
};

class SIMemoryLegalizer final : public MachineFunctionPass {
private:
  /// \brief Machine module info.
  const AMDGPUMachineModuleInfo *MMI = nullptr;

  /// \brief Instruction info.
  const SIInstrInfo *TII = nullptr;

  /// \brief Immediate for "vmcnt(0)".
  unsigned Vmcnt0Immediate = 0;

  /// \brief Opcode for cache invalidation instruction (L1).
  unsigned Wbinvl1Opcode = 0;

  /// \brief List of atomic pseudo instructions.
  std::list<MachineBasicBlock::iterator> AtomicPseudoMIs;

  /// \brief Sets named bit (BitName) to "true" if present in \p MI. Returns
  /// true if \p MI is modified, false otherwise.
  template <uint16_t BitName>
  bool enableNamedBit(const MachineBasicBlock::iterator &MI) const {
    int BitIdx = AMDGPU::getNamedOperandIdx(MI->getOpcode(), BitName);
    if (BitIdx == -1)
      return false;

    MachineOperand &Bit = MI->getOperand(BitIdx);
    if (Bit.getImm() != 0)
      return false;

    Bit.setImm(1);
    return true;
  }

  /// \brief Sets GLC bit to "true" if present in \p MI. Returns true if \p MI
  /// is modified, false otherwise.
  bool enableGLCBit(const MachineBasicBlock::iterator &MI) const {
    return enableNamedBit<AMDGPU::OpName::glc>(MI);
  }

  /// \brief Sets SLC bit to "true" if present in \p MI. Returns true if \p MI
  /// is modified, false otherwise.
  bool enableSLCBit(const MachineBasicBlock::iterator &MI) const {
    return enableNamedBit<AMDGPU::OpName::slc>(MI);
  }

  /// \brief Inserts "buffer_wbinvl1_vol" instruction \p Before or after \p MI.
  /// Always returns true.
  bool insertBufferWbinvl1Vol(MachineBasicBlock::iterator &MI,
                              bool Before = true) const;
  /// \brief Inserts "s_waitcnt vmcnt(0)" instruction \p Before or after \p MI.
  /// Always returns true.
  bool insertWaitcntVmcnt0(MachineBasicBlock::iterator &MI,
                           bool Before = true) const;

  /// \brief Removes all processed atomic pseudo instructions from the current
  /// function. Returns true if current function is modified, false otherwise.
  bool removeAtomicPseudoMIs();

  /// \brief Expands load operation \p MI. Returns true if instructions are
  /// added/deleted or \p MI is modified, false otherwise.
  bool expandLoad(const SIMemOpInfo &MOI,
                  MachineBasicBlock::iterator &MI);
  /// \brief Expands store operation \p MI. Returns true if instructions are
  /// added/deleted or \p MI is modified, false otherwise.
  bool expandStore(const SIMemOpInfo &MOI,
                   MachineBasicBlock::iterator &MI);
  /// \brief Expands atomic fence operation \p MI. Returns true if
  /// instructions are added/deleted or \p MI is modified, false otherwise.
  bool expandAtomicFence(const SIMemOpInfo &MOI,
                         MachineBasicBlock::iterator &MI);
  /// \brief Expands atomic cmpxchg operation \p MI. Returns true if
  /// instructions are added/deleted or \p MI is modified, false otherwise.
  bool expandAtomicCmpxchg(const SIMemOpInfo &MOI,
                           MachineBasicBlock::iterator &MI);
  /// \brief Expands atomic rmw operation \p MI. Returns true if
  /// instructions are added/deleted or \p MI is modified, false otherwise.
  bool expandAtomicRmw(const SIMemOpInfo &MOI,
                       MachineBasicBlock::iterator &MI);

public:
  static char ID;

  SIMemoryLegalizer() : MachineFunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override {
    return PASS_NAME;
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end namespace anonymous

/* static */
Optional<SIMemOpInfo> SIMemOpInfo::constructFromMIWithMMO(
    const MachineBasicBlock::iterator &MI) {
  assert(MI->getNumMemOperands() > 0);

  const MachineFunction *MF = MI->getParent()->getParent();
  const AMDGPUMachineModuleInfo *MMI =
      &MF->getMMI().getObjFileInfo<AMDGPUMachineModuleInfo>();

  SyncScope::ID SSID = SyncScope::SingleThread;
  AtomicOrdering Ordering = AtomicOrdering::NotAtomic;
  AtomicOrdering FailureOrdering = AtomicOrdering::NotAtomic;
  bool IsNonTemporal = true;

  // Validator should check whether or not MMOs cover the entire set of
  // locations accessed by the memory instruction.
  for (const auto &MMO : MI->memoperands()) {
    const auto &IsSyncScopeInclusion =
        MMI->isSyncScopeInclusion(SSID, MMO->getSyncScopeID());
    if (!IsSyncScopeInclusion) {
      reportUnknownSyncScope(MI);
      return None;
    }

    SSID = IsSyncScopeInclusion.getValue() ? SSID : MMO->getSyncScopeID();
    Ordering =
        isStrongerThan(Ordering, MMO->getOrdering()) ?
            Ordering : MMO->getOrdering();
    FailureOrdering =
        isStrongerThan(FailureOrdering, MMO->getFailureOrdering()) ?
            FailureOrdering : MMO->getFailureOrdering();

    if (!(MMO->getFlags() & MachineMemOperand::MONonTemporal))
      IsNonTemporal = false;
  }

  return SIMemOpInfo(SSID, Ordering, FailureOrdering, IsNonTemporal);
}

/* static */
Optional<SIMemOpInfo> SIMemOpInfo::getLoadInfo(
    const MachineBasicBlock::iterator &MI) {
  assert(MI->getDesc().TSFlags & SIInstrFlags::maybeAtomic);

  if (!(MI->mayLoad() && !MI->mayStore()))
    return None;

  // Be conservative if there are no memory operands.
  if (MI->getNumMemOperands() == 0)
    return SIMemOpInfo(SyncScope::System,
                       AtomicOrdering::SequentiallyConsistent);

  return SIMemOpInfo::constructFromMIWithMMO(MI);
}

/* static */
Optional<SIMemOpInfo> SIMemOpInfo::getStoreInfo(
    const MachineBasicBlock::iterator &MI) {
  assert(MI->getDesc().TSFlags & SIInstrFlags::maybeAtomic);

  if (!(!MI->mayLoad() && MI->mayStore()))
    return None;

  // Be conservative if there are no memory operands.
  if (MI->getNumMemOperands() == 0)
    return SIMemOpInfo(SyncScope::System,
                       AtomicOrdering::SequentiallyConsistent);

  return SIMemOpInfo::constructFromMIWithMMO(MI);
}

/* static */
Optional<SIMemOpInfo> SIMemOpInfo::getAtomicFenceInfo(
    const MachineBasicBlock::iterator &MI) {
  assert(MI->getDesc().TSFlags & SIInstrFlags::maybeAtomic);

  if (MI->getOpcode() != AMDGPU::ATOMIC_FENCE)
    return None;

  SyncScope::ID SSID =
      static_cast<SyncScope::ID>(MI->getOperand(1).getImm());
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI->getOperand(0).getImm());
  return SIMemOpInfo(SSID, Ordering);
}

/* static */
Optional<SIMemOpInfo> SIMemOpInfo::getAtomicCmpxchgInfo(
    const MachineBasicBlock::iterator &MI) {
  assert(MI->getDesc().TSFlags & SIInstrFlags::maybeAtomic);

  if (!(MI->mayLoad() && MI->mayStore()))
    return None;

  // Be conservative if there are no memory operands.
  if (MI->getNumMemOperands() == 0)
    return SIMemOpInfo(SyncScope::System,
                       AtomicOrdering::SequentiallyConsistent,
                       AtomicOrdering::SequentiallyConsistent);

  return SIMemOpInfo::constructFromMIWithMMO(MI);
}

/* static */
Optional<SIMemOpInfo> SIMemOpInfo::getAtomicRmwInfo(
    const MachineBasicBlock::iterator &MI) {
  assert(MI->getDesc().TSFlags & SIInstrFlags::maybeAtomic);

  if (!(MI->mayLoad() && MI->mayStore()))
    return None;

  // Be conservative if there are no memory operands.
  if (MI->getNumMemOperands() == 0)
    return SIMemOpInfo(SyncScope::System,
                       AtomicOrdering::SequentiallyConsistent);

  return SIMemOpInfo::constructFromMIWithMMO(MI);
}

/* static */
void SIMemOpInfo::reportUnknownSyncScope(
    const MachineBasicBlock::iterator &MI) {
  DiagnosticInfoUnsupported Diag(*MI->getParent()->getParent()->getFunction(),
                                 "Unsupported synchronization scope");
  LLVMContext *CTX = &MI->getParent()->getParent()->getFunction()->getContext();
  CTX->diagnose(Diag);
}

bool SIMemoryLegalizer::insertBufferWbinvl1Vol(MachineBasicBlock::iterator &MI,
                                               bool Before) const {
  MachineBasicBlock &MBB = *MI->getParent();
  DebugLoc DL = MI->getDebugLoc();

  if (!Before)
    ++MI;

  BuildMI(MBB, MI, DL, TII->get(Wbinvl1Opcode));

  if (!Before)
    --MI;

  return true;
}

bool SIMemoryLegalizer::insertWaitcntVmcnt0(MachineBasicBlock::iterator &MI,
                                            bool Before) const {
  MachineBasicBlock &MBB = *MI->getParent();
  DebugLoc DL = MI->getDebugLoc();

  if (!Before)
    ++MI;

  BuildMI(MBB, MI, DL, TII->get(AMDGPU::S_WAITCNT)).addImm(Vmcnt0Immediate);

  if (!Before)
    --MI;

  return true;
}

bool SIMemoryLegalizer::removeAtomicPseudoMIs() {
  if (AtomicPseudoMIs.empty())
    return false;

  for (auto &MI : AtomicPseudoMIs)
    MI->eraseFromParent();

  AtomicPseudoMIs.clear();
  return true;
}

bool SIMemoryLegalizer::expandLoad(const SIMemOpInfo &MOI,
                                   MachineBasicBlock::iterator &MI) {
  assert(MI->mayLoad() && !MI->mayStore());

  bool Changed = false;

  if (MOI.isAtomic()) {
    if (MOI.getSSID() == SyncScope::System ||
        MOI.getSSID() == MMI->getAgentSSID()) {
      if (MOI.getOrdering() == AtomicOrdering::Acquire ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= enableGLCBit(MI);

      if (MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= insertWaitcntVmcnt0(MI);

      if (MOI.getOrdering() == AtomicOrdering::Acquire ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent) {
        Changed |= insertWaitcntVmcnt0(MI, false);
        Changed |= insertBufferWbinvl1Vol(MI, false);
      }

      return Changed;
    }

    if (MOI.getSSID() == SyncScope::SingleThread ||
        MOI.getSSID() == MMI->getWorkgroupSSID() ||
        MOI.getSSID() == MMI->getWavefrontSSID()) {
      return Changed;
    }

    llvm_unreachable("Unsupported synchronization scope");
  }

  // Atomic instructions do not have the nontemporal attribute.
  if (MOI.isNonTemporal()) {
    Changed |= enableGLCBit(MI);
    Changed |= enableSLCBit(MI);
    return Changed;
  }

  return Changed;
}

bool SIMemoryLegalizer::expandStore(const SIMemOpInfo &MOI,
                                    MachineBasicBlock::iterator &MI) {
  assert(!MI->mayLoad() && MI->mayStore());

  bool Changed = false;

  if (MOI.isAtomic()) {
    if (MOI.getSSID() == SyncScope::System ||
        MOI.getSSID() == MMI->getAgentSSID()) {
      if (MOI.getOrdering() == AtomicOrdering::Release ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= insertWaitcntVmcnt0(MI);

      return Changed;
    }

    if (MOI.getSSID() == SyncScope::SingleThread ||
        MOI.getSSID() == MMI->getWorkgroupSSID() ||
        MOI.getSSID() == MMI->getWavefrontSSID()) {
      return Changed;
    }

    llvm_unreachable("Unsupported synchronization scope");
  }

  // Atomic instructions do not have the nontemporal attribute.
  if (MOI.isNonTemporal()) {
    Changed |= enableGLCBit(MI);
    Changed |= enableSLCBit(MI);
    return Changed;
  }

  return Changed;
}

bool SIMemoryLegalizer::expandAtomicFence(const SIMemOpInfo &MOI,
                                          MachineBasicBlock::iterator &MI) {
  assert(MI->getOpcode() == AMDGPU::ATOMIC_FENCE);

  bool Changed = false;

  if (MOI.isAtomic()) {
    if (MOI.getSSID() == SyncScope::System ||
        MOI.getSSID() == MMI->getAgentSSID()) {
      if (MOI.getOrdering() == AtomicOrdering::Acquire ||
          MOI.getOrdering() == AtomicOrdering::Release ||
          MOI.getOrdering() == AtomicOrdering::AcquireRelease ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= insertWaitcntVmcnt0(MI);

      if (MOI.getOrdering() == AtomicOrdering::Acquire ||
          MOI.getOrdering() == AtomicOrdering::AcquireRelease ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= insertBufferWbinvl1Vol(MI);

      AtomicPseudoMIs.push_back(MI);
      return Changed;
    }

    if (MOI.getSSID() == SyncScope::SingleThread ||
        MOI.getSSID() == MMI->getWorkgroupSSID() ||
        MOI.getSSID() == MMI->getWavefrontSSID()) {
      AtomicPseudoMIs.push_back(MI);
      return Changed;
    }

    SIMemOpInfo::reportUnknownSyncScope(MI);
  }

  return Changed;
}

bool SIMemoryLegalizer::expandAtomicCmpxchg(const SIMemOpInfo &MOI,
                                            MachineBasicBlock::iterator &MI) {
  assert(MI->mayLoad() && MI->mayStore());

  bool Changed = false;

  if (MOI.isAtomic()) {
    if (MOI.getSSID() == SyncScope::System ||
        MOI.getSSID() == MMI->getAgentSSID()) {
      if (MOI.getOrdering() == AtomicOrdering::Release ||
          MOI.getOrdering() == AtomicOrdering::AcquireRelease ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent ||
          MOI.getFailureOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= insertWaitcntVmcnt0(MI);

      if (MOI.getOrdering() == AtomicOrdering::Acquire ||
          MOI.getOrdering() == AtomicOrdering::AcquireRelease ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent ||
          MOI.getFailureOrdering() == AtomicOrdering::Acquire ||
          MOI.getFailureOrdering() == AtomicOrdering::SequentiallyConsistent) {
        Changed |= insertWaitcntVmcnt0(MI, false);
        Changed |= insertBufferWbinvl1Vol(MI, false);
      }

      return Changed;
    }

    if (MOI.getSSID() == SyncScope::SingleThread ||
        MOI.getSSID() == MMI->getWorkgroupSSID() ||
        MOI.getSSID() == MMI->getWavefrontSSID()) {
      Changed |= enableGLCBit(MI);
      return Changed;
    }

    llvm_unreachable("Unsupported synchronization scope");
  }

  return Changed;
}

bool SIMemoryLegalizer::expandAtomicRmw(const SIMemOpInfo &MOI,
                                        MachineBasicBlock::iterator &MI) {
  assert(MI->mayLoad() && MI->mayStore());

  bool Changed = false;

  if (MOI.isAtomic()) {
    if (MOI.getSSID() == SyncScope::System ||
        MOI.getSSID() == MMI->getAgentSSID()) {
      if (MOI.getOrdering() == AtomicOrdering::Release ||
          MOI.getOrdering() == AtomicOrdering::AcquireRelease ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent)
        Changed |= insertWaitcntVmcnt0(MI);

      if (MOI.getOrdering() == AtomicOrdering::Acquire ||
          MOI.getOrdering() == AtomicOrdering::AcquireRelease ||
          MOI.getOrdering() == AtomicOrdering::SequentiallyConsistent) {
        Changed |= insertWaitcntVmcnt0(MI, false);
        Changed |= insertBufferWbinvl1Vol(MI, false);
      }

      return Changed;
    }

    if (MOI.getSSID() == SyncScope::SingleThread ||
        MOI.getSSID() == MMI->getWorkgroupSSID() ||
        MOI.getSSID() == MMI->getWavefrontSSID()) {
      Changed |= enableGLCBit(MI);
      return Changed;
    }

    llvm_unreachable("Unsupported synchronization scope");
  }

  return Changed;
}

bool SIMemoryLegalizer::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  const SISubtarget &ST = MF.getSubtarget<SISubtarget>();
  const IsaInfo::IsaVersion IV = IsaInfo::getIsaVersion(ST.getFeatureBits());

  MMI = &MF.getMMI().getObjFileInfo<AMDGPUMachineModuleInfo>();
  TII = ST.getInstrInfo();

  Vmcnt0Immediate =
      AMDGPU::encodeWaitcnt(IV, 0, getExpcntBitMask(IV), getLgkmcntBitMask(IV));
  Wbinvl1Opcode = ST.getGeneration() <= AMDGPUSubtarget::SOUTHERN_ISLANDS ?
      AMDGPU::BUFFER_WBINVL1 : AMDGPU::BUFFER_WBINVL1_VOL;

  for (auto &MBB : MF) {
    for (auto MI = MBB.begin(); MI != MBB.end(); ++MI) {
      if (!(MI->getDesc().TSFlags & SIInstrFlags::maybeAtomic))
        continue;

      if (const auto &MOI = SIMemOpInfo::getLoadInfo(MI))
        Changed |= expandLoad(MOI.getValue(), MI);
      else if (const auto &MOI = SIMemOpInfo::getStoreInfo(MI))
        Changed |= expandStore(MOI.getValue(), MI);
      else if (const auto &MOI = SIMemOpInfo::getAtomicFenceInfo(MI))
        Changed |= expandAtomicFence(MOI.getValue(), MI);
      else if (const auto &MOI = SIMemOpInfo::getAtomicCmpxchgInfo(MI))
        Changed |= expandAtomicCmpxchg(MOI.getValue(), MI);
      else if (const auto &MOI = SIMemOpInfo::getAtomicRmwInfo(MI))
        Changed |= expandAtomicRmw(MOI.getValue(), MI);
    }
  }

  Changed |= removeAtomicPseudoMIs();
  return Changed;
}

INITIALIZE_PASS(SIMemoryLegalizer, DEBUG_TYPE, PASS_NAME, false, false)

char SIMemoryLegalizer::ID = 0;
char &llvm::SIMemoryLegalizerID = SIMemoryLegalizer::ID;

FunctionPass *llvm::createSIMemoryLegalizerPass() {
  return new SIMemoryLegalizer();
}
