/**
 * @file FocusTimer.hpp
 * @brief Timer system for Pomodoro-style focus sessions.
 * 
 * The FocusTimer class manages timed work/break intervals with callbacks
 * for state transitions. It uses a background thread for timing with
 * atomic operations for thread safety.
 */
#pragma once

#include "globals.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

/**
 * @enum TimerState
 * @brief Represents the current state of the focus timer.
 */
enum class TimerState {
    Idle,       ///< Timer is not running
    Working,    ///< User is in a work interval
    Break,      ///< User is in a break interval
    Paused,     ///< Timer is paused
    Completed   ///< Session has completed all intervals
};

/**
 * @class FocusTimer
 * @brief Manages Pomodoro-style work/break intervals with callbacks.
 * 
 * The timer runs on a background thread and notifies the main thread
 * of state changes via callbacks. All public methods are thread-safe.
 * 
 * Usage:
 * @code
 *     FocusTimer timer;
 *     timer.setOnWorkStart([]() { showNotification("Work time!"); });
 *     timer.setOnBreakStart([]() { showNotification("Break time!"); });
 *     timer.configure(120, 25, 5);  // 2h session, 25min work, 5min break
 *     timer.start();
 * @endcode
 */
class FocusTimer {
public:
    using Callback = std::function<void()>;
    using TickCallback = std::function<void(int minutesRemaining, TimerState state)>;

    FocusTimer();
    ~FocusTimer();

    // Non-copyable, non-movable (owns a thread)
    FocusTimer(const FocusTimer&) = delete;
    FocusTimer& operator=(const FocusTimer&) = delete;

    /**
     * @brief Configure the timer with durations.
     * @param totalMinutes Total session duration in minutes
     * @param workMinutes Duration of each work interval
     * @param breakMinutes Duration of each break interval
     */
    void configure(int totalMinutes, int workMinutes, int breakMinutes);

    /**
     * @brief Start the focus timer.
     * @return true if started successfully, false if already running
     */
    bool start();

    /**
     * @brief Stop the timer completely.
     */
    void stop();

    /**
     * @brief Pause the timer (preserves remaining time).
     */
    void pause();

    /**
     * @brief Resume a paused timer.
     */
    void resume();

    /**
     * @brief Get the current timer state.
     * @return Current TimerState
     */
    TimerState getState() const { return m_state.load(); }

    /**
     * @brief Get remaining time in the current interval.
     * @return Remaining seconds
     */
    int getRemainingSeconds() const;

    /**
     * @brief Get total elapsed time since session start.
     * @return Elapsed seconds
     */
    int getElapsedSeconds() const;

    /**
     * @brief Check if we're currently in a break.
     * @return true if in break interval
     */
    bool isBreakTime() const { return m_state.load() == TimerState::Break; }

    /**
     * @brief Check if timer is actively running.
     * @return true if in Working or Break state
     */
    bool isRunning() const {
        auto state = m_state.load();
        return state == TimerState::Working || state == TimerState::Break;
    }

    // Callback setters
    void setOnWorkStart(Callback cb) { m_onWorkStart = std::move(cb); }
    void setOnBreakStart(Callback cb) { m_onBreakStart = std::move(cb); }
    void setOnSessionComplete(Callback cb) { m_onSessionComplete = std::move(cb); }
    void setOnTick(TickCallback cb) { m_onTick = std::move(cb); }

private:
    void timerLoop();
    void transitionToWork();
    void transitionToBreak();
    void invokeCallback(const Callback& cb);

    // Configuration
    std::chrono::minutes m_totalDuration{120};
    std::chrono::minutes m_workInterval{25};
    std::chrono::minutes m_breakInterval{5};

    // State
    std::atomic<TimerState> m_state{TimerState::Idle};
    std::chrono::steady_clock::time_point m_sessionStart;
    std::chrono::steady_clock::time_point m_intervalStart;
    std::chrono::seconds m_pausedRemaining{0};
    int m_completedWorkIntervals{0};

    // Thread management
    std::thread m_timerThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_shouldStop{false};

    // Callbacks (invoked on timer thread, should be lightweight)
    Callback m_onWorkStart;
    Callback m_onBreakStart;
    Callback m_onSessionComplete;
    TickCallback m_onTick;
};
