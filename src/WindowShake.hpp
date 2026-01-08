/**
 * @file WindowShake.hpp
 * @brief Visual feedback system for denied workspace switches.
 * 
 * The WindowShake class provides a visual cue when a user attempts to
 * switch to a restricted workspace. It creates a brief "shake" animation
 * on the focused window to indicate the action was denied.
 */
#pragma once

#include "globals.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

/**
 * @class WindowShake
 * @brief Animates a window with a horizontal shake effect.
 * 
 * When triggered, the window rapidly oscillates horizontally for a brief
 * period, providing clear visual feedback that an action was blocked.
 * The animation is non-blocking and runs on a dedicated thread.
 * 
 * ## Animation Logic
 * 
 * The shake animation works by temporarily modifying the window's position:
 * 
 * 1. Store the original window position
 * 2. Apply a sinusoidal offset to the X position over time
 * 3. The offset follows: `intensity * sin(2π * t / frequency)`
 * 4. After the duration elapses, restore the original position
 * 
 * This creates a smooth back-and-forth motion that's visually distinct
 * from normal window movement.
 * 
 * ## Thread Safety
 * 
 * The shake operation runs on a dedicated thread to avoid blocking
 * the main Hyprland event loop. Multiple shake requests are coalesced -
 * if a shake is already in progress, new requests are ignored.
 */
class WindowShake {
public:
    WindowShake();
    ~WindowShake();

    // Non-copyable
    WindowShake(const WindowShake&) = delete;
    WindowShake& operator=(const WindowShake&) = delete;

    /**
     * @brief Configure shake animation parameters.
     * @param intensityPx Maximum displacement in pixels
     * @param durationMs Total animation duration in milliseconds
     * @param frequencyMs Oscillation period in milliseconds
     */
    void configure(int intensityPx, int durationMs, int frequencyMs);

    /**
     * @brief Trigger a shake animation on the focused window.
     * 
     * This method returns immediately. The animation runs asynchronously.
     * If a shake is already in progress, this call is ignored.
     */
    void shake();

    /**
     * @brief Trigger a shake animation on a specific window.
     * @param pWindow The window to shake
     */
    void shakeWindow(PHLWINDOW pWindow);

    /**
     * @brief Check if a shake animation is currently running.
     * @return true if shaking
     */
    bool isShaking() const { return m_isShaking.load(); }

    /**
     * @brief Stop any ongoing shake animation immediately.
     */
    void stopShake();

private:
    void shakeLoop();
    void performShake(PHLWINDOW pWindow);

    // Configuration
    int m_intensity{15};       // Pixels
    int m_duration{300};       // Milliseconds
    int m_frequency{50};       // Milliseconds per oscillation

    // State
    std::atomic<bool> m_isShaking{false};
    std::atomic<bool> m_shouldStop{false};
    PHLWINDOW m_targetWindow;
    
    // Thread management
    std::thread m_shakeThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};
