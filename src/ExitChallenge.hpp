/**
 * @file ExitChallenge.hpp
 * @brief Minigame system to discourage stopping focus sessions.
 * 
 * The ExitChallenge class provides a configurable challenge that users
 * must complete before they can stop a focus session. This adds friction
 * to discourage impulsive session stops.
 */
#pragma once

#include "globals.hpp"
#include <random>
#include <string>

/**
 * @enum ChallengeType
 * @brief Types of exit challenges available.
 */
enum class ChallengeType {
    None,           ///< No challenge, immediate stop
    TypePhrase,     ///< User must type a specific phrase
    MathProblem,    ///< User must solve a simple math problem
    Countdown       ///< User must wait and confirm multiple times
};

/**
 * @class ExitChallenge
 * @brief Manages the exit challenge minigame system.
 * 
 * When enabled, stopping a focus session requires completing a challenge.
 * This adds intentional friction to prevent impulsive session stops.
 * 
 * ## Challenge Types
 * 
 * - **TypePhrase**: User must type "I want to stop focusing" (or custom phrase)
 * - **MathProblem**: User must solve a random addition/multiplication problem
 * - **Countdown**: User must confirm 3 times with 5-second delays
 * 
 * ## Usage Flow
 * 
 * 1. User triggers `hyfocus:stop`
 * 2. If challenge enabled, `initiateChallenge()` is called
 * 3. User receives instructions via notification
 * 4. User submits answer via `hyfocus:confirm <answer>`
 * 5. If correct, session stops. If wrong, they must try again.
 */
class ExitChallenge {
public:
    ExitChallenge();
    ~ExitChallenge() = default;

    /**
     * @brief Configure the challenge system.
     * @param type Type of challenge to use
     * @param customPhrase Custom phrase for TypePhrase challenge (optional)
     */
    void configure(ChallengeType type, const std::string& customPhrase = "");

    /**
     * @brief Set the challenge type.
     * @param type Challenge type
     */
    void setChallengeType(ChallengeType type) { m_type = type; }

    /**
     * @brief Get the current challenge type.
     * @return Current ChallengeType
     */
    ChallengeType getChallengeType() const { return m_type; }

    /**
     * @brief Check if a challenge is required.
     * @return true if challenge type is not None
     */
    bool isEnabled() const { return m_type != ChallengeType::None; }

    /**
     * @brief Start a new challenge.
     * @return Instructions string to show the user
     */
    std::string initiateChallenge();

    /**
     * @brief Check if a challenge is currently active.
     * @return true if waiting for user response
     */
    bool isChallengeActive() const { return m_challengeActive; }

    /**
     * @brief Validate the user's answer.
     * @param answer The user's submitted answer
     * @return true if correct, false if wrong
     */
    bool validateAnswer(const std::string& answer);

    /**
     * @brief Cancel the current challenge.
     */
    void cancelChallenge();

    /**
     * @brief Get hint for current challenge.
     * @return Hint string
     */
    std::string getHint() const;

    /**
     * @brief Get number of remaining attempts/confirmations.
     * @return Remaining count
     */
    int getRemainingAttempts() const { return m_remainingConfirms; }

private:
    std::string generateMathProblem();
    std::string normalizeAnswer(const std::string& input) const;

    ChallengeType m_type{ChallengeType::None};
    std::string m_customPhrase{"I want to stop focusing"};
    
    // Current challenge state
    bool m_challengeActive{false};
    std::string m_expectedAnswer;
    std::string m_currentPrompt;
    int m_remainingConfirms{3};
    
    // For math problems
    std::mt19937 m_rng;
};
