// ============================================================================
// File: include/MathConstants.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Costanti matematiche compile-time (PI, TWO_PI) e funzioni
//              helper inline per generazione casuale e distribuzione polare.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <cmath>
#include <cstdlib>

namespace MathConstants
{
    // Costante matematica Pi-Greco a 32 bit di precisione (compile-time).
    inline constexpr float PI = 3.14159265358979323846f;

    // Angolo giro completo in radianti (2 * PI ≈ 6.283185f = 360°).
    inline constexpr float TWO_PI = 2.0f * PI;

    // Restituisce un numero pseudo-casuale continuo uniforme nell'intervallo [minVal, maxVal].
    inline float randRange(float minVal, float maxVal)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + r * (maxVal - minVal);
    }

    // Genera una posizione in coordinate polari attorno al centro dello schermo (con clamping ai margini).
    inline glm::vec2 sampleRingPosition(
        int index, int count, float minDist, float maxDist,
        float jitterMaxRad, float screenWidth, float screenHeight, float margin = 80.0f)
    {
        float angle = (index * TWO_PI) / static_cast<float>(count) + randRange(0.0f, jitterMaxRad);
        float dist  = randRange(minDist, maxDist);
        glm::vec2 pos(
            screenWidth * 0.5f + std::cos(angle) * dist,
            screenHeight * 0.5f + std::sin(angle) * dist
        );
        return glm::clamp(pos, glm::vec2(margin), glm::vec2(screenWidth - margin, screenHeight - margin));
    }
}
