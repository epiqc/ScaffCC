//===----- SymbolStringPoolTest.cpp - Unit tests for SymbolStringPool -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ExecutionEngine/Orc/SymbolStringPool.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::orc;

namespace {

TEST(SymbolStringPool, UniquingAndEquality) {
  SymbolStringPool SP;
  auto P1 = SP.intern("hello");

  std::string S("hel");
  S += "lo";
  auto P2 = SP.intern(S);

  auto P3 = SP.intern("goodbye");

  EXPECT_EQ(P1, P2) << "Failed to unique entries";
  EXPECT_NE(P1, P3) << "Inequal pooled symbol strings comparing equal";
}

TEST(SymbolStringPool, ClearDeadEntries) {
  SymbolStringPool SP;
  {
    auto P1 = SP.intern("s1");
    SP.clearDeadEntries();
    EXPECT_FALSE(SP.empty()) << "\"s1\" entry in pool should still be retained";
  }
  SP.clearDeadEntries();
  EXPECT_TRUE(SP.empty()) << "pool should be empty";
}

}
