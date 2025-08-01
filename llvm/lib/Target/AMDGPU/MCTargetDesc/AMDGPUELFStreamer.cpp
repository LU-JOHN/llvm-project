//===-------- AMDGPUELFStreamer.cpp - ELF Object Output -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AMDGPUELFStreamer.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

namespace {

class AMDGPUELFStreamer : public MCELFStreamer {
public:
  AMDGPUELFStreamer(const Triple &T, MCContext &Context,
                    std::unique_ptr<MCAsmBackend> MAB,
                    std::unique_ptr<MCObjectWriter> OW,
                    std::unique_ptr<MCCodeEmitter> Emitter)
      : MCELFStreamer(Context, std::move(MAB), std::move(OW),
                      std::move(Emitter)) {}
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;  
};

} // anonymous namespace

MCELFStreamer *
llvm::createAMDGPUELFStreamer(const Triple &T, MCContext &Context,
                              std::unique_ptr<MCAsmBackend> MAB,
                              std::unique_ptr<MCObjectWriter> OW,
                              std::unique_ptr<MCCodeEmitter> Emitter) {
  return new AMDGPUELFStreamer(T, Context, std::move(MAB), std::move(OW),
                               std::move(Emitter));
}

cl::opt<bool> PreventHalfCacheLineStraddling(
    "amdgpu-prevent-half-cache-line-straddling", cl::Hidden,
    cl::desc(
        "Add NOPs to prevent instructions from straddling half a cache-line"),
    cl::init(true));

void AMDGPUELFStreamer::emitInstruction(const MCInst &Inst,
                                     const MCSubtargetInfo &STI) {
  MCBoundaryAlignFragment *PendingBA = nullptr;
  if (PreventHalfCacheLineStraddling) {
    PendingBA = getContext().allocFragment<MCBoundaryAlignFragment>(Align(32), STI, MCBoundaryAlignFragment::DontCrossBoundary);
    insert(PendingBA);
  }
  this->MCObjectStreamer::emitInstruction(Inst, STI);
  if (PendingBA) {
    PendingBA->setLastFragment(getCurrentFragment());
    PendingBA=nullptr;

    // We need to ensure that further data isn't added to the current
    // DataFragment, so that we can get the size of instructions later in
    // MCAssembler::relaxBoundaryAlign. The easiest way is to insert a new empty
    // DataFragment.
    this->MCObjectStreamer::newFragment();
  }
}
