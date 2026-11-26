// ============================================================================
// GUIDA RAPIDA PER L'ORALE - HEADER: MathConstants.h (Costanti Matematiche)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Centralizza le costanti matematiche PI e TWO_PI (2*PI) in un unico punto,
//      eliminando i "numeri magici" (3.14159f / 6.28318f) sparsi nei moduli.
//    - Usa 'inline constexpr' (C++17) dentro un namespace dedicato: NON è una
//      variabile globale mutabile, ma una costante valutata a compile-time.
//
// 2. A CHI È COLLEGATA:
//    - Inclusa da: ShapeGenerator.cpp, AsteroidManager.cpp, Collectible.cpp,
//      HermiteObject.cpp, ParticleSystem.cpp.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Radianti e Trigonometria (Domande 5, 11, 13):
//      Un angolo giro misura 2*PI radianti (360°). Le funzioni std::sin/cos
//      lavorano in radianti, usati per coordinate polari e oscillazioni armoniche.
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
