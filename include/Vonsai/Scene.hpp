#pragma once

#include <functional>
#include <mutex>

#include <chrono>
using Clock = std::chrono::high_resolution_clock;


namespace Vonsai {

class Scene {
public:
  explicit Scene() = default;

  inline unsigned int getID() { return m_sceneGlobalIdx; }

  unsigned int getFPS();
  float        getDeltaTime();

  std::function<void(void)> const &getOnGuiFn() const;
  void                             setOnGuiFn(std::function<void(void)> a_fn);

  std::function<unsigned int(void)> const &getOnUpdateFn() const;
  void                                     setOnUpdateFn(std::function<void(void)> a_fn);

  void setClearColor(float a_r, float a_g, float a_b) const;

private:
  static inline unsigned int s_sceneCounter;
  unsigned int               m_sceneGlobalIdx{s_sceneCounter++};

  std::function<void(void)>                m_onGui{nullptr}; // * User defined
  mutable std::function<void(void)> const *m_internalOnGui{nullptr};

  std::function<void(void)>                        m_onUpdate{nullptr}; // * User defined
  mutable std::function<unsigned int(void)> const *m_internalOnUpdate{nullptr};

  mutable float             m_deltaTime{0.f};
  mutable Clock::time_point m_deltaTimeStamp{Clock::now()};

  mutable unsigned int      m_exposedFrameCounter{0u};
  mutable unsigned int      m_internalFrameCounter{0u};
  mutable Clock::time_point m_frameCounterTimeStamp{Clock::now()};

  void updateFPS() const;
  void updateDeltaTime() const;
};
} // namespace Vonsai
