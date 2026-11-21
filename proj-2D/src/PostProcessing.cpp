// ============================================================================
// File: src/PostProcessing.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del gestore FBO e del rendering di post-processing.
// ============================================================================

#include "PostProcessing.h"
#include <iostream>

PostProcessing::PostProcessing()
{
}

PostProcessing::~PostProcessing()
{
    destroy();
}

bool PostProcessing::init(int width, int height, const std::string& assetsPath)
{
    m_width = width;
    m_height = height;

    std::string vertPath = assetsPath + "/shaders/postprocess.vert";
    std::string fragPath = assetsPath + "/shaders/postprocess.frag";

    if (!m_shader.load(vertPath.c_str(), fragPath.c_str()))
    {
        std::cerr << "[ERRORE] Caricamento shader di post-processing fallito!\n";
        return false;
    }

    setupQuad();
    allocateBuffers(width, height);

    return true;
}

void PostProcessing::setupQuad()
{
    float quadVertices[] = {
        // Coordinate NDC (x, y)  // Coordinate UV (u, v)
        -1.0f,  1.0f,              0.0f, 1.0f,
        -1.0f, -1.0f,              0.0f, 0.0f,
         1.0f, -1.0f,              1.0f, 0.0f,

        -1.0f,  1.0f,              0.0f, 1.0f,
         1.0f, -1.0f,              1.0f, 0.0f,
         1.0f,  1.0f,              1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Attributo 0: Posizione vec2
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Attributo 1: Coordinate UV vec2
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void PostProcessing::allocateBuffers(int width, int height)
{
    m_width = width;
    m_height = height;

    // Rilascia eventuali buffer precedenti
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(1, &m_texture);
        glDeleteRenderbuffers(1, &m_rbo);
        m_fbo = 0;
        m_texture = 0;
        m_rbo = 0;
    }

    // 1. Alloca il Framebuffer Object principale
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // 2. Alloca la Texture Colore collegata all'FBO
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // 3. Alloca il Renderbuffer per Profondità e Stencil
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    // Verifica dell'integrità dell'FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "[ERRORE] Framebuffer di Post-Processing non completo!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::resize(int width, int height)
{
    if (width > 0 && height > 0 && (width != m_width || height != m_height))
    {
        allocateBuffers(width, height);
    }
}

void PostProcessing::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessing::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::update(float dt)
{
    m_shockwaveTimer += dt;

    if (m_damageFlashTimer > 0.0f)
    {
        m_damageFlashTimer -= dt * 2.5f;
        if (m_damageFlashTimer < 0.0f)
            m_damageFlashTimer = 0.0f;
    }
}

void PostProcessing::triggerShockwave(const glm::vec2& worldPos, float worldWidth, float worldHeight, float strength, float duration)
{
    float u = worldPos.x / (worldWidth > 0.0f ? worldWidth : 1280.0f);
    float v = worldPos.y / (worldHeight > 0.0f ? worldHeight : 720.0f);

    m_shockwaveCenter = glm::vec2(glm::clamp(u, 0.0f, 1.0f), glm::clamp(v, 0.0f, 1.0f));
    m_shockwaveTimer = 0.0f;
    m_shockwaveStrength = strength;
    m_shockwaveMaxTime = duration;
}

void PostProcessing::triggerDamageFlash(float intensity)
{
    m_damageFlashTimer = intensity;
}

void PostProcessing::draw(float currentTime)
{
    m_shader.use();

    // Uniform temporali e di shockwave
    m_shader.setFloat("uTime", currentTime);
    m_shader.setVec2("uShockwaveCenter", m_shockwaveCenter);
    m_shader.setFloat("uShockwaveTime", m_shockwaveTimer);
    m_shader.setFloat("uShockwaveMaxTime", m_shockwaveMaxTime);
    m_shader.setFloat("uShockwaveStrength", m_shockwaveStrength);

    // Uniform CRT & Aberrazione Cromatica
    m_shader.setBool("uEnableCRT", m_enableCRT);
    m_shader.setFloat("uScanlineIntensity", m_scanlineIntensity);
    m_shader.setFloat("uChromaticAberration", m_chromaticAberration);

    // Uniform Danno e Vignettatura
    m_shader.setFloat("uDamageFlash", m_damageFlashTimer);
    m_shader.setFloat("uVignetteIntensity", m_vignetteIntensity);

    // Attivazione texture dell'FBO
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    m_shader.setInt("uSceneTexture", 0);

    // Disegno del quad a pieno schermo
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessing::destroy()
{
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_texture != 0)
    {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_rbo != 0)
    {
        glDeleteRenderbuffers(1, &m_rbo);
        m_rbo = 0;
    }
    if (m_quadVAO != 0)
    {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_quadVBO != 0)
    {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }
}
