// ============================================================================
// File: src/Texture.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del caricamento texture tramite libreria stb_image.
//              Configura texture wrapping (CLAMP_TO_EDGE), filtri min/mag (GL_LINEAR)
//              e trasferisce la matrice di pixel in formato RGBA 32-bit su VRAM.
// ============================================================================

#include "Texture.h"
#include <iostream>

// Abilita l'implementazione header-only di stb_image in questa unità di compilazione.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ============================================================================
// Costruttore
// ============================================================================
Texture::Texture()
{
}

// ============================================================================
// Distruttore (RAII)
// ============================================================================
Texture::~Texture()
{
    // Se la texture è stata creata su GPU, ne rilascia la memoria video.
    if (m_textureID != 0)
    {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

// ============================================================================
// Costruttore per Movimento
// ============================================================================
Texture::Texture(Texture&& other) noexcept
    : m_textureID(other.m_textureID)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_channels(other.m_channels)
{
    // Invalida l'oggetto sorgente per evitare la distruzione accidentale dell'handle GPU.
    other.m_textureID = 0;
    other.m_width = 0;
    other.m_height = 0;
    other.m_channels = 0;
}

// ============================================================================
// Assegnazione per Movimento
// ============================================================================
Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        // Dealloca l'eventuale texture preesistente in questo oggetto.
        if (m_textureID != 0)
        {
            glDeleteTextures(1, &m_textureID);
        }

        // Trasferisce i dati dall'oggetto sorgente.
        m_textureID = other.m_textureID;
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;

        // Invalida la sorgente.
        other.m_textureID = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_channels = 0;
    }
    return *this;
}

// ============================================================================
// Caricamento Immagine da File ed Invio alla GPU
// ============================================================================
bool Texture::load(const std::string& filepath)
{
    // Se la texture era già caricata, la deallochiamo prima del nuovo caricamento.
    if (m_textureID != 0)
    {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }

    // OpenGL mappa le coordinate UV con l'origine (0,0) nell'angolo in basso a sinistra.
    // I formati immagine standard (PNG, JPG) memorizzano i pixel dall'alto verso il basso.
    // Ribaltiamo l'asse Y durante il caricamento per ottenere l'orientamento corretto.
    stbi_set_flip_vertically_on_load(1);

    // Carica i pixel dell'immagine dalla memoria di massa:
    // Forziamo 4 canali (RGBA) per garantire trasparenza uniforme e allineamento a 32 bit.
    unsigned char* data = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channels, 4);
    if (!data)
    {
        std::cerr << "[ERRORE Texture] File immagine non trovato o illeggibile: " << filepath << std::endl;
        return false;
    }

    // 1. Genera 1 identificatore di texture su GPU.
    glGenTextures(1, &m_textureID);

    // 2. Lega la texture come target GL_TEXTURE_2D corrente.
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // 3. Configura le modalità di wrapping (ripetizione / bloccaggio sui bordi):
    // GL_CLAMP_TO_EDGE impedisce che i pixel del bordo opposto si fondano (evita artefatti visivi).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // asse orizzontale U
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // asse verticale V

    // 4. Configura i filtri di campionamento (Filtering):
    // GL_LINEAR: interpolazione bilineare per ingrandimento (magnification) e rimpicciolimento (minification).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 5. Invia i byte dei pixel alla GPU:
    // Parametri:
    // - target: GL_TEXTURE_2D
    // - level: 0 (livello di dettaglio base per i mipmap)
    // - internalformat: GL_RGBA (formato con cui memorizzare i dati in VRAM)
    // - width, height: dimensioni dell'immagine
    // - border: 0 (deve sempre essere 0 secondo specifica OpenGL)
    // - format: GL_RGBA (formato dei pixel nella memoria RAM sorgente)
    // - type: GL_UNSIGNED_BYTE (ogni canale è un byte senza segno [0, 255])
    // - data: puntatore ai pixel decodificati da stb_image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Scollega la texture.
    glBindTexture(GL_TEXTURE_2D, 0);

    // 6. Rilascia la memoria RAM occupata dall'immagine decodificata (ora risiede in VRAM).
    stbi_image_free(data);

    std::cout << "[Texture] Caricata con successo (" << m_width << "x" << m_height << "): " << filepath << std::endl;
    return true;
}

// ============================================================================
// Binding della Texture su un'Unità di Texture (Texture Slot)
// ============================================================================
void Texture::bind(GLuint slot) const
{
    // Attiva lo slot texture desiderato (es. GL_TEXTURE0, GL_TEXTURE1, ...).
    glActiveTexture(GL_TEXTURE0 + slot);

    // Lega l'handle della nostra texture al target GL_TEXTURE_2D di tale slot.
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

// ============================================================================
// Unbinding della Texture
// ============================================================================
void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
