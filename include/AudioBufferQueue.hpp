#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class AudioBufferQueue
{
public:
    void push(const juce::AudioBuffer<float>& buffer)
    {
        //TODO: Kein Lock verwenden..... aber whatever solange es funktioniert
        const juce::ScopedLock lock(mutex);
        latestBuffer = buffer;
    }

    juce::AudioBuffer<float> getBuffer()
    {
        const juce::ScopedLock lock(mutex);
        return latestBuffer;
    }

private:
    juce::CriticalSection mutex;
    juce::AudioBuffer<float> latestBuffer;
};
