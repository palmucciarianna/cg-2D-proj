// ORIGINE: Caricamento texture con stb_image e filtri OpenGL dai laboratori della prof, incapsulato in classe C++ RAII con Rule of Five.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: Texture (Incapsulamento Texture 2D RAII)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Incapsula e gestisce l'oggetto OpenGL Texture 2D (`m_textureId`).
//    - Carica immagini da file (BMP, PNG, JPG) tramite la libreria 'stb_image.h'.
//    - Trasferisce i pixel dalla RAM della CPU alla VRAM della GPU con glTexImage2D.
//    - Configura il campionamento, i filtri bilineari (GL_LINEAR) e il wrapping (GL_CLAMP_TO_EDGE).
//    - Applica il pattern RAII con Rule of Five (move semantics, copia eliminata).
//
// 2. A CHI È COLLEGATA:
//    - Utilizzata da: Player (per caricare 'assets/textures/player_face.bmp' e mappare il volto del pilota).
//    - Collabora con:
//      * stb_image.h: decoder delle immagini compresse e raw;
//      * Shader texture (texture.vert e texture.frag): campiona la texture tramite uniform sampler2D.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Discrepanza del Sistema di Coordinate Immagine vs OpenGL (Domanda 2):
//      Nelle immagini su file l'origine (0,0) si trova nell'angolo in alto a sinistra;
//      in OpenGL lo spazio coordinate texture UV ha origine (0,0) nell'angolo in basso a sinistra.
//      Viene invocato stbi_set_flip_vertically_on_load(true) per raddrizzare l'immagine.
//    - Filtri di Campionamento e Ricostruzione (Domande 2, 15):
//      * GL_TEXTURE_MIN_FILTER = GL_LINEAR: interpolazione bilineare tra i 4 texel adiacenti;
//      * GL_TEXTURE_MAG_FILTER = GL_LINEAR: evita la sgranatura a blocchi (pixel art) in ingrandimento.
//    - Wrapping & Clamping (Domanda 15):
//      GL_CLAMP_TO_EDGE fissa le coordinate UV eccedenti ai bordi, prevenendo artefatti di ripetizione.
//    - Deallocazione RAII (Requisito 8):
//      Il distruttore invoca glDeleteTextures(1, &m_textureId), liberando immediatamente la VRAM.
// ============================================================================

#pragma once

#include "OpenGL_Headers.h"
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
