#include <cassert>

#include <cpptrace/basic.hpp>
#include <cpptrace/from_current.hpp>
#include <exception>
#include <iostream>

#include "antigo/Context.h"
#include "antigo/impl/OnstackContextImpl.h"
#include "antigo/impl/ExecutionData.h"

#ifndef WIN32
#define ANTIGO_TRY CPPTRACE_TRY
#define ANTIGO_CATCH CPPTRACE_CATCH
#else
#define ANTIGO_TRY try
#define ANTIGO_CATCH catch
#endif

namespace Antigo {

OnstackContextImpl::OnstackContextImpl(const char* filename_, size_t linenum_, const char* funcname_): h{}, dataFrames{} {
  h.filename = filename_;
  h.linenum = std::min<size_t>(linenum_, std::numeric_limits<decltype(h.linenum)>::max());
  h.dataFramesCnt = 0;
  h.skippedDataFramesCnt = 0;
  h.destructing = false;
  h.errorOnTop = false;
  h.funcname = funcname_;

  h.downCtx = nullptr;
  if (GetCurrentExecutionData().stackCtxChain.size()) {
    assert(GetCurrentExecutionData().stackCtxChain.back());
    h.downCtx = GetCurrentExecutionData().stackCtxChain.back();
  }

  // h.rawTrace = cpptrace::generate_raw_trace(); // XXX disable for emscripten (can't build)
  h.uncaughtExceptions = std::uncaught_exceptions();

  if (h.uncaughtExceptions) {
    AddMessage("ctx: has uncaught exceptions; count=");
    AddUnsigned(h.uncaughtExceptions);
  }

  // 20241227: if debug include message thread id

  GetCurrentExecutionData().stackCtxChain.push_back(this);

  // 20241218 1: push_back & pop_back for reserve storage and noexcept?
  // next step: common buffer for prepared message frames?
  // exception safety: cancel adding in case of error; also skip all dtor logic in that case

  // 20241227 evg: self-trace - писать в цикличный буфер в таком же +- формате в локальный сторадж потока + в себя же
}

OnstackContextImpl::~OnstackContextImpl() {
  ANTIGO_TRY {
    h.destructing = true;

    if (!(++GetCurrentExecutionData().ticker & 0xffff)) {
      GetCurrentExecutionData().orphans.emplace_back(ResolveCtxStackImpl("ticker"));
    }

    assert(GetCurrentExecutionData().stackCtxChain.size() && GetCurrentExecutionData().stackCtxChain.back() == this);
    if (std::uncaught_exceptions() != h.uncaughtExceptions || (h.downCtx && h.downCtx->h.uncaughtExceptions != h.uncaughtExceptions)) {
      auto& w = GetCurrentExecutionData().errorWitnesses;
      if (!h.errorOnTop) {
        // XXX 20250112 1508 related: условие здесь должно как-то учитывать, что мы могли провалиться сверху с другим слоем эксепшена. или пофиг?)
        ANTIGO_TRY {
          w.emplace_back(ResolveCtxStackImpl("exception"));
        } ANTIGO_CATCH (const std::exception& e) {
          cpptrace::from_current_exception().print();
        }
      }
      if (h.downCtx == nullptr) {
        while (!w.empty()) {
          GetCurrentExecutionData().orphans.push_back(std::move(w.back()));
          w.pop_back();
        }
      } else {
        h.downCtx->h.errorOnTop = true;
      }
    } else {
      auto& w = GetCurrentExecutionData().errorWitnesses;
      while (!w.empty()) {
        GetCurrentExecutionData().orphans.push_back(std::move(w.back()));
        w.pop_back();
      }
    }
    GetCurrentExecutionData().stackCtxChain.pop_back();
  } ANTIGO_CATCH (const std::exception& e) {
    std::cerr << "fatal exception, what: " << e.what() << "\n";
    cpptrace::from_current_exception().print();
    std::terminate();
    // terminating here on purpose, our state could get inconsistent
  }
}

void OnstackContextImpl::ResolveCurrentImpl(ResolvedContext& to) const {
  auto& entry = to.entries.emplace_back();
  entry.sourceLoc.filename = h.filename;
  entry.sourceLoc.line = h.linenum;
  entry.sourceLoc.func = h.funcname;

  for (size_t i = 0; i < h.dataFramesCnt; ++i) {
    entry.messages.emplace_back(dataFrames[i].Resolve());
  }

  if (h.skippedDataFramesCnt) {
    std::string msg = std::to_string(h.skippedDataFramesCnt);
    if (h.skippedDataFramesCnt == std::numeric_limits<decltype(h.skippedDataFramesCnt)>::max()) {
      msg += "+";
    }
    msg += " last messages didn't fit into buffer";
    entry.messages.push_back(ResolvedMessageEntry{"meta", std::move(msg)});
  }
}

ResolvedContext OnstackContextImpl::ResolveCtxStackImpl(std::string reason) const {
  ResolvedContext result;
  result.reason = std::move(reason);
  const auto& chain = GetCurrentExecutionData().stackCtxChain;
  for (size_t i = chain.size(); i > 0; --i) {
    chain[i - 1]->ResolveCurrentImpl(result);
  }
  if (!result.entries.empty()) {
    result.entries[0].messages.push_back({"stacktrace", cpptrace::generate_trace().to_string()});
  }
  return result;
}

ResolvedContext OnstackContextImpl::Resolve() const {
  return ResolveCtxStackImpl("ondemand");
}

void OnstackContextImpl::Orphan() const {
  GetCurrentExecutionData().orphans.emplace_back(Resolve());
}
}
