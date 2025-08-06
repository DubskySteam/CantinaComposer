#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @class AudioBufferQueue
 * @brief A simple, thread-safe queue to transfer audio buffers between threads.
 *
 * This class is designed to solve a common problem in audio plugins: how to get
 * data from the real-time audio thread (which cannot be blocked) to the UI/message
 * thread (which can be slower) for visualization. It uses a juce::CriticalSection
 * (a mutex) to ensure that reading and writing the buffer are mutually exclusive,
 * preventing data corruption.
 * @ingroup Utilities
 */
class AudioBufferQueue
{
public:
    /**
     * @brief Pushes a new audio buffer into the queue.
     * This is called from the high-priority audio thread.
     * @param buffer The audio buffer to be copied into the queue.
     */
    void push(const juce::AudioBuffer<float>& buffer)
    {
        //TODO: Kein Lock verwenden..... aber es ist 5 Uhr morgens und die Stimmen werden lauter.
        
        // A ScopedLock ensures that the mutex is locked when this object is created
        // and automatically unlocked when it goes out of scope. This is a safe way
        // to prevent race conditions where the UI thread might try to read the
        // buffer while the audio thread is writing to it.
        const juce::ScopedLock lock(mutex);
        latestBuffer = buffer;
    }

    /**
     * @brief Gets the latest audio buffer from the queue.
     * This is called from the lower-priority UI thread.
     * @return A copy of the latest audio buffer.
     */
    juce::AudioBuffer<float> getBuffer()
    {
        const juce::ScopedLock lock(mutex);
        return latestBuffer;
    }

    private:
    /// @brief The mutex used to synchronize access to the buffer.
    juce::CriticalSection mutex;
    /// @brief The buffer that stores the latest block of audio data.
    juce::AudioBuffer<float> latestBuffer;
};
