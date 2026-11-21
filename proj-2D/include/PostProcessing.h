// ============================================================================
// File: include/PostProcessing.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Sottosistema di Post-Processing basato su Framebuffer Object (FBO).
//              Renderizza l'intero mondo di gioco su una texture off-screen prima
//              di applicare filtri a schermo intero (Onde d'Urto rifrattive,
//              simulazione monitor CRT arcade, aberrazione cromatica, vignettatura
//              e flash di allarme per danni allo scafo).
// ============================================================================

#pragma once

#include "OpenGL_Headers.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <string>

class PostProcessing
{
public:
    PostProcessing();
    ~PostProcessing();

    // Disabilita la copia (Rule of Five) per evitare duplicazione di FBO/Texture GPU
    PostProcessing(const PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;

    // Inizializza l'FBO, la texture colore, il renderbuffer e compila lo shader di post-process
    bool init(int width, int height, const std::string& assetsPath);

    // Gestisce il ridimensionamento dinamico del framebuffer (es. su display Retina o resize finestra)
    void resize(int width, int height);

    // Attiva il rendering su FBO off-screen (tutti i draw successivi vanno nella texture)
    void bind();

    // Ripristina il framebuffer predefinito (schermo fisico)
    void unbind();

    // Disegna il quad a schermo intero applicando tutti gli effetti di post-processing
    void draw(float currentTime);

    // Aggiorna i timer interni degli effetti transitori (shockwave, flash danno)
    void update(float dt);

    // Avvia un'onda d'urto ottica a partire da una coordinata del mondo di gioco
    void triggerShockwave(const glm::vec2& worldPos, float worldWidth, float worldHeight, float strength = 0.045f, float duration = 0.65f);

    // Avvia il flash rosso di allarme scafo
    void triggerDamageFlash(float intensity = 1.0f);

    // Rilascia tutte le risorse GPU
    void destroy();

    // Getter e Setter per l'interfaccia grafica ImGui
    bool& crtEnabled() { return m_enableCRT; }
    float& scanlineIntensity() { return m_scanlineIntensity; }
    float& chromaticAberration() { return m_chromaticAberration; }
    float& vignetteIntensity() { return m_vignetteIntensity; }
    bool isShockwaveActive() const { return m_shockwaveTimer < m_shockwaveMaxTime; }

private:
    void setupQuad();
    void allocateBuffers(int width, int height);

    GLuint m_fbo = 0;
    GLuint m_texture = 0;
    GLuint m_rbo = 0;

    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;

    Shader m_shader;
    int m_width = 1280;
    int m_height = 720;

    // Parametri Shockwave
    glm::vec2 m_shockwaveCenter = glm::vec2(0.5f, 0.5f);
    float m_shockwaveTimer = 10.0f; // Inizialmente inattiva
    float m_shockwaveMaxTime = 0.65f;
    float m_shockwaveStrength = 0.04f;

    // Parametri CRT & Estetica Retrò
    bool m_enableCRT = false;
    float m_scanlineIntensity = 0.45f;
    float m_chromaticAberration = 0.0035f;

    // Parametri Danno e Vignettatura
    float m_damageFlashTimer = 0.0f;
    float m_vignetteIntensity = 0.55f;
};
