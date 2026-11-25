// ============================================================================
// File: include/Texture.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Incapsulamento RAII di una Texture 2D OpenGL.
//              Carica immagini da file (PNG, JPG, BMP) tramite stb_image,
//              configura i filtri bilineari e il campionamento per il texture mapping.
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <string>

class Texture
{
public:
    // Costruttore predefinito: inizializza una texture vuota non allocata su GPU.
    Texture();

    // Distruttore RAII: dealloca la risorsa texture dalla memoria VRAM della GPU.
    ~Texture();

    // Disabilita la copia (Rule of Five) per impedire doppie deallocazioni dello stesso ID texture.
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Abilita la semantica di movimento (move semantics) per il trasferimento di proprietà a costo zero.
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Carica l'immagine dal disco, genera l'handle OpenGL e carica i pixel in memoria video.
    // Restituisce true se l'operazione ha avuto successo, false altrimenti.
    bool load(const std::string& filepath);

    // Lega la texture a una determinata unità di campionamento (Texture Unit slot, es. 0 per GL_TEXTURE0).
    void bind(GLuint slot = 0) const;

    // Scollega la texture dall'unità di campionamento corrente.
    void unbind() const;

    // Metodi di ispezione dello stato:
    bool isValid() const { return m_textureID != 0; }
    GLuint id() const { return m_textureID; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    GLuint m_textureID = 0; // Handle univoco generato da glGenTextures
    int m_width = 0;        // Larghezza dell'immagine in pixel
    int m_height = 0;       // Altezza dell'immagine in pixel
    int m_channels = 0;     // Numero di canali colore (es. 4 per RGBA)
};
